/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/TrainLiveStatusModelTestCases.cpp $
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

#include "TrainLiveStatusModelTestCases.h"
#include "common.h"

#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "bus/pa/cachedmaps/src/PaStationDvaMessageCachedMap.h"
#include "app/pa/PAManager/src/TrainLiveStatusModel.h"

#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/TaAssert.h"
#include "bus/mfc_extensions/src/selection_helpers/SelectionHelper.h"

#include "WindowStub.h"
#include "GenericStatusPageStub.h"

using namespace TA_App;
using namespace TA_IRS_Bus;

#include "app/pa/PAManager/src/TrainLiveStatusDlg.h"

TrainLiveStatusModelTestCases::TrainLiveStatusModelTestCases ( const std::string & name ) 
: 
TestCase ( name ),
m_statusPage(0),
m_statusModel(0)
{
}

/////////////////////////////////////////////////
//
// processProgressUpdateData Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusModelTestCases::testProcessProgressUpdateData()
{   
    // Test this function by sending through data that we would
    // be expecting in real world (in correct order - connecting, in progress, continue required etc..)
    // We simply confirm as trains enter / leave states, the results of associated external functions are kept up to date
    IPAAgentCorbaDef::TrainProgressSeq trainData;

    enum { COUNT = 6 };
    trainData.length(COUNT);

    // Set up initial state
    for (int i = 0; i < COUNT; i ++)
    {
        trainData[i].trainId = i;
        trainData[i].trainState = TA_IRS_Bus::IPAAgentCorbaDef::TRAIN_STATE_CONNECTING;
    }
    
    srand(20);
    
    std::vector<TrainLiveStatusModel::TrainContinueState> expectedNotifications;

    for (int iteration = 0; iteration < 200; iteration ++)
    {
        bool oneOrMoreConnected = false;

        for (int i = 0; i < COUNT; i ++)
        {    
            IPAAgentCorbaDef::TrainProgress& tp = trainData[i];         
          
            updateTrainProgress(expectedNotifications, tp);

            if (TA_IRS_Bus::IPAAgentCorbaDef::TRAIN_STATE_CONNECTED == tp.trainState)
            {   
                oneOrMoreConnected = true;
            }
        }
            
        IPAAgentCorbaDef::BroadcastProgressUpdateData updateData; 
        updateData.theProgressReportSpecifics.theTrainProgressSeq(trainData);
    
        // This update should propogate the continue required state as we require
        getStatusModel().processProgressUpdateData(updateData);

        Assert(oneOrMoreConnected == getStatusModel().existsTrainsWaitingForBroadcast());
        
        confirmExpectedNotificationsMatch(expectedNotifications);
    }
}


/////////////////////////////////////////////////
//
// processContinueRequiredStateChange Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusModelTestCases::testProcessContinueRequiredStateChange()
{
    // Very similar to testProcessProgressUpdateData (see that function for notes)
    IPAAgentCorbaDef::TrainProgressSeq trainData;

    enum { COUNT = 6 };
    trainData.length(COUNT);

    // Set up initial state
    for (int i = 0; i < COUNT; i ++)
    {
        trainData[i].trainId = i;
        trainData[i].trainState = TA_IRS_Bus::IPAAgentCorbaDef::TRAIN_STATE_CONNECTING;
    }
    
    srand(20);
    
    std::vector<TrainLiveStatusModel::TrainContinueState> expectedNotifications;

    for (int iteration = 0; iteration < 200; iteration ++)
    {
        bool oneOrMoreConnected = false;

        for (int i = 0; i < COUNT; i ++)
        {    
            IPAAgentCorbaDef::TrainProgress& tp = trainData[i];       
            
            updateTrainProgress(expectedNotifications, tp);

            // Dispatch the update
            getStatusModel().processContinueRequiredStateChange(tp.trainId, tp.trainState);

            confirmExpectedNotificationsMatch(expectedNotifications);
        }
    }    
}


