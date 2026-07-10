/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/DvaMessageChangeSubscriberTestCases.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma warning (disable : 4786)

#include "StdAfx.h"
#include "DvaMessageChangeSubscriberTestCases.h"
#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/TaAssert.h"
#include "app/pa/PAManager/src/DvaMessageChangeSubscriber.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/IRunParamChangedListener.h"
#include "core/utilities/src/RunParams.h"
#include "bus/pa/cachedmaps/src/PaStationDvaMessageCachedMap.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"


#include "TestUtils.h"

#include "Common.h"

using namespace TA_Base_Core;
using namespace TA_IRS_Bus;

enum 
{ 
    MESSAGE_KEY_A = 50,
    MESSAGE_KEY_B,
    MESSAGE_KEY_C,
    MESSAGE_KEY_D
};
 
enum 
{
    MESSAGE_TYPE_N = 'N',
    MESSAGE_TYPE_E = 'E'
};

const std::string MESSAGE_LABEL_A = "LabelA";
const std::string MESSAGE_LABEL_B = "LabelB";

/////////////////////////////////////////////////
//
// onCachedMapChanged Tests
//
/////////////////////////////////////////////////
void DvaMessageChangeSubscriberTestCases::testOnCachedMapChanged()
{
    // This is called whenever there are changed detected via the config editor channel (subscriber)
    // indicating that one of the DVA Messages changed - the message update should be
    // propogated to the window queue (and able to be retrieved)
    bool changesDetected = false;       // This is only used for error message display

    {
        m_subscriber->onCachedMapChanged(MESSAGE_KEY_A, changesDetected);

        // Our window stub should have received this update
        const WindowStub::MessageQueue data = m_windowStub.getReceived();
        Assert(1 == data.size());
        Assert(WM_PA_DVA_MESSAGE_UPDATE == data[0].message);
    
        // Confirm the wParam corresponds to expected key for subscriber
        // (getMessageData will fail if it's not)
        const std::set<unsigned long>& keys = m_subscriber->getMessageData(data[0].wParam,     
                                                                             m_windowStub.getHWnd());
        Assert(1 == keys.size());
        Assert(MESSAGE_KEY_A == *(keys.begin()));   
    }

    m_windowStub.reset();

    {
        // Now send through multiple messages, should be bundled into same packet
        m_subscriber->onCachedMapChanged(MESSAGE_KEY_A, changesDetected);
        m_subscriber->onCachedMapChanged(MESSAGE_KEY_B, changesDetected);
        m_subscriber->onCachedMapChanged(MESSAGE_KEY_A, changesDetected);
        
        const WindowStub::MessageQueue data = m_windowStub.getReceived();
        // We should have received a single notification - single we didn't
        // collection the data between the notifications
        Assert(1 == data.size());
        Assert(WM_PA_DVA_MESSAGE_UPDATE == data[0].message);

        const std::set<unsigned long>& keys  = m_subscriber->getMessageData(data[0].wParam,     
                                                                                m_windowStub.getHWnd());

        // Should have received the two keys in a single packet (there were two duplicates, will be merged)
        Assert(2 == keys.size());
        Assert(MESSAGE_KEY_A == *(keys.begin()));
        // Second key
        Assert(MESSAGE_KEY_B == *(++keys.begin()));
    }
}

