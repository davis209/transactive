/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:  $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/MmsControllerModel.h $
  * @author Jade Lee
  * @version $Revision: #2 $
  *
  * Last modification: $DateTime: 2009/01/08 14:06:20 $
  * Last modified by: $Author: builder $
  * 
  * Provides business logic of MMS Controller
  *
  */

#pragma once

#include <map>
#include "app/maintenance_management/MmsController/src/InhibitResult.h"
#include "core/message/src/NameValuePair.h"
#include "core/synchronisation/src/ReEntrantThreadLockable.h"
#include "bus/scada/proxy_library/src/DataNodeProxySmartPtr.h"
#include "bus/scada/proxy_library/src/IEntityUpdateEventProcessor.h"
#include "core/data_access_interface/entity_access/src/EntityStatusData.h"

namespace TA_Base_Core
{
    class AuditMessageSender;
    class MessageType;
    class DataNodeEntityData;
	class EntityData;
	class IEntityData;
	class ConfigEntity;
}


namespace TA_IRS_App
{
	class MmsControllerOnlineUpdater;
    /**
     *
     *  The required audit logging will be performed for set functions 
     * - failures are silent (sending out audit messages)
     */
    class MmsControllerModel : public TA_Base_Bus::IEntityUpdateEventProcessor
    {
        //friend class MmsControllerModelTestCases;

    public:

        MmsControllerModel();
        virtual ~MmsControllerModel();

        /**
         * get routine for the global MMS inbibit functionality
         * @exception ... upon error
         */ 
        bool getGlobalMmsInhibit() const;

        /**
         * set routine for the global MMS inbibit functionality
         * ExceptionChecked
         */
        void setGlobalMmsInhibit(bool globalMmsInhibit);

        /**
         * get routine for the subsystem MMS inbibit functionality
         * @param subsystemName gives the literal name of the subsystem in question
         * @exception ScadaProxyException; ... upon error
         * Note: Can't be const because of listener that must be passed to scada proxy factory
         */ 
        bool getSubsystemMmsInhibit(const std::string& subsystemName);

        /** 
          * setSubsystemMmsInhibit
          *
          * Inhibits a set of subsystems.  It will audit the successful and failed inhibits and display an
          * error message for the subsystem that the operator does not have the rights to control.
          *
          * @param subsystems The subsystems to inhibit
          * @param subsystemMmsInhibit defines new inhibited state to apply
          */
        void setSubsystemMmsInhibit(std::vector<TA_Base_Core::DataNodeEntityData*>& subsystems, const bool subsystemMmsInhibit);

        /** 
          * processEntityUpdateEvent
          *
          * Required by IEntityUpdateEventProcessor.
          *
          * @param entityKey    Who the update is for; probably will be this->m_corbaName.getEntityKey()
          *                     or its parent entity id.
          * @param updateType   What type of update (see bus/scada/proxies2/ProxyDefs.h)
          *
          */
        virtual void processEntityUpdateEvent(unsigned long entityKey, TA_Base_Bus::ScadaEntityUpdateType updateType);

		/** 
		  * setNonPhysicalSubsystemMmsInhibit
		  * 
		  * Commits the changes of the submitted entity data's MMS inhibit status. 
		  * This will encapsulate the checking of rights and audit messages sending functions.
		  *
		  * @param entityList	a set of entity data that has the inhibition status changed
		  *
		  */
		bool setNonPhysicalSubsystemMmsInhibit(std::vector<TA_Base_Core::ConfigEntity*>& entityList, bool mmsSystemInhibt = true);

		
		/** 
		  * getNonPhysicalSubsystems
		  * 
		  * returns all the non-physical subsystem entities
		  *
		  */
		std::map<unsigned long, TA_Base_Core::ConfigEntity*>& getNonPhysicalSubsystems();
	
		/** 
		  * getNonPhysicalSubsystemMmsInhibit
		  * 
		  * returns the non-physical subsystem entities that are MMS inhibited
		  *
		  */

		std::map<unsigned long, TA_Base_Core::IEntityData*>& getNonPhysicalSubsystemMmsInhibit(bool bForceUpdate = false);
					
