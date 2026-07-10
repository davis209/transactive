/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/TrainLiveStatusDlgTestCases.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma warning (disable : 4786 4290)

#include "StdAfx.h"

#include <deque>

#include "TrainLiveStatusDlgTestCases.h"
#include "common.h"

#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "bus/pa/cachedmaps/src/PaStationDvaMessageCachedMap.h"
#include "app/pa/PAManager/src/TrainLiveStatusModel.h"
#include "app/pa/PAManager/src/PaProgressReportSubscriber.h"
#include "app/pa/PAManager/src/TrainRadioPttMonitor.h"
#include "app/pa/PAManager/src/CachedConfig.h"

#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/TaAssert.h"
#include "bus/mfc_extensions/src/selection_helpers/SelectionHelper.h"

#include "WindowStub.h"
#include "GenericStatusPageStub.h"
#include "TrainLiveStatusModelTestCases.h"
#include "UnitTestAccessBridge.h"
#include "app/pa/PAManager/src/TrainLiveStatusDlg.h"

const std::string BROADCAST_ID = "TestBroadcast";

using namespace TA_IRS_Bus;

TrainLiveStatusDlgTestCases::TrainLiveStatusDlgTestCases ( const std::string & name ) 
: 
TestCase ( name ),
m_statusDlg(0),
m_broadcastId(BROADCAST_ID)
{
}


/////////////////////////////////////////////////
//
// constructor Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testConstructor()
{
    Assert(BROADCAST_ID == m_statusDlg->m_openingBroadcastId);

    WindowStub::processMsgQueue();

    // Test various accessors (pure virtual implementations)
    Assert(m_statusDlg->m_trainStatusModel == m_statusDlg->getStatusModelPointer());
    Assert(&m_statusDlg->m_failuresList == &m_statusDlg->getFailuresListControl());
    Assert(&m_statusDlg->m_progressList == &m_statusDlg->getProgressListControl());
    Assert(&m_statusDlg->m_announceStatusControl == &m_statusDlg->getStatusTextListBox());
    Assert(&m_statusDlg->m_retryAllButton == &m_statusDlg->getRetryAllButton());
    Assert(&m_statusDlg->m_retryFailuresButton == &m_statusDlg->getRetryFailedButton());

    // Test internal states of variables (that they're initialised correctly)
    Assert(0 != m_statusDlg->m_trainStatusModel);
    Assert(0 != m_statusDlg->m_trainRadioPttMonitor);
    
    // None of the buttons should be enabled by default
    Assert(!m_statusDlg->m_beginButton.IsWindowEnabled());
    Assert(!m_statusDlg->m_continueButton.IsWindowEnabled());
    Assert(!m_statusDlg->m_endButton.IsWindowEnabled());

    Assert(!m_statusDlg->m_retryFailuresButton.IsWindowEnabled());
    Assert(!m_statusDlg->m_retryAllButton.IsWindowEnabled());
    
    // Normal option should be enabled
    Assert(TRUE == m_statusDlg->IsDlgButtonChecked(IDC_RADIO_NORMAL) );
    // Override should be disabled by default
    Assert(FALSE == m_statusDlg->IsDlgButtonChecked(IDC_RADIO_OVERRIDE) );
    
    Assert(BROADCAST_ID == m_statusDlg->m_trainStatusModel->getBroadcastId());
}


/////////////////////////////////////////////////
//
// OnReportUpdate Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testOnReportUpdate_continueRequired()
{
    // Rather than calling OnReportUpdate directly, should use the subscriber
    // (so test out the message hook for the relevant report update windows message)
    GenericStatusModel* pModel = m_statusDlg->getStatusModelPointer();

    // Send some data to the models progress report subscriber (simulating PA Agents role)
    PaProgressReportSubscriber& subscriber = UnitTestAccessBridge::getProgressReportSubscriber(*pModel);

    IPAAgentCorbaDef::TrainProgressSeq trainData;
    enum { TRAINCOUNT = 2 };
    trainData.length(TRAINCOUNT);
    // Fill in some train data - add in a single train that requires a continue confirmation
    for (int i = 0; i < TRAINCOUNT; i ++)
    {
        IPAAgentCorbaDef::TrainProgress& tp = trainData[i];
        tp.trainId = i;
        tp.trainState = IPAAgentCorbaDef::TRAIN_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED;
    }

    TA_IRS_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData broadcastUpdateData;
    broadcastUpdateData.broadcastId = CORBA::string_dup(BROADCAST_ID.c_str());
    broadcastUpdateData.broadcastState = TA_IRS_Bus::IPAAgentCorbaDef::Runtime;    
    // Failure field unused
    // EBroadcastStateFailure           broadcastStateFailure;
    broadcastUpdateData.theProgressReportSpecifics.theTrainProgressSeq(trainData);

    {
        Assert(m_statusDlg->m_continueRequiredQueue.size() == 0);
    
        // Pushing a progress report to the subscriber should cause an OnReportUpdate call on status dialog
        UnitTestAccessBridge::pushProgressReport(subscriber, broadcastUpdateData);          
        // Must process the message queue in order to receive the update
        WindowStub::processMsgQueue();
        
        // We can tell if it was processed by checking if the continue required
        // state has changed (having sent through a train that requires a continue)
        Assert(TRAINCOUNT == m_statusDlg->m_continueRequiredQueue.size());

        // Confirm that the window has process all the waiting continue required trains..
        Assert(!m_statusDlg->m_trainStatusModel->doesNewContinueRequiredTrainExist());
    }
}