/////////////////////////////////////////////////
//
// registerWatchOnTypeChange Tests
//
/////////////////////////////////////////////////
void DvaMessageChangeSubscriberTestCases::testRegisterWatchOnTypeChange()
{
    // Before registering - add in some dummy expected config editor updates
    // these should be cleared when adding in a new watch (in this way, if
    // a config editor comes in late, it won't cause confusion - we will have disowned it)
    m_subscriber->addExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_TYPE, MESSAGE_KEY_B);
    m_subscriber->addExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_LABEL, MESSAGE_KEY_A);
    m_subscriber->addExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_TYPE, MESSAGE_KEY_C);
    Assert(3 == m_subscriber->m_expectedConfigUpdates.size());

    // Get the subscriber to listen out for a certain type change
    m_subscriber->registerWatchOnTypeChange(MESSAGE_KEY_A, MESSAGE_TYPE_N);

    // Confirm the type config updates were cleared (and only label one remains)
    // NOTE: registerWatchOnTypeChange will add its own expected update..
    Assert(2 == m_subscriber->m_expectedConfigUpdates.size());
    Assert(DvaMessageChangeSubscriber::CHANGE_LABEL == m_subscriber->m_expectedConfigUpdates[0].first);
    // The second one was the one added by registerWatchOnTypeChange
    Assert(DvaMessageChangeSubscriber::CHANGE_TYPE == m_subscriber->m_expectedConfigUpdates[1].first);
    Assert(MESSAGE_KEY_A == m_subscriber->m_expectedConfigUpdates[1].second);

    // Do some cleanup before proceeding (clear all 'foreign' watches)
    m_subscriber->clearExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_LABEL, 0);
    // Only watch generated by registerWatchOnTypeChange should remain
    Assert(1 == m_subscriber->m_expectedConfigUpdates.size());

    try
    {
        // Shouldn't allow a second registration (violate precondition)
        m_subscriber->registerWatchOnTypeChange(MESSAGE_KEY_A, MESSAGE_TYPE_N);
        Assert(false);
    }
    catch (...)
    {
        Assert(true);
    }

    Assert(m_subscriber->m_receiveTypeWatch.hasBeenSet());
    Assert(MESSAGE_KEY_A == m_subscriber->m_receiveTypeWatch.getValue().messageKey);
    Assert(MESSAGE_TYPE_N == m_subscriber->m_receiveTypeWatch.getValue().changedData);

    Assert(1 == m_subscriber->m_expectedConfigUpdates.size());
    Assert(DvaMessageChangeSubscriber::CHANGE_TYPE == m_subscriber->m_expectedConfigUpdates[0].first);
    Assert(MESSAGE_KEY_A == m_subscriber->m_expectedConfigUpdates[0].second);

    // Confirm able to clear the type change watch OK
    m_subscriber->clearTypeChangeWatch();
    
    Assert(!m_subscriber->m_receiveTypeWatch.hasBeenSet());
    Assert(0 == m_subscriber->m_expectedConfigUpdates.size());

    // Should be able to call again safely (just gets ignored)
    m_subscriber->clearTypeChangeWatch();
}

/////////////////////////////////////////////////
//
// registerWatchOnLabelChange Tests
//
/////////////////////////////////////////////////
void DvaMessageChangeSubscriberTestCases::testRegisterWatchOnLabelChange()
{
    // Before registering - add in some dummy expected config editor updates
    // these should be cleared when adding in a new watch (in this way, if
    // a config editor comes in late, it won't cause confusion - we will have disowned it)
    m_subscriber->addExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_TYPE, MESSAGE_KEY_B);
    m_subscriber->addExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_LABEL, MESSAGE_KEY_B);
    m_subscriber->addExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_LABEL, MESSAGE_KEY_C);
    m_subscriber->addExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_TYPE, MESSAGE_KEY_C);
    Assert(4 == m_subscriber->m_expectedConfigUpdates.size());

    // Get the subscriber to listen out for a certain Label change
    m_subscriber->registerWatchOnLabelChange(MESSAGE_KEY_A, MESSAGE_LABEL_A);
    
    // Confirm the label config updates were cleared (and only type ones remains)
    // NOTE: A new label watch will be added by the registerWatchOnLabelChange
    Assert(3 == m_subscriber->m_expectedConfigUpdates.size());
    Assert(DvaMessageChangeSubscriber::CHANGE_TYPE == m_subscriber->m_expectedConfigUpdates[0].first);
    Assert(DvaMessageChangeSubscriber::CHANGE_TYPE == m_subscriber->m_expectedConfigUpdates[1].first);
    Assert(DvaMessageChangeSubscriber::CHANGE_LABEL == m_subscriber->m_expectedConfigUpdates[2].first);
    Assert(MESSAGE_KEY_A == m_subscriber->m_expectedConfigUpdates[2].second);

    // Do some cleanup before proceeding (clear remaining type watches)
    m_subscriber->clearExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_TYPE, 0);
    // Only newly registered watch should remain
    Assert(1 == m_subscriber->m_expectedConfigUpdates.size());

    try
    {
        // Shouldn't allow a second registration (violate precondition)
        m_subscriber->registerWatchOnLabelChange(MESSAGE_KEY_A, MESSAGE_LABEL_A);
        Assert(false);
    }
    catch (...)
    {
        Assert(true);
    }

    Assert(m_subscriber->m_receiveLabelWatch.hasBeenSet());
    Assert(MESSAGE_KEY_A == m_subscriber->m_receiveLabelWatch.getValue().messageKey);
    Assert(MESSAGE_LABEL_A == m_subscriber->m_receiveLabelWatch.getValue().changedData);

    Assert(1 == m_subscriber->m_expectedConfigUpdates.size());
    Assert(DvaMessageChangeSubscriber::CHANGE_LABEL == m_subscriber->m_expectedConfigUpdates[0].first);
    Assert(MESSAGE_KEY_A == m_subscriber->m_expectedConfigUpdates[0].second);

    // Confirm able to clear the Label change watch OK
    m_subscriber->clearLabelChangeWatch();
    
    Assert(!m_subscriber->m_receiveLabelWatch.hasBeenSet());
    Assert(0 == m_subscriber->m_expectedConfigUpdates.size());

    // Should be able to call again safely (just gets ignored)
    m_subscriber->clearLabelChangeWatch();
}

