/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  * 
  * Source:    $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/MmsConnection.cpp $
  * @author:   Chris DeWolfe
  * @version:  $Revision: #1 $
  * @see:      9999-Q01-02-0016-GenericAgent_Usage_Guidelines.doc
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last mofified by:  $Author: builder $
  *
  * MmsConnection class implimentation. See header file for class descrption
  */

#if defined(_MSC_VER)
#pragma warning(disable:4786 4503)
#endif // defined (_MSC_VER)


#include "app/maintenance_management/mms_agent/src/MmsConnection.h"
#include "core/data_access_interface/entity_access/src/MmsConnectiontEntityData.h"
#include "app/maintenance_management/mms_agent/src/MmsConnectionException.h"
#include "core/data_access_interface/entity_access/src/MmsAgentEntityData.h"

#include "app/maintenance_management/mms_agent/src/MmsMessageUtil.h"
#include "app/maintenance_management/mms_agent/src/MmsServerState.h"

#include "core/data_access_interface/entity_access/src/EntityAccessFactory.h"
#include "bus/generic_agent/src/GenericAgent.h"
#include "core/utilities/src/TAAssert.h"
#include "core/message/src/MessageConfig.h"
#include "core/message/src/NameValuePair.h"

#include "core/data_access_interface/src/ResourceAccessFactory.h"
#include "core/data_access_interface/src/IResource.h"
#include "bus/security/rights_library/src/RightsLibrary.h"
#include "bus/security/access_control/actions/src/AccessControlledActions.h"
#include "bus/security/authentication_library/src/AuthenticationLibrary.h"

#include <sstream>
#include <string>
#include <iomanip> 
#include <algorithm>
//Mintao++
//TD8524
#include "core/data_access_interface/src/AlarmTypeAccessFactory.h"
#include "core/data_access_interface/src/IAlarmTypeData.h"
#include "core/data_access_interface/src/AlarmTypeData.h"

#include "core/alarm/src/AlarmConfig.h"

//Mintao++
//TD8524


namespace TA_IRS_App
{
	//init static instance member
	MmsConnection* MmsConnection::m_instance = 0;
	TA_Base_Core::ReEntrantThreadLockable MmsConnection::m_lock;

	const int MEASURE_DP_PRECISION = 3;
	const int COUNTER_DP_PRECISION = 2;




    MmsConnection* MmsConnection::getInstance()
	{

		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("getInstance");		

		if(0 == m_instance)
		{
			m_instance = new MmsConnection();
		}


		FUNCTION_EXIT;

		return m_instance;
	}

	MmsConnection::MmsConnection()
		: m_entityKey(0)
	{
		FUNCTION_ENTRY("MmsConnection");		

        TA_Base_Bus::RightsLibraryFactory rlFactory;
    	m_rightsLibrary = rlFactory.buildRightsLibrary();

		FUNCTION_EXIT;
	}


	MmsConnection::~MmsConnection()
	{
		FUNCTION_ENTRY("~MmsConnection");

		//stop the thread if required
		if(TA_Base_Core::Thread::THREAD_STATE_UNKNOWN != m_thread.getCurrentState() &&
		   TA_Base_Core::Thread::THREAD_STATE_TERMINATED != m_thread.getCurrentState() )	
		{
			m_thread.terminateAndWait();
		}
		
		m_instance = 0;

        delete m_rightsLibrary;
        m_rightsLibrary = NULL;

		FUNCTION_EXIT;
	}


	void MmsConnection::initConfiguration(TA_Base_Core::MmsConnectiontEntityDataPtr pData)
	{
		FUNCTION_ENTRY("initConfiguration");

		
		m_data = pData;
		m_data->invalidate();
		m_entityName = m_data->getName();
		m_entityKey = m_data->getKey();

		

		//update the MmsApiWrapper instance
		MmsApiWrapper::getInstance()->setTimeout( m_data->getTimeoutTenthSecs() );

		//update the connection tester
		m_thread.initConfiguration(m_data);

		//I don't set this here because the associated datapoint might not have been created
		//yet. This is initialized via MmsServerState::initMmsServerState();

		//MmsServerState::getInstance()->setGlobalInhibitDataPoint(pData->getGlobalMmsInhibit() );

		//API REF
		//void MmsServerState::setData(TA_Base_Core::MmsConnectiontEntityData* pData); 
		MmsServerState::getInstance()->setData(m_data); 

		FUNCTION_EXIT;
	}

