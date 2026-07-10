/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/test/module/src/MmsConnectionModuleTest.h $
  * @author:  C. DeWolfe
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $ 
  * 
  * This class provides the module tests for the MmsConnection class
  * The MmsConnection class is responcible for the following:
  *		--providing a remote interface for submitting messages to the MMS Server, via the MMS API provided by LTA
  *     --periodically clearing any incomming messages from the queue - note that we do not need
  *    to read the messages - just clear them
  *     --periodically pinging the MMS Server to make sure that it is still there
  */

#if !defined(MmsConnectionModuleTest_H)
#define MmsConnectionModuleTest_H

#include "cots/CppUnit/src/TestCase.h"
#include "cots/CppUnit/src/TestFramework.h"

#include "core/naming/src/NamedObject.h"
#include "bus/maintenance_management/mmsCorbaDef/idl/src/IMmsConnectionCorbaDef.h"

namespace TA_Base_Bus
{
	class JobRequestMessage;
}

class MmsConnectionModuleTest : public TestCase  
{

protected:

	//see implimentation for comments:
	void submitAlarmMessageTest();
	void submitJobRequestMessageTest();
	void submitCounterMessageTest();
	void submitMeasurementMessageTest();
	void pingTest();
	void clearMessagesTest();

private:

	// Object being tested
	TA_Base_Core::NamedObject<TA_Base_Bus::IMmsConnectionCorbaDef,
				TA_Base_Bus::IMmsConnectionCorbaDef_ptr,
				TA_Base_Bus::IMmsConnectionCorbaDef_var> m_mmsConnection;

	std::string   m_sessionID;

public:

	//Test suite methods:

	MmsConnectionModuleTest ( const std::string & name ) : TestCase ( name )
	{
	}

	virtual ~MmsConnectionModuleTest()
	{
	}


	static void initAlarmStruct(TA_Base_Core::AlarmDetailCorbaDef& details);

  /**
    * suite
    * 
    * This adds all the test methods to the suite of tests and returns it.
    *
    * @return TestSuite* A collection of all tests that can be run.
    *
    */
    TestSuite * suite();

  /**
    * setUp
    * 
    * Initialises variables before each method is run.
    *
    */
    void setUp( const std::string & INFile , const std::string & OUTFile );


  /**
    * tearDown
    * 
    * Destroys variables after each method is run.
    *
    */
    void tearDown( const std::string & INFile );


  /**
    * setUpSuite
    * 
    * Sets up variables before a suite of tests are run.
    *
    */
    virtual void setUpSuite( const std::string & inFile );


  /**
    * setUpSuite
    * 
    * Destroys variables after a suite of tests have been run.
    *
    */
    void tearDownSuite();

};

#endif // !defined(MmsConnectionModuleTest_H)