// Randomly adjusts the state of a train - based on it's current state.  Updates expectedNotifications if 
// entering a state that suggest the operator should be notified of continue required (or continue confirmed)              
void TrainLiveStatusModelTestCases::updateTrainProgress(std::vector<TrainLiveStatusModel::TrainContinueState>& expectedNotifications,
                                                        TA_IRS_Bus::IPAAgentCorbaDef::TrainProgress& tp)
{          
    switch (tp.trainState)
    {            
    case TA_IRS_Bus::IPAAgentCorbaDef::TRAIN_STATE_CONNECTING:
        if (0 == rand() % 2)
        {
            tp.trainState = TA_IRS_Bus::IPAAgentCorbaDef::TRAIN_STATE_CONNECTED;
        }
        break;
    case TA_IRS_Bus::IPAAgentCorbaDef::TRAIN_STATE_CONNECTED:
        switch (rand() % 3)
        {
        case 0:
            tp.trainState = IPAAgentCorbaDef::TRAIN_STATE_IN_PROGRESS;
            break;
        case 1:
            // Simulate a condition whereby we skip the in progress state, and required continue immediately
            // (shouldn't expect this to happen in real life, unless high latency circumstances)
            tp.trainState = IPAAgentCorbaDef::TRAIN_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED;
    
            // Expect a 'continue required' notification
            TrainLiveStatusModel::TrainContinueState state = {tp.trainId, true};
            expectedNotifications.push_back(state);
            break;
        }
        break;
    case TA_IRS_Bus::IPAAgentCorbaDef::TRAIN_STATE_IN_PROGRESS:
        if (0 == rand() % 10)
        {
            // Enter completed state, should be no continue required anymore (so don't enqueue) 
            tp.trainState = IPAAgentCorbaDef::TRAIN_STATE_COMPLETED;
        }
        else if (0 == rand() % 2)
        {
            // Enter continue required again
            tp.trainState = IPAAgentCorbaDef::TRAIN_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED;
    
            // Expect a 'continue required' notification
            TrainLiveStatusModel::TrainContinueState state = {tp.trainId, true};
            expectedNotifications.push_back(state);
        }
        break;
    case TA_IRS_Bus::IPAAgentCorbaDef::TRAIN_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED:
        // From continue required, can enter completed, or in progress again
        if (0 == rand() % 10)
        {
            // If complete, don't expect a 'continue required' acknowledgement
            tp.trainState = IPAAgentCorbaDef::TRAIN_STATE_COMPLETED;
        }
        else if (0 == rand() % 2)
        {
            // Enter in progress again
            tp.trainState = IPAAgentCorbaDef::TRAIN_STATE_IN_PROGRESS;
    
            // Expect a 'continue required' confirmation message
            TrainLiveStatusModel::TrainContinueState state = {tp.trainId, false};
            expectedNotifications.push_back(state);
        }
        break;
    case TA_IRS_Bus::IPAAgentCorbaDef::TRAIN_STATE_COMPLETED:
        // Go immediately into a connecting state again (re-activate the train)
        tp.trainState = TA_IRS_Bus::IPAAgentCorbaDef::TRAIN_STATE_CONNECTING;
        break;
    default:
        Assert(false);
    }
}


// Confirms the notifications passed in correspond perfectly with notifications waiting in model
void TrainLiveStatusModelTestCases::confirmExpectedNotificationsMatch(std::vector<TrainLiveStatusModel::TrainContinueState>& expectedNotifications)
{    
    // Confirm after each update that state remains valid            
    Assert(expectedNotifications.size() == getStatusModel().m_continueStateChangeQueue.size());
    
    while (getStatusModel().doesNewContinueRequiredTrainExist())
    {
        TrainLiveStatusModel::TrainContinueState state = getStatusModel().popFrontNewContinueRequiredTrain();
    
        // Confirm that the notification matches one of expected
        for (std::vector<TrainLiveStatusModel::TrainContinueState>::iterator itr = expectedNotifications.begin();
                itr != expectedNotifications.end(); itr ++)
        {
            if (itr->trainId == state.trainId && itr->continueRequired == state.continueRequired)
            {
                // Found required match
                break;
            }
        }
    
        Assert(itr != expectedNotifications.end());
        // May purge from expected list
        expectedNotifications.erase(itr);
    }
}   
    


