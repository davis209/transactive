/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  * 
  * Source:    $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/MmsJobRequestSubmitter.h $
  * @author:   Chris DeWolfe
  * @version:  $Revision: #1 $
  * @see:      9999-Q01-02-0016-GenericAgent_Usage_Guidelines.doc
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last mofified by:  $Author: builder $
  *
  * Provides the implimentation of IMmsJobRequestSubmitterCorbaDef
  */
#ifndef MMS_JOB_REQUEST_SUBMITTER_H
#define MMS_JOB_REQUEST_SUBMITTER_H

#if defined(_MSC_VER)
#pragma warning(disable:4786)
#endif // defined (_MSC_VER)




#include "bus/generic_agent/src/IEntity.h"
#include "bus/maintenance_management/mmsCorbaDef/idl/src/IMmsJobRequestSubmitterCorbaDef.h"
#include "core/corba/src/ServantBase.h"
#include "core/synchronisation/src/ReEntrantThreadLockable.h"
#include "core/data_access_interface/entity_access/src/MmsJobRequestSubmitterEntityData.h"

#include <deque>
#include <vector>

namespace TA_IRS_App
{

    class MmsJobRequestSubmitter 
		: public virtual TA_Base_Bus::IEntity,
		  public virtual POA_TA_Base_Bus::IMmsJobRequestSubmitterCorbaDef,
		  public virtual TA_Base_Core::ServantBase
    {
    public:

        /**
          * The only implimnented CTOR
          */
        MmsJobRequestSubmitter();

        /**
          * DTOR
		  * 
          */
        virtual ~MmsJobRequestSubmitter();

        /**
          * initConfiguration
		  *
		  *
		 */
		void initConfiguration(TA_Base_Core::MmsJobRequestSubmitterEntityDataPtr pData);


     //IMmsJobRequestSubmitterCorbaDef methods:

		/**
		*
		* Use this to get the data you need to populate the JRM when the JRM is
		* opened via an alarm.
		*
		* This method does not raise events - the JRM does that.
		* This method does report MMS Server health based on result of call to server 
		*
		* @param alarmUID: the selected alarm
		* @param alarmLocationKey: the location of the alarm agent that owns the
		* alarm
		* @param sessionID: The session on the MFT
		*
		* @throws TA_Base_Core::OperationModeException
		* @throws DependencyFailure
		* @throws InvalidArg exception for programatic error
		*/
		virtual TA_Base_Bus::JobRequestDetailCorbaDef* getDataFromAlarm(const char* alarmUID, 
			CORBA::ULong alarmLocationKey, const char* sessionID);

		/**
		*
		* Use this to get the data you need to populate the JRM when the JRM is
		* opened via a schematic symbol
		*
		* This method does not raise events - the JRM does that.
		* This method does report MMS Server health based on result of call to server 
		*
		* @param entityName: the selected datanode
		* 
		* @param sessionID: The session on the MFT
		*
		* JobRequestDetailCorbaDef::alarmUID is set to "NOT_FROM_ALARM"
		* 
		* @throws TA_Base_Core::OperationModeException
		* @throws DependencyFailure
		* @throws InvalidArg exception for programatic error
		*/
		virtual TA_Base_Bus::JobRequestDetailCorbaDef* getDataFromSymbol(const char* entityName, 
			const char* sessionID);

		/**
		*
		* Submit a Job Request. Note that the data in the jobRequest param is used to determine
		* if the JRM was opened via an alarm or schematic
		*
		* This method does not raise events - the JRM does that.
		* This method does report MMS Server health based on result of call to server 
		*
		* @throws TA_Base_Core::OperationModeException
		* @throws DependencyFailure
		* @throw InvalidArg exception for programatic error
		* @throw SubmissionDenied if the alarm has been normalized or closed, or
		* a Job Request has already been submitted on this alarm. Or operator does
		* not have sufficient rights
		*/
		virtual void submitJobRequest(const TA_Base_Bus::JobRequestDetailCorbaDef& jobRequest, 
			const char* sessionID);			

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
        MmsJobRequestSubmitter( const MmsJobRequestSubmitter& con);
        MmsJobRequestSubmitter& operator=(const MmsJobRequestSubmitter& con);

		 /**
          * checkOperationMode
          *
		  *
          * @throw OperationModeException if not in Control
          */
		void checkOperationMode();

		 /**
          * getOperatorDesc
          *
		  *
          * @return the description (full name) of the primary operator
		  * logged on under the specifed sessionID - up to an including 20 characters.
		  * Any characters after the first 20 are truncated
		  *
		  * @throws DependencyFailure if the data can't be retrieved
          */
		std::string getOperatorDesc(const std::string& sessionID);

		 /**
          * alarmStateIsOk
          *
		  *
          * @return true if the alarm is in a state that allows a Job Request
		  * to be submitted from it. If not populate the reason param with 
		  * the reason why not. 
		  *
		  * All that is required is that the alarm exists, is not normalized. and a 
		  * Job Request is not already raised on the alarm
		  * 
		  *
		  * @throws DependencyFailure if the data can't be retrieved
          */
		bool alarmStateIsOk(const TA_Base_Bus::JobRequestDetailCorbaDef& jobRequest, 
			std::string& reason);
    };
} // namespace TA_IRS_App

#endif // !defined(MMS_JOB_REQUEST_SUBMITTER_H)