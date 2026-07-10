/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/test/module/src/MmsConnectionModuleTest.cpp $
  * @author:  C. DeWolfe
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * implimentation of MmsConnectionModuleTest class
  *
  */

#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4146)
#pragma warning(disable:4018)
#endif // WIN32

#include "MmsConnectionModuleTest.h"

#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/RunParams.h"
#include "core/exceptions/src/AssertException.h"
#include "core/message/types/MessageTypes.h"

#include <time.h>
#include <string>

using TA_Base_Core::RunParams;



/////////////////////////////////////////////////////////
// Test Framework Methods
/////////////////////////////////////////////////////////

void MmsConnectionModuleTest::setUpSuite( const std::string & inFile )
{

}


void MmsConnectionModuleTest::tearDownSuite()
{
	//TA_Base_Core::CorbaUtil::getInstance().shutdown();
	//TA_Base_Core::CorbaUtil::getInstance().cleanup();
}


void MmsConnectionModuleTest::setUp( const std::string & INFile , const std::string & OUTFile )
{

	TA_Base_Core::CorbaUtil::getInstance().initialise();
	TA_Base_Core::CorbaUtil::getInstance().activate();

	// Get the object
	m_mmsConnection.setEntityName("MmsConnection_occ");
	Assert( !m_mmsConnection->_non_existent() );

	// Set up session ID 
	m_sessionID = TA_Base_Core::RunParams::getInstance().get(RPARAM_SESSIONID);
}


void MmsConnectionModuleTest::tearDown( const std::string & INFile )
{

}

TestSuite* MmsConnectionModuleTest::suite ()
{
    TestSuite * testSuite = new TestSuite( "MmsConnectionModuleTest" );  

  //  Test MmsMessageReader utility methods:
    testSuite->addTest( new TestCaller< MmsConnectionModuleTest >(
                       "submitAlarmMessageTest",
                       &MmsConnectionModuleTest::submitAlarmMessageTest, this) );

    testSuite->addTest( new TestCaller< MmsConnectionModuleTest >(
                       "submitJobRequestMessageTest",
                       &MmsConnectionModuleTest::submitJobRequestMessageTest, this) );

    testSuite->addTest( new TestCaller< MmsConnectionModuleTest >(
                       "submitCounterMessageTest",
                       &MmsConnectionModuleTest::submitCounterMessageTest, this) );
	
  // ctor method tests:
    testSuite->addTest( new TestCaller< MmsConnectionModuleTest >(
                       "submitMeasurementMessageTest",
                       &MmsConnectionModuleTest::submitMeasurementMessageTest, this) );

    testSuite->addTest( new TestCaller< MmsConnectionModuleTest >(
                       "pingTest",
                       &MmsConnectionModuleTest::pingTest, this) );

  //test the various get and set methods
    testSuite->addTest( new TestCaller< MmsConnectionModuleTest >(
                       "clearMessagesTest",
                       &MmsConnectionModuleTest::clearMessagesTest, this) );

	


	return testSuite;
}

/////////////////////////////////////////////////////////
//  Test methods
/////////////////////////////////////////////////////////

void MmsConnectionModuleTest::initAlarmStruct(TA_Base_Core::AlarmDetailCorbaDef& details)
{
	time_t now;
	time(&now);
    details.sourceTime.time = now;

    details.transactiveTime.time = now;

    details.ackTime = 0;

    details.closeTime = 0;

    details.messageTypeKey = CORBA::string_dup("DpTrue");

    details.messageContext = CORBA::string_dup("messageContext");

    details.assocEntityKey = 1234;

    details.assocEntityTypeKey = 18;

    details.assetName = CORBA::string_dup("assetName");

	std::string id( TA_Base_Core::TA_Uuid::generateUUID() );
    details.alarmID = CORBA::string_dup(id.c_str());

    details.alarmSeverity = 1;

    details.alarmDescription = CORBA::string_dup("alarmDescription");

    details.alarmParameterList = CORBA::string_dup("");

    details.alarmAcknowledgedBy = CORBA::string_dup("");

    //details.alarmOpen = CORBA::Boolean(true);

    details.alarmComments = CORBA::string_dup("alarmComments");

    details.entityName = CORBA::string_dup("entityName");

    details.locationKey = 1;

    details.subsystemKey = 0;

    details.parentAlarmID = CORBA::string_dup("0");

    details.isHeadOfAvalanche = CORBA::Boolean(false);

    details.decisionSupportState = TA_Base_Core::NoRelatedPlan;

    details.mmsState = TA_Base_Core::MMS_NONE;

    details.messageSequenceNumber = 0;
}

