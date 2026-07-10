/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/TrainLiveStatusModelTestCases.h $
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

#include "bus/pa/cachedmaps/src/CachedMapDefinitions.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "WindowStub.h"
#include "app/pa/PAManager/src/TrainLiveStatusModel.h"

class TrainLiveStatusModel;
class GenericStatusPageStub;

class TrainLiveStatusModelTestCases : public TestCase
{

public:
	TrainLiveStatusModelTestCases ( const std::string & name );

	virtual ~TrainLiveStatusModelTestCases()
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

    /// Helper function for other unit test classes (so don't
    /// have to keep adding friend classes to main code)
    static bool getOverrideOption(TrainLiveStatusModel& model);

protected:
    
    void testProcessProgressUpdateData();
    void testProcessContinueRequiredStateChange();
    // tested in testPopFrontNewContinueRequiredTrain
    //void testDoesNewContinueRequiredTrainExist();
    void testPopFrontNewContinueRequiredTrain();
    // This function is trivial, doesn't require direct testing
    //void testResetContinueRequiredStates();

    // These functions can't be tested until some agent
    // stubs are developed (simple passthrough functions)
    //void testRequestContinueOnTrain();
    //void testEndLiveAnnouncement();
    //void testBeginLiveAnnouncement();

private:

    void updateTrainProgress(std::vector<TrainLiveStatusModel::TrainContinueState>& expectedNotifications,
                                                        TA_IRS_Bus::IPAAgentCorbaDef::TrainProgress& tp);
    void confirmExpectedNotificationsMatch(std::vector<TrainLiveStatusModel::TrainContinueState>& expectedNotifications);
    
    TrainLiveStatusModel& getStatusModel();

    GenericStatusPageStub* m_statusPage;   
    TrainLiveStatusModel* m_statusModel;
    std::string m_broadcastId;

    WindowStub m_dummyWindow;
};



