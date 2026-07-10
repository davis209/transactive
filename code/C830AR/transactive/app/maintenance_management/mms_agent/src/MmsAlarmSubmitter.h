/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  * 
  * Source:    $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/MmsAlarmSubmitter.h $
  * @author:   Chris DeWolfe
  * @version:  $Revision: #1 $
  * @see:      9999-Q01-02-0016-GenericAgent_Usage_Guidelines.doc
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last mofified by:  $Author: builder $
  *
  * Provides the implimentation of IMmsAlarmSubmitterCorbaDef
  */
#ifndef MMS_ALARM_SUBMITTER_H
#define MMS_ALARM_SUBMITTER_H

#if defined(_MSC_VER)
#pragma warning(disable:4786)
#endif // defined (_MSC_VER)




#include "bus/generic_agent/src/IEntity.h"
#include "bus/maintenance_management/mmsCorbaDef/idl/src/IMmsAlarmSubmitterCorbaDef.h"
#include "core/corba/src/ServantBase.h"
#include "core/synchronisation/src/ReEntrantThreadLockable.h"
#include "core/data_access_interface/entity_access/src/MmsAlarmSubmitterEntityData.h"

#include <deque>
#include <vector>

namespace TA_IRS_App
{

    class MmsAlarmSubmitter 
		: public virtual TA_Base_Bus::IEntity,
		  public virtual POA_TA_Base_Bus::IMmsAlarmSubmitterCorbaDef,
		  public virtual TA_Base_Core::ServantBase
    {
    public:

        /**
          * The only implimnented CTOR
          */
        MmsAlarmSubmitter();

        /**
          * DTOR
		  * Calls setMmsAlarmSubmitter(0) on all the registered users
		  * Diconnects from the MMS Server.
		  * Deletes the instance of this class
		  * 
          */
        virtual ~MmsAlarmSubmitter();

        /**
          * initConfiguration
		  *
		  *
		 */
		void initConfiguration(TA_Base_Core::MmsAlarmSubmitterEntityDataPtr pData);


     //IMmsAlarmSubmitterCorbaDef methods:

		/**
		 * Submit a MMS alarm message for the specified alarm.
		 *
		 * Note that no confirmation of success or failure is returned by this
		 * method. Hence, the class that impliments this interface is 
		 * responcible for approriate failure handling, e.g. raising alarm
		 *
		 * raises( TA_Base_Core::OperationModeException, TA_Base_Bus::MmsConnectionFailure )
		 */
		virtual void submitMmsAlarmMessage(const TA_Base_Core::AlarmDetailCorbaDef& p_alarm);
    
			

	//IEntity methods:
        /**
          * isValid
          *
          * allways returns true
          */
        virtual bool isValid();

        /**
          * start
          *
          * does nothing
          */
        virtual void start();

        /**
          * stop
          *
          * does nothing
          */
        virtual void stop();

        /**
          * update
          *
          * This method is called by the agent controlling the entity to ask 
          * the entity to update its configuration. Called after an update 
          * configuration event is received.
          */
        virtual void update(const TA_Base_Core::ConfigUpdateDetails& updateEvent);

        /**
          * remove
          *
          * does nothing
          */
        virtual void remove();

        /**
          * setToControlMode
          *
          * does nothing. see class comments
          */
        virtual void setToControlMode();

        /**
          * setToMonitorMode
          *
          * does nothing. see class comments
          */
        virtual void setToMonitorMode();

	//end IEntity methods


	private: //data members:
		std::string m_entityName;

		//provides a thread lock
		TA_Base_Core::ReEntrantThreadLockable m_lock;

	private: //methods:

        // Disable copy constructor and assignment operator
        MmsAlarmSubmitter( const MmsAlarmSubmitter& con);
        MmsAlarmSubmitter& operator=(const MmsAlarmSubmitter& con);

		 /**
          * checkOperationMode
          *
		  *
          * @throw OperationModeException if not in Control
          */
		void checkOperationMode();
		
    };
} // namespace TA_IRS_App

#endif // !defined(MMS_ALARM_SUBMITTER_H)