void TrainLiveStatusDlgTestCases::testOnReportUpdate_invalidation()
{
    // TD Unit Test Defect #64
    // Confirm that if there is a progress report where the state of the broadcast
    // doesn't change (while in 'Execution'), 
    // but trains enter a state where they are waiting for a broadcast
    // then the dialog correctly updates its state (previously this did not happen, present as a bug)
    generateTrainProgressReport(IPAAgentCorbaDef::TRAIN_STATE_CONNECTING, TA_IRS_Bus::IPAAgentCorbaDef::Execution);

    // Currently the status dialog should have the 'begin' button disabled, as no trains connected
    Assert(FALSE == m_statusDlg->m_beginButton.IsWindowEnabled());

    CString rString;
    m_statusDlg->getStatusTextListBox().ResetContent();

    // Now send through a connected train, while remaining in execution
    generateTrainProgressReport(IPAAgentCorbaDef::TRAIN_STATE_CONNECTED, TA_IRS_Bus::IPAAgentCorbaDef::Execution);
    
    // Begin button should be enabled, some 
    Assert(TRUE == m_statusDlg->m_beginButton.IsWindowEnabled());
    
    // Some feedback text should exist to indicate ready to go
    Assert(1 == m_statusDlg->getStatusTextListBox().GetCount());
    m_statusDlg->getStatusTextListBox().GetText(0, rString);
    Assert(TrainLiveStatusDlg::s_pressBeginToStart == rString.GetBuffer(0));    
}



/////////////////////////////////////////////////
//
// OnTrainPttStateChange Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testOnTrainPttStateChange()
{
    // This function confirms the train radio PTT state updates the status feedback area properly
    // Once again, test indirectly using PostMessage calls..    
    for (int iteration = 0; iteration < 4; iteration ++)
    {
        // Clear the status feedback dialog
        m_statusDlg->getStatusTextListBox().ResetContent();

        if (iteration < 2)
        {
            m_statusDlg->m_trainRadioPttMonitor->onPTTGranted();
        }
        else
        {
            m_statusDlg->m_trainRadioPttMonitor->onPTTReleased();
        }
        WindowStub::processMsgQueue();

        CString rString;
        // Should only be one additional feedback item
        Assert(1 == m_statusDlg->getStatusTextListBox().GetCount());
        m_statusDlg->getStatusTextListBox().GetText(0, rString);
        
        // Confirm the relevant feedback message came through)
        std::string expectedString = "";

        if (iteration < 2)
        {   
            expectedString = TrainLiveStatusDlg::s_operatorPressedPtt;
        }
        else
        {
            expectedString = TrainLiveStatusDlg::s_operatorReleasedPtt;
        }

        Assert(expectedString == rString.GetBuffer(0));           
    }
}



/////////////////////////////////////////////////
//
// OnButtonBeginLiveAnnouncement Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testOnButtonBeginLiveAnnouncement()
{
    CachedConfig::getInstance()->getAgentObject().clearBeginRequestCount(BROADCAST_ID);
    // Post a message to the button, confirm it gets processed 
    simulateButtonPress(m_statusDlg->m_beginButton.m_hWnd);

    // Confirm request received
    Assert(1 == CachedConfig::getInstance()->getAgentObject().getBeginRequestCount(BROADCAST_ID));
}



/////////////////////////////////////////////////
//
// OnButtonContinueLiveAnnouncement Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testOnButtonContinueLiveAnnouncement()
{
    // Send through 2 'continue required' state trains to test the button functionality
    generateTrainProgressReport(IPAAgentCorbaDef::TRAIN_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED,
                                        IPAAgentCorbaDef::Runtime,
                                        2);
    // Confirm button has started flashing
    Assert(TRUE == m_statusDlg->m_continueButton.IsWindowEnabled());

    CachedConfig::getInstance()->getAgentObject().clearContinueRequestCount(BROADCAST_ID);

    // Post a message to the button, confirm it gets processed
    simulateButtonPress(m_statusDlg->m_continueButton.m_hWnd);
   
    // Confirm request received
    Assert(1 == CachedConfig::getInstance()->getAgentObject().getContinueRequestCount(BROADCAST_ID));

    // Additionally confirm the continue required button is still flashing, as only
    // one of the trains has been confirmed
    Assert(TRUE == m_statusDlg->m_continueButton.IsWindowEnabled());

    // Press the button again, both trains should be confirmed; button flashing should cease (ie disabled)
    simulateButtonPress(m_statusDlg->m_continueButton.m_hWnd);
    Assert(FALSE == m_statusDlg->m_continueButton.IsWindowEnabled());

    //////////////////////////////
    // Confirm correct confirmation text comes through when actual updated train states come through
    m_statusDlg->getStatusTextListBox().ResetContent();

    CString expectedMsg, rString;

    generateTrainProgressReport(IPAAgentCorbaDef::TRAIN_STATE_IN_PROGRESS,
                                        IPAAgentCorbaDef::Runtime,
                                        2);
    // Should display text 'train 1 confirmed' followed by 'train 2 confirmed' only
    // once the actual agent (progress report) data indicates the train has been confirmed
    // (meaning there may be significant delay between pressing button and confirmation)
    Assert(2 == m_statusDlg->getStatusTextListBox().GetCount());
    for (int i = 0; i < 2; i ++)
    {
        m_statusDlg->getStatusTextListBox().GetText(i, rString);
        expectedMsg.Format(TrainLiveStatusDlg::s_callConfirmedOnTrain.c_str(), i + 1);
        Assert(rString == expectedMsg);
    }
}



