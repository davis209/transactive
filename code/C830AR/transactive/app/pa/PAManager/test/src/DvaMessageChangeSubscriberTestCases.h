/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/DvaMessageChangeSubscriberTestCases.h $
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
#include "WindowStub.h"

class DvaMessageChangeSubscriber;

class DvaMessageChangeSubscriberTestCases : public TestCase
{

public:
	DvaMessageChangeSubscriberTestCases ( const std::string & name ) : TestCase ( name )
	{
	}

	virtual ~DvaMessageChangeSubscriberTestCases()
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


protected:

	//
    // The actual test methods
    //
    void testOnCachedMapChanged();
    void testRegisterWatchOnTypeChange();
    void testRegisterWatchOnLabelChange();
    void testClearAllChangeWatches();
    // tested in testRegisterWatchOnTypeChange
    //void testClearTypeChangeWatch();
    // tested in testRegisterWatchOnLabelChange
    //void testClearLabelChangeWatch();


    void testHasTypeChangeWatchBeenCleared();
    // tested in testHasTypeChangeWatchBeenCleared
    //void testHasLabelChangeWatchBeenCleared();
    void testUpdateMessageQueue_multipleMessages();    
    void testUpdateMessageQueue_singleMessage();    
    void testGetNotificationData();
    void testOnSubscriptionSuccessful();
    void testOnPreProcessMessage();

    // Does nothing (provided to satisfy pure virtual)
    // void testOnPostProcessMessage();
    
    void testDisplayForeignUpdateWarning();
    void testClearExpectedConfigUpdate();
    // tested in function testClearExpectedConfigUpdate
    // void testAddExpectedConfigUpdate();
    void testClearFirstMatchingExpectedConfigUpdateKey();
    void testTestForReceiptOfFlaggedDvaChange();
    void testResetCachedMapDvaMessageChangeObserverRegistration();
    
    void testDvaUpdateReceivedDetection();
    
private:

    // Helper function
    bool confirmDvaErrorReceivedAndClearWindowStub();

    DvaMessageChangeSubscriber* m_subscriber;
    WindowStub m_windowStub;
};