/////////////////////////////////////////////////
//
// clearAllChangeWatches Tests
//
/////////////////////////////////////////////////
void DvaMessageChangeSubscriberTestCases::testClearAllChangeWatches()
{
    m_subscriber->registerWatchOnLabelChange(MESSAGE_KEY_A, MESSAGE_LABEL_A);
    m_subscriber->registerWatchOnTypeChange(MESSAGE_KEY_A, MESSAGE_TYPE_N);

    Assert(m_subscriber->m_receiveTypeWatch.hasBeenSet());
    Assert(m_subscriber->m_receiveLabelWatch.hasBeenSet());
    Assert(2 == m_subscriber->m_expectedConfigUpdates.size());

    // Confirmed cleaned up OK
    m_subscriber->clearAllChangeWatches();

    Assert(!m_subscriber->m_receiveTypeWatch.hasBeenSet());
    Assert(!m_subscriber->m_receiveLabelWatch.hasBeenSet());
    Assert(0 == m_subscriber->m_expectedConfigUpdates.size());

    // Should be able to call again without error
    m_subscriber->clearAllChangeWatches();
}

/////////////////////////////////////////////////
//
// hasTypeChangeWatchBeenCleared Tests
//
/////////////////////////////////////////////////
void DvaMessageChangeSubscriberTestCases::testHasTypeChangeWatchBeenCleared()
{
    Assert(m_subscriber->hasTypeChangeWatchBeenCleared());
    Assert(m_subscriber->hasLabelChangeWatchBeenCleared());
    m_subscriber->registerWatchOnTypeChange(MESSAGE_KEY_A, MESSAGE_TYPE_E);
    m_subscriber->registerWatchOnLabelChange(MESSAGE_KEY_A, MESSAGE_LABEL_A);
    Assert(!m_subscriber->hasTypeChangeWatchBeenCleared());
    Assert(!m_subscriber->hasLabelChangeWatchBeenCleared());

    m_subscriber->clearTypeChangeWatch();
    m_subscriber->clearTypeChangeWatch();
    Assert(m_subscriber->hasTypeChangeWatchBeenCleared());
    Assert(!m_subscriber->hasLabelChangeWatchBeenCleared());

    m_subscriber->clearAllChangeWatches();  
    Assert(m_subscriber->hasTypeChangeWatchBeenCleared());
    Assert(m_subscriber->hasLabelChangeWatchBeenCleared());
}


/////////////////////////////////////////////////
//
// updateMessageQueue Tests
//
/////////////////////////////////////////////////
void DvaMessageChangeSubscriberTestCases::testUpdateMessageQueue_multipleMessages()
{
    // The updateMessageQueue function is used to update the input
    // msg queue to contain the data in 'updateData' - posting to windows
    // in 'windowHwnds' as necessary (not necessary if notification already sent out)   
    // std::set<unsigned long> DvaMessageUpdateData
	TA_IRS_Bus::UpdateBuffer<DvaMessageUpdateData> msgQueue;
    
    // Test sending out many individual messages before collection, to confirm
    // they're all bundled into one
    int i;
    enum { STUB_COUNT = 3 };
    WindowStub stubs[STUB_COUNT];
    std::set<HWND> windowHandles;

    for (i = 0; i < STUB_COUNT; i ++)
    {
        windowHandles.insert(stubs[i].getHWnd());
    }    

    // This will send out MAX_LENGTH * (MAX_LENGTH / 2) messages (50 * 100 = 5000)
    enum { MAX_LENGTH = 100 };
    enum { USED_STUB = 0 };

    // The subscriber should post out notification to 'windowHandles)
    for (i = 0; i < MAX_LENGTH; i ++)
    {
        // Send through increasingly large chunks of update data
        // With overlapping information (because the prefix data remains consistent)
        DvaMessageUpdateData updateData;
        for (int a = 0; a <= i; a ++)
        {
            updateData.insert(MESSAGE_KEY_A + a);
        }

        stubs[USED_STUB].reset();       // Clear messages before new ones get dispatched

        m_subscriber->updateMessageQueue(msgQueue, windowHandles, updateData);

        // In between each send, get a single window to process the message
        const WindowStub::MessageQueue& data = stubs[USED_STUB].getReceived();
        // This next line picked up Unit Test TD #58
        // - updateMessageQueue was using its internal window set, not 'windowHandles'
        Assert(1 == data.size());
        Assert(WM_PA_DVA_MESSAGE_UPDATE == data[0].message);
        DvaMessageUpdateData buf = msgQueue.fetchUpdate(data[0].wParam, stubs[USED_STUB].getHWnd());

        Assert(updateData == buf);        
    }

    // Formulate a set defining what we expect the unused window stubs to contain
    DvaMessageUpdateData expectedResult;
    for (i = 0; i < MAX_LENGTH; i ++)
    {
        expectedResult.insert(MESSAGE_KEY_A + i);
    }

    for (i = 0; i < STUB_COUNT; i ++)
    {
        if (USED_STUB == i) 
        {   // Don't consider the user up window stub
            continue; 
        }       

        // Should be a single message for the entire set of keys
        const WindowStub::MessageQueue& data = stubs[i].getReceived();
        Assert(1 == data.size());
        Assert(WM_PA_DVA_MESSAGE_UPDATE == data[0].message);
        
        // Should be "MAX_LENGTH" keys present
        DvaMessageUpdateData buf = msgQueue.fetchUpdate(data[0].wParam, stubs[i].getHWnd());

        // The data returned for this window should correspond to the data send out to it
        Assert(buf == expectedResult);
    }
}