/////////////////////////////////////////////////
//
// OnButtonEndLiveAnnouncement Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testOnButtonEndLiveAnnouncement()
{
    CachedConfig::getInstance()->getAgentObject().clearEndRequestCount(BROADCAST_ID);
    // Post a message to the button, confirm it gets processed
    simulateButtonPress(m_statusDlg->m_endButton.m_hWnd);

    // Confirm received relevant error
    Assert(1 == CachedConfig::getInstance()->getAgentObject().getEndRequestCount(BROADCAST_ID));
}



/////////////////////////////////////////////////
//
// OnRadioOverride Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testOnRadioOverride()
{
    m_statusDlg->m_trainStatusModel->setHasOverrideOption(false);

    for (int i = 0; i < 2; i ++)
    {
        // Press the override button
        simulateButtonPress(m_statusDlg->GetDlgItem(IDC_RADIO_OVERRIDE)->m_hWnd);
        
        // Should be reflected in model
        Assert(getHasOverrideOptionBeenSet());
    }
}


/////////////////////////////////////////////////
//
// OnRadioNormal Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testOnRadioNormal()
{
    m_statusDlg->m_trainStatusModel->setHasOverrideOption(true);

    for (int i = 0; i < 2; i ++)
    {
        // Press the normal button
        simulateButtonPress(m_statusDlg->GetDlgItem(IDC_RADIO_NORMAL)->m_hWnd);
                
        // Should be reflected in model, no longer in override
        Assert(!getHasOverrideOptionBeenSet());
    }
}


bool TrainLiveStatusDlgTestCases::getHasOverrideOptionBeenSet()
{
    return TrainLiveStatusModelTestCases::getOverrideOption(*m_statusDlg->m_trainStatusModel);
}



/////////////////////////////////////////////////
//
// OnDestroy Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testOnDestroy()
{
    // Call destroywindow, which will in turn call OnDestroy, which should
    // kill off the status model and PTT monitor (both of which rely on window handle being valid)
    m_statusDlg->DestroyWindow();
    Assert(0 == m_statusDlg->m_trainRadioPttMonitor);
    Assert(0 == m_statusDlg->m_trainStatusModel);
    
    // Recreate window once again, so other tests may continue
    createStatusDialog();
}



/////////////////////////////////////////////////
//
// destroyStatusModel Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testDestroyStatusModel()
{
    Assert(0 != m_statusDlg->m_trainStatusModel);
    m_statusDlg->destroyStatusModel();
    // Confirm status model deleted
    Assert(0 == m_statusDlg->m_trainStatusModel);
    
    // Recreate window once again, so other tests may continue
    createStatusDialog();
}



/////////////////////////////////////////////////
//
// OnTrainStatusCloseBtn Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testOnTrainStatusCloseBtn()
{
    // Now test closing the dialog via the button command        
    Assert(2 != m_statusDlg->m_nModalResult);
    simulateButtonPress(m_statusDlg->GetDlgItem(ID_TRAIN_STATUS_CLOSE_BTN)->m_hWnd);
    // Confirm the dialog received a cancel modal state request
    Assert(2 == m_statusDlg->m_nModalResult);    

    // Won't actually destroy anything (because window running as modeless, unlike typical operation)
    
    // Recreate window once again, so other tests may continue
    createStatusDialog();
}


/////////////////////////////////////////////////
//
// OnTrainStatusRetryFailuresBtn Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testOnTrainStatusRetryFailuresBtn()
{
    srand(20);

    for (int iteration = 0; iteration < 20; iteration ++)
    {
        IPAAgentCorbaDef::TrainProgressSeq updateData;
        generateMixedErrorStateProgressReport(rand(), updateData);

        CachedConfig::getInstance()->getAgentObject().clearTrainRetryCache(BROADCAST_ID);
        simulateButtonPress(m_statusDlg->getRetryFailedButton().m_hWnd);
        
        // Confirm that retry failed only dispatched the trains with 
        // an 'error occurred' state to the agent stub
        for (UINT i = 0; i < updateData.length(); i ++)
        {
            bool retryAttempted = CachedConfig::getInstance()->getAgentObject().
                                            hasTrainRetryBeenAttemptedOn(BROADCAST_ID, updateData[i].trainId);

            // Confirm a retry was passed through only for those trains where an error occurred
            Assert(retryAttempted == updateData[i].errorOccurred);
        }
    }
}


