/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DvaMessageChangeSubscriber.cpp $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * DvaMessageChangeSubscriber subscriber to listen out for changes to
 * DVA message changes
 *
 */

#include "app/pa/PAManager/src/StdAfx.h"
#include "app/pa/PAManager/src/DvaMessageChangeSubscriber.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "bus/pa/cachedmaps/src/PaStationDvaMessageCachedMap.h"
#include "app/pa/PAManager/src/CachedMapLoader.h"
#include "app/pa/PAManager/src/CachedConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_IRS_Bus;

/*
DvaMessageChangeSubscriber* g_subscriberTestObject = 0;

DWORD WINAPI MessageName_Tester(void* val)
{
    try
    {
        for(;;)
        {
            // Send a whole bunch of messages through
            if (g_subscriberTestObject)
            {
                for (int i = 0; i < 1000; i ++)
                {
                TA_Base_Bus::IPAAgentCorbaDef::DvaMessagesUpdateData data;
                data.length(1);
                data[0].messageKey = 1 + rand() % 10;
                data[0].messageId = 500 + rand() % 500;
                data[0].locationKey = 4;
                data[0].messageLabel = CORBA::string_dup("Test message");
                data[0].messageType = 'E';
                data[0].tisKey = -1;

                g_subscriberTestObject->DEBUG_processMessage(data);
                }
            }
            Sleep(1000);
        }
    }
    catch (...)
    {
    }
    ExitThread(0);
    return 0;
}

DWORD g_change_threadID = 0;
HANDLE g_change_hThread = CreateThread(0, 0, MessageName_Tester, 0, 0, &g_change_threadID);
*/
////////////////////

DvaMessageChangeSubscriber::DvaMessageChangeSubscriber()
    :
    GenericMessageSubscriber<DvaMessagesUpdateData, DvaMessageUpdateData>(WM_PA_DVA_MESSAGE_UPDATE, CachedConfig::getInstance()->getEntityLocationKey()),
    m_subscribedToCachedMapChanges(false)
{
}

DvaMessageChangeSubscriber::~DvaMessageChangeSubscriber()
{
    resetCachedMapDvaMessageChangeObserverRegistration(false);
}

const TA_Base_Core::MessageType& DvaMessageChangeSubscriber::getMessageType() const
{
    return TA_Base_Core::PAAgentComms::DvaMessagesUpdate;
}

void DvaMessageChangeSubscriber::resetCachedMapDvaMessageChangeObserverRegistration(bool subscribe)
{
    if (!subscribe)
    {
        // Unsubscribe (if not done so already)
        if (m_subscribedToCachedMapChanges)
        {
            TA_IRS_Bus::CachedMaps::getInstance()->deRegisterPaStationDvaMessageChangeObserver(this);
            m_subscribedToCachedMapChanges = false;
        }
    }
    else
    {
        // Subscribe (if not already subscribed)
        if (!m_subscribedToCachedMapChanges)
        {
            TA_IRS_Bus::CachedMaps::getInstance()->registerPaStationDvaMessageChangeObserver(this);
            m_subscribedToCachedMapChanges = true;
        }
    }
}

void DvaMessageChangeSubscriber::onSubscriptionSuccessful()
{
    // Begin listening to changes from cached map (as well as from PAAgent)
    resetCachedMapDvaMessageChangeObserverRegistration(true);

    // Each time AFTER we begin a new subscription, we must refresh the
    // messages from the database - to ensure cachedMaps continues to be up to date
    // (if we did this before subscribing, might miss some updates)
    CachedMapLoader::refreshKeyToStationDvaMessageRecordMap(
        CachedConfig::getInstance()->getEntityLocationKey());
}

