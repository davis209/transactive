/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:  $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/MmsControllerModel.cpp $
  * @author Jade Lee
  * @version $Revision: #3 $
  *
  * Last modification: $DateTime: 2009/02/10 16:25:11 $
  * Last modified by: $Author: builder $
  * 
  * Provides business logic of MMS Controller
  *
  */

#pragma warning ( disable : 4250 4786 4284 )

#include "app/maintenance_management/MmsController/src/stdAfx.h"
#include "app/maintenance_management/MmsController/src/CachedConfig.h"
#include "app/maintenance_management/MmsController/src/MmsControllerModel.h"
#include "app/maintenance_management/MmsController/src/MmsControllerOnlineUpdater.h"

#include "core/message/types/MmsAudit_MessageTypes.h"
#include "core/message/src/AuditMessageSender.h"
#include "core/message/src/MessagePublicationManager.h"

#include "core/data_access_interface/entity_access/src/EntityAccessFactory.h"
#include "core/data_access_interface/system_controller/src/SystemControllerAccessFactory.h"
#include "core/data_access_interface/entity_access/src/Process.h"
#include "core/data_access_interface/entity_access/src/DataNodeEntityData.h"
#include "core/data_access_interface/entity_access/src/ConfigEntity.h"
#include "core/data_access_interface/entity_access/src/EntityData.h"
#include "core/data_access_interface/system_controller/src/ISystemControllerData.h"
#include "core/exceptions/src/DatabaseException.h"
#include "core/exceptions/src/DataException.h"
#include "bus/scada/proxy_library/src/ScadaProxyFactory.h"
#include "bus/scada/proxy_library/src/IDataNodeProxy.h"
#include "core/exceptions/src/ScadaProxyException.h"
#include "core/data_access_interface/entity_access/src/IEntityData.h"
#include "bus/security/access_control/actions/src/AccessControlledActions.h"

#include "bus/generic_gui/src/TransactiveMessage.h" // TD14164 

namespace TA_IRS_App
{
    MmsControllerModel::MmsControllerModel()
    : m_auditSender(NULL)
	, m_targetWnd(NULL)
    {
		FUNCTION_ENTRY("MmsControllerModel");
        try
        {
            m_auditSender = TA_Base_Core::MessagePublicationManager::getInstance().
                                getAuditMessageSender( TA_Base_Core::MmsAudit::Context );

            // Criticial we set the proxy factory to control mode, otherwise
            // they won't be initialised as they are created from the factory
            TA_Base_Bus::ScadaProxyFactory::getInstance().setProxiesToControlMode();

			m_onlineSystemAlarmMmsUpdater = new MmsControllerOnlineUpdater;
			m_onlineSystemAlarmMmsUpdater->registerCallback(this);
        }
        catch (...)
        {
            // UE-??
			// TD14164 ++
			// Change message box to logging based on San's feedback
            /*AfxMessageBox("Unable to create Audit Message Sender entity.");*/
			LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "Unable to create Audit Message Sender entity.");
			// ++ TD14164 
            PostQuitMessage(0);
        }