/////////////////////////////////////////////////
//
// OnTrainStatusRetryAllBtn Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testOnTrainStatusRetryAllBtn()
{
    srand(20);

    for (int iteration = 0; iteration < 20; iteration ++)
    {
        IPAAgentCorbaDef::TrainProgressSeq updateData;

        generateMixedErrorStateProgressReport(rand(), updateData);

        CachedConfig::getInstance()->getAgentObject().clearTrainRetryCache(BROADCAST_ID);        
        simulateButtonPress(m_statusDlg->getRetryAllButton().m_hWnd);
        
        // Confirm that retry all sent through all the train IDs to the agent stub
        for (UINT i = 0; i < updateData.length(); i ++)
        {
            bool retryAttempted = CachedConfig::getInstance()->getAgentObject().
                                            hasTrainRetryBeenAttemptedOn(BROADCAST_ID, updateData[i].trainId);
            Assert(retryAttempted);
        }
    }
}


/////////////////////////////////////////////////
//
// OnBroadcastStateChanged Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testOnBroadcastStateChanged_invalid()
{
    try
    {
        // Should be invalid state to pass in
        m_statusDlg->onBroadcastStateChanged(TA_IRS_Bus::IPAAgentCorbaDef::EBroadcastState(-1));
        Assert(false);
    }
    catch (...)
    {
        Assert(true);
    }
}


void TrainLiveStatusDlgTestCases::testOnBroadcastStateChanged_creation()
{ 
    // Enter initial state where buttons enabled, to confirm refresh works
    m_statusDlg->onBroadcastStateChanged(TA_IRS_Bus::IPAAgentCorbaDef::Runtime);

    m_statusDlg->onBroadcastStateChanged(TA_IRS_Bus::IPAAgentCorbaDef::Creation);    
    Assert(TRUE == m_statusDlg->m_closeButton.IsWindowEnabled());   // Close button should always be available

    // Other buttons disabled until valid state entered
    Assert(FALSE == m_statusDlg->m_beginButton.IsWindowEnabled());
    Assert(FALSE == m_statusDlg->m_continueButton.IsWindowEnabled());
    Assert(FALSE == m_statusDlg->m_endButton.IsWindowEnabled());
}


void TrainLiveStatusDlgTestCases::testOnBroadcastStateChanged_execution()
{ 
    CString rString;
    m_statusDlg->getStatusTextListBox().ResetContent();

    generateTrainProgressReport(IPAAgentCorbaDef::TRAIN_STATE_CONNECTING);
    m_statusDlg->onBroadcastStateChanged(TA_IRS_Bus::IPAAgentCorbaDef::Execution);    
    Assert(TRUE == m_statusDlg->m_closeButton.IsWindowEnabled());       // Close button should always be available
    Assert(FALSE == m_statusDlg->m_beginButton.IsWindowEnabled());      // Trains still waiting to connect
    Assert(FALSE == m_statusDlg->m_continueButton.IsWindowEnabled());
    Assert(FALSE == m_statusDlg->m_endButton.IsWindowEnabled());

    // Should be no feedback text currently, no trains are connected yet
    Assert(0 == m_statusDlg->getStatusTextListBox().GetCount());
    
    generateTrainProgressReport(IPAAgentCorbaDef::TRAIN_STATE_CONNECTED);
    m_statusDlg->onBroadcastStateChanged(TA_IRS_Bus::IPAAgentCorbaDef::Execution);    
    Assert(TRUE == m_statusDlg->m_closeButton.IsWindowEnabled());       // Close button should always be available
    Assert(TRUE == m_statusDlg->m_beginButton.IsWindowEnabled());       // Trains connected
    Assert(FALSE == m_statusDlg->m_continueButton.IsWindowEnabled());
    Assert(FALSE == m_statusDlg->m_endButton.IsWindowEnabled());

    // Should be a single feedback line telling operator ready to go
    Assert(1 == m_statusDlg->getStatusTextListBox().GetCount());
    m_statusDlg->getStatusTextListBox().GetText(0, rString);
    Assert(TrainLiveStatusDlg::s_pressBeginToStart == rString.GetBuffer(0));    

    // If we call again with same state, shouldn't have displayed for a second time..
    m_statusDlg->onBroadcastStateChanged(TA_IRS_Bus::IPAAgentCorbaDef::Execution);    
    Assert(1 == m_statusDlg->getStatusTextListBox().GetCount());
}