void MmsConnectionModuleTest::submitAlarmMessageTest()
{
	//Verify that the following method provides the functionality
	//specified in its contract - see API ref:
		/**
		 * Submit a MMS Alarm Message to the MMS Server.
		 *
		 * @param alarmSeverity: text desc of alarm's severity value
		 * @param p_alarm: the alarm for which the message is being submitted
		 *
		 * @throw TA_Base_Bus::MmsConnectionFailure if submission fails for any reason
		 * other than OperationModeException
		 */
		//virtual void submitAlarmMessage(const char* alarmSeverity, const TA_Base_Core::AlarmDetailCorbaDef& p_alarm);

	//typical values
	try
	{
		const char* alarmSeverity = "1- Urgent.";

		//create a fake "alarm"
		TA_Base_Core::AlarmDetailCorbaDef p_alarm;

		initAlarmStruct(p_alarm);
		
		p_alarm.alarmDescription = CORBA::string_dup("This is the TransActive Alarm Description.");
		
		p_alarm.assetName = CORBA::string_dup("This is the asset ID");

		p_alarm.alarmParameterList = CORBA::string_dup("DataPointValue:FAULT,");

		m_mmsConnection->submitAlarmMessage( alarmSeverity, p_alarm);
	}
	catch(...)
	{
		Assert(false);
	}

	//Test where all strings need to be truncated
	try
	{
		const char* alarmSeverity = "AS345678901234567890123456789012345678901234567890";

		//create a fake "alarm"
		TA_Base_Core::AlarmDetailCorbaDef p_alarm;
		initAlarmStruct(p_alarm);
		
		p_alarm.alarmDescription = CORBA::string_dup("TAAD5678901234567890123456789012345678901234567890");
		
		p_alarm.assetName = CORBA::string_dup("AI3456789012345678901234567890123456LASTXXXXX");

		p_alarm.alarmParameterList = CORBA::string_dup("DataPointValue:DPV4567890123456789012345678901234567890123456LASTXXXXXX,");

		m_mmsConnection->submitAlarmMessage( alarmSeverity, p_alarm);
	}
	catch(...)
	{
		Assert(false);
	}
}

void MmsConnectionModuleTest::submitJobRequestMessageTest()
{
	//Verify that the following method provides the functionality
	//specified in its contract - see API ref:
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
	//virtual void submitJobRequestMessage(const TA_Base_Bus::JobRequestDetailCorbaDef& details);
	
	//Test with typical values
	try
	{
		const char* assetId = "This is the asset ID";

		TA_Base_Bus::JobRequestDetailCorbaDef details;

		details.assetId = CORBA::string_dup(assetId);
		details.requestor = CORBA::string_dup("Joe Bloggs");
		details.targetDateEnabled = true;

		time_t now;
		time(&now);
		details.targetDate = now;
		details.alarmDescField = CORBA::string_dup("This is the alarm description");
		details.problemReport = CORBA::string_dup("This is the problem report");


		m_mmsConnection->submitJobRequestMessage( details);
	}
	catch(...)
	{
		Assert(false);
	}

	//Test where all strings need to be truncated
	try
	{
		const char* assetId = "AI3456789012345678901234567890123456LASTXXXXX";

		TA_Base_Bus::JobRequestDetailCorbaDef details;

		details.assetId = CORBA::string_dup(assetId);
		details.requestor = CORBA::string_dup("REQ4567890123456LASTXXXXX");
		details.targetDateEnabled = true;

		time_t now;
		time(&now);
		details.targetDate = now;
		details.alarmDescField = CORBA::string_dup("AD34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678LASTXXXXXXXXXXXXX");
		details.problemReport = CORBA::string_dup("PR34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890LASTXXXXXXXXXXXX");


		m_mmsConnection->submitJobRequestMessage( details);
	}
	catch(...)
	{
		Assert(false);
	}

	//Test with disabled target date
	try
	{
		const char* assetId = "AI3456789012345678901234567890123456LASTXXXXX";

		TA_Base_Bus::JobRequestDetailCorbaDef details;

		details.assetId = CORBA::string_dup(assetId);
		details.requestor = CORBA::string_dup("REQ4567890123456LASTXXXXX");
		details.targetDateEnabled = false;

		time_t now;
		time(&now);
		details.targetDate = now;
		details.alarmDescField = CORBA::string_dup("AD34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678LASTXXXXXXXXXXXXX");
		details.problemReport = CORBA::string_dup("PR34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890LASTXXXXXXXXXXXX");


		m_mmsConnection->submitJobRequestMessage( details);
	}
	catch(...)
	{
		Assert(false);
	}
}

void MmsConnectionModuleTest::submitCounterMessageTest()
{
	//Verify that the following method provides the functionality
	//specified in its contract - see API ref:
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
		//virtual void submitCounterMessage(const char* assetId, const char* dataPointName, 
		//	const char* dataPointDesc, CORBA::Long dataPointValue);
	try
	{
		const char* assetId = "asset Id";
		const char* dataPointName = "data Point Name";
		const char* dataPointDesc = "data Point Desc";
		CORBA::Long dataPointValue = 3456;


		m_mmsConnection->submitCounterMessage(assetId, dataPointName, dataPointDesc, dataPointValue);
	}
	catch(...)
	{
		Assert(false);
	}

	//with strings that need truncating:
	try
	{
		const char* assetId = "AI3456789012345678901234567890123456LASTXXXXX";
		const char* dataPointName = "DPN456789012345LASTXXXXXXXXX";
		const char* dataPointDesc = "DPD456789012345678901234567890123456LASTXXXXX";
		CORBA::Long dataPointValue = 3;


		m_mmsConnection->submitCounterMessage(assetId, dataPointName, dataPointDesc, dataPointValue);
	}
	catch(...)
	{
		Assert(false);
	}
}

void MmsConnectionModuleTest::submitMeasurementMessageTest()
{
	//Verify that the following method provides the functionality
	//specified in its contract - see API ref:
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
		//virtual void submitMeasurementMessage(const char* assetId, const char* dataPointName, const char* dataPointDesc, CORBA::Long dataPointValue);
	try
	{
		const char* assetId = "This is the asset ID";
		const char* dataPointName = "This is the datapoint Name.";
		const char* dataPointDesc = "This is the datapoint Description.";
		CORBA::Long dataPointValue = 3456;


		m_mmsConnection->submitMeasurementMessage(assetId, dataPointName, dataPointDesc, dataPointValue);
	}
	catch(...)
	{
		Assert(false);
	}
	//with strings that need truncating:
	try
	{
		const char* assetId = "AI3456789012345678901234567890123456LASTXXXXX";
		const char* dataPointName = "DPN456789012345LASTXXXXXXXXX";
		const char* dataPointDesc = "DPD456789012345678901234567890123456LASTXXXXX";
		CORBA::Long dataPointValue = 3;


		m_mmsConnection->submitMeasurementMessage(assetId, dataPointName, dataPointDesc, dataPointValue);
	}
	catch(...)
	{
		Assert(false);
	}
}

//todo NOTE THAT THIS TEST WILL NOT WORK WITH THE STUB FROM STE - it works with 
//my stub
void MmsConnectionModuleTest::pingTest()
{
	//Verify that the following method provides the functionality
	//specified in its contract - see API ref:
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
		//virtual void ping();
	bool itThrew = false;
	try
	{
		//my stub is designed so that ping fails the first call and succed
		m_mmsConnection->ping();
	}
	catch(...)
	{
		itThrew = true;
	}
	//todo my stub is only set up to work correctly with this the first time the
	//test is run. This is a pain in the ass and should be fixed.
	//Assert(itThrew);
	try
	{
		//my stub is designed so that ping fails the first call and succeds following calls
		m_mmsConnection->ping();
	}
	catch(...)
	{
		Assert(false);
	}

}

void MmsConnectionModuleTest::clearMessagesTest()
{
	//Verify that the following method provides the functionality
	//specified in its contract - see API ref:
        /**
          * clearMessages
		  *
		  * if there are any messages in the queue waiting for you pop them and throw 
		  * them in the bin. If there are no messages waiting do nothing
		  *
		  * @throw TA_Base_Bus::MmsConnectionFailure if transaction fails for any reason
		  * other than OperationModeException
          */
		//virtual void clearMessages();

	//todo MY STUB ONLY is designed to respond with 4 messages before
	//the stub says there are no more left
	try
	{
		//my stub is designed so that ping fails the first call and succed
		m_mmsConnection->clearMessages();
	}
	catch(...)
	{
		Assert(false);
	}

}
