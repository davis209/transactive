/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source: $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/CachedConfig.cpp $
  * @author Jade Lee
  * @version $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by: $Author: builder $
  * 
  * Caches local parameters specific to MMSController application
  *
  */

#if defined(_MSC_VER)
#pragma warning(disable : 4786 4146 4018 4250 4290 4284)
#endif // defined _MSC_VER

#include <string>
#include <vector>

#include "StdAfx.h"
#include "CachedConfig.h"
#include "IRightsChangeListener.h"

#include "core/synchronisation/src/ThreadGuard.h"
#include "core/utilities/src/DebugUtil.h"
#include "core/data_access_interface/src/LocationAccessFactory.h"

#include "bus/security/rights_library/src/RightsLibrary.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////

    CachedConfig::CachedConfig()
    {
        // Have us listen out for changes to session ID
        TA_Base_Core::RunParams::getInstance().registerRunParamUser(this, RPARAM_SESSIONID);

    	TA_Base_Bus::RightsLibraryFactory   rlf;
        // TA_Base_Bus::ALWAYS_TRUE_RL
        m_rightsLibrary = rlf.buildRightsLibrary(TA_Base_Bus::DEFAULT_RL, TA_Base_Bus::RightsLibraryFactory::MANAGER);
    }


    CachedConfig::~CachedConfig()
    {
    }


    CachedConfig& CachedConfig::getInstance()
    {      
        static CachedConfig instance;
        return instance;
    }


    void CachedConfig::removeInstance( )
    {
	    // Nothing to do
    }


    void CachedConfig::setEntityKey(unsigned long param)
    {
        ThreadGuard guard( m_lockForConfigUpdate );
        m_entityKey.setValue(param);
    }


    void CachedConfig::setEntityName(const std::string& param)
    {
        ThreadGuard guard( m_lockForConfigUpdate );
        m_entityName.setValue(param);
    }


    void CachedConfig::setEntityTypeKey(unsigned long param)
    {
        ThreadGuard guard( m_lockForConfigUpdate );
        m_entityTypeKey.setValue(param);
    }


    void CachedConfig::setSessionId(const std::string& sessionId)
    {
        ThreadGuard guard( m_lockForConfigUpdate );
        m_sessionId.setValue(sessionId);
    }


    void CachedConfig::onRunParamChange(const std::string& name, const std::string& value)
    {        
        if (RPARAM_SESSIONID == name)
        {
            // The session ID has changed, so must propogate associated rights changes throughout GUI
            setSessionId(value);
            
            CachedConfig::getInstance().flagChangeInRights();
        }
    }


    void CachedConfig::setEntityLocationKey(unsigned long param)
    {
        if (param == 0 || param == -1)
        {
            TA_THROW( TA_Base_Core::TransactiveException("Invalid LocationKey set for Radio Monitor") );
        }

        ThreadGuard guard( m_lockForConfigUpdate );
        m_entityLocationKey.setValue(param);
    }

    void CachedConfig::setEntitySubsystemKey(unsigned long param)
    {
        if ( param == -1)
        {
            TA_THROW( TA_Base_Core::TransactiveException("Invalid SubsystemKey set for Radio Monitor") );
        }

        ThreadGuard guard( m_lockForConfigUpdate );
        m_entitySubsystemKey.setValue(param);
    }


    void CachedConfig::setPhysicalLocationKey(unsigned long key)
    {
        ThreadGuard guard( m_lockForConfigUpdate );
        m_physicalLocationKey.setValue(key);
    }


    void CachedConfig::setMmsInhibitAgentEntityName(const std::string& name)
    {
        ThreadGuard guard( m_lockForConfigUpdate );
        m_mmsInhibitAgentEntityName = name;
    
        m_mmsInhibitAgentObject.setEntityName(m_mmsInhibitAgentEntityName,true);
    }


    void CachedConfig::setMmsInhibitAgentEntityKey(unsigned long key)
    {
        ThreadGuard guard( m_lockForConfigUpdate );
        m_mmsInhibitAgentEntityKey.setValue(key);        
    }


    unsigned long CachedConfig::getPhysicalLocationKey()
    {
        ThreadGuard guard( m_lockForConfigUpdate );
        return m_physicalLocationKey.getValue();
    }


    unsigned long CachedConfig::getEntityKey()
    {
        ThreadGuard guard( m_lockForConfigUpdate );
        return m_entityKey.getValue();
    }

    std::string CachedConfig::getEntityName()
    {
        ThreadGuard guard( m_lockForConfigUpdate );
        return m_entityName.getValue();
    }

    unsigned long CachedConfig::getEntityTypeKey()
    {
        ThreadGuard guard( m_lockForConfigUpdate );
        return m_entityTypeKey.getValue();
    }

    unsigned long CachedConfig::getEntityLocationKey()
    {
        ThreadGuard guard( m_lockForConfigUpdate );
        return m_entityLocationKey.getValue();
    }

    unsigned long CachedConfig::getEntitySubsystemKey()
    {
        ThreadGuard guard( m_lockForConfigUpdate );
        return m_entitySubsystemKey.getValue();
    }

    std::string CachedConfig::getSessionId()
    {
        ThreadGuard guard( m_lockForConfigUpdate );
        return m_sessionId.getValue();
    }

    void CachedConfig::registerRightsChangeListener(IRightsChangeListener& listener)
    {
        TA_Base_Core::ThreadGuard sync(m_listenerLock);
    
        if (!m_rightsChangeListeners.insert(&listener).second)
        {
			LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Listener already registered" );
        }
    }
    
    
    void CachedConfig::deRegisterRightsChangeListener(IRightsChangeListener& listener) 
    {
        TA_Base_Core::ThreadGuard sync(m_listenerLock);
    
        std::set<IRightsChangeListener*>::iterator itr = m_rightsChangeListeners.find(&listener);
    
        if (itr == m_rightsChangeListeners.end())
        {
			LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Could not find listener to deRegister" );
        }
        else
        {
            m_rightsChangeListeners.erase(itr);
        }
    }
    
    
    void CachedConfig::flagChangeInRights()
    {
        TA_Base_Core::ThreadGuard sync(m_listenerLock);
    
        for (std::set<IRightsChangeListener*>::iterator itr = m_rightsChangeListeners.begin();
                itr != m_rightsChangeListeners.end();
                itr ++)
        {
            (*itr)->onRightsChanged();
        }
    }
    

    bool CachedConfig::isMmsFunctionPermitted(TA_Base_Bus::AccessControlledAction action)
    {
        // Use the MMS Agent key as resource provider - for default implementation
        return isFunctionPermitted(action, m_mmsInhibitAgentEntityKey.getValue());
    }


    bool CachedConfig::isFunctionPermitted(TA_Base_Bus::AccessControlledAction action, unsigned long entityKey)
    {    
        bool        isPermitted = false;
        static bool haveWarnedUser = false;
        std::string reason;

        try
        {
            TA_Base_Bus::EDecisionModule decisionModule = TA_Base_Bus::UNDEFINED_DM;
            std::string sessionId = getSessionId();
    
            std::auto_ptr<TA_Base_Core::IResource> resource
                (TA_Base_Core::ResourceAccessFactory::getInstance().getResourceFromEntity(entityKey));

			if (0 == resource.get())
			{
				reason = "Get resource failed";
                LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Resource is NULL, should have thrown exception" );
				TA_THROW(TA_Base_Core::TransactiveException("Resource is NULL"));
			}
            unsigned long resourceID = resource->getKey();

			if (NULL == m_rightsLibrary)
			{
				reason =  "Get rights library failed";
				LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugError, "m_rightsLibrary is NULL, should have thrown exception" );
                TA_THROW(TA_Base_Core::TransactiveException("m_rightsLibrary is NULL"));
			}
    
            isPermitted =  m_rightsLibrary->isActionPermittedOnResource(sessionId, 
    													          resourceID, 
    													          action, 
    													          reason, 														 
    													          decisionModule);
            haveWarnedUser = false;  // Working again now!!
        }
        catch (...)
        {
            // Error checking rights
            // Log the problem
            std::string messageText = "Unable to check access rights, reason: " + reason;
            LOG_EXCEPTION_CATCH(SourceInfo, "isFunctionPermitted Exception",reason.c_str());
    
            // Let the user know (but only the first time)
            if (!haveWarnedUser)
            {
    	        //AfxMessageBox(messageText.c_str());
    	        haveWarnedUser = true;
            }
        }
    
        return isPermitted;
    }

} // end namespace TA_IRS_App
