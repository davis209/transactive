/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/test/module/src/MmsAlarmSubmitterTest.h $
  * @author:  C. DeWolfe
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $ 
  * 
  * This class provides the module tests for the MmsAlarmSubmitter class
  */

#if !defined(MmsAlarmSubmitterTest_H)
#define MmsAlarmSubmitterTest_H

#include "cots/CppUnit/src/TestCase.h"
#include "cots/CppUnit/src/TestFramework.h"

#include "core/naming/src/NamedObject.h"
#include "bus/maintenance_management/mmsCorbaDef/idl/src/IMmsAlarmSubmitterCorbaDef.h"


class MmsAlarmSubmitterTest : public TestCase  
{

protected:

	//see implimentation for comments:
	void submitMmsAlarmMessageTest();


private:

	// Object being tested:
	// The object exposed by the MMS Agent for Job Request submission:
	TA_Base_Core::NamedObject<TA_Base_Bus::IMmsAlarmSubmitterCorbaDef,
			TA_Base_Bus::IMmsAlarmSubmitterCorbaDef_ptr,
			TA_Base_Bus::IMmsAlarmSubmitterCorbaDef_var> m_submitter;

	std::string   m_sessionID;

public:

	//Test suite methods:

	MmsAlarmSubmitterTest ( const std::string & name ) : TestCase ( name )
	{
	}

	virtual ~MmsAlarmSubmitterTest()
	{
	}


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

#endif // !defined(MmsAlarmSubmitterTest_H)