void TrainLiveStatusDlgTestCases::testOnBroadcastStateChanged_runtime()
{ 
    CString rString;
    m_statusDlg->getStatusTextListBox().ResetContent();

    m_statusDlg->onBroadcastStateChanged(TA_IRS_Bus::IPAAgentCorbaDef::Runtime);
    
    Assert(TRUE == m_statusDlg->m_closeButton.IsWindowEnabled());       // Close button should always be available
    Assert(FALSE == m_statusDlg->m_beginButton.IsWindowEnabled());       
    Assert(FALSE == m_statusDlg->m_continueButton.IsWindowEnabled());
    Assert(TRUE == m_statusDlg->m_endButton.IsWindowEnabled());     // Able to terminate once active

    // Confirm feedback text displayed to operator
    Assert(1 == m_statusDlg->getStatusTextListBox().GetCount());
    m_statusDlg->getStatusTextListBox().GetText(0, rString);
    Assert(TrainLiveStatusDlg::s_pressPttToTalk == rString.GetBuffer(0));    

    // Confirm that prompting this state again doesn't cause text to be repeated
    m_statusDlg->onBroadcastStateChanged(TA_IRS_Bus::IPAAgentCorbaDef::Runtime);
    Assert(1 == m_statusDlg->getStatusTextListBox().GetCount());
}


void TrainLiveStatusDlgTestCases::testOnBroadcastStateChanged_termination_pendingRemoval()
{
    // Both the termination and pending removal states should be handled in the same manner
    for (unsigned int i = 0; i < 2; i ++)
    {
        // Perform a manual flush at start of each iteration
        resetStatusDialog();

        // For increased testing, enabled the continue required button initially, and confirm
        // that it is disabled when leaving the runtime state
        generateTrainProgressReport(TA_IRS_Bus::IPAAgentCorbaDef::TRAIN_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED,
                                            TA_IRS_Bus::IPAAgentCorbaDef::Runtime);

        // Confirm expected state was entered
        Assert(TRUE == m_statusDlg->m_continueButton.IsWindowEnabled());

        CString rString;
        m_statusDlg->getStatusTextListBox().ResetContent();
        
        TA_IRS_Bus::IPAAgentCorbaDef::EBroadcastState state;
        if (0 == i)
        {
            state = TA_IRS_Bus::IPAAgentCorbaDef::Termination;
        }
        else
        {
            state = TA_IRS_Bus::IPAAgentCorbaDef::PendingRemoval;
        }

        m_statusDlg->onBroadcastStateChanged(state);
    
        Assert(TRUE == m_statusDlg->m_closeButton.IsWindowEnabled());       // Close button should always be available
        Assert(FALSE == m_statusDlg->m_beginButton.IsWindowEnabled());       
        Assert(FALSE == m_statusDlg->m_continueButton.IsWindowEnabled());
        Assert(FALSE == m_statusDlg->m_endButton.IsWindowEnabled());

        // Confirm feedback text (broadcast complete) displayed to operator
        Assert(1 == m_statusDlg->getStatusTextListBox().GetCount());
        m_statusDlg->getStatusTextListBox().GetText(0, rString);
        Assert(TrainLiveStatusDlg::s_broadcastComplete == rString.GetBuffer(0));    
        
        // Confirm that prompting this state again doesn't cause text to be repeated
        m_statusDlg->onBroadcastStateChanged(state);
        Assert(1 == m_statusDlg->getStatusTextListBox().GetCount());

        // Make this call to reset the broadcast into different state (to enter valid
        // state for the next iteration)
        m_statusDlg->onBroadcastStateChanged(TA_IRS_Bus::IPAAgentCorbaDef::Runtime);
    }
}


/////////////////////////////////////////////////
//
// setContinueButtonEnabledState Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testSetContinueButtonEnabledState()
{
    m_statusDlg->setContinueButtonEnabledState(true);
    Assert(TRUE == m_statusDlg->m_continueButton.IsWindowEnabled());
    m_statusDlg->setContinueButtonEnabledState(true);   // Ok to make redundant calls

    m_statusDlg->setContinueButtonEnabledState(false);
    Assert(FALSE == m_statusDlg->m_continueButton.IsWindowEnabled());
    m_statusDlg->setContinueButtonEnabledState(false);  // Ok to make redundant calls
}


/////////////////////////////////////////////////
//
// updateContinueButtonState Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testUpdateContinueButtonState()
{
    // Can't test this any farther than testing if button gets enabled / disabled
    // (no way to confirm the button is actually flashing)
    m_statusDlg->updateContinueButtonState(true);
    Assert(TRUE == m_statusDlg->m_continueButton.IsWindowEnabled());
    m_statusDlg->updateContinueButtonState(true);   // Ok to make redundant calls

    m_statusDlg->updateContinueButtonState(false);
    Assert(FALSE == m_statusDlg->m_continueButton.IsWindowEnabled());
    m_statusDlg->updateContinueButtonState(false);  // Ok to make redundant calls
}