	void MmsConnection::getDescAndReportFields(const TA_Base_Core::AlarmDetailCorbaDef& alarm,
		std::string& alarmDescField, std::string& problemReportField)
	{
		FUNCTION_ENTRY("getDescAndReportFields");

		std::string alarmSeverity = MmsMessageUtil::getInstance()->getAlarmSeverityName(alarm);
		if(alarmSeverity.empty())
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "failed to retrieve severity name");
		}

		std::string dataPointValue = toDataPointValue(alarm);
		if(dataPointValue.empty())
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "failed to data point value description");
		}

		// ECP257 - added alarm source time when creating for the MmsAlarm description
		std::string alarmDesc = getAlarmDescription( alarm );
		std::string untruncatedDesc = createMmsAlarmDesc(alarm.sourceTime.time , alarmSeverity, alarmDesc, dataPointValue,
			false);

		//if less than 152 chars just set references
		if(untruncatedDesc.length() <= 152)
		{
			alarmDescField = untruncatedDesc;
			problemReportField = "";
		}
		else //split the result up
		{
			alarmDescField = untruncatedDesc.substr(0, 152);
			//256 - 152 - 2 = 102 max chars
			problemReportField = untruncatedDesc.substr(152, 102);

		}
		FUNCTION_EXIT;
	}


    bool MmsConnection::isGlobalMmsInhibitPermitted(const std::string& sessionID)
    {
        FUNCTION_ENTRY("isGlobalMmsInhibitPermitted");

		try
		{
			unsigned long agentEntityKey = MmsMessageUtil::getInstance()->getAgentData()->getKey();
			TA_Base_Core::IResource* r = 
				TA_Base_Core::ResourceAccessFactory::getInstance().getResourceFromEntity( agentEntityKey );
			unsigned long resourceID = r->getKey();
			delete r;

			unsigned long actionKey = TA_Base_Bus::aca_MMS_GLOBAL_INHIBIT;
			TA_Base_Bus::EDecisionModule decisionModule = TA_Base_Bus::UNDEFINED_DM;
  		    std::string reason;
			
			bool response = m_rightsLibrary->isActionPermittedOnResource( sessionID, resourceID, 
				actionKey, reason, decisionModule );

   			FUNCTION_EXIT;
			return response;
		}
		catch(...)
		{
			throw TA_Base_Core::AccessDeniedException("Rights could not be determined for MMS global inhibit.");
		}
    }


    bool MmsConnection::isRaiseJobRequestPermitted(const std::string& sessionID, unsigned long dataNodeKey)
    {
        FUNCTION_ENTRY("isRaiseJobRequestPermitted");

		try
		{
			TA_Base_Core::IResource* r = 
                TA_Base_Core::ResourceAccessFactory::getInstance().getResourceFromEntity( dataNodeKey );
			unsigned long resourceID = r->getKey();
			delete r;

			unsigned long actionKey = TA_Base_Bus::aca_DATA_NODE_RAISE_JOB_REQUEST;
			TA_Base_Bus::EDecisionModule decisionModule = TA_Base_Bus::UNDEFINED_DM;
			std::string reason;

			bool response = m_rightsLibrary->isActionPermittedOnResource( sessionID, resourceID, 
				actionKey, reason, decisionModule );

			FUNCTION_EXIT;
			return response;
		}
		catch(...)
		{
			throw TA_Base_Core::AccessDeniedException("Rights could not be determined for raising job requests.");
		}
    }


  //IMmsGlobalMmsInhibitCorbaDef methods:

	CORBA::Boolean MmsConnection::getGlobalMmsInhibit()
	{

		FUNCTION_ENTRY("getGlobalMmsInhibit");

		checkOperationMode();

		bool state = MmsServerState::getInstance()->getGlobalInhibit();

		//for logging:
		std::string tOrf = "true";
		if(!state) tOrf = "false";
        //Mintao++
        //do not think we need to log at the info level, change to log at the debug level only
        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug, 
			"getGlobalMmsInhibit() returning %s", tOrf.c_str());

		FUNCTION_EXIT;

		return CORBA::Boolean(state);
		

	}

	void MmsConnection::setGlobalMmsInhibit(CORBA::Boolean applied, const char* sessionID)
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("setGlobalMmsInhibit");

		checkOperationMode();

		//for logging:
		std::string tOrf = "true";
		if(!applied) tOrf = "false";

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo, 
			"setGlobalMmsInhibit(%s, %s) called", tOrf.c_str(), sessionID);


		if(0 == sessionID)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "0 == sessionID");
			FUNCTION_EXIT;
			throw TA_Base_Core::AccessDeniedException("0 == sessionID");
			
		}

        //Mintao++
        //TD15080
        //remove the checking on the right to perform global inhibit status in MMS agent
        //This is because the checking is already done on the MMS Controller, which is the
        //the only way to modify the global inhibit status.
        //Mintao++
        //TD15080
		//see if the requested state is already set
		const bool currentState = MmsServerState::getInstance()->getGlobalInhibit();

		if(currentState == applied)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo, "requested state is already applied");
			FUNCTION_EXIT;
			return;
		}


		try
		{
			m_data->setGlobalMmsInhibit(applied);
		}
		catch(...)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Could not write the new state to the database");
			throw TA_Base_Core::DatabaseErrorException("Could not write the new state to the database");

			//note that the data point has not been written to yet
		}

		//this is a critical write so the call asserts if it fails
		MmsServerState::getInstance()->setGlobalInhibitDataPoint(applied);

		FUNCTION_EXIT;
	}

	
	CORBA::ULong MmsConnection::getGlobalInhibitKey()
	{

		FUNCTION_ENTRY("getGlobalInhibitKey");

		checkOperationMode();

		unsigned long key =  MmsServerState::getInstance()->getGlobalInhibitKey();

		if(0 == key)
		{
			throw TA_Base_Core::DatabaseErrorException("Could not retrieve the key for this datapoint");
		}

		FUNCTION_EXIT;
		return key;
	}


  //MmsConnectionCorbaDef methods:


	void MmsConnection::submitAlarmMessage(const char* alarmSeverity, const TA_Base_Core::AlarmDetailCorbaDef& p_alarm)
	{

		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("submitAlarmMessage");

		checkOperationMode();

		std::string dataPointValue = toDataPointValue(p_alarm);

		//ECP257 - appended the time the alarm occurred
		std::string alarmDesc = getAlarmDescription( p_alarm );
        std::string message = createMmsAlarmMessageBody(p_alarm.sourceTime.time, alarmSeverity, alarmDesc,
			dataPointValue, p_alarm.assetName.in() );

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"Next call is MmsApiWrapper::getInstance()->sendMessage(message, MmsApiWrapper::ALARM);" );

		try
		{
			MmsApiWrapper::getInstance()->sendMessage(message, MmsApiWrapper::ALARM);
		}
		catch(MmsConnectionException& ex)
		{
			throw TA_Base_Bus::MmsConnectionFailure(ex.getErrorMessage().c_str(), ex.getErrorCode());
		}

		FUNCTION_EXIT;
	}


	void MmsConnection::submitJobRequestMessage(const TA_Base_Bus::JobRequestDetailCorbaDef& details)
	{

		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("submitJobRequestMessage");

		checkOperationMode();

		std::ostringstream message;
		//empty spaces are to right so left justify
		message << std::left << std::setfill(' ');

		//Asset ID A(40)
		message << std::setw(40) << std::string(details.assetId.in(), 0, 40);


		//requestor A(20)
		message << std::setw(20) << std::string(details.requestor.in(), 0, 20);

		//the alarm message field is 152 chars max - it is used to compose the
		//problem report field - see below
		std::string combinedFields = std::string(details.alarmDescField.in(), 0, 152);
		
		//the problem report field in the message is the alarm message + problem report
		// that appears in the GUI - 256 char max
		if(! combinedFields.empty() ) combinedFields += ", ";
		combinedFields += details.problemReport.in();		
		message << std::setw(256) << std::string(combinedFields, 0, 256);

		std::string dateStr;
		if(details.targetDateEnabled)
		{
			dateStr = timeToString(details.targetDate);
		}

		//Target Completion Date(19)
		//note that we do not need to optionaly truncate this field because the 
		//timeToString method always does the right thing
		message << std::setw(19) << dateStr;

		std::string msg = message.str();
		int length = msg.length();
		TA_ASSERT(length == 335, "message.length() should be 335");

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"Next call is MmsApiWrapper::getInstance()->sendMessage(message, MmsApiWrapper::JOB_REQUEST);" );

		try
		{
			MmsApiWrapper::getInstance()->sendMessage(msg, MmsApiWrapper::JOB_REQUEST);
		}
		catch(MmsConnectionException& ex)
		{
			throw TA_Base_Bus::MmsConnectionFailure(ex.getErrorMessage().c_str(), ex.getErrorCode());
		}

		FUNCTION_EXIT;
	}

	std::string MmsConnection::doubleToDataPointValueString(double dataPointValue, int iPrecision)
	{
        /**
         * doubleToDataPointValueString
         *
         * @param dataPointValue the value of the datapoint to convert
         * @param iPrecision - the value of maximum precision (numbers of decimal)
		 *
         * @return a string representation of the datapoint.
		 * it must have the form 9(12) with leading zeros padding as required
		 * note that number may be negative, e.g. -01234567890
		 * number is rounded to nearest integer representation
		 *
		 * numbers out of range will be reported as max closest in range
         */

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo, "zhangnan enter doubleToDataPointValueString! ");
		//see if the number is too positive
		if(dataPointValue > 9999999999.0)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"dataPointValue too positive, truncated to 9999999999");

			return "009999999999";
		}
		//see if the number is too negative
		if(dataPointValue < -999999999.0)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"dataPointValue too negative, truncated to -999999999");
			return "-00999999999";
		}

		bool isNegative = false;
		if(dataPointValue < 0.0) 
		{
			isNegative = true;
			dataPointValue *= -1.0;
		}

		std::ostringstream number;
		number << dataPointValue;		
		std::string dbValueStr = number.str();		// create the original double string

		int precValue = 0;			// value that set the precision

		std::string::size_type dotPointPos = dbValueStr.find('.');		// find the position of '.'
		if ( dotPointPos != std::string::npos)
		{
			precValue = dbValueStr.length() - dotPointPos - 1;
			
			if ( precValue >= iPrecision)
			{
				precValue = iPrecision;		// the maximum precision is iPrecision
			}
		}
		
		number.str("");		//	reinitialize the string use the format that already defined
		number.precision( precValue );		// set the precision
		number << std::right << std::setw(12) << std::setfill('0') << std::fixed << dataPointValue;

		std::string msg;
		msg = number.str();

		if(isNegative)
		{
			msg.replace(0, 1, "-");
		}

		if (msg.length() > 12)	// TD18924++ - insure that the return value have 12 digits
		{
			msg = msg.substr(0, 12);
		}						// ++TD18924
		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo, "zhangnan exit doubleToDataPointValueString return msg = %s! ", msg.c_str());

		return msg;
	}

	void MmsConnection::submitPreventiveMessage(MmsApiWrapper::messageType type, const char* assetId, const char* dataPointName, 
		const char* dataPointDesc, CORBA::Double dataPointValue)
	{

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"zhangnan begin to call submitPreventiveMessage"  );


		std::string typeStr = "MmsApiWrapper::COUNTER";

		int dpPrecision = COUNTER_DP_PRECISION;
		if (MmsApiWrapper::MEASUREMENT == type) 
		{
			typeStr = "MmsApiWrapper::MEASUREMENT";
			dpPrecision = MEASURE_DP_PRECISION;
		}


		std::ostringstream message;
		//empty spaces are to right so left justify
		message << std::left << std::setfill(' ');

		//Asset ID A(40)
		message << std::setw(40) << std::string(assetId, 0, 40);

		//dataPointName(19)
		message << std::setw(19) << std::string(dataPointName, 0, 19);

		//dataPointDesc A(40)
		message << std::setw(40) << std::string(dataPointDesc, 0, 40);
		
		//dataPointValue 9(12)
		//the following number should be padded with left zero
		message << std::setw(12) << doubleToDataPointValueString(dataPointValue, dpPrecision);

		std::string msg = message.str();

		int length = msg.length();
		TA_ASSERT(length == 111, "message.length() should be 111");

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"zhangnan Next call is MmsApiWrapper::getInstance()->sendMessage(message, %s), length=%d", typeStr.c_str(), msg.length() );

		try
		{
			MmsApiWrapper::getInstance()->sendMessage(msg, type);
		}
		catch(MmsConnectionException& ex)
		{
			throw TA_Base_Bus::MmsConnectionFailure(ex.getErrorMessage().c_str(), ex.getErrorCode());
		}

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"zhangnan end to call submitPreventiveMessage"  );

	}

	void MmsConnection::submitCounterMessage(const char* assetId, const char* dataPointName, 
		const char* dataPointDesc, CORBA::Double dataPointValue)
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("submitCounterMessage");
	
		submitPreventiveMessage( MmsApiWrapper::COUNTER, assetId, dataPointName, 
			dataPointDesc, dataPointValue);

		FUNCTION_EXIT;
	}


	void MmsConnection::submitMeasurementMessage(const char* assetId, const char* dataPointName, 
		const char* dataPointDesc, CORBA::Double dataPointValue)
	{

		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("submitMeasurementMessage");

		submitPreventiveMessage( MmsApiWrapper::MEASUREMENT, assetId, dataPointName, 
			dataPointDesc, dataPointValue);


		FUNCTION_EXIT;
	}



	void MmsConnection::ping()
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("ping");

		checkOperationMode();

		try
		{
			MmsApiWrapper::getInstance()->ping();
		}
		catch(MmsConnectionException& ex)
		{
			throw TA_Base_Bus::MmsConnectionFailure(ex.getErrorMessage().c_str(), ex.getErrorCode());
		}

		FUNCTION_EXIT;
	}

	void MmsConnection::clearMessages()
	{

		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("clearMessages");

		checkOperationMode();

		try
		{
			MmsApiWrapper::getInstance()->clearMessages();
		}
		catch(MmsConnectionException& ex)
		{
			throw TA_Base_Bus::MmsConnectionFailure(ex.getErrorMessage().c_str(), ex.getErrorCode());
		}

		FUNCTION_EXIT;
	}

	//IEntity methods:

    bool MmsConnection::isValid()
	{
		FUNCTION_ENTRY("isValid");


		FUNCTION_EXIT;

		//Can't think why this would ever not be valid
		return true;
	}


    void MmsConnection::start()
	{
		FUNCTION_ENTRY("start");

		activateServantWithName(m_entityName);

		FUNCTION_EXIT;
	}


    void MmsConnection::stop()
	{
		FUNCTION_ENTRY("stop");
		
		deactivateAndDeleteServant();

		FUNCTION_EXIT;
	}


    void MmsConnection::update(const TA_Base_Core::ConfigUpdateDetails& updateEvent)
	{
		FUNCTION_ENTRY("update");
		
		//verify that this update is for me
		unsigned long keyToUpdate = updateEvent.getKey();
		if(keyToUpdate != m_entityKey)
		{
			FUNCTION_EXIT;
		}

		
		TA_Base_Core::MmsConnectiontEntityData* pData = 
			dynamic_cast<TA_Base_Core::MmsConnectiontEntityData*>(
			TA_Base_Core::EntityAccessFactory::getInstance().getEntity(updateEvent.getKey()));


		if (pData == 0)
		{
			//I gather, but am not sure that this should never happen
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"update() was called with ConfigUpdateDetails of unexpected subtype" );

			return;
			FUNCTION_EXIT;

		}

		pData->invalidate();

        std::vector<std::string> paramNames = updateEvent.getChangedParams();
		std::vector<std::string>::const_iterator itNames;
		for(itNames = paramNames.begin(); itNames != paramNames.end(); itNames++)
		{
			if(TA_Base_Core::MmsConnectiontEntityData::PING_PERIOD_SEC == *itNames)
			{
				m_thread.setPingPeriodSec( pData->getPingPeriodSeconds() );
			}
			if(TA_Base_Core::MmsConnectiontEntityData::MSG_CHECK_PERIOD_SEC == *itNames)
			{
				m_thread.setMsgCheckPeriodSec( pData->getMessageCheckSeconds() );
			}
			if(TA_Base_Core::MmsConnectiontEntityData::TIMEOUT_TENTH_SEC == *itNames)
			{
				MmsApiWrapper::getInstance()->setTimeout( pData->getTimeoutTenthSecs() );
			}
			if(TA_Base_Core::MmsConnectiontEntityData::GLOBAL_MMS_INHIBIT == *itNames)
			{
				MmsServerState::getInstance()->setGlobalInhibitDataPoint(pData->getGlobalMmsInhibit() );
			}
		}


		FUNCTION_EXIT;
	}


    void MmsConnection::remove()
	{
		FUNCTION_ENTRY("remove");

        //does nothing
		FUNCTION_EXIT;
	}


    void MmsConnection::setToControlMode()
	{
		FUNCTION_ENTRY("setToControlMode");

		//chenlei--
		//init MmsServerState at the beginning of the thread
		//move to the beginning of MmsConnectionTester::run() function
		//MmsServerState::getInstance()->initMmsServerState();

		if(TA_Base_Core::Thread::THREAD_STATE_RUNNING != m_thread.getCurrentState())
		{
			m_thread.start();
		}

		FUNCTION_EXIT;
	}


    void MmsConnection::setToMonitorMode()
	{
		FUNCTION_ENTRY("setToMonitorMode");

		if(TA_Base_Core::Thread::THREAD_STATE_UNKNOWN != m_thread.getCurrentState() &&
		   TA_Base_Core::Thread::THREAD_STATE_TERMINATED != m_thread.getCurrentState() )	
		{
			m_thread.terminateAndWait();
		}

		FUNCTION_EXIT;
	}

	void MmsConnection::checkOperationMode()
	{
		TA_Base_Bus::GenericAgent::ensureControlMode();
	}


	//end IEntity methods

	//utility methods:

    // Helper class used to find a matching name
	class HasName  
	{
		public:
			HasName(const std::string& name)
				:m_name(name)
			{
            }

			bool operator () (TA_Base_Core::NameValuePair* nameValue)
			{
				return (nameValue->name == m_name);
			}

		private:
			std::string m_name;
	};

	unsigned int MmsConnection::toUint(const std::string& str)
	{
		FUNCTION_ENTRY("toUint");

		//see if it is empty
		TA_ASSERT( !str.empty(), "string is empty");

		//see if it is all digits:
		std::string::size_type loc = str.find_first_not_of("0123456789");
		TA_ASSERT( std::string::npos == loc, "non-digit in string");

		std::istringstream strm(str);
		unsigned int value;
		strm >> value;
		return value;

		FUNCTION_EXIT;
	}



	std::string MmsConnection::timeToString(const time_t t)
	{
		FUNCTION_ENTRY("timeToString");

		tm timeStruct;


		/* Obtain coordinated universal time: */
		timeStruct = *(localtime( &t ));

		std::ostringstream strm;

		//we will justify right and pad zeros to the left
		strm << std::right;

		//DD/
		strm << std::setw(2) << std::setfill('0') << timeStruct.tm_mday << "/";

		//MM/
		strm << std::setw(2) << std::setfill('0') << (timeStruct.tm_mon + 1) << "/";

		//YY/
		strm << std::setw(4) << (timeStruct.tm_year + 1900) << "/";

		//HH:
		strm << std::setw(2) << std::setfill('0') << timeStruct.tm_hour << ":";

		//MM:
		strm << std::setw(2) << std::setfill('0') << timeStruct.tm_min << ":";

		//SS
		strm << std::setw(2) << std::setfill('0') << timeStruct.tm_sec;

		
		std::string str = strm.str();
		TA_ASSERT(19 == str.length(), "The date string should be 19 chars");

		FUNCTION_EXIT;

		return str;
	}

	std::string MmsConnection::createMmsAlarmDesc( 
		const time_t alarmOccurence,
		const std::string& alarmSeverity,
		const std::string& alarmDesc,
		const std::string& dataPointValue,
		bool truncate /*= true*/)
	{
		FUNCTION_ENTRY("createMmsAlarmDesc");

		// ECP 257 - append the time of the alarm occurence
		// std::string alarmMessage = alarmSeverity + ", " + alarmDesc  + ", " + dataPointValue;
		std::string strAlarmTime = timeToString(alarmOccurence);		
		std::string alarmMessage = strAlarmTime + ", " + alarmSeverity + ", " + alarmDesc  + ", " + dataPointValue;		

		//if less than 152 chars or truncating is disabled you do not need to truncate
		if(alarmMessage.length() <= 152 || truncate == false)
		{
			FUNCTION_EXIT;
			return alarmMessage;
		}
		//else truncate
		std::string truncated = alarmMessage.substr(0, 152);

		FUNCTION_EXIT;
		return truncated;
	}


	std::string MmsConnection::createMmsAlarmMessageBody( 
		const time_t alarmOccurence,
		const std::string& alarmSeverity,
		const std::string& alarmDesc,
		const std::string& dataPointValue,
		const std::string& assetName)
	{
		FUNCTION_ENTRY("createMmsAlarmMessageBody");
		
		std::ostringstream message;
		//empty spaces are to right so left justify
		message << std::left << std::setfill(' ');

		//Alarm Description A(152)
		//createMmsAlarmDesc() has already truncated if required
		// ECP257 - appended the time of alarm occurence
		message << std::setw(152) << createMmsAlarmDesc(alarmOccurence, alarmSeverity, alarmDesc, dataPointValue);

		//Asset ID A(40)
		message << std::setw(40) << std::string(assetName, 0, 40);

		std::string str = message.str();
		int length = str.length();
		TA_ASSERT(length == 192, "message.length() should be 192");

		FUNCTION_EXIT;		
		return str;
	}

    std::string MmsConnection::toDataPointValue(const TA_Base_Core::AlarmDetailCorbaDef& p_alarm)
    {
		FUNCTION_ENTRY("toDataPointValue");

		//Xinsong ++
		return p_alarm.alarmValue.in();

        //Mintao ++
        //TD8524 
		// The value will be: "AlarmType Value"
       std::string text;
       std::string alarmtypetext;

        // The value is stored in one of the description parameters
        std::string alarmParameters(p_alarm.alarmParameterList);
        
        TA_Base_Core::DescriptionParameters parameters;
        TA_Base_Core::gGetParametersFromString(alarmParameters, parameters);
        
        TA_Base_Core::DescriptionParameters::iterator valueParam = std::find_if(parameters.begin(),
                                                                           parameters.end(),
                                                                           HasName("DataPointValue"));


		// TD 12785 Only display the value without the alarm type if there is parameter value
		// if not, use the alarm type description
        if(valueParam != parameters.end())
        {
			bool isAck = false;

			// TD 12785 check whether it is acknowledged if yes, add "ACK" to the front
			// and add the param value
			std::string ackedBy = p_alarm.alarmAcknowledgedBy.in();
			if ( !(ackedBy.empty()) ) isAck = true;
			
           try
           {
    
            TA_Base_Core::IAlarmTypeData* typedata = TA_Base_Core::AlarmTypeAccessFactory::getInstance().getAlarmTypeByKey(p_alarm.messageTypeKey,false);
            alarmtypetext = typedata->getName();

            delete typedata;
            typedata = NULL;
           }
           catch (...)
           {
		    LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo, 
			    "failed to get the type key for alarm(%s) ", p_alarm.alarmID.in());

           }
            //Mintao++ TD8524 
            //Phase 1 alarm test does not require the ACK
            /*
			if (isAck)
			{
				text = "ACK ";
            }
            */
            //Mintao++ TD8524 
            //Phase 1 alarm test
			if (alarmtypetext.compare("HI") == 0 || alarmtypetext.compare("LO") == 0 || alarmtypetext.compare("VH") == 0||
                alarmtypetext.compare("VL") == 0 || alarmtypetext.compare("OH") == 0 || alarmtypetext.compare("OL") == 0)
			{
                //for analogue datapoint alarm, the 'HI', 'LO', 'VH', 'VL' 'OH' and 'OL' should be added before the 
                //measurement data
				text += alarmtypetext;
				text += (*valueParam)->value.c_str();
			}
			else
			{
				text += (*valueParam)->value.c_str();
			}
        }

        for (int i = 0;i < parameters.size();i++)
        {
            delete parameters[i];
            parameters[i] = NULL;
        }
        //Mintao ++
        //TD8524 

		FUNCTION_EXIT;
        return text;
    }

	std::string MmsConnection::getAlarmDescription( const TA_Base_Core::AlarmDetailCorbaDef& p_alarm )
	{
		// generate alarm description
		// Wenguang++ Alarm Message Size Reduction
		return TA_Base_Core::AlarmTypeTableAccessor::getInstance().getExpandedDescription( p_alarm.messageTypeKey,  p_alarm.alarmParameterList.in());
	}

} //namespace TA_IRS_App