void DvaMessageChangeSubscriber::onPreProcessMessage(const DvaMessagesUpdateData& updateData)
{
    // Post the update to cached maps (to keep it up to date)
    // This is ideally done before (pre) the message notification being sent out
    // because if the window acknowledges it's received bad data, it will
    // possibly attempt to reload from TA_IRS_Bus::CachedMaps (so update TA_IRS_Bus::CachedMaps first)
    bool changesMade = TA_IRS_Bus::CachedMaps::getInstance()->processDvaMessageUpdates(updateData);

    if (1 == updateData.length())
    {
        // For the time being, we only pop up messages, and check for
        // receipt of flagged changes for packets with single element
        // (to avoid excessive processing, also because this is true of PAAgent)
        // This is probably preferable (when a hardware update occurs and we get
        //  a bundle of updates, could gets lots of 'external update' notifications)
        testForReceiptOfFlaggedDvaChange(updateData[0], changesMade);
    }
    else
    {
        // TD #2269
        // If there is more than one update, indicates a batch hardware (or config editor)
        // change - notify the operator of this
        PaErrorHandler::displayError(PaErrorHandler::NEW_DVA_LIBRARY);
    }
}

DvaMessageUpdateData DvaMessageChangeSubscriber::getNotificationData(const DvaMessagesUpdateData& updateData)
{
    // DVA Message changes only contain a set of KEY information
    DvaMessageUpdateData result;

    for (unsigned int i = 0; i < updateData.length(); i++)
    {
        result.insert(updateData[i].m_pKey);
    }

    return result;
}

void DvaMessageChangeSubscriber::updateMessageQueue(
    UpdateBuffer<DvaMessageUpdateData>& msgQueue,
    const std::set<HWND>& windowHwnds,
    const DvaMessageUpdateData& updateData) const
{
    std::set<HWND> windowsRequiringNewNotification;

    for (std::set<HWND>::const_iterator itr = windowHwnds.begin();
         itr != windowHwnds.end();
         itr++)
    {
        HWND currentWnd = (*itr);

        unsigned long numInQueue = msgQueue.getNumberOfQueuedUpdates(currentWnd);

        // We check to see if window has a update waiting to be collected
        // If it doesn't, we simply add the new update
        if (numInQueue > 0)
        {
            // The DVA Message change subscriber should never have more
            // than one item queued (it always merges into existing queue if present)
            TA_ASSERT(1 == numInQueue, "Unexpected number of queued items");

            // May add to existing queue item
            LockedPacket<DvaMessageUpdateData> lockedData =
                msgQueue.obtainLockedDataOnFirstItemInQueue(currentWnd);

            // Add in all of the additional key data
            for (DvaMessageUpdateData::const_iterator itr = updateData.begin();
                 itr != updateData.end();
                 itr++)
            {
                // It is possible for the new data to already be present -
                // in which case we save time double processing the updated key
                lockedData.getMutableData().insert(*itr);
            }
        }
        else
        {
            // Otherwise must add in new item to queue (in which case we must
            // send out a notification)
            windowsRequiringNewNotification.insert(currentWnd);
        }
    }

    // If any windows didn't have items left in their queues, send out a new notification
    if (windowsRequiringNewNotification.size() > 0)
    {
        addItemToQueueAndNotifyWindows(msgQueue, windowsRequiringNewNotification, updateData);
    }
}

void DvaMessageChangeSubscriber::onPostProcessMessage(const DvaMessagesUpdateData& updateData)
{
    // DO NOTHING
    return;
}

void DvaMessageChangeSubscriber::onCachedMapChanged(unsigned long pKey, bool changesMade)
{
    // Some external source (ie config editor) has made a change
    // to the DVA messages, manually dispatch to update to windows
    // attached to this subscriber
    FUNCTION_ENTRY("DvaMessageChangeSubscriber::onCachedMapChange");

    // If the input key isn't found in expectedConfigUpdates, then this change
    // wasn't a local change (local changes echo the update notification)
    if (!clearFirstMatchingExpectedConfigUpdateKey(pKey))
    {
        // TD #2269
        // We weren't the source of this update, tell operator about it
        // - only need to display the error if changes were made

        // remove the popup message since when first time to connect to
        // hardware, there are a lot of pre-defined messages
        // manager side will have a lot of 'pop-up' messages
        //displayForeignUpdateWarning(changesMade);
        LOG_DEBUG("DVA message %i has been changed in CacheMap", pKey);
    }

    try
    {
        DvaMessageUpdateData data;
        data.insert(pKey);

        // This will send the update to all the windows
        queueDataAndDispatchNotification(data);
    }
    catch (...)
    {
        LOG(SourceInfo,
            TA_Base_Core::DebugUtil::ExceptionCatch,
            "Unknown",
            "Caught in DvaMessageChangeSubscriber::onCachedMapChanged");

        // There shouldn't be any exceptions thrown from the above..
        PaErrorHandler::displayError(PaErrorHandler::DVA_MESSAGE_UPDATE_FAILURE);
    }

    FUNCTION_EXIT;
}