/////////////////////////////////////////////////
//
// processContinueRequiredStateChange Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testProcessContinueRequiredStateChange()
{
    CString rString;
    m_statusDlg->getStatusTextListBox().ResetContent();

    // Confirm continue required button initially inactive
    Assert(FALSE == m_statusDlg->m_continueButton.IsWindowEnabled());

    TrainLiveStatusModel::TrainContinueState state;
    state.trainId = 1;
    state.continueRequired = true;
    
    // Send through continue required train
    m_statusDlg->processContinueRequiredStateChange(state);

    // Confirm feedback text (broadcast complete) displayed to operator
    Assert(1 == m_statusDlg->getStatusTextListBox().GetCount());
    m_statusDlg->getStatusTextListBox().GetText(0, rString);
    CString expectedMsg;
    expectedMsg.Format(TrainLiveStatusDlg::s_callRequiredOnTrain.c_str(), state.trainId);
    Assert(expectedMsg == rString);    

    // Confirm sending same train through again does not show message for a second time
    m_statusDlg->processContinueRequiredStateChange(state);       
    Assert(1 == m_statusDlg->getStatusTextListBox().GetCount());
  
    ////////////////////////
    // Now confirm a train
    m_statusDlg->getStatusTextListBox().ResetContent();

    state.continueRequired = false;
    m_statusDlg->processContinueRequiredStateChange(state);

    Assert(1 == m_statusDlg->getStatusTextListBox().GetCount());
    m_statusDlg->getStatusTextListBox().GetText(0, rString);
    expectedMsg.Format(TrainLiveStatusDlg::s_callConfirmedOnTrain.c_str(), state.trainId);
    Assert(expectedMsg == rString);    
    
    // If the train is confirmed for a second time, it should show message for a second time
    // (independent of data present status dialog class) - as the continue state is cleared
    // from the internal state array in a different function
    m_statusDlg->processContinueRequiredStateChange(state);
    Assert(2 == m_statusDlg->getStatusTextListBox().GetCount());
}


/////////////////////////////////////////////////
//
// resetContinueRequiredButton Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::testResetContinueRequiredButton()
{
    generateTrainProgressReport(IPAAgentCorbaDef::TRAIN_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED,
                                        IPAAgentCorbaDef::Runtime,
                                        2);    
    Assert(TRUE == m_statusDlg->m_continueButton.IsWindowEnabled());

    m_statusDlg->resetContinueRequiredButton();

    // Confirm that button ceases flashing
    Assert(FALSE == m_statusDlg->m_continueButton.IsWindowEnabled());

    // Confirm the internally queued data has been cleared
    Assert(0 == m_statusDlg->m_continueRequiredQueue.size());
}


void TrainLiveStatusDlgTestCases::testTDDefect_1981()
{
    enum { TRAINCOUNT = 5 };
    enum { STATE_MAX = 6 };
    enum { CONFIRM_CUTOFF = 2 };
    // Define a lifecycle of train states
    TA_IRS_Bus::IPAAgentCorbaDef::ETrainState trainStates[STATE_MAX] = 
    {
        IPAAgentCorbaDef::TRAIN_STATE_CONNECTING,
        IPAAgentCorbaDef::TRAIN_STATE_CONNECTED,
        IPAAgentCorbaDef::TRAIN_STATE_IN_PROGRESS,
        IPAAgentCorbaDef::TRAIN_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED,
        IPAAgentCorbaDef::TRAIN_STATE_IN_PROGRESS,
        IPAAgentCorbaDef::TRAIN_STATE_COMPLETED
    };

    CString rString;
    
    // Test for TD Defect #1981
    for (int stage = 0; stage < STATE_MAX; stage ++)
    {       
        m_statusDlg->getStatusTextListBox().ResetContent();

        IPAAgentCorbaDef::TrainProgressSeq data;
        data.length(TRAINCOUNT);
        for (UINT i = 0; i < TRAINCOUNT; i ++)
        {
            data[i].trainId = i + 1;
            data[i].trainState = trainStates[stage];
        }

        if (4 == stage)
        {
            // Set first final three trains to IPAAgentCorbaDef::TRAIN_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED
            // so only the first two are 'confirmed'
            for (UINT i = CONFIRM_CUTOFF; i < TRAINCOUNT; i ++)
            {
                data[i].trainState = IPAAgentCorbaDef::TRAIN_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED;
            }
        }
        
        dispatchProgressReport(IPAAgentCorbaDef::Runtime, data);

        if (3 == stage)
        {
            Assert(TRAINCOUNT == m_statusDlg->getStatusTextListBox().GetCount());

            // All trains should be requesting a continue
            CString expectedMsg;            
            for (UINT i = 0; i < TRAINCOUNT; i ++)
            {
                expectedMsg.Format(TrainLiveStatusDlg::s_callRequiredOnTrain.c_str(), i + 1);
                m_statusDlg->getStatusTextListBox().GetText(i, rString);
                Assert(expectedMsg == rString);
            }
        }
        else if (4 == stage)
        {
            // The first 'CONFIRM_CUTOFF' trains should have been confirmed
            Assert(CONFIRM_CUTOFF == m_statusDlg->getStatusTextListBox().GetCount());
            CString expectedMsg;            
            for (UINT i = 0; i < CONFIRM_CUTOFF; i ++)
            {
                expectedMsg.Format(TrainLiveStatusDlg::s_callConfirmedOnTrain.c_str(), i + 1);
                m_statusDlg->getStatusTextListBox().GetText(i, rString);
                Assert(expectedMsg == rString);
            }
        }
        else if (5 == stage)
        {
            // This is where the defect previously had a failure - 
            // setting the state to completed caused the trains to be 'confirmed'
            // However the correct behaviour is no feedback to operator
            Assert(0 == m_statusDlg->getStatusTextListBox().GetCount());            
        }
    }
}