		FUNCTION_EXIT;
    }
    

    MmsControllerModel::~MmsControllerModel()
    {
		FUNCTION_ENTRY("~MmsControllerModel");
        try
        {
            delete m_auditSender;
            m_auditSender = NULL;

			if (NULL != m_onlineSystemAlarmMmsUpdater)
				m_onlineSystemAlarmMmsUpdater->deregister();
			delete m_onlineSystemAlarmMmsUpdater;
			m_onlineSystemAlarmMmsUpdater = NULL;

			MmsDataNodeCache::iterator it;
			for(it=m_dataNodeCache.begin(); it!=m_dataNodeCache.end(); it++)
			{
				delete (it->second);
			}
			m_dataNodeCache.clear();

        }
        catch ( ... )
        {
            LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "Caught in destructor");
        }
		FUNCTION_EXIT;
    }
    
    
    bool MmsControllerModel::getGlobalMmsInhibit() const
    {
        CWaitCursor wait;
    
        // Allow all exceptions to propogate
        CORBA::Boolean ret;//limin
        CORBA_CALL_RETURN( ret, CachedConfig::getInstance().getMmsInhibitAgent(), getGlobalMmsInhibit, () );//limin
        return static_cast< bool >( ret );//limin
    }
    
    
    void MmsControllerModel::setGlobalMmsInhibit(bool globalMmsInhibit)
    {
        CWaitCursor wait;

        if (!CachedConfig::getInstance().isMmsFunctionPermitted(TA_Base_Bus::aca_MMS_GLOBAL_INHIBIT))
        {
            // Shouldn't be able to get here if the button states are updating correctly
            // UE-??
			// TD14164 ++
			/*AfxMessageBox("Insufficient operator privileges for this operation.");*/
			TA_Base_Bus::TransActiveMessage userMsg;
			userMsg.showMsgBox(IDS_UE_190001);
			// ++ TD14164 
            return;
        }

        // Catch all exceptions, dispatch failure audit message (but no error message)
        EInhibitResult result = INHIBIT_FAILED;
    
        try
        {
			CORBA_CALL( CachedConfig::getInstance().getMmsInhibitAgent(),
			            setGlobalMmsInhibit,
			            ( globalMmsInhibit, CachedConfig::getInstance().getSessionId().c_str() ) );
            result = (globalMmsInhibit ? INHIBIT_APPLIED : INHIBIT_REMOVED);
        }
        catch (const TA_Base_Core::AccessDeniedException&)
        {
            LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "AccessDeniedException", "setGlobalMmsInhibit");
        }
        catch (const TA_Base_Core::DatabaseErrorException&)
        {
            LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "DatabaseErrorException", "setGlobalMmsInhibit");
        }
        catch (...)
        {
            LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "setGlobalMmsInhibit");
        }
    
        sendGlobalInhibitAuditMessage(result);
    }
    
    
    bool MmsControllerModel::getSubsystemMmsInhibit(const std::string& subsystemName)
    {
        CWaitCursor wait;
    
        try
        {
            return (*getDatanodeProxy(subsystemName, true))->getInhibitMms();
        }
        catch (...)
        {            
            // Allow all exceptions to propagate
            throw;
        }                    
    }    


    void MmsControllerModel::setSubsystemMmsInhibit(std::vector<TA_Base_Core::DataNodeEntityData*>& subsystems, const bool subsystemMmsInhibit)
    {
        CWaitCursor wait;

        // failedSubsystem is going to store any subsystem names that failed to change its inhibition status; successSubsystems are the ones that successfully changed their inhibition status
        std::vector<std::string> insufficientRights, failedSubsystems, successSubsystems;

        for (std::vector<TA_Base_Core::DataNodeEntityData*>::iterator itr = subsystems.begin();
                itr != subsystems.end();
                ++itr )
        {
            std::string name = (*itr)->getName();

            // Must use the subsystem datanode entity key as rights checking parameter
            if (!CachedConfig::getInstance().isFunctionPermitted(TA_Base_Bus::aca_MMS_SUBSYSTEM_INHIBIT, (*itr)->getKey()))
            {
                insufficientRights.push_back(name);
            }
            else
            {
                try
                {
                    (*getDatanodeProxy(name,true))->setInhibitMms(subsystemMmsInhibit, CachedConfig::getInstance().getSessionId());
                    successSubsystems.push_back(name);
                }        
                catch (...)
                {
                    LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "setSubsystemMmsInhibit");
                    failedSubsystems.push_back(name);
                }
            }        
        }

        if (!insufficientRights.empty())
        {
            // output the error message
	        std::ostringstream errorStr;
			// TD14164 ++
	        /*errorStr << "Insufficient operator privileges to modify inhibit status on the following subsystems:" << std::endl;*/
			TA_Base_Bus::TransActiveMessage userMsg;
			// ++ TD14164 

	        for (unsigned int i = 0; i < insufficientRights.size(); ++i )
	        {
	            errorStr << "\t" << insufficientRights[i] << std::endl;
	        }

			// TD14164 ++
			userMsg << errorStr.str(); 

	        /*AfxMessageBox(errorStr.str().c_str());*/
			userMsg.showMsgBox(IDS_UE_190002);
			// ++ TD14164 
        }
        if (!successSubsystems.empty())
        {
            // audit the success
            sendSubsystemInhibitAuditMessage(successSubsystems, subsystemMmsInhibit ? INHIBIT_APPLIED : INHIBIT_REMOVED);
        }
        if (!failedSubsystems.empty())
        {
            // audit the failure
            sendSubsystemInhibitAuditMessage(failedSubsystems, INHIBIT_FAILED);

			// marvin++
			// bug 636 (TD12213)
			std::ostringstream errorStr;

			// TD14164 ++
			TA_Base_Bus::TransActiveMessage userMsg;
			CString action;
			// ++ TD14164 

			if(subsystemMmsInhibit==true)
			{
				// TD14164 ++
				/*errorStr << "Failed to inhibit the following subsystems:";*/
				action = "inhibit";
				userMsg << action;
				// ++ TD14164 
			}
			else
			{
				// TD14164 ++
				/*errorStr << "Failed to uninhibit the following subsystems:";*/
				action = "uninhibit";
				userMsg << action;
				// ++ TD14164 
			}
			
			errorStr << failedSubsystems[0];
            for (unsigned int i = 1; i < failedSubsystems.size(); i++)
            {
				errorStr << ", " << failedSubsystems[i];
            }

			// TD14164 ++
			userMsg << errorStr.str();

			/*AfxMessageBox(errorStr.str().c_str());*/
			userMsg.showMsgBox(IDS_UE_190003);
			// ++ TD14164 

			// ++marvin
			// bug 636 (TD12213)
        }
    }


    TA_Base_Bus::DataNodeProxySmartPtr * MmsControllerModel::getDatanodeProxy(const std::string equipmentName, bool waitForInit)
    {
		TA_Base_Core::ThreadGuard guard( m_dataNodeCacheLock );

        //  The DataNodeProxySmartPtr needs to be kept allocated so that we receive updates
        //  to the proxies (such as ConfigAvailable, to read the state of the MMS Inhibit flag).
        //  We can allocate as many proxies for the same entity as we want, but we don't know
        //  when we will be finished with them, so just keep a list and use the proxies from there.
        try
        {
        
            TA_Base_Core::IEntityData* entitydata = TA_Base_Core::EntityAccessFactory::getInstance().getEntity(equipmentName);	
            unsigned long entitykey = entitydata->getKey();
            delete entitydata;
            MmsDataNodeCache::iterator dnIter = m_dataNodeCache.find(entitykey);
            if (dnIter != m_dataNodeCache.end())
            {
                return dnIter->second;
            }

	    
            TA_Base_Bus::DataNodeProxySmartPtr * result = new TA_Base_Bus::DataNodeProxySmartPtr();
			TA_Base_Bus::ScadaProxyFactory::getInstance().createDataNodeProxy( entitykey, *this, *result);

		    m_dataNodeCache.insert( MmsDataNodeCache::value_type( entitykey, result));
		    return result;
        }
		catch(...)   
        {
			LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugError, "getDatanodeProxy can not return a datanode proxy" );
			throw;
        }
		return NULL;
    }


    void MmsControllerModel::sendGlobalInhibitAuditMessage(const EInhibitResult& result)
    {
        try
        {
            TA_Base_Core::DescriptionParameters descriptionParameters;

            switch (result)
            {
            case INHIBIT_APPLIED:
                submitAuditMessage(TA_Base_Core::MmsAudit::MMSGlobalInhibitApplied, descriptionParameters);
                break;
            case INHIBIT_REMOVED:
                submitAuditMessage(TA_Base_Core::MmsAudit::MMSGlobalInhibitRemoved, descriptionParameters);
                break;
            case INHIBIT_FAILED:
                submitAuditMessage(TA_Base_Core::MmsAudit::MMSGlobalInhibitFailed, descriptionParameters);
                break;
            default:
				LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugError, "sendGlobalInhibitAuditMessage: Unrecognized inhibit result" );
				break;
            }
        }
        catch (...)
        {
            LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "sendGlobalInhibitAuditMessage");
        }
    }
   
     
    void MmsControllerModel::sendSubsystemInhibitAuditMessage(const std::vector<std::string>& subsystemName, const EInhibitResult& result)
    {
		if (subsystemName.empty())
		{
			LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugError, "No subsystem is specified" );
			return;
		}
        try
        {
               
            TA_Base_Core::DescriptionParameters descriptionParameters;
			TA_Base_Core::DescriptionParameters alarmDescriptionParameters;

            std::string names = subsystemName[0];
            for (unsigned int i = 1; i < subsystemName.size(); ++i)
            {
                names += ", " + subsystemName[i];
            }

            TA_Base_Core::NameValuePair pair ( "SubsystemName", names );
			TA_Base_Core::NameValuePair alarmPair ( "SubsystemNames", names );
            descriptionParameters.push_back ( &pair );
			alarmDescriptionParameters.push_back ( &alarmPair );

            switch (result)
            {
            case INHIBIT_APPLIED:
                submitAuditMessage(TA_Base_Core::MmsAudit::MMSSubsystemInhibitApplied, descriptionParameters);
                break;
            case INHIBIT_REMOVED:
                submitAuditMessage(TA_Base_Core::MmsAudit::MMSSubsystemInhibitRemoved, descriptionParameters);
                break;
            case INHIBIT_FAILED:
                submitAuditMessage(TA_Base_Core::MmsAudit::MMSSubsystemInhibitFailed, descriptionParameters);
                break;
            default:
				LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugError, "sendGlobalInhibitAuditMessage: Unrecognized inhibit result" );
                break;
            }
        }
        catch (...)
        {
            LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "sendSubsystemInhibitAuditMessage");
        }
    }

    
    void MmsControllerModel::submitAuditMessage( 
        const TA_Base_Core::MessageType& messageType,
        const TA_Base_Core::DescriptionParameters& descriptionParameters)
    {
        FUNCTION_ENTRY("submitAuditMessage");   
        
        try
        {
			if (NULL == m_auditSender)
			{
				LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugError, "m_auditSender is NULL" );

				FUNCTION_EXIT;
				return;
			}

            std::string sessionId = CachedConfig::getInstance().getSessionId();
            unsigned long entityKey = CachedConfig::getInstance().getEntityKey();
    
            m_auditSender->sendAuditMessage(
                messageType,  // Message Type
                entityKey,                                // Entity key
                descriptionParameters,                    // Description
                "",                                       // Additional details
                sessionId,                                // SessionID if applicable
                "",                                       // AlarmID if an alarm associated with event
                "",                                       // IncidentKey if incident associated with event
                "");                                      // EventID of a parent event, used to link events
    
            LOG( SourceInfo, 
                TA_Base_Core::DebugUtil::GenericLog, 
                TA_Base_Core::DebugUtil::DebugInfo, 
                "submitAuditMessage: Audit message dispatched");
        } 
        catch(...)
        {
            LOG(SourceInfo,     
                TA_Base_Core::DebugUtil::GenericLog,  
                TA_Base_Core::DebugUtil::DebugError, 
                "Failed to submit MMS Audit message");
        }    
    
        FUNCTION_EXIT;
    }


    void MmsControllerModel::processEntityUpdateEvent(unsigned long entityKey, TA_Base_Bus::ScadaEntityUpdateType updateType)
    {
        //  This function is executing in a non-UI thread, so ask the UI to update itself in its own thread.
         //::PostMessage( AfxGetMainWnd()->GetSafeHwnd(), WM_UPDATE_SUBSYSTEM_CHECKBOX, 0, 0);
		if (NULL != m_targetWnd) ::PostMessage( m_targetWnd, WM_UPDATE_DIALOG_CONTROLS, 0, 0);
    }

	void MmsControllerModel::processNonPhysicalEntityMMSUpdate(unsigned long entityKey, const TA_Base_Core::EntityStatusData& entityStatus, EInhibitResult systemInhibit)
	{
		TA_Base_Core::ThreadGuard guard( m_nonPhyisicalEntitiesInhibitCacheLock );

	    typedef std::map<unsigned long, TA_Base_Core::ConfigEntity*>::iterator ConfigIterator;
		// locate the entity to be updated from the list of non physical entities
		ConfigIterator confIter = m_nonPhysicalSubsystemEntities.find(entityKey);
		if (confIter != m_nonPhysicalSubsystemEntities.end())
		{
			// update the entity's status data
			TA_Base_Core::ConfigEntity* configData = dynamic_cast<TA_Base_Core::ConfigEntity*>(confIter->second);
			configData->setEntityStatusData(entityStatus);

			if (systemInhibit == SYSTEM_ALARM_INHIBIT_APPLIED)
			{
				// add in to the list of mms inhibited entities
				TA_Base_Core::IEntityData* entityData = dynamic_cast<TA_Base_Core::IEntityData*>(configData);		
				m_systemAlarmMmsInhibitedList.insert(std::map<unsigned long, TA_Base_Core::IEntityData*>::value_type(entityKey,
						configData));			
			}
			else if (systemInhibit == SYSTEM_ALARM_INHIBIT_REMOVED)
			{	// otherwise, remove it from the list
				std::map<unsigned long, TA_Base_Core::IEntityData*>::iterator entityIter;
				entityIter = m_systemAlarmMmsInhibitedList.find(entityKey);
				if (entityIter != m_systemAlarmMmsInhibitedList.end())
					m_systemAlarmMmsInhibitedList.erase(entityIter);

			}
		}
		if (NULL != m_targetWnd) ::PostMessage( m_targetWnd, WM_UPDATE_DIALOG_CONTROLS, 0, 0);
	}


	bool MmsControllerModel::setNonPhysicalSubsystemMmsInhibit(std::vector<TA_Base_Core::ConfigEntity*>& entityList, bool mmsSystemInhibt)
	{
		std::vector<std::string> insufficientRights, failedSubsystems, successSubsystems;
		
		EInhibitResult systemInhibit = (true == mmsSystemInhibt) ? SYSTEM_ALARM_INHIBIT_APPLIED : SYSTEM_ALARM_INHIBIT_REMOVED;

		for (std::vector<TA_Base_Core::ConfigEntity*>::iterator it = entityList.begin(); 
			it != entityList.end(); 
			++it)
		{
			TA_Base_Core::ConfigEntity* configData = dynamic_cast<TA_Base_Core::ConfigEntity*>(*it);
			std::string name = configData->getName();
			unsigned long entityKey = configData->getKey();
			if (!CachedConfig::getInstance().isFunctionPermitted(TA_Base_Bus::aca_MMS_SYSTEM_ALARM_INHIBIT, entityKey))
			{				
				insufficientRights.push_back(name);
			}
			else
			{	
				try
				{
					////TD18520
					configData->justApplyEntityStatusChange();							
					
					if (systemInhibit == SYSTEM_ALARM_INHIBIT_APPLIED)
					{
						m_systemAlarmMmsInhibitedList[configData->getKey()] = dynamic_cast<TA_Base_Core::IEntityData*>(configData);
					}
					else if (systemInhibit == SYSTEM_ALARM_INHIBIT_REMOVED)
					{
						std::map<unsigned long, TA_Base_Core::IEntityData*>::iterator confIter = 
							m_systemAlarmMmsInhibitedList.find(configData->getKey());
						if (confIter != m_systemAlarmMmsInhibitedList.end())
						{
							m_systemAlarmMmsInhibitedList.erase(confIter);
						}
					}	
					
					successSubsystems.push_back(name);
					if (NULL != m_onlineSystemAlarmMmsUpdater)
					{
						m_onlineSystemAlarmMmsUpdater->sendMmsStateUpdateMessage(configData->getEntityStatusData(TA_Base_Core::MMSInhibitStatus), systemInhibit);
					}
					
				}
				catch(TA_Base_Core::DataException&)
				{
				    LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "DataException", "setNonPhysicalSubsystemMmsInhibit");
					failedSubsystems.push_back(name);					 
				}
				catch(...)
				{
				    LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "setNonPhysicalSubsystemMmsInhibit");
					failedSubsystems.push_back(name);
				}
				
			}
		}

		if (!insufficientRights.empty())
        {
            // output the error message
	        std::ostringstream errorStr;
			TA_Base_Bus::TransActiveMessage userMsg;

	        for (unsigned int i = 0; i < insufficientRights.size(); ++i )
	        {
	            errorStr << "\t" << insufficientRights[i] << std::endl;
	        }
			userMsg << errorStr.str(); 

			userMsg.showMsgBox(IDS_UE_190002);
        }
       
		if (!successSubsystems.empty())
        {
            // audit the success
            sendNonPhysicalSubsystemInhibitAuditMessage(successSubsystems, systemInhibit);
        }
        if (!failedSubsystems.empty())
        {			
 			sendNonPhysicalSubsystemInhibitAuditMessage(failedSubsystems, SYSTEM_ALARM_INHIBIT_FAILED);

			std::ostringstream errorStr;
			TA_Base_Bus::TransActiveMessage userMsg;
			CString action; 
			action = "commit changes for"; 
			userMsg << action;
 
			errorStr << failedSubsystems[0];
            for (unsigned int i = 1; i < failedSubsystems.size(); i++)
            {				
				errorStr << ", " << failedSubsystems[i] << std::endl;
             }

			userMsg << errorStr.str(); 

			userMsg.showMsgBox(IDS_UE_190003);
        }
	 
		updateNonPhysicalSubsystemMMSInhibitStatus();

		return true;
	}

	std::map<unsigned long, TA_Base_Core::ConfigEntity*>& MmsControllerModel::getNonPhysicalSubsystems()
	{
		TA_Base_Core::ThreadGuard guard( m_nonPhyisicalEntitiesCacheLock );

		if (m_nonPhysicalSubsystemEntities.empty())
		{
			typedef std::vector<TA_Base_Core::IEntityData*> IEntityList;
			typedef std::vector<TA_Base_Core::ISystemControllerData*> ISystemControllerList;				
			try
			{				
				IEntityList configEntityList;	
				configEntityList = TA_Base_Core::EntityAccessFactory::getInstance().getEntitiesOfType(TA_Base_Core::Process::getStaticType(), true);
				
				IEntityList::iterator itc;		
                for (itc =configEntityList.begin(); itc != configEntityList.end(); ++itc)
				{
					TA_Base_Core::ConfigEntity* configItem = dynamic_cast<TA_Base_Core::ConfigEntity*>(*itc);
					unsigned long configItemKey = (*itc)->getKey();
					m_nonPhysicalSubsystemEntities.insert(std::map<unsigned long, TA_Base_Core::ConfigEntity*>::value_type(configItemKey,
								configItem));
                }
                                                
                //Mintao++ TD16762
				IEntityList configChildList; 
                configChildList = TA_Base_Core::EntityAccessFactory::getInstance().getAllChildEntitiesOfType(TA_Base_Core::Process::getStaticType(), true);
				std::vector<TA_Base_Core::IEntityData*>::iterator it;		
				for (it = configChildList.begin(); it != configChildList.end(); ++it)
				{
					TA_Base_Core::ConfigEntity* configData = dynamic_cast<TA_Base_Core::ConfigEntity*>(*it);
					if (configData != NULL)
					{							
						m_nonPhysicalSubsystemEntities.insert(std::map<unsigned long, TA_Base_Core::ConfigEntity*>::value_type(configData->getKey(),
							configData));				
					}
				}					
                //Mintao++ TD16762
			}
			catch (TA_Base_Core::DatabaseException&)
			{
				LOG_EXCEPTION_CATCH(SourceInfo, "DatabaseException", "Could not retrieve monitored process data");
			}
			catch (TA_Base_Core::DataException&)
			{
				LOG_EXCEPTION_CATCH(SourceInfo, "DataException", "Could not retrieve monitored process data");				
			}
			catch (...)
			{
				LOG_EXCEPTION_CATCH(SourceInfo, "Unknown Exception", "Could not retrieve monitored process data");
			}

			try
			{
				ISystemControllerList systemControllerList; 
				systemControllerList = TA_Base_Core::SystemControllerAccessFactory::getInstance().getAllSystemControllers(true);
				ISystemControllerList::iterator systemIter;			 
				TA_Base_Core::ConfigEntity* configItem = NULL; 
				for ( systemIter = systemControllerList.begin(); systemIter!= systemControllerList.end(); ++systemIter )
				{
					configItem =  dynamic_cast<TA_Base_Core::ConfigEntity*>(TA_Base_Core::EntityAccessFactory::getInstance().getEntity((*systemIter)->getServerEntityKey(), true));
					if (configItem != NULL)
					{						
						m_nonPhysicalSubsystemEntities.insert(std::map<unsigned long, TA_Base_Core::ConfigEntity*>::value_type(configItem->getKey(),
							configItem));
					}
				}				 
			}
			catch (TA_Base_Core::DatabaseException&)
			{
				LOG_EXCEPTION_CATCH(SourceInfo, "DatabaseException", "Could not retrieve server entities");
			}
			catch (TA_Base_Core::DataException&)
			{
				LOG_EXCEPTION_CATCH(SourceInfo, "Data Exception", "Could not retrieve server entities");				
			}
			catch (...)
			{
				LOG_EXCEPTION_CATCH(SourceInfo, "Unknown exception", "Could not retrieve server entities");	
			}
		}
		
		return m_nonPhysicalSubsystemEntities;
	}


	std::map<unsigned long, TA_Base_Core::IEntityData*>& MmsControllerModel::getNonPhysicalSubsystemMmsInhibit(bool bForceUpdate)
	{	// mms Inhibited non physical subsystems are stored in a separate map, initialized together when all
		// non physical subsystems are retrieved from the database

		TA_Base_Core::ThreadGuard guard( m_nonPhyisicalEntitiesInhibitCacheLock );
		
		if (m_nonPhysicalSubsystemEntities.empty())
		{
			getNonPhysicalSubsystems();
		}

		
		if (bForceUpdate)
		{
			m_systemAlarmMmsInhibitedList.clear();			
		}
		
		if (!m_nonPhysicalSubsystemEntities.empty() && m_systemAlarmMmsInhibitedList.empty())
		{	
			typedef std::map<unsigned long, TA_Base_Core::ConfigEntity*>::iterator ConfigIterator;
			for(ConfigIterator itr = m_nonPhysicalSubsystemEntities.begin(); itr != m_nonPhysicalSubsystemEntities.end(); itr++)
			{
				TA_Base_Core::ConfigEntity* configData = dynamic_cast<TA_Base_Core::ConfigEntity*>(itr->second);
				try
				{					
					if (true == configData->getBoolEntityStatusValue(TA_Base_Core::MMSInhibitStatus))
					{
						m_systemAlarmMmsInhibitedList.insert(std::map<unsigned long, TA_Base_Core::IEntityData*>::value_type(configData->getKey(),
							dynamic_cast<TA_Base_Core::IEntityData*>(configData)));
					}
				}
				catch (TA_Base_Core::DataException&)
				{
					 // do not log anything
				}
				
			}
		}
		

		return m_systemAlarmMmsInhibitedList;
	}

	void MmsControllerModel::sendNonPhysicalSubsystemInhibitAuditMessage(const std::vector<std::string>& nonPhysicalsystemName, const EInhibitResult inhibitResult)
	{
		if (nonPhysicalsystemName.empty())
		{
			LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugError, "No non-physical system is specified" );
			return;
		}
        try
        {
			 std::string names = nonPhysicalsystemName[0];
            for (unsigned int i = 1; i < nonPhysicalsystemName.size(); ++i)
            {
                names += ", " + nonPhysicalsystemName[i];
            }

			TA_Base_Core::DescriptionParameters descriptionParameters;
            TA_Base_Core::NameValuePair pair ( "EntityName", names );
            descriptionParameters.push_back ( &pair );
 
            switch (inhibitResult)
            {
            case SYSTEM_ALARM_INHIBIT_APPLIED:
                submitAuditMessage(TA_Base_Core::MmsAudit::MMSSystemAlarmInhibitApplied, descriptionParameters);
                break;
            case SYSTEM_ALARM_INHIBIT_REMOVED:
                submitAuditMessage(TA_Base_Core::MmsAudit::MMSSystemAlarmInhibitRemoved, descriptionParameters);
                break;
            case SYSTEM_ALARM_INHIBIT_FAILED:
                submitAuditMessage(TA_Base_Core::MmsAudit::MMSSystemAlarmInhibitFailed, descriptionParameters);
                break;
            default:
				LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugError, "sendNonPhysicalSubsystemInhibitAuditMessage: Unrecognized inhibit result" );
                break;
            }
        }
        catch (...)
        {
            LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "sendNonPhysicalSubsystemInhibitAuditMessage");
        }

	} 

	void MmsControllerModel::updateNonPhysicalSubsystemMMSInhibitStatus()
	{
		//::PostMessage( AfxGetMainWnd()->GetSafeHwnd(), WM_UPDATE_SUBSYSTEM_CHECKBOX, 0, 0);
		if (NULL != m_targetWnd) ::PostMessage( m_targetWnd, WM_UPDATE_DIALOG_CONTROLS, 0, 0);
	}
}