/////////////////////////////////////////////////
//
// popFrontNewContinueRequiredTrain Tests
//
/////////////////////////////////////////////////
void TrainLiveStatusModelTestCases::testPopFrontNewContinueRequiredTrain()
{   
    // Confirm the doesNewContinueRequiredTrainExist precondition
    Assert(!getStatusModel().doesNewContinueRequiredTrainExist());

    try
    {
        getStatusModel().popFrontNewContinueRequiredTrain();
        Assert(false);
    }
    catch (...)
    {
        Assert(true);
    }

    UINT i;
    enum { COUNT = 6 };
    enum { SPLITPOINT = 3 };

    // Set up internal state to test the functions
    for (i = 0; i < COUNT; i ++)
    {
        std::deque<TrainLiveStatusModel::TrainContinueState>& queue = getStatusModel().m_continueStateChangeQueue;
        
        TrainLiveStatusModel::TrainContinueState state;
        state.trainId = i;
        state.continueRequired = (i < SPLITPOINT) ? true : false;
        queue.push_back(state);
    }
    
    for (i = 0; i < COUNT; i ++)
    {
        Assert(getStatusModel().doesNewContinueRequiredTrainExist());      
        TrainLiveStatusModel::TrainContinueState state = 
                    getStatusModel().popFrontNewContinueRequiredTrain();

        // Should get results back in FIFO order (since popping front of container)
        Assert(state.trainId == (int)i);
        if (i < SPLITPOINT)
        {
            Assert(state.continueRequired);
        }
        else
        {   
            // Remaining trains have no continue required
            Assert(!state.continueRequired);
        }
    }

    // No trains should remain
    Assert(!getStatusModel().doesNewContinueRequiredTrainExist());     
}

void TrainLiveStatusModelTestCases::tearDownSuite() 
{
    TA_App::PaTest::restoreLocation();
}


void TrainLiveStatusModelTestCases::setUpSuite( const std::string & inFile ) 
{
    TA_App::PaTest::setToValidLocation();
}


TrainLiveStatusModel& TrainLiveStatusModelTestCases::getStatusModel()
{
    // The dummy page will have it's own train live model, can't use that currently..
    /*
    GenericStatusModel* model = m_statusPage->getStatusModelPointer();
    TrainLiveStatusModel* trainModel = dynamic_cast<TrainLiveStatusModel*>(model);
    Assert(NULL != trainModel);
    return *trainModel;
    */

    return *m_statusModel;
}


void TrainLiveStatusModelTestCases::setUp( const std::string & INFile , const std::string & OUTFile )
{    
    m_broadcastId = "TrainLiveStatusModelTestCases";

    m_statusPage = new GenericStatusPageStub();    
    m_statusModel = new TrainLiveStatusModel(*m_statusPage, m_broadcastId);

    m_statusPage->m_statusModel = m_statusModel;
}

void TrainLiveStatusModelTestCases::tearDown( const std::string & INFile ) 
{
    delete m_statusModel;
    m_statusModel = 0;

    delete m_statusPage;
    m_statusPage = 0;
}


TestSuite * TrainLiveStatusModelTestCases::suite()
{
    TestSuite * testSuite = new TestSuite( "TrainLiveStatusModelTestCases" );  
    
// Use temporary hashdef to add tests
#define ADD_UNIT_TEST(function) \
    testSuite->addTest( new TestCaller< TrainLiveStatusModelTestCases >(\
                        #function,\
                        &TrainLiveStatusModelTestCases::##function,this ) );

    //
    // processProgressUpdateData tests
    //
    ADD_UNIT_TEST(testProcessProgressUpdateData);

    //
    // processContinueRequiredStateChange tests
    //    
    ADD_UNIT_TEST(testProcessContinueRequiredStateChange);

    //
    // popFrontNewContinueRequiredTrain tests
    //        
    ADD_UNIT_TEST(testPopFrontNewContinueRequiredTrain);

#undef ADD_UNIT_TEST

	return testSuite;
}


bool TrainLiveStatusModelTestCases::getOverrideOption(TrainLiveStatusModel& model)
{
    return model.getHasOverrideOption();
}