void DvaMessageChangeSubscriberTestCases::testUpdateMessageQueue_singleMessage()
{
    // see testUpdateMessageQueue_multipleMessages for comments
	TA_IRS_Bus::UpdateBuffer<DvaMessageUpdateData> msgQueue;
    DvaMessageUpdateData updateData;
    updateData.insert(MESSAGE_KEY_A);
    updateData.insert(MESSAGE_KEY_B);
    
    int i;
    enum { STUB_COUNT = 3 };
    WindowStub stubs[STUB_COUNT];
    std::set<HWND> windowHandles;

    for (i = 0; i < STUB_COUNT; i ++)
    {
        windowHandles.insert(stubs[i].getHWnd());
    }    

    // The subscriber should post out notification to 'windowHandles)
    m_subscriber->updateMessageQueue(msgQueue, windowHandles, updateData);

    for (i = 0; i < STUB_COUNT; i ++)
    {
        WindowStub::MessageQueue data = stubs[i].getReceived();
        Assert(1 == data.size());
        Assert(WM_PA_DVA_MESSAGE_UPDATE == data[0].message);
        
        const DvaMessageUpdateData& buf = msgQueue.fetchUpdate(data[0].wParam, stubs[i].getHWnd());

        // The data returned for this window should correspond to the data send out to it
        Assert(buf == updateData);
    }
}


/////////////////////////////////////////////////
//
// getNotificationData Tests
//
/////////////////////////////////////////////////
void DvaMessageChangeSubscriberTestCases::testGetNotificationData()
{
    TA_IRS_Bus::IPAAgentCorbaDef::DvaMessagesUpdateData updateData;
    updateData.length(3);
    updateData[0].m_pKey = MESSAGE_KEY_C;
    updateData[1].m_pKey = MESSAGE_KEY_A;
    updateData[2].m_pKey = MESSAGE_KEY_A;

    DvaMessageUpdateData expectedData;
    expectedData.insert(MESSAGE_KEY_C);
    expectedData.insert(MESSAGE_KEY_A);

    Assert(expectedData == m_subscriber->getNotificationData(updateData));
}


/////////////////////////////////////////////////
//
// onSubscriptionSuccessful Tests
//
/////////////////////////////////////////////////
void DvaMessageChangeSubscriberTestCases::testOnSubscriptionSuccessful()
{
    m_subscriber->resetCachedMapDvaMessageChangeObserverRegistration(false);

    Assert(!m_subscriber->m_subscribedToCachedMapChanges);

    // When subscription successful, should reset its subscription to cached maps, and refresh
    // the database (to ensure it's up to date)
    // The second condition is unimportant (and in fact may be taken out in future) - 
    // so only confirm that subscribe to cached maps
    m_subscriber->onSubscriptionSuccessful();

    Assert(m_subscriber->m_subscribedToCachedMapChanges);
}


