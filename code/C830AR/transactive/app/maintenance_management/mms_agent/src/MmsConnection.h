/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  * 
  * Source:    $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/MmsConnection.h $
  * @author:   Chris DeWolfe
  * @version:  $Revision: #1 $
  * @see:      9999-Q01-02-0016-GenericAgent_Usage_Guidelines.doc
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last mofified by:  $Author: builder $
  *
  * This class provides the following:
  * - methods required to send messages to the MMS Server.
  * - responds to test messages from the MMS server - it checks for them at the configured rate. 
  * - pings the MMS server at the configured rate.
  * - impliments the IMmsGlobalMmsInhibitCorbaDef interface so that people can remotly get and set
  * - the Global MMS inhibition state
  *
  * You should NOT employ the IMmsConnectionCorbaDef methods in this class unless you do not 
  * require any of the business logic associated with message submission. 
  * This class does NOT provide the business logic required with message submission:
  * as example, this classes submitAlarmMessage() method does nothing 
  * more than send a message to the MMS Server. It does not raise an alarm on failure or notify the 
  * alarm agent when the submission succedes.
  *
  * 
  */
#ifndef MMS_CONNECTION_H
#define MMS_CONNECTION_H

#if defined(_MSC_VER)
#pragma warning(disable:4786)
#endif // defined (_MSC_VER)

#include "bus/generic_agent/src/IEntity.h"
#include "bus/maintenance_management/mmsCorbaDef/idl/src/IMmsConnectionCorbaDef.h"
//#include "bus/maintenance_management/mmsCorbaDef/idl/src/IMmsGlobalInhibitCorbaDef.h"
#include "core/corba/src/ServantBase.h"

#include "app/maintenance_management/mms_agent/src/MmsApiWrapper.h"
#include "app/maintenance_management/mms_agent/src/MmsConnectionTester.h"
#include "core/synchronisation/src/ReEntrantThreadLockable.h"
#include "core/data_access_interface/entity_access/src/MmsConnectiontEntityData.h"

#include <time.h>

namespace TA_Base_Bus
{
    class RightsLibrary;
}

