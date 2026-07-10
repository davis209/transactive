/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/TrainLiveStatusDlgTestCases.h $
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

class TrainLiveStatusDlg;

class TrainLiveStatusDlgTestCases : public TestCase
{

public:
	TrainLiveStatusDlgTestCases ( const std::string & name );

	virtual ~TrainLiveStatusDlgTestCases()
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
    
    void testConstructor();
    void testOnReportUpdate_continueRequired();
    void testOnReportUpdate_invalidation();
    void testOnTrainPttStateChange();
    void testOnButtonBeginLiveAnnouncement();
    void testOnButtonContinueLiveAnnouncement();
    void testOnButtonEndLiveAnnouncement();
    void testOnRadioOverride();
    void testOnRadioNormal();

    // Tested in testConstructor (this is window construction..)
	//void testOnInitDialog();
    // Not implemented currently (nothing to test)
	//void testOnTrainHelp();

    void testOnDestroy();
    void testDestroyStatusModel();
    void testOnTrainStatusCloseBtn();
	
    void testOnTrainStatusRetryFailuresBtn();
    void testOnTrainStatusRetryAllBtn();
    
    // These are higher level tests (quite more complicated than unit tests)
    void testOnBroadcastStateChanged_invalid();
    void testOnBroadcastStateChanged_creation();
    void testOnBroadcastStateChanged_execution();
    void testOnBroadcastStateChanged_runtime();
    void testOnBroadcastStateChanged_termination_pendingRemoval();

    void testSetContinueButtonEnabledState();
    void testUpdateContinueButtonState();
    void testProcessContinueRequiredStateChange();
    void testResetContinueRequiredButton();

    void testTDDefect_1981();

private:

    TrainLiveStatusDlg* m_statusDlg;
    std::string m_broadcastId;

    WindowStub  m_errorHandlerWindow;

    std::vector<unsigned long> clearQueuedErrors();
    void simulateButtonPress(HWND hWnd);
    bool getHasOverrideOptionBeenSet();
    void createStatusDialog();
    void destroyStatusDialog();
    // Dispatches a progress report to window with mixed 'errorsOccurred' states
    void generateMixedErrorStateProgressReport(UINT seed, TA_IRS_Bus::IPAAgentCorbaDef::TrainProgressSeq& data);
    // Dispatches a progress report with a single train present, with input state
    void generateTrainProgressReport(
        TA_IRS_Bus::IPAAgentCorbaDef::ETrainState state,
        TA_IRS_Bus::IPAAgentCorbaDef::EBroadcastState broadcastState = TA_IRS_Bus::IPAAgentCorbaDef::Creation,
        unsigned int count = 1);
    // Generates a progress report from the input arguments, dispatching to window
    void dispatchProgressReport(
        TA_IRS_Bus::IPAAgentCorbaDef::EBroadcastState broadcastState, 
        const TA_IRS_Bus::IPAAgentCorbaDef::TrainProgressSeq& data);

    void resetStatusDialog();
};




