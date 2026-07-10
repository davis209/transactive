/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/MmsControllerModelTestCases.h $
  * @author:  Jade Lee
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  * <description>
  *
  */

#pragma once

#include <map>
#include <set>
#include <TestCase.h>
#include <TestFramework.h>
#include <windows.h>


class MmsControllerModelTestCases : public TestCase
{

public:
	MmsControllerModelTestCases ( const std::string & name ) : TestCase ( name )
	{
	}

	virtual ~MmsControllerModelTestCases()
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
    void tearDown( const std::string & INFile ) {}


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

protected:
    
    void testgetGlobalMmsInhibit();    

    void testsetGlobalMmsInhibit_success();
    void testsetGlobalMmsInhibit_failure();    
    
    void testgetSubsystemMmsInhibit();    
    void testsetSubsystemMmsInhibit_success();    
    void testsetSubsystemMmsInhibit_failure();    

    void testgetDatanodeProxy();    
    void testsendGlobalInhibitAuditMessage();       
    void testsendSubsystemInhibitAuditMessage();    

    void testsubmitAuditMessage();    

private:

};