namespace TA_IRS_App
{
    class MmsConnection
		: public virtual TA_Base_Bus::IEntity,
		  public virtual POA_TA_Base_Bus::IMmsConnectionCorbaDef,
		  public virtual TA_Base_Core::ServantBase
    {
    public:

		friend class MmsConnectionTester;

        /**
		  * getInstance
		  *
          * This class is a singleton.
          */
		static MmsConnection* getInstance();

        /**
          * DTOR
		  * Calls setMmsConnection(0) on all the registered users
		  * Diconnects from the MMS Server.
		  * Deletes the instance of this class
		  * 
          */
        virtual ~MmsConnection();

        /**
          * initConfiguration
		  *
		  * Precondition: pData != 0
		  * Will apply the data in the config to the object
		  * The ping rate and incomming message check rate are updatable
		  *
		 */
		void initConfiguration(TA_Base_Core::MmsConnectiontEntityDataPtr pData);

		 /**
          * getDescAndReportFields
          *
		  *
          * @retrieves the strings that should be used to init the alarm description
		  * and problem report fields in the JRM, given the specified alarm
		  *
          */
		void getDescAndReportFields(const TA_Base_Core::AlarmDetailCorbaDef& alarm,
			std::string& alarmDescField, std::string& problemReportField);

        /** 
         * isGlobalMmsInhibitPermitted
         *
         * Checks if the operator has rights to set Global MMS inhibit.
         *
         * @param sessionID  The operator's session ID.
         *
         * @return True if Global MMS inhibit is permitted, and false otherwise.
         *
         * @exception AccessDeniedException  Thrown if an error occurs while determining rights.
         */
        bool isGlobalMmsInhibitPermitted(const std::string& sessionID);

        /** 
         * isRaiseJobRequestPermitted
         *
         * Checks if the operator has rights to raise job requests for a particular datanode.
         *
         * @param sessionID   The operator's session ID.
         * @param dataNodeKey The key of the datanode that the job request will be raised on.
         *
         * @return True if raising a job request is permitted, and false otherwise.
         *
         * @exception AccessDeniedException  Thrown if an error occurs while determining rights.
         */
        bool isRaiseJobRequestPermitted(const std::string& sessionID, unsigned long dataNodeKey);


  //IMmsGlobalMmsInhibitCorbaDef methods:

		/**
		*
		* get the global MMS Inhibit state
		*
		* @throws TA_Base_Core::OperationModeException
		*/	
		virtual CORBA::Boolean getGlobalMmsInhibit();

		/**
		*
		* get the pkey of the datapoint representing the global MMS Inhibit state
		*
		* @throws TA_Base_Core::OperationModeException
		* @throws TA_Base_Core::DatabaseErrorException
		*/	
		virtual CORBA::ULong getGlobalInhibitKey();

		/**
		*
		* set the global MMS Inhibit state
		*
		* @throws TA_Base_Core::OperationModeException
		* @throws TA_Base_Core::AccessDeniedException
		* @throws TA_Base_Core::DatabaseErrorException
		*/	
		virtual void setGlobalMmsInhibit(CORBA::Boolean applied, const char* sessionID);

  //IMmsConnectionCorbaDef methods:

		/**
		 * Submit a MMS Alarm Message to the MMS Server.
		 *
		 * @param alarmSeverity: text desc of alarm's severity value
		 * @param p_alarm: the alarm for which the message is being submitted
		 *
		 * @throw TA_Base_Bus::MmsConnectionFailure if submission fails for any reason
		 * other than OperationModeException
		 */
		virtual void submitAlarmMessage(const char* alarmSeverity, const TA_Base_Core::AlarmDetailCorbaDef& p_alarm);

		/**
		 * Submit a Job Request Message to the MMS Server.
		 *
		 * @param details are used to construct the message
		 *
		 * The alarmDescField of the details param should be populated as specifed
		 * in submitAlarmMessage() if applicable. If not applicable, it must be blank
		 *
		 * @throw TA_Base_Bus::MmsConnectionFailure if submission fails for any reason
		 * other than OperationModeException
		 */
		virtual void submitJobRequestMessage(const TA_Base_Bus::JobRequestDetailCorbaDef& details);

		/**
		 * Submit a Counter Message to the MMS Server.
		 *
		 * @param assetId: The "Asset Name" from the datanode config
		 * @param dataPointName: characters following last '.' in entity name of dp
		 * @param dataPointDesc: desc field of datapoint entity config
		 * @param dataPointValue: the value of the datapoint
		 *
		 * @throw TA_Base_Bus::MmsConnectionFailure if submission fails for any reason
		 * other than OperationModeException
		 */
		virtual void submitCounterMessage(const char* assetId, const char* dataPointName, const char* dataPointDesc, CORBA::Double dataPointValue);

		/**
		 * Submit a Measurement Message to the MMS Server.
		 *
		 * @param assetId: The "Asset Name" from the datanode config
		 * @param dataPointName: characters following last '.' in entity name of dp
		 * @param dataPointDesc: desc field of datapoint entity config
		 * @param dataPointValue: the value of the datapoint
		 *
		 * @throw TA_Base_Bus::MmsConnectionFailure if submission fails for any reason
		 * other than OperationModeException
		 */
		virtual void submitMeasurementMessage(const char* assetId, const char* dataPointName, const char* dataPointDesc, CORBA::Double dataPointValue);

        /**
          * ping
		  *
		  * Test if the MMS Server is contactable. 
		  * The method will wait for the configured timeout period 
		  * for a responce before assuming failure.
		  *
		  * @throw TA_Base_Bus::MmsConnectionFailure if transaction fails for any reason
		  * other than OperationModeException
          */
		virtual void ping();

        /**
          * clearMessages
		  *
		  * if there are any messages in the queue waiting for you pop them and throw 
		  * them in the bin. If there are no messages waiting do nothing
		  *
		  * @throw TA_Base_Bus::MmsConnectionFailure if transaction fails for any reason
		  * other than OperationModeException
          */
		virtual void clearMessages();


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
		  * Will update tester and wrapper as required
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
          * clear alarms that connection may have raised
		  * start the MmsConnectionTester thread
		  * 
          */
        virtual void setToControlMode();

        /**
          * setToMonitorMode
          *
          * stop the MmsConnectionTester thread
          */
        virtual void setToMonitorMode();

	//end IEntity methods

		std::string getAlarmDescription( const TA_Base_Core::AlarmDetailCorbaDef& p_alarm );

	private: //data members:

		static MmsConnection* m_instance;

		TA_Base_Core::MmsConnectiontEntityDataPtr m_data;
		unsigned long m_entityKey;
		std::string m_entityName;
        TA_Base_Bus::RightsLibrary* m_rightsLibrary;

		MmsConnectionTester m_thread;

		//provides a thread lock
		static TA_Base_Core::ReEntrantThreadLockable m_lock;


	private: //methods:

        /**
          * The only implimnented CTOR
          */
        MmsConnection();

        // Disable copy constructor and assignment operator
        MmsConnection( const MmsConnection& con);
        MmsConnection& operator=(const MmsConnection& con);

		 /**
          * checkOperationMode
          *
		  *
          * @throw OperationModeException if not in Control
          */
		void checkOperationMode();

	//helper methods:

		void submitPreventiveMessage(MmsApiWrapper::messageType type, const char* assetId, const char* dataPointName, 
			const char* dataPointDesc, CORBA::Double dataPointValue);

		/**
		* toUint
		*
		* PRECONDITION: param str contains 
		*anything other than digits or is empty
		*
		* Returns a uint representation of the string passed.
		*/
		unsigned int toUint(const std::string& str);

		/**
		* timeToString
		*
		* Returns a string representation of the param value passed
		* The string returned will have the form DD/MM/YYYY/HH:MM:SS
		*
		* @param t  The time is represented as seconds elapsed since 
		*midnight (00:00:00), January 1, 1970
		*
		*/
		std::string timeToString(const time_t t);

		/**
		* createMmsAlarmDesc
		*
		* @return the text that should be used to populate the Alarm Description field
		* in MMS Alarm and Job Request Messages
		* This field has the form:
		*<Alarm Severity>, <description field of TransActive alarm>, <Data Point Value>
		* If param truncate is true then result is truncated to 152 characters if required.
		* Note that this method does not pad to 152 characters.
		* 
		*/
		std::string createMmsAlarmDesc( 
			const time_t alarmOccurence, // ECP257
			const std::string& alarmSeverity,
			const std::string& alarmDesc,
			const std::string& dataPointValue,
			bool truncate = true);


		/**
		* createMmsAlarmMessageBody
		*
		* @return the text that should be used to populate the body of a MMS
		* Alarm Message
		*/
		std::string createMmsAlarmMessageBody( 
			const time_t alarmOccurence, // ECP257
			const std::string& alarmSeverity,
			const std::string& alarmDesc,
			const std::string& dataPointValue,
			const std::string& assetName);

        /**
         * toDataPointValue
         *
         * @param AlarmDetailCorbaDef& - The alarm to retrieve information for
         *
         * @return string - This returns the text to display in the column for the specified alarm.
		 *
		 * Note that no truncation is performed. Also note that this is not simple a text
		 * representation of a numberic value, i.e. it may contain letters as well as numbers
         */
		std::string toDataPointValue(const TA_Base_Core::AlarmDetailCorbaDef& p_alarm);

        /**
         * doubleToDataPointValueString
         *
         * @param dataPointValue the value of the datapoint to convert
		 * @param iPrecision - the value of maximum precision (numbers of decimal)
         *
         * @return a string representation of the datapoint.
		 * it must have the form 9(10) with leading zeros padding as required
		 * note that number may be negative, e.g. -012345678
		 * number is rounded to nearest integer representation
		 *
		 * numbers out of range will be reported as max closest in range
         */
		std::string doubleToDataPointValueString(double dataPointValue, int iPrecision);

		
    };
} // namespace TA_IRS_App

#endif // !defined(MMS_CONNECTION_H)