/////////////////////////////////////////////////
//
// onPreProcessMessage Tests
//
/////////////////////////////////////////////////
void DvaMessageChangeSubscriberTestCases::testOnPreProcessMessage()
{
    PaErrorHandler::setPrimaryWindowHandle(m_windowStub.getHWnd());

    // Confirm that when an unrecognized DVA Message update or
    // batch of updates come through, a warning dialog appears
    TA_IRS_Bus::IPAAgentCorbaDef::StationDvaMessageRecords updateData;
    StationDvaMessageRecords records = CachedMaps::getInstance()->getPaStationDvaMessageCachedMap().getAllRecords();
    Assert(records.size() > 1);

    updateData.length(2);
    // Set some valid keys
    updateData[0].m_pKey = records.begin()->m_pKey;
    updateData[1].m_pKey = (++records.begin())->m_pKey;

    m_subscriber->onPreProcessMessage(updateData);
    Assert(confirmDvaErrorReceivedAndClearWindowStub());

    updateData.length(1);
    // make sure recognized as ad-hoc message (single length normal DVA message updates have no need
    // to be handled currently)
    updateData[0].m_id = 1; 
    m_subscriber->onPreProcessMessage(updateData);
    Assert(confirmDvaErrorReceivedAndClearWindowStub());
    
    PaErrorHandler::setPrimaryWindowHandle(0);
}


/////////////////////////////////////////////////
//
// displayForeignUpdateWarning Tests
//
/////////////////////////////////////////////////
void DvaMessageChangeSubscriberTestCases::testDisplayForeignUpdateWarning()
{
    PaErrorHandler::setPrimaryWindowHandle(m_windowStub.getHWnd());

    // Fairly trivial function here - confirm requests error dialog
    m_subscriber->displayForeignUpdateWarning(true);

    WindowStub::MessageQueue data = m_windowStub.getReceived();

    Assert(1 == data.size());
    Assert(WM_PA_ERROR_MESSAGE == data[0].message);
    Assert(PaErrorHandler::DVA_MESSAGE_UPDATE_RECEIVED_EXTERNALLY == data[0].wParam);
    
    PaErrorHandler::confirmReceiptOfRequest(data[0].wParam);

    PaErrorHandler::setPrimaryWindowHandle(0);
}

/////////////////////////////////////////////////
//
// clearExpectedConfigUpdate Tests
//
/////////////////////////////////////////////////
void DvaMessageChangeSubscriberTestCases::testClearExpectedConfigUpdate()
{
    m_subscriber->addExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_TYPE, MESSAGE_KEY_B);
    m_subscriber->addExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_LABEL, MESSAGE_KEY_B);
    m_subscriber->addExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_TYPE, MESSAGE_KEY_C);
    m_subscriber->addExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_LABEL, MESSAGE_KEY_C);
   
    Assert(4 == m_subscriber->m_expectedConfigUpdates.size());

    // Clear our a single update
    unsigned long msgB = MESSAGE_KEY_B;
    m_subscriber->clearExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_TYPE, &msgB);
    Assert(3 == m_subscriber->m_expectedConfigUpdates.size());
    // Confirm second messages moved to first positive in the queue
    Assert(DvaMessageChangeSubscriber::CHANGE_LABEL == m_subscriber->m_expectedConfigUpdates[0].first);

    // Clear out a subset of updates
    m_subscriber->clearExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_LABEL, 0);
    Assert(1 == m_subscriber->m_expectedConfigUpdates.size());
    Assert(DvaMessageChangeSubscriber::CHANGE_TYPE == m_subscriber->m_expectedConfigUpdates[0].first);
    Assert(MESSAGE_KEY_C == m_subscriber->m_expectedConfigUpdates[0].second);
    unsigned long msgC = MESSAGE_KEY_C;
    m_subscriber->clearExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_TYPE, &msgC);
    
    Assert(0 == m_subscriber->m_expectedConfigUpdates.size());    
}


/////////////////////////////////////////////////
//
// clearFirstMatchingExpectedConfigUpdateKey Tests
//
/////////////////////////////////////////////////
void DvaMessageChangeSubscriberTestCases::testClearFirstMatchingExpectedConfigUpdateKey()
{
    m_subscriber->addExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_TYPE, MESSAGE_KEY_A);
    m_subscriber->addExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_LABEL, MESSAGE_KEY_B);
    m_subscriber->addExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_TYPE, MESSAGE_KEY_C);
    m_subscriber->addExpectedConfigUpdate(DvaMessageChangeSubscriber::CHANGE_LABEL, MESSAGE_KEY_D);
   
    Assert(4 == m_subscriber->m_expectedConfigUpdates.size());

    for (int erase = 0; erase < 4; erase ++)
    {
        int keyToErase;

        switch (erase)
        {
        case 0:     keyToErase = MESSAGE_KEY_C;            break;
        case 1:     keyToErase = MESSAGE_KEY_D;            break;
        case 2:     keyToErase = MESSAGE_KEY_A;            break;
        case 3:     keyToErase = MESSAGE_KEY_B;            break;
        };

        m_subscriber->clearFirstMatchingExpectedConfigUpdateKey(keyToErase);

        Assert(3 - erase == m_subscriber->m_expectedConfigUpdates.size());
        
        for (int i = 0; i < m_subscriber->m_expectedConfigUpdates.size(); i ++)
        {
            Assert(m_subscriber->m_expectedConfigUpdates[i].second != keyToErase);
        }
    }
}

