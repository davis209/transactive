/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/test/module/src/MmsAlarmSubmitterTest.cpp $
  * @author:  C. DeWolfe
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * implimentation of MmsAlarmSubmitterTest class
  *
  */

#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4146)
#pragma warning(disable:4018)
#pragma warning(disable:4250)
#endif // WIN32

#include "MmsAlarmSubmitterTest.h"

#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/RunParams.h"
#include "core/exceptions/src/AssertException.h"
#include "core/message/types/MessageTypes.h"

#include "Util.h" 
#include "MmsConnectionModuleTest.h"

#include <time.h>
#include <string>

using TA_Base_Core::RunParams;



/////////////////////////////////////////////////////////
// Test Framework Methods
/////////////////////////////////////////////////////////

void MmsAlarmSubmitterTest::setUpSuite( const std::string & inFile )
{

}


void MmsAlarmSubmitterTest::tearDownSuite()
{
	TA_Base_Core::CorbaUtil::getInstance().shutdown();
	TA_Base_Core::CorbaUtil::getInstance().cleanup();
}


void MmsAlarmSubmitterTest::setUp( const std::string & INFile , const std::string & OUTFile )
{

	TA_Base_Core::CorbaUtil::getInstance().initialise();
	TA_Base_Core::CorbaUtil::getInstance().activate();

	// Get the object
	m_submitter.setEntityName("MmsAlarmSubmitter_occ");
	Assert( !m_submitter->_non_existent() );

	// Set up session ID 
	m_sessionID = TA_Base_Core::RunParams::getInstance().get(RPARAM_SESSIONID);
}


void MmsAlarmSubmitterTest::tearDown( const std::string & INFile )
{

}

TestSuite* MmsAlarmSubmitterTest::suite ()
{
    TestSuite * testSuite = new TestSuite( "MmsAlarmSubmitterTest" );  

    testSuite->addTest( new TestCaller< MmsAlarmSubmitterTest >(
                       "submitMmsAlarmMessageTest",
                       &MmsAlarmSubmitterTest::submitMmsAlarmMessageTest, this) );

	return testSuite;
}

/////////////////////////////////////////////////////////
//  Test methods
/////////////////////////////////////////////////////////


void MmsAlarmSubmitterTest::submitMmsAlarmMessageTest()
{
	//Verify that the following method provides the functionality
	//specified in its contract - see API ref:
     //IMmsAlarmSubmitterCorbaDef methods:

		/**
		 * Submit a MMS alarm message for the specified alarm.
		 *
		 * Note that no confirmation of success or failure is returned by this
		 * method. Hence, the class that impliments this interface is 
		 * responcible for approriate failure handling, e.g. raising alarm
		 *
		 * raises( TA_Base_Core::OperationModeException, MmsConnectionFailure )
		 */
		//virtual void submitMmsAlarmMessage(const TA_Base_Core::AlarmDetailCorbaDef& p_alarm);

	//create a AlarmDetailCorbaDef to base submission on
	TA_Base_Core::AlarmDetailCorbaDef details;
	MmsConnectionModuleTest::initAlarmStruct(details);

	//set the fields to the values specified in the runparams
	std::string alarmUID = RunParams::getInstance().get(Util::AUTO_ALARM_UID.c_str());				
	details.alarmID = alarmUID.c_str();

	std::string loc = RunParams::getInstance().get(Util::AUTO_ALARM_LOC.c_str());
	CORBA::ULong alarmLocationKey( atoi(loc.c_str()) );
	details.locationKey = alarmLocationKey;

	try
	{
		m_submitter->submitMmsAlarmMessage(details);
	}
	catch(TA_Base_Core::OperationModeException)
	{
		Assert(false);
	}
	catch(TA_Base_Bus::MmsConnectionFailure)
	{
		Assert(false);
	}
	catch(...)
	{
		Assert(false);
	}

}