/////////////////////////////////////
/////////////////////////////////////
// Test class specific helper functions
/////////////////////////////////////
/////////////////////////////////////
void TrainLiveStatusDlgTestCases::simulateButtonPress(HWND hWnd)
{
    int nID = GetDlgCtrlID(hWnd);

    Assert(nID > 0);

    m_statusDlg->PostMessage(WM_COMMAND, nID, (LPARAM)hWnd);

    WindowStub::processMsgQueue();   
}


std::vector<unsigned long> TrainLiveStatusDlgTestCases::clearQueuedErrors()
{
    std::vector<unsigned long> queue;
    WindowStub::MessageQueue data = m_errorHandlerWindow.getReceived();

    // Confirm all error messages waiting in queue
    for (UINT i = 0; i < data.size(); i ++)
    {   
        if (WM_PA_ERROR_MESSAGE == data[i].message)
        {
            PaErrorHandler::confirmReceiptOfRequest(data[i].wParam);
            queue.push_back(data[i].wParam);
        }
    }

    m_errorHandlerWindow.reset();

    return queue;
}


void TrainLiveStatusDlgTestCases::createStatusDialog()
{
    destroyStatusDialog();

    m_statusDlg = new TrainLiveStatusDlg(m_broadcastId, 0);
    
    // Create the dialog modeless, so unit tests may proceed
    m_statusDlg->Create(IDD_TRAIN_BROADCAST_LIVE_STATUS, 0);
    // don't need to show for unit tests (but may if want to..)
    // m_statusDlg->ShowWindow(SW_SHOW);
}


void TrainLiveStatusDlgTestCases::generateMixedErrorStateProgressReport(UINT seed, IPAAgentCorbaDef::TrainProgressSeq& data)
{
    // Generates a random set of train data and dispatches to progress report
    srand(seed);

    PaProgressReportSubscriber& subscriber = UnitTestAccessBridge::getProgressReportSubscriber(
                                                   *m_statusDlg->getStatusModelPointer());
    // Send through a progress report with a non-failed train
    // the retry failed request should have no effect (because there
    // are no failed trains)
    //IPAAgentCorbaDef::TrainProgressSeq data;
    enum { COUNT = 15 };
    data.length(COUNT);
    for (int i = 0; i < COUNT; i ++)
    {
        data[i].trainId = i;
        int randomState = rand() % IPAAgentCorbaDef::TRAIN_STATE_COMPLETED;
        data[i].trainState = IPAAgentCorbaDef::ETrainState(randomState);
        data[i].errorOccurred = rand() % 2 == 0 ? true : false;
    }
    
    dispatchProgressReport(TA_IRS_Bus::IPAAgentCorbaDef::PendingRemoval, data);
}


void TrainLiveStatusDlgTestCases::generateTrainProgressReport
(
 TA_IRS_Bus::IPAAgentCorbaDef::ETrainState state,
 TA_IRS_Bus::IPAAgentCorbaDef::EBroadcastState broadcastState,
 unsigned int count
)
{
    IPAAgentCorbaDef::TrainProgressSeq data;
    data.length(count);

    for (UINT i = 0; i < count; i ++)
    {
        data[i].trainId = i + 1;
        data[i].trainState = state;
        data[i].errorOccurred = false;
    }

    dispatchProgressReport(broadcastState, data);
}


void TrainLiveStatusDlgTestCases::dispatchProgressReport
(
 TA_IRS_Bus::IPAAgentCorbaDef::EBroadcastState broadcastState, 
 const IPAAgentCorbaDef::TrainProgressSeq& data
)
{
    PaProgressReportSubscriber& subscriber = UnitTestAccessBridge::getProgressReportSubscriber(
                                                   *m_statusDlg->getStatusModelPointer());

    TA_IRS_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData broadcastUpdateData;
    broadcastUpdateData.broadcastId = CORBA::string_dup(BROADCAST_ID.c_str());
    broadcastUpdateData.broadcastState = broadcastState;    
    broadcastUpdateData.theProgressReportSpecifics.theTrainProgressSeq(data);

    UnitTestAccessBridge::pushProgressReport(subscriber, broadcastUpdateData);
    WindowStub::processMsgQueue();
}


void TrainLiveStatusDlgTestCases::destroyStatusDialog()
{
    if (0 != m_statusDlg)
    {   
        if (0 != m_statusDlg->m_hWnd)
        {
            m_statusDlg->DestroyWindow();
        }
        delete m_statusDlg;
        m_statusDlg = 0;
    }
}