    	   /** 
          * processNonPhysicalEntityMMSUpdate
          *
          * This function is to process and update the list of nonphysical entities
		  * in the MMS inhibited entity data lst
          *
          * @param entityKey     Who the update is for  
          * @param entityStatus  entity status data details for the status update
          * @param systemInhibit inhibit result that the entity will be updated to
		  * 
          */
        virtual void processNonPhysicalEntityMMSUpdate(unsigned long entityKey, const TA_Base_Core::EntityStatusData& entityStatus, EInhibitResult systemInhibit);
		
		/** 
		  * setTargetWnd
		  * 
		  * sets the parent window for notification
		  *
		  */
		void setTargetWnd(HWND targetWnd) { m_targetWnd = targetWnd; };
    private:
	    void updateNonPhysicalSubsystemMMSInhibitStatus();

        MmsControllerModel(const MmsControllerModel&);
    	MmsControllerModel& operator=(MmsControllerModel&);

        /**
         * @return the datanode proxy for the datanode defined by specified entity equipment name
		 * @param waitForInit true if the function should block until the data node returned is
		 *			initialised 
		 * @exception TransactiveException if waitForInit is true and the maximum timeout of 2 seconds expires
		 */
        TA_Base_Bus::DataNodeProxySmartPtr * getDatanodeProxy(const std::string equipmentName, bool waitForInit = false);

        /**
         * Dispatches a global inhibit audit message based on the status provided in 'result'
         * @param result defines the outcome of the inhibit change request that had been carried out
         * ExceptionChecked
         */
        void sendGlobalInhibitAuditMessage(const EInhibitResult& result);
    
        /**
         * Dispatches a subsystem inhibit audit message based on input parameters
         * @param subsystemName gives the literal name of the subsystem in question
         * @param result defines the outcome of the inhibit change request that had been carried out
         * ExceptionChecked
         */
        void sendSubsystemInhibitAuditMessage(const std::vector<std::string>& subsystemName, const EInhibitResult& result);

        /**
         * submits the audit message of the specified type
         *
         * @param messageType defines the audit message
         * @param descriptionParameters optionally defines any parameters that need to
         *          be passed to the audit message sender
         * ExceptionChecked
         */
        void submitAuditMessage(const TA_Base_Core::MessageType& messageType, 
                                    const TA_Base_Core::DescriptionParameters& descriptionParameters);

		/** 
		  * submits the audit message:
		  * 
		  * MMSSystemAlarmInhibitApplied (for Add & Applied)
		  * MMSSystemAlarmInhibitRemoved (for Remove & Applied)
		  * MMSSystemAlarmInhibitFailed (for Failure to send out MMS update message 
		  *									or update the database)
		  *
		  * @param nonPhysicalsystemName gives the entity name of the non physical 
		  * @param inhibitResult	the inhibition action taken on the entity
		  *
		  */
		void sendNonPhysicalSubsystemInhibitAuditMessage(const std::vector<std::string>& nonPhysicalsystemName, 
														const EInhibitResult inhibitResult);


		/**
		  * save/update the change in the MMS Inhibit Status for the submitted entity-data in the DB 
		  *
		  * @param entityData		referenced entity data that has its inhibition status changed
		  * @param inhibitResult	the inhibition action taken on the entity
		  *
		  */


        TA_Base_Core::AuditMessageSender*            m_auditSender;

        typedef std::map< unsigned long, TA_Base_Bus::DataNodeProxySmartPtr* >  MmsDataNodeCache;
        MmsDataNodeCache m_dataNodeCache;

		TA_Base_Core::ReEntrantThreadLockable m_dataNodeCacheLock;
		
		TA_Base_Core::ReEntrantThreadLockable m_nonPhyisicalEntitiesCacheLock;
		TA_Base_Core::ReEntrantThreadLockable m_nonPhyisicalEntitiesInhibitCacheLock;


		std::map<unsigned long, TA_Base_Core::ConfigEntity*> m_nonPhysicalSubsystemEntities;
		std::map<unsigned long, TA_Base_Core::IEntityData*> m_systemAlarmMmsInhibitedList;

		MmsControllerOnlineUpdater* m_onlineSystemAlarmMmsUpdater;		
		
		// This variable is used to store the handle of the parent window for event notification
		HWND m_targetWnd;
    };
}