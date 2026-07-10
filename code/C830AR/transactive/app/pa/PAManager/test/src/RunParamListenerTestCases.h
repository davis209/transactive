/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/RunParamListenerTestCases.h $
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

class RunParamListenerTestCases : public TestCase
{

public:
	RunParamListenerTestCases ( const std::string & name ) : TestCase ( name )
	{
	}

	virtual ~RunParamListenerTestCases()
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
    void setUp( const std::string & INFile , const std::string & OUTFile ) {}


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


protected:

	//
    // The actual test methods
    //
    // This method is tested by testProcessInternalRunParamChange (passthrough function)
    // void testOnRunParamChange();
    void testRegisterListener();
    void testDeRegisterListener();
    void testRetrieveInitialRunParamIdentity();
    
    // Simple accessors (and deprecated)
    //void setIsCurrentlyVisible();
    //void isCurrentlyVisible();

    // tested explicitly in testRetrieveInitialRunParamIdentity
    // void testFindIdentityString();
    void testFindVirtualLocation();
    void testProcessInternalRunParamChange();
    // This function is trivial (simple setter at the moment)
    // void testSetNewIdentityFromIdentifier();
    void testExtractActualComand();
    void testRegisterTag();
    void testFindChangeStruct();
    void testDispatchChangeToListener();
    void testFindEndOfIdentityString();
    void testExtractStringIdentifierString();

private:
};