void DvaMessageChangeSubscriber::testForReceiptOfFlaggedDvaChange(
    const TA_Base_Bus::IPAAgentCorbaDef::StationDvaMessageRecord& item,
    bool changesDetected)
{
    /// For subscriber updates only (cached map updates don't get in here)
    /// check if the updated data matches any of the receipt watches
    TA_Base_Core::ThreadGuard sync(m_receiveCheckLock);

    bool isAdhocMessage = PaStationDvaMessageCachedMap::isAdhocMessageId(
        CachedConfig::getAdhocSet(),
        item.m_id);

// Flag to indicate if change was recognized as local ad-hoc update
    bool changeWasLocal = false;
    bool typeChangeReceived = false;

    if (m_receiveTypeWatch.hasBeenSet())
    {
        // Inspect all items in update
        if (m_receiveTypeWatch.getValue().messageKey == item.m_pKey &&
            m_receiveTypeWatch.getValue().changedData == item.m_type)
        {
            // Found the update we've been waiting for, clear watch
            m_receiveTypeWatch.resetValue();

            changeWasLocal = true;
            typeChangeReceived = true;
        }
    }

    // We are only allowed to clear one watch at a time (since we can only
    // set one at a time) - if we allow them to be cleared all at once, then
    // we incorrectly identify the change as foreign, and display
    // the message DVA_MESSAGE_UPDATE_RECEIVED_EXTERNALLY to the local terminal (which is wrong)
    if (!typeChangeReceived)
    {
        if (m_receiveLabelWatch.hasBeenSet())
        {
            // Inspect all items in update
            if (m_receiveLabelWatch.getValue().messageKey == item.m_pKey &&
                m_receiveLabelWatch.getValue().changedData == std::string(item.m_label))
            {
                // Found the update we've been waiting for, clear watch
                m_receiveLabelWatch.resetValue();

                changeWasLocal = true;
            }
        }
    }

    // TD #2269: Display message box regardless of if ad-hoc or not..
    // If change is local, suppress displaying message..
    if (/*isAdhocMessage && */!changeWasLocal)
    {
        if (isAdhocMessage)
        {
            // Notify the user that an ad-hoc update has been received from another operator
            displayForeignUpdateWarning(changesDetected);
        }
        else
        {
            PaErrorHandler::displayError(PaErrorHandler::NEW_DVA_LIBRARY);
        }
    }
}

void DvaMessageChangeSubscriber::clearAllChangeWatches()
{
    clearTypeChangeWatch();
    clearLabelChangeWatch();
}

bool DvaMessageChangeSubscriber::hasTypeChangeWatchBeenCleared()
{
    TA_Base_Core::ThreadGuard sync(m_receiveCheckLock);
    return !m_receiveTypeWatch.hasBeenSet();
}

bool DvaMessageChangeSubscriber::hasLabelChangeWatchBeenCleared()
{
    TA_Base_Core::ThreadGuard sync(m_receiveCheckLock);
    return !m_receiveLabelWatch.hasBeenSet();
}

void DvaMessageChangeSubscriber::clearTypeChangeWatch()
{
    TA_Base_Core::ThreadGuard sync(m_receiveCheckLock);

    // May clear the corresponding config update key
    if (m_receiveTypeWatch.hasBeenSet())
    {
        clearExpectedConfigUpdate(CHANGE_TYPE, &m_receiveTypeWatch.getValue().messageKey);
    }

    m_receiveTypeWatch.resetValue();
}

void DvaMessageChangeSubscriber::clearLabelChangeWatch()
{
    TA_Base_Core::ThreadGuard sync(m_receiveCheckLock);

    // May clear the corresponding config update key
    if (m_receiveLabelWatch.hasBeenSet())
    {
        clearExpectedConfigUpdate(CHANGE_LABEL, &m_receiveLabelWatch.getValue().messageKey);
    }

    m_receiveLabelWatch.resetValue();
}

bool DvaMessageChangeSubscriber::clearFirstMatchingExpectedConfigUpdateKey(unsigned long pKey)
{
    TA_Base_Core::ThreadGuard sync(m_receiveCheckLock);

    for (ExpectedConfigUpdates::iterator itr = m_expectedConfigUpdates.begin();
         itr != m_expectedConfigUpdates.end();
         itr++)
    {
        if (itr->second == pKey)
        {
            m_expectedConfigUpdates.erase(itr);
            return true;
        }
    }

    return false;
}

void DvaMessageChangeSubscriber::addExpectedConfigUpdate(EWatchType type, unsigned long pKey)
{
    // When a 'Config Editor' update comes through for this key
    // we don't need to display 'External update received'
    m_expectedConfigUpdates.push_back(ExpectedConfigUpdate(type, pKey));
}

void DvaMessageChangeSubscriber::clearExpectedConfigUpdate(EWatchType type, const unsigned long* pKey)
{
    bool itemErased;

    do
    {
        itemErased = false;

        for (ExpectedConfigUpdates::iterator itr = m_expectedConfigUpdates.begin();
             itr != m_expectedConfigUpdates.end();
             itr++)
        {
            if (itr->first == type)
            {
                if (0 == pKey || itr->second == *pKey)
                {
                    // Matching item, may clear
                    m_expectedConfigUpdates.erase(itr);

                    itemErased = true;
                    break;
                }
            }
        }
    }
    while (itemErased);

    // May not find any expected updates (if pKey specified), condition can occur in normal operation:
    // We change a message locally, the config editor change comes through, but the Agent
    // update never comes through  - when we go to manually clear the Agent watch
    // it comes into this function to clear the associated config update - but it's
    // already been cleared as we've received it
    // What happens if the agent update is received but the config editor update isn't?
    // - next time we register a watch, we clear all queued config updates in list for
    //  the specified watch (and if we receive them later - the check in place to see
    //  if the update actually caused a change will inhibit the warning from being displayed)
}

void DvaMessageChangeSubscriber::registerWatchOnTypeChange(unsigned long pKey,
                                                           unsigned char type)
{
    TA_Base_Core::ThreadGuard sync(m_receiveCheckLock);

    TA_ASSERT(!m_receiveTypeWatch.hasBeenSet(), "Type watch already set");

    // Clear any existing watches with CHANGE_TYPE
    clearExpectedConfigUpdate(CHANGE_TYPE);

    TypeChange tc;
    tc.messageKey = pKey;
    tc.changedData = type;
    m_receiveTypeWatch.setValue(tc);

    addExpectedConfigUpdate(CHANGE_TYPE, pKey);
}

void DvaMessageChangeSubscriber::registerWatchOnLabelChange(unsigned long pKey,
                                                            const std::string& label)
{
    TA_Base_Core::ThreadGuard sync(m_receiveCheckLock);

    TA_ASSERT(!m_receiveLabelWatch.hasBeenSet(), "Label watch already set");

    // Clear any existing watches with CHANGE_TYPE
    clearExpectedConfigUpdate(CHANGE_LABEL);

    LabelChange tc;
    tc.messageKey = pKey;
    tc.changedData = label;
    m_receiveLabelWatch.setValue(tc);

    addExpectedConfigUpdate(CHANGE_LABEL, pKey);
}

void DvaMessageChangeSubscriber::displayForeignUpdateWarning(bool changesDetected)
{
    // Perform a kind of last-barrier check to confirm we're correctly displaying 'changesDetected'
    // In real world situation, we can get two consecutive update notifications, but both
    // are processed before user can confirm it (so they don't have the hassle of having two message boxes pop up).
    // But in case there are delays, this final check will stop a message popping up for a second time..
    if (changesDetected)
    {
        // changesMade will be false when:
        // 1) the config editor update comes through from the PA Agent
        //      (at a foreign terminal) after already receiving an update via the Agent DvaMessageSubscriber
        //      (because local cache already updated to match)
        // 2) As above, but in reverse (config editor update received first)
        PaErrorHandler::displayError(PaErrorHandler::DVA_MESSAGE_UPDATE_RECEIVED_EXTERNALLY);
    }
}