void TrainLiveStatusDlgTestCases::resetStatusDialog()
{
    PaProgressReportSubscriber& subscriber = UnitTestAccessBridge::getProgressReportSubscriber(
                                                   *m_statusDlg->getStatusModelPointer());

    // This should flush some states back into default modes (depending on progress report data)
    TA_IRS_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData broadcastUpdateData;
    broadcastUpdateData.broadcastId = CORBA::string_dup(BROADCAST_ID.c_str());

    // Start with creation
    broadcastUpdateData.broadcastState = TA_IRS_Bus::IPAAgentCorbaDef::Creation;    
    UnitTestAccessBridge::pushProgressReport(subscriber, broadcastUpdateData);
    WindowStub::processMsgQueue();

    // State change to complete
    broadcastUpdateData.broadcastState = TA_IRS_Bus::IPAAgentCorbaDef::PendingRemoval;    
    UnitTestAccessBridge::pushProgressReport(subscriber, broadcastUpdateData);
    WindowStub::processMsgQueue();
}


//////////////////////////////////////////////////
//////////////////////////////////////////////////
// Unit test framework (implementation) functions
//////////////////////////////////////////////////
//////////////////////////////////////////////////
void TrainLiveStatusDlgTestCases::setUpSuite( const std::string & inFile ) 
{
    TA_App::PaTest::setToValidLocation();
    // Set the error handler window to suppress popup of error dialogs
    PaErrorHandler::setPrimaryWindowHandle(m_errorHandlerWindow.getHWnd());

    createStatusDialog();
}


void TrainLiveStatusDlgTestCases::tearDownSuite() 
{
    destroyStatusDialog();

    TA_App::PaTest::restoreLocation();

    // To avoid popups, clear queued errors before resetting window handle
    clearQueuedErrors();

    // Restore the error dialog handler class
    PaErrorHandler::setPrimaryWindowHandle(0);
}


void TrainLiveStatusDlgTestCases::setUp( const std::string & INFile , const std::string & OUTFile )
{    
}


void TrainLiveStatusDlgTestCases::tearDown( const std::string & INFile ) 
{
    resetStatusDialog();
}


TestSuite * TrainLiveStatusDlgTestCases::suite()
{
    TestSuite * testSuite = new TestSuite( "TrainLiveStatusDlgTestCases" );  
    
// Use temporary hashdef to add tests
#define ADD_UNIT_TEST(function) \
    testSuite->addTest( new TestCaller< TrainLiveStatusDlgTestCases >(\
                        #function,\
                        &TrainLiveStatusDlgTestCases::##function,this ) );

    //
    // constructor tests
    //
    ADD_UNIT_TEST(testConstructor);

    //
    // OnReportUpdate tests
    //
    ADD_UNIT_TEST(testOnReportUpdate_continueRequired);
    ADD_UNIT_TEST(testOnReportUpdate_invalidation);

    //
    // OnTrainPttStateChange tests
    //
    ADD_UNIT_TEST(testOnTrainPttStateChange);

    //
    // OnButtonBeginLiveAnnouncement tests
    //
    ADD_UNIT_TEST(testOnButtonBeginLiveAnnouncement);

    //
    // OnButtonContinueLiveAnnouncement tests
    //
    ADD_UNIT_TEST(testOnButtonContinueLiveAnnouncement);

    //
    // OnButtonEndLiveAnnouncement tests
    //
    ADD_UNIT_TEST(testOnButtonEndLiveAnnouncement);

    //
    // OnRadioOverride tests
    //
    ADD_UNIT_TEST(testOnRadioOverride);

    //
    // OnRadioNormal tests
    //
    ADD_UNIT_TEST(testOnRadioNormal);

    //
    // OnDestroy tests
    //
    ADD_UNIT_TEST(testOnDestroy);

    //
    // destroyStatusModel tests
    //
    ADD_UNIT_TEST(testDestroyStatusModel);

    //
    // OnTrainStatusCloseBtn tests
    //
    ADD_UNIT_TEST(testOnTrainStatusCloseBtn);

    //
    // OnTrainStatusRetry:FailuresBtn tests
    //
    ADD_UNIT_TEST(testOnTrainStatusRetryFailuresBtn);

    //
    // OnTrainStatusRetryAllBtn tests
    //
    ADD_UNIT_TEST(testOnTrainStatusRetryAllBtn);

    //
    // onBroadcastStateChanged tests
    //
    ADD_UNIT_TEST(testOnBroadcastStateChanged_invalid);
    ADD_UNIT_TEST(testOnBroadcastStateChanged_creation);
    ADD_UNIT_TEST(testOnBroadcastStateChanged_execution);
    ADD_UNIT_TEST(testOnBroadcastStateChanged_runtime);
    ADD_UNIT_TEST(testOnBroadcastStateChanged_termination_pendingRemoval);

    //
    // setContinueButtonEnabledState tests
    //
    ADD_UNIT_TEST(testSetContinueButtonEnabledState);

    //
    // updateContinueButtonState tests
    //
    ADD_UNIT_TEST(testUpdateContinueButtonState);

    //
    // processContinueRequiredStateChange tests
    //
    ADD_UNIT_TEST(testProcessContinueRequiredStateChange);

    //
    // resetContinueRequiredButton tests
    //
    ADD_UNIT_TEST(testResetContinueRequiredButton);

    //
    // Test Director specific Defect tests
    //
    ADD_UNIT_TEST(testTDDefect_1981);

#undef ADD_UNIT_TEST

	return testSuite;
}