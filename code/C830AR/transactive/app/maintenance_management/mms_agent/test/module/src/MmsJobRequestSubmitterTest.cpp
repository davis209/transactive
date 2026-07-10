/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/test/module/src/MmsJobRequestSubmitterTest.cpp $
  * @author:  C. DeWolfe
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * implimentation of MmsJobRequestSubmitterTest class
  *
  */

#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4146)
#pragma warning(disable:4018)
#pragma warning(disable:4250)
#endif // WIN32

#include "MmsJobRequestSubmitterTest.h"

#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/RunParams.h"
#include "core/exceptions/src/AssertException.h"
#include "core/message/types/MessageTypes.h"

#include "Util.h" 


#include <time.h>
#include <string>

using TA_Base_Core::RunParams;



/////////////////////////////////////////////////////////
// Test Framework Methods
/////////////////////////////////////////////////////////

void MmsJobRequestSubmitterTest::setUpSuite( const std::string & inFile )
{

}


void MmsJobRequestSubmitterTest::tearDownSuite()
{
	//TA_Base_Core::CorbaUtil::getInstance().shutdown();
	//TA_Base_Core::CorbaUtil::getInstance().cleanup();
}


void MmsJobRequestSubmitterTest::setUp( const std::string & INFile , const std::string & OUTFile )
{

	TA_Base_Core::CorbaUtil::getInstance().initialise();
	TA_Base_Core::CorbaUtil::getInstance().activate();

	// Get the object
	m_submitter.setEntityName("MmsJobRequestSubmitter_occ");
	Assert( !m_submitter->_non_existent() );

	// Set up session ID 
	m_sessionID = TA_Base_Core::RunParams::getInstance().get(RPARAM_SESSIONID);
}


void MmsJobRequestSubmitterTest::tearDown( const std::string & INFile )
{

}

TestSuite* MmsJobRequestSubmitterTest::suite ()
{
    TestSuite * testSuite = new TestSuite( "MmsJobRequestSubmitterTest" );  

  //  Test MmsMessageReader utility methods:
    testSuite->addTest( new TestCaller< MmsJobRequestSubmitterTest >(
                       "submitJobRequestFromAlarmTest",
                       &MmsJobRequestSubmitterTest::submitJobRequestFromAlarmTest, this) );

    testSuite->addTest( new TestCaller< MmsJobRequestSubmitterTest >(
                       "submitJobRequestFromSchematicTest",
                       &MmsJobRequestSubmitterTest::submitJobRequestFromSchematicTest, this) );

	return testSuite;
}

/////////////////////////////////////////////////////////
//  Test methods
/////////////////////////////////////////////////////////


void MmsJobRequestSubmitterTest::submitJobRequestFromAlarmTest()
{
	//Verify that the following method provides the functionality
	//specified in its contract when submitting via an alarm - see API ref:
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
		//virtual void submitJobRequest(const TA_Base_Bus::JobRequestDetailCorbaDef& jobRequest, 
		//	const char* sessionID);	
	//this test also does the following:

	//Verify that the following method provides the functionality
	//specified in its contract - see API ref:
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
		//virtual TA_Base_Bus::JobRequestDetailCorbaDef* getDataFromAlarm(const char* alarmUID, 
		//	CORBA::ULong alarmLocationKey, const char* sessionID);

	TA_Base_Bus::JobRequestDetailCorbaDef_var details;
	std::string sessionID = RunParams::getInstance().get(RPARAM_SESSIONID);

	try
	{
		std::string alarmUID = RunParams::getInstance().get(Util::SEMI_ALARM_UID.c_str());				


		std::string loc = RunParams::getInstance().get(Util::SEMI_ALARM_LOC.c_str());

		CORBA::ULong alarmLocationKey( atoi(loc.c_str()) );
		
		details = m_submitter->getDataFromAlarm( alarmUID.c_str(), alarmLocationKey, sessionID.c_str());
	}
	catch(TA_Base_Core::OperationModeException)
	{
		Assert(false);
	}
	catch(TA_Base_Bus::DependencyFailure& df)
	{
		std::cout << "caught TA_Base_Bus::DependencyFailure: " << df.reason.in() << std::endl;
		Assert(false);
	}
	catch(TA_Base_Bus::InvalidArg)
	{
		Assert(false);
	}
	catch(...)
	{
		Assert(false);
	}

	try
	{
		m_submitter->submitJobRequest(details, sessionID.c_str());	
	}
	catch(TA_Base_Core::OperationModeException)
	{
		Assert(false);
	}
	catch(TA_Base_Bus::DependencyFailure& df)
	{
		std::cout << "caught TA_Base_Bus::DependencyFailure: " << df.reason.in() << std::endl;
		Assert(false);
	}
	catch(TA_Base_Bus::InvalidArg)
	{
		Assert(false);
	}
	catch(TA_Base_Bus::SubmissionDenied sd)
	{
		std::cout << "caught TA_Base_Bus::SubmissionDenied: " << sd.reason.in() << std::endl;
		Assert(false);
	}
	catch(...)
	{
		Assert(false);
	}	

}

void MmsJobRequestSubmitterTest::submitJobRequestFromSchematicTest()
{
	//Verify that the following method provides the functionality
	//specified in its contract when submitting via a schematic  - see API ref:
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
		//virtual void submitJobRequest(const TA_Base_Bus::JobRequestDetailCorbaDef& jobRequest, 
		//	const char* sessionID);	
	
	//This test also does the following
	//Verify that the following method provides the functionality
	//specified in its contract - see API ref:
		//API REF:
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
		//virtual TA_Base_Bus::JobRequestDetailCorbaDef* getDataFromSymbol(const char* entityName, 
		//	const char* sessionID);
	std::string sessionID = RunParams::getInstance().get(RPARAM_SESSIONID);

	std::string nodeName = RunParams::getInstance().get(Util::DATA_NODE_ENTITY_NAME.c_str());

	TA_Base_Bus::JobRequestDetailCorbaDef_var details;
	//TA_Base_Bus::JobRequestDetailCorbaDef* details = 0;
	try
	{
		
		details = m_submitter->getDataFromSymbol(nodeName.c_str(), sessionID.c_str());
	}
	catch(...)
	{
		Assert(false);
	}

	try
	{
		m_submitter->submitJobRequest(details, sessionID.c_str());	
	}
	catch(TA_Base_Core::OperationModeException)
	{
		Assert(false);
	}
	catch(TA_Base_Bus::DependencyFailure& df)
	{
		std::cout << "caught TA_Base_Bus::DependencyFailure: " << df.reason.in() << std::endl;
		Assert(false);
	}
	catch(TA_Base_Bus::InvalidArg)
	{
		Assert(false);
	}
	catch(TA_Base_Bus::SubmissionDenied sd)
	{
		std::cout << "caught TA_Base_Bus::SubmissionDenied: " << sd.reason.in() << std::endl;
		Assert(false);
	}
	catch(...)
	{
		Assert(false);
	}		
}