/////////////////////////////////////////////////
//
// testForReceiptOfFlaggedDvaChange Tests
//
/////////////////////////////////////////////////
void DvaMessageChangeSubscriberTestCases::testTestForReceiptOfFlaggedDvaChange()
{
    PaErrorHandler::setPrimaryWindowHandle(m_windowStub.getHWnd());

    // Confirm that this function pops up an error dialog when appropriate (that is, 
    // when a foreign message comes through)
    TA_IRS_Bus::IPAAgentCorbaDef::StationDvaMessageRecord adhocMsg;
    adhocMsg.m_id = 1;  // This makes it an ad-hoc type message (at OCC)
    Assert(PaStationDvaMessageCachedMap::isAdhocMessageId(
                                        CachedConfig::getAdhocSet(),
                                        adhocMsg.m_id));


    // Type change
    {
        adhocMsg.m_pKey = MESSAGE_KEY_A;
        adhocMsg.m_type = MESSAGE_TYPE_N;
        adhocMsg.m_label = CORBA::string_dup(MESSAGE_LABEL_A.c_str());
    
        // By default sending this message through will trip an error dialog
        m_subscriber->testForReceiptOfFlaggedDvaChange(adhocMsg, true);
        Assert(confirmDvaErrorReceivedAndClearWindowStub());
        
        m_subscriber->registerWatchOnTypeChange(MESSAGE_KEY_B, MESSAGE_TYPE_N);
        
        // Should still attempt to display error
        m_subscriber->testForReceiptOfFlaggedDvaChange(adhocMsg, true);
        Assert(confirmDvaErrorReceivedAndClearWindowStub());
          
        adhocMsg.m_pKey = MESSAGE_KEY_B;  
        adhocMsg.m_type = MESSAGE_TYPE_E;
    
        // Still not a match..
        m_subscriber->testForReceiptOfFlaggedDvaChange(adhocMsg, true);
        Assert(confirmDvaErrorReceivedAndClearWindowStub());
        
        adhocMsg.m_pKey = MESSAGE_KEY_B;  
        adhocMsg.m_type = MESSAGE_TYPE_N;
        // Now this should the type change watch
        m_subscriber->testForReceiptOfFlaggedDvaChange(adhocMsg, true);
        Assert(m_subscriber->hasTypeChangeWatchBeenCleared());
        
        Assert(!confirmDvaErrorReceivedAndClearWindowStub());
        
        // Type watch should be cleared, so sending through a second time should show error
        m_subscriber->testForReceiptOfFlaggedDvaChange(adhocMsg, true);
        Assert(confirmDvaErrorReceivedAndClearWindowStub());
    }

    // Label change
    {
        m_subscriber->testForReceiptOfFlaggedDvaChange(adhocMsg, true);
        Assert(confirmDvaErrorReceivedAndClearWindowStub());

        m_subscriber->registerWatchOnLabelChange(MESSAGE_KEY_A, MESSAGE_LABEL_A);

        adhocMsg.m_pKey = MESSAGE_KEY_A;  
        adhocMsg.m_label = CORBA::string_dup(MESSAGE_LABEL_B.c_str());
        
        m_subscriber->testForReceiptOfFlaggedDvaChange(adhocMsg, true);
        Assert(confirmDvaErrorReceivedAndClearWindowStub());

        adhocMsg.m_pKey = MESSAGE_KEY_B;  
        adhocMsg.m_label = CORBA::string_dup(MESSAGE_LABEL_A.c_str());

        m_subscriber->testForReceiptOfFlaggedDvaChange(adhocMsg, true);
        Assert(confirmDvaErrorReceivedAndClearWindowStub());

        adhocMsg.m_pKey = MESSAGE_KEY_A;  

        // This call should not have an error dialog
        m_subscriber->testForReceiptOfFlaggedDvaChange(adhocMsg, true);
        Assert(m_subscriber->hasLabelChangeWatchBeenCleared());
        
        Assert(!confirmDvaErrorReceivedAndClearWindowStub());

        // Second call should
        m_subscriber->testForReceiptOfFlaggedDvaChange(adhocMsg, true);
        Assert(confirmDvaErrorReceivedAndClearWindowStub());
    }

    PaErrorHandler::setPrimaryWindowHandle(0);
}


bool DvaMessageChangeSubscriberTestCases::confirmDvaErrorReceivedAndClearWindowStub()
{
    return TestUtils::confirmErrorReceivedAndClearWindowStub(
                PaErrorHandler::DVA_MESSAGE_UPDATE_RECEIVED_EXTERNALLY, 
                m_windowStub);
}


void DvaMessageChangeSubscriberTestCases::testDvaUpdateReceivedDetection()
{
    PaErrorHandler::setPrimaryWindowHandle(m_windowStub.getHWnd());

    // This function is not for exlusive testing of any one function, but just
    // confirms the interleaving of DVA message update notifications from the
    // agent and cached maps to confirm correct handling
    TA_IRS_Bus::IPAAgentCorbaDef::StationDvaMessageRecord msg1, msg2;
    msg1.m_id = msg2.m_id = 1;
    
    msg1.m_pKey = MESSAGE_KEY_A;
    msg1.m_type = MESSAGE_TYPE_N;
    msg1.m_label = CORBA::string_dup(MESSAGE_LABEL_A.c_str());
    
    msg2.m_pKey = MESSAGE_KEY_B;
    msg2.m_type = MESSAGE_TYPE_E;
    msg2.m_label = CORBA::string_dup(MESSAGE_LABEL_B.c_str());
    
    //////////////////////////////////////////
    // Need to test that receiving the cached map update before or after doesn't affect outcome    
    // There are quite a few interleavings, each of which should suppress the error dialog displaying
    enum { MAX_COUNTER = 256 }; // 4^4

    // Counter defines the current enumeration (some of which are invalid, due to duplicate elements)
    for(int counter = 0; counter < MAX_COUNTER; counter ++)
    {   
        // This array defines the ordering, each element must be unique before processing
        // and in range 0-3
        int orderArray[4] = {0, 0, 0, 0};
        
        orderArray[0] = (counter / 64) % 4;
        orderArray[1] = (counter / 16) % 4;
        orderArray[2] = (counter / 4) % 4;
        orderArray[3] = counter % 4;

        std::set<int> noDuplicates;
        for (int i = 0; i < 4; i ++)
        {
            noDuplicates.insert(std::set<int>::value_type(orderArray[i]));
        }
        
        // If there are duplicates within the set, error message should be displayed      
        // because the first time we receive it, we note it was changed locally, but second
        // time we don't recognize the update as our own      
        // - if no duplicates, then error should be suppressed
        bool shouldWarningBeDisplayed = (noDuplicates.size() < 4);
        
        m_subscriber->registerWatchOnTypeChange(MESSAGE_KEY_A, MESSAGE_TYPE_N);
        m_subscriber->registerWatchOnLabelChange(MESSAGE_KEY_B, MESSAGE_LABEL_B);
        Assert(2 == m_subscriber->m_expectedConfigUpdates.size());
    
        bool warningDisplayed = false;

        // There are four functions, which need to be called each time in a different ordering each time
        // Treat each one as a base 4 decimal
        for (int order = 0; order < 4; order ++)
        {
            switch (orderArray[order])
            {
            case 0:
                // msg1 update from PA Agent
                m_subscriber->testForReceiptOfFlaggedDvaChange(msg1, true);
                break;
            case 1:
                // msg1 update from config editor
                m_subscriber->onCachedMapChanged(msg1.m_pKey, true);
                break;
            case 2:
                // msg2 update from PA Agent
                m_subscriber->testForReceiptOfFlaggedDvaChange(msg2, true);
            break;
            case 3:
                // msg2 update from config editor
                m_subscriber->onCachedMapChanged(msg2.m_pKey, true);
            break;
            };
            
            warningDisplayed |= confirmDvaErrorReceivedAndClearWindowStub();

            // Confirm that no error dialogs come up at any stage
            if (!shouldWarningBeDisplayed)
            {
                Assert(!warningDisplayed);
            }            
        }
        
        if (shouldWarningBeDisplayed)
        {
            // There should have been at least one warning, when an update
            // came through consecutively
            Assert(warningDisplayed);
            
            // Get back into known state
            m_subscriber->clearAllChangeWatches();
        }
        else
        {
            // Confirm that all config editor / type watches are cleared
            Assert(m_subscriber->hasTypeChangeWatchBeenCleared());
            Assert(m_subscriber->hasLabelChangeWatchBeenCleared());
            Assert(0 == m_subscriber->m_expectedConfigUpdates.size());    
        }
    }

    PaErrorHandler::setPrimaryWindowHandle(0);
}


/////////////////////////////////////////////////
//
// resetCachedMapDvaMessageChangeObserverRegistration Tests
//
/////////////////////////////////////////////////
void DvaMessageChangeSubscriberTestCases::testResetCachedMapDvaMessageChangeObserverRegistration()
{
    m_subscriber->resetCachedMapDvaMessageChangeObserverRegistration(false);
    m_subscriber->resetCachedMapDvaMessageChangeObserverRegistration(false);
    Assert(!m_subscriber->m_subscribedToCachedMapChanges);

    m_subscriber->resetCachedMapDvaMessageChangeObserverRegistration(true);
    Assert(m_subscriber->m_subscribedToCachedMapChanges);    
    m_subscriber->resetCachedMapDvaMessageChangeObserverRegistration(true);
    Assert(m_subscriber->m_subscribedToCachedMapChanges);    
}

void DvaMessageChangeSubscriberTestCases::setUpSuite( const std::string & inFile ) 
{
    TA_App::PaTest::setToValidLocation();

    m_subscriber = new DvaMessageChangeSubscriber();
    m_subscriber->addListenerWindow(m_windowStub.getHWnd());
}


void DvaMessageChangeSubscriberTestCases::tearDownSuite() 
{
    m_subscriber->removeListenerWindow(m_windowStub.getHWnd());

    delete m_subscriber;
    m_subscriber = 0;

    TA_App::PaTest::restoreLocation();
}
   

void DvaMessageChangeSubscriberTestCases::setUp( const std::string & INFile , const std::string & OUTFile )
{
    // Keep the window stub free of messages at beginning of each test
    m_windowStub.reset();

}


void DvaMessageChangeSubscriberTestCases::tearDown( const std::string & INFile )
{
}


TestSuite * DvaMessageChangeSubscriberTestCases::suite()
{
    TestSuite * testSuite = new TestSuite( "DvaMessageChangeSubscriberTestCases" );  

#define ADD_UNIT_TEST(function) \
    testSuite->addTest( new TestCaller< DvaMessageChangeSubscriberTestCases >(\
                        #function,\
                        &DvaMessageChangeSubscriberTestCases::##function,this ) );

    //
    // processInternalRunParamChange tests
    //
    ADD_UNIT_TEST(testOnCachedMapChanged);

    //
    // registerWatchOnTypeChange tests
    //
    ADD_UNIT_TEST(testRegisterWatchOnTypeChange);

    //
    // registerWatchOnLabelChange tests
    //
    ADD_UNIT_TEST(testRegisterWatchOnLabelChange);

    //
    // clearAllChangeWatches tests
    //
    ADD_UNIT_TEST(testClearAllChangeWatches);
    
    //
    // hasTypeChangeWatchBeenCleared tests
    //
    ADD_UNIT_TEST(testHasTypeChangeWatchBeenCleared);
    
    //
    // updateMessageQueue tests
    //
    ADD_UNIT_TEST(testUpdateMessageQueue_multipleMessages);
    ADD_UNIT_TEST(testUpdateMessageQueue_singleMessage);

    //
    // getNotificationData Tests
    //
    ADD_UNIT_TEST(testGetNotificationData);
    
    //
    // onSubscriptionSuccessful Tests
    //
    ADD_UNIT_TEST(testOnSubscriptionSuccessful);
    
    //
    // onPreProcessMessage Tests
    //
    ADD_UNIT_TEST(testOnPreProcessMessage);
    
    //
    // displayForeignUpdateWarning Tests
    //
    ADD_UNIT_TEST(testDisplayForeignUpdateWarning);
    
    //
    // clearExpectedConfigUpdate Tests
    //
    ADD_UNIT_TEST(testClearExpectedConfigUpdate);

    //
    // clearFirstMatchingExpectedConfigUpdateKey Tests
    //
    ADD_UNIT_TEST(testClearFirstMatchingExpectedConfigUpdateKey);
    
    //
    // testForReceiptOfFlaggedDvaChange Tests
    //
    ADD_UNIT_TEST(testTestForReceiptOfFlaggedDvaChange);
    
    //
    // resetCachedMapDvaMessageChangeObserverRegistration Tests
    //
    ADD_UNIT_TEST(testResetCachedMapDvaMessageChangeObserverRegistration);

    // non-function specific tests
    ADD_UNIT_TEST(testDvaUpdateReceivedDetection);

#undef ADD_UNIT_TEST


	return testSuite;
}