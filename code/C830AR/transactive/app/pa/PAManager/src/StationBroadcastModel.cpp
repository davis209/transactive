#if defined(_MSC_VER)
#pragma warning(disable : 4786 4146 4018 4250 4290)
#endif // defined _MSC_VER

/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/StationBroadcastModel.cpp $
 * @author:  Ripple
 * @version: $Revision: #5 $
 *
 * Last modification: $DateTime: 2025/02/11 18:06:18 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "app/pa/PAManager/src/stdafx.h"

#include "app/pa/PAManager/src/BroadcastParametersDlg.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/DvaListCtrlModel.h"
#include "app/pa/PAManager/src/IStationBroadcastPage.h"
#include "app/pa/PAManager/src/PaLayoutHelper.h"
#include "app/pa/PAManager/src/PaManagerModel.h"
#include "app/pa/PAManager/src/PaRightsManager.h"
#include "app/pa/PAManager/src/StationBroadcastModel.h"
#include "app/pa/PAManager/src/StationDvaStatusModel.h"
#include "app/pa/PAManager/src/pamanager.h"

#include "bus/generic_gui/src/TransactiveMessage.h" // TD14164
#include "bus/pa/cachedmaps/src/PaStationDvaMessageCachedMap.h"
#include "bus/pa/cachedmaps/src/PaZoneGroupCachedMap.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"

#include "core/exceptions/src/ValueNotSetException.h"

#include <stdlib.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_Base_Core;
using namespace TA_Base_Bus;

static const int MAX_SELECTED_ITEMS = 4;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

StationBroadcastModel::StationBroadcastModel(IStationBroadcastPage& page)
    :
    GenericBroadcastModel(page),
    m_selectedZoneGroupName(""),
    m_dwellInSecs(0),
    m_startTime(0),
    m_stopTime(0),
    m_periodInSecs(0),
    m_isSynchronisedWithTIS(false),
    m_hasChime(true),
    m_cyclicalEnabled(false),
    m_stationBroadcastPage(page),
    // liuyu++ TD8857
    //  m_broadcastMusicSource(TA_Base_Bus::IPAAgentCorbaDef::NoMusic)
    m_broadcastMusicSource(TA_Base_Bus::IPAAgentCorbaDef::NoOccMusic)
    // ++liuyu TD8857
    // Please ensure that this initialisation list corresponds with the state of the
{
    FUNCTION_ENTRY("StationBroadcastModel");

    m_selectedMessages.clear();
    m_selectedZones.clear();

    m_broadcastParametersDlg.Create(IDD_BROADCAST_PARAMETERS_DLG);

    FUNCTION_EXIT;
}

StationBroadcastModel::~StationBroadcastModel()
{
    FUNCTION_ENTRY("~StationBroadcastModel");

    try
    {
        // Must destroy this attached window with this call
        // Before its destructor is called..
        m_broadcastParametersDlg.DestroyWindow();
    }
    catch (...)
    {
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "Caught in destructor");
    }

    FUNCTION_EXIT;
}

void StationBroadcastModel::executeBroadcast(const Keys& selectedZones, const std::string& selectedGroupName)
{
    LOG_SCOPE("StationBroadcastModel::executeBroadcast");
    FUNCTION_ENTRY("executeBroadcast");

    {
        ThreadGuard sync(m_lock);
        m_selectedZones = selectedZones;

        // if it is a schematc group then simplify the name
        if (selectedGroupName.find("Schematic") != std::string::npos)
        {
            m_selectedZoneGroupName = "Schematic";
        }
        else
        {
            m_selectedZoneGroupName = selectedGroupName;
        }
    }

    // Make cursor into hour glass for duration of the execution
    // Note: If the function succeeds and brings up the new dialog,
    // the cursor will automatically be restored to normal..
    CWaitCursor hourGlassCursor;

    try
    {
        switch (m_broadcastType.getValue())
        {
            // Music broadcasts are no longer executed (performed automatically)
            /*
            case TA_Base_Bus::IPAAgentCorbaDef::StationMusic:
                {
                    executeMusic();
                }
                break;
            */
            case TA_Base_Bus::IPAAgentCorbaDef::StationDva:
            {
                executeDVA();
            }
            break;

            case TA_Base_Bus::IPAAgentCorbaDef::StationLive:
            {
                executeLive();
            }
            break;

            default:
                TA_ASSERT(false, "Unrecognised BroadcastType");
                break;
        }
    }
    catch (const TA_Base_Bus::IPAAgentCorbaDef::BroadcastLimitReached&)
    {
        PaErrorHandler::displayError(PaErrorHandler::MAXIMUM_BROADCAST_COUNT_REACHED);
    }
    catch (const TA_Base_Core::BadParameterException&)
    {
        // The request failed (just won't be carried out)
        PaErrorHandler::displayError(PaErrorHandler::INVALID_REQUEST_PARAMS);
    }
    catch (const TA_Base_Core::AccessDeniedException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);
    }
    catch (const TA_Base_Core::ValueNotSetException&)
    {
        // The broadcast type hasn't been set, abort broadcast
        PaErrorHandler::displayError(PaErrorHandler::INVALID_REQUEST_PARAMS);
    }
    catch (const CORBA::Exception&)
    {
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }

    FUNCTION_EXIT;
}

void StationBroadcastModel::executeMusic(const TA_Base_Bus::IPAAgentCorbaDef::EMusicType& musicSource)
{
    LOG_SCOPE(boost::format("StationBroadcastModel::executeMusic[%s]") % st::enum_to_string_value(musicSource));
    FUNCTION_ENTRY("executeMusic");

    ThreadGuard sync(m_lock);

    m_broadcastMusicSource = musicSource;

    try
    {
        if (PaRightsManager::getInstance().hasRight(PaRightsManager::PA_BROADCAST_STATION_MUSIC))
        {
            LOG_DEBUG("executeMusic(): IPAAgentCorbaDef::broadcastStationMusic: %s", nvps(musicSource));

            CORBA_CALL(CachedConfig::getInstance()->getAgentObject(), broadcastStationMusic,
                       (musicSource, CachedConfig::getInstance()->getSessionId().c_str()));
        }
        else
        {
            // No right to perform this operation
            PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);
        }
    }
    catch (const TA_Base_Bus::IPAAgentCorbaDef::BroadcastLimitReached&)
    {
        PaErrorHandler::displayError(PaErrorHandler::MAXIMUM_BROADCAST_COUNT_REACHED);
    }
    catch (const TA_Base_Core::BadParameterException&)
    {
        // The request failed (just won't be carried out)
        PaErrorHandler::displayError(PaErrorHandler::INVALID_REQUEST_PARAMS);
    }
    catch (const TA_Base_Core::AccessDeniedException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);
    }
    catch (const TA_Base_Core::ValueNotSetException&)
    {
        // The broadcast type hasn't been set, abort broadcast
        PaErrorHandler::displayError(PaErrorHandler::INVALID_REQUEST_PARAMS);
    }
    catch (const CORBA::Exception&)
    {
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }

    FUNCTION_EXIT;
}

void StationBroadcastModel::validateAndUpdateStartAndStop(const CTime& startInCTime, const CTime& stopInCTime)
{
    FUNCTION_ENTRY("validateAndUpdateStartAndStop");

    time_t now = time(NULL);
    time_t start = startInCTime.GetTime();
    time_t stop = stopInCTime.GetTime();
    CTime nowInCTime(now);

    // Start two days (2 Jan 1970) after earliest possible time -
    CTime testTime(1970, 1, 2, 0, 0, 0);
    TA_ASSERT(startInCTime > testTime, "start time doesn't have date value");
    TA_ASSERT(stopInCTime > testTime, "start time doesn't have date value");

    if (start < now)
    {
        // Start time before current time - a modification required
        m_broadcastPage.setTimeStartDtp(nowInCTime);

        setStart(now);

        start = now;
    }
    else
    {
        setStart(start);
    }

    if (stop < start)
    {
        // Stop time cannot be before start time, set stop = start
        m_broadcastPage.setTimeStopDtp(start);

        // This is effectively a single broadcast.
        setStart(0);
        setStop(0);
    }
    else
    {
        setStop(stop);
    }

    FUNCTION_EXIT;
}

void StationBroadcastModel::executeDVA()
{
    LOG_SCOPE("StationBroadcastModel::executeDVA");
    FUNCTION_ENTRY("executeDVA");

    std::auto_ptr<ThreadGuard> sync(new ThreadGuard(m_lock));

    KeysIt it;

    TA_Base_Bus::IPAAgentCorbaDef::MessageKeySeq messages;
    messages.length(m_selectedMessages.size());
    int i = 0;

    // TES#748 - Traverse the selected messages list in reverse order, because items
    // at bottom of list (visually) are located at the head of the array (because
    // they were added to the list most recently, and the old items pop off the top
    // of the list) - and we want items at the top of the list to broadcast first
    for (Keys::reverse_iterator messageItr = m_selectedMessages.rbegin();
         messageItr != m_selectedMessages.rend();
         ++messageItr, ++i)
    {
        messages[i] = *messageItr;
    }

    TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones;
    zones.length(m_selectedZones.size());
    i = 0;

    for (it = m_selectedZones.begin(); it != m_selectedZones.end(); ++it, ++i)
    {
        zones[i] = *it;
    }

    // PaLayoutHelper::convertToUTC
    // Use absolute (UTC) time values for start/stop time values
    // passed to agent (the time values should already be in UTC)
    time_t startTime = m_startTime;
    time_t stopTime = m_stopTime;

    if (!m_cyclicalEnabled)
    {
        // Not a cyclical broadcast, so cannot set start / stop time
        startTime = stopTime = 0;
    }

    if (!PaRightsManager::getInstance().hasRight(
        PaRightsManager::PA_BROADCAST_STATION_DVA))
    {
        // No right to make this broadcast
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);

        FUNCTION_EXIT;
        return;
    }

    // TD17589
    DWORD selectedMessageTypeFlags;

    selectedMessageTypeFlags = getSelectedMessageFlags();

    if ((selectedMessageTypeFlags & EMERGENCY_ADHOC) || (selectedMessageTypeFlags & EMERGENCY_PRERECORDED))
    {
        TA_Base_Bus::TransActiveMessage userMsg;
        UINT selectedButton = userMsg.showMsgBox(IDS_UW_610014);

        if (IDNO == selectedButton)
        {
            FUNCTION_EXIT;
            return;
        }
    }

    // TD17589

    LOG_DEBUG("executeDVA(): IPAAgentCorbaDef::broadcastStationDva: %s", nvps(messages, zones, m_dwellInSecs, m_hasChime, startTime, stopTime, m_periodInSecs, m_isSynchronisedWithTIS, m_selectedZoneGroupName));

    std::string broadcastId;
    CORBA_CALL_RETURN(broadcastId, CachedConfig::getInstance()->getAgentObject(), broadcastStationDva, (
        messages,
        zones,
        unsigned short(m_dwellInSecs),
        m_hasChime,
        startTime,
        stopTime,
        unsigned short(m_periodInSecs),
        m_isSynchronisedWithTIS,
        m_selectedZoneGroupName.c_str(),
        CachedConfig::getInstance()->getSessionId().c_str()));

    // We must release the model lock before displaying broadcast - since
    // there may be interactions while the stationBroadcastPage modal dialog displayed
    delete (sync.release());

    LOG_DEBUG("executeDVA(): IPAAgentCorbaDef::broadcastStationDva: %s", nvps(broadcastId));

    // Pop up the dialog
    m_stationBroadcastPage.displayDvaStatusDialog(broadcastId);

    FUNCTION_EXIT;
}

void StationBroadcastModel::executeLive()
{
    LOG_SCOPE("StationBroadcastModel::executeLive");
    FUNCTION_ENTRY("executeLive");

    std::auto_ptr<ThreadGuard> sync(new ThreadGuard(m_lock));

    KeysIt it;
    TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones;

    zones.length(m_selectedZones.size());

    int i = 0;

    for (it = m_selectedZones.begin(); it != m_selectedZones.end(); ++it, ++i)
    {
        zones[i] = *it;
    }

    if (!PaRightsManager::getInstance().hasRightToBroadcastStationLive())
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);
        FUNCTION_EXIT;
        return;
    }

    LOG_DEBUG("executeLive(): IPAAgentCorbaDef::broadcastStationLive: %s", nvps(CachedConfig::getInstance()->getLocalConsoleRecordingSourceId(), zones, m_selectedZoneGroupName));

    std::string broadcastId;
    CORBA_CALL_RETURN(broadcastId, CachedConfig::getInstance()->getAgentObject(), broadcastStationLive, (
        (unsigned char)CachedConfig::getInstance()->getLocalConsoleRecordingSourceId(),
        zones,
        m_selectedZoneGroupName.c_str(),
        CachedConfig::getInstance()->getSessionId().c_str()));

    // We must release the model lock before displaying broadcast - since
    // there may be interactions while the stationBroadcastPage modal dialog displayed
    delete (sync.release());

    LOG_DEBUG("executeDVA(): IPAAgentCorbaDef::broadcastStationLive: %s", nvps(broadcastId));

    m_stationBroadcastPage.displayLiveStatusDialog(broadcastId);

    FUNCTION_EXIT;
}

void StationBroadcastModel::setSelectedMessages(const Keys& newSelectedMessages, bool updateView)
{
    FUNCTION_ENTRY("setSelectedMessages");

    {
        ThreadGuard sync(m_lock);
        m_selectedMessages = newSelectedMessages;
    }

    if (updateView)
    {
        // Update changes visually
        onSelectedMessagesListUpdate();
    }

    FUNCTION_EXIT;
}

void StationBroadcastModel::unsetBroadcastType()
{
    FUNCTION_ENTRY("unsetBroadcastType");

    ThreadGuard sync(m_lock);

    m_broadcastType.resetValue();

    FUNCTION_EXIT;
}

void StationBroadcastModel::setBroadcastType(TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType broadcastType)
{
    FUNCTION_ENTRY("setBroadcastType");

    TA_ASSERT(broadcastType != TA_Base_Bus::IPAAgentCorbaDef::StationMusic, "Use extended function");

    ThreadGuard sync(m_lock);
    m_broadcastType.setValue(broadcastType);

    FUNCTION_EXIT;
}

void StationBroadcastModel::setChime(bool chime)
{
    FUNCTION_ENTRY("setChime");

    ThreadGuard sync(m_lock);
    m_hasChime = chime;

    FUNCTION_EXIT;
}

void StationBroadcastModel::setStisBroadcast(bool stisBroadcast)
{
    FUNCTION_ENTRY("setStisBroadcast");

    ThreadGuard sync(m_lock);
    m_isSynchronisedWithTIS = stisBroadcast;

    FUNCTION_EXIT;
}

void StationBroadcastModel::setDwell(time_t dwell)
{
    FUNCTION_ENTRY("setDwell");

    ThreadGuard sync(m_lock);
    m_dwellInSecs = dwell;

    FUNCTION_EXIT;
}

void StationBroadcastModel::setCyclicalEnabled(bool enabled)
{
    FUNCTION_ENTRY("setCyclicalEnabled");

    ThreadGuard sync(m_lock);
    m_cyclicalEnabled = enabled;

    FUNCTION_EXIT;
}

bool StationBroadcastModel::getCyclicalEnabled()
{
    FUNCTION_ENTRY("getCyclicalEnabled");

    ThreadGuard sync(m_lock);

    FUNCTION_EXIT;
    return m_cyclicalEnabled;
}

void StationBroadcastModel::setPeriod(time_t period)
{
    FUNCTION_ENTRY("setPeriod");

    ThreadGuard sync(m_lock);
    m_periodInSecs = period;

    FUNCTION_EXIT;
}

void StationBroadcastModel::setStart(time_t start)
{
    FUNCTION_ENTRY("setStart");

    ThreadGuard sync(m_lock);
    m_startTime = start;

    FUNCTION_EXIT;
}

void StationBroadcastModel::setStop(time_t stop)
{
    ThreadGuard sync(m_lock);
    m_stopTime = stop;
}

time_t StationBroadcastModel::getStart()
{
    FUNCTION_ENTRY("getStart");

    ThreadGuard sync(m_lock);

    FUNCTION_EXIT;
    return m_startTime;
}

time_t StationBroadcastModel::getStop()
{
    FUNCTION_ENTRY("getStop");

    ThreadGuard sync(m_lock);

    FUNCTION_EXIT;
    return m_stopTime;
}

Keys StationBroadcastModel::getSelectedMessages()
{
    FUNCTION_ENTRY("getSelectedMessages");

    ThreadGuard sync(m_lock);

    FUNCTION_EXIT;
    return m_selectedMessages;
}

TA_IRS_Bus::StationDvaMessageRecordPointers StationBroadcastModel::getNormalMessages(const std::string filterKeyword)
{
    FUNCTION_ENTRY("getNormalMessages");

    TA_Base_Bus::IPAAgentCorbaDef::EDvaSet set;

    switch (CachedConfig::getInstance()->getLocationType())
    {
        case CachedConfig::LOCATION_TYPE_OCC:
            set = TA_Base_Bus::IPAAgentCorbaDef::SetA;
            break;

        case CachedConfig::LOCATION_TYPE_STATION:
            set = TA_Base_Bus::IPAAgentCorbaDef::SetB;
            break;

        case CachedConfig::LOCATION_TYPE_DEPOT:
            set = TA_Base_Bus::IPAAgentCorbaDef::SetD;
            break;

        default:
            TA_ASSERT(false, "Unexpected location type");
    }

    // Ownership transferred
    FUNCTION_EXIT;
    return TA_IRS_Bus::CachedMaps::getInstance()->getClonedPrerecordedMessageRecords(set, DvaListCtrlModel::getNormalType(), filterKeyword);
}

TA_IRS_Bus::StationDvaMessageRecordPointers StationBroadcastModel::getEmergencyMessages(const std::string filterKeyword)
{
    FUNCTION_ENTRY("getEmergencyMessages");

    TA_Base_Bus::IPAAgentCorbaDef::EDvaSet set;

    switch (CachedConfig::getInstance()->getLocationType())
    {
        case CachedConfig::LOCATION_TYPE_OCC:
            set = TA_Base_Bus::IPAAgentCorbaDef::SetA;
            break;

        case CachedConfig::LOCATION_TYPE_STATION:
            set = TA_Base_Bus::IPAAgentCorbaDef::SetB;
            break;

        case CachedConfig::LOCATION_TYPE_DEPOT:
            set = TA_Base_Bus::IPAAgentCorbaDef::SetD;
            break;

        default:
            TA_ASSERT(false, "Unexpected location type");
    }

    // Ownership transferred
    FUNCTION_EXIT;
    return TA_IRS_Bus::CachedMaps::getInstance()->getClonedPrerecordedMessageRecords(set, DvaListCtrlModel::getEmergencyType(), filterKeyword);
}

TA_IRS_Bus::StationDvaMessageRecordPointers StationBroadcastModel::getAdhocMessages()
{
    FUNCTION_ENTRY("getAdhocMessages");

    TA_Base_Bus::IPAAgentCorbaDef::EDvaSet set = CachedConfig::getAdhocSet();

    // Ownership transferred
    FUNCTION_EXIT;
    return TA_IRS_Bus::CachedMaps::getInstance()->getClonedAdhocMessageRecords(set);
}

void StationBroadcastModel::onChangeNormalFilterTb(const std::string& filterKeyword)
{
    FUNCTION_ENTRY("onChangeNormalFilterTb");

    m_stationBroadcastPage.updateNormalMessageList(getNormalMessages(filterKeyword));

    FUNCTION_EXIT;
}

void StationBroadcastModel::onChangeEmergencyFilterTb(const std::string& filterKeyword)
{
    FUNCTION_ENTRY("onChangeEmergencyFilterTb");

    m_stationBroadcastPage.updateEmergencyMessageList(getEmergencyMessages(filterKeyword));

    FUNCTION_EXIT;
}

void StationBroadcastModel::onAdhocListUpdate()
{
    FUNCTION_ENTRY("onAdhocListUpdate");

    m_stationBroadcastPage.updateAdhocMessageList(getAdhocMessages());

    FUNCTION_EXIT;
}

void StationBroadcastModel::onSelectedMessagesListUpdate()
{
    FUNCTION_ENTRY("onSelectedMessagesListUpdate");

    // Clone records for the update of the list control
    Keys keys = getSelectedMessages();

    TA_IRS_Bus::StationDvaMessageRecordPointers records;
    TA_IRS_Bus::StationDvaMessageRecord* clonedRecord;

    for (KeysIt it = keys.begin(); it != keys.end(); ++it)
    {
        // This memory cleaned up by stationBroadCastPage (upon release from updateSelectedMessagesList)
        clonedRecord = new TA_IRS_Bus::StationDvaMessageRecord;
        *clonedRecord = TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(*it);
        records.push_back(clonedRecord);
    }

    m_stationBroadcastPage.updateSelectedMessagesList(records);
    m_stationBroadcastPage.updateExecuteButtonEnabledState();

    FUNCTION_EXIT;
}

void StationBroadcastModel::onClearOneBtn(int nItem)
{
    FUNCTION_ENTRY("onClearOneBtn");

    Keys keys = getSelectedMessages();
    // The messages are sorted added to the list in such a way
    // that they are numbered inversely (that is, '1' visually at the
    // top of list is the last element of this vector array)
    KeysIt it = keys.begin();

    // nItem will be between 0 and size-1
    for (int i = keys.size() - 1; i > nItem; --i)
    {
        it++;
    }

    keys.erase(it);

    setSelectedMessages(keys, true);

    FUNCTION_EXIT;
}

void StationBroadcastModel::onClearAllBtn()
{
    FUNCTION_ENTRY("onClearAllBtn");

    Keys keys;
    setSelectedMessages(keys, true);

    FUNCTION_EXIT;
}

void StationBroadcastModel::addToSelectedMessagesList(unsigned long key, bool isMultiChecked)
{
    FUNCTION_ENTRY("addToSelectedMessagesList");

    Keys newKeys;
    newKeys.push_back(key);

    // Would be better to use push_front here, then lop off the back end..
    if (isMultiChecked)
    {
        Keys currentKeys = getSelectedMessages();
        KeysIt it = currentKeys.begin();
        int count = 1;

        while (count < MAX_SELECTED_ITEMS && it != currentKeys.end())
        {
            unsigned long nextKey = *it;
            newKeys.push_back(nextKey);
            ++it;
            ++count;
        }
    }

    setSelectedMessages(newKeys, true);

    FUNCTION_EXIT;
}

void StationBroadcastModel::enableMultiArea(bool isEnabled)
{
    FUNCTION_ENTRY("enableMultiArea");

    Keys oldKeys = getSelectedMessages();

    if (!isEnabled && oldKeys.size() > 0)
    {
        Keys newKeys;
        // Set the back message of existing messages as sole
        // remaining message (because it's the message at top of list)
        newKeys.push_back(oldKeys.back());
        setSelectedMessages(newKeys, true);
    }

    FUNCTION_EXIT;
}

const CachedConfig::ELocationType StationBroadcastModel::getLocationType() const
{
    FUNCTION_ENTRY("getLocationType");
    FUNCTION_EXIT;
    return CachedConfig::getInstance()->getLocationType();
}

void StationBroadcastModel::validateNewAdhocLabel(LPSTR text)
{
    FUNCTION_ENTRY("validateNewAdhocLabel");

    if (strlen(text) > MAX_ADHOC_LABEL_LEN)
    {
        // Truncate end of string if too long
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "InvalidState", "Adhoc label allowed to be too long");

        text[MAX_ADHOC_LABEL_LEN] = '\0';
    }

    FUNCTION_EXIT;
}

bool StationBroadcastModel::postAdhocLabelChange(unsigned long messageKey, const std::string& label)
{
    LOG_SCOPE("StationBroadcastModel::postAdhocLabelChange");
    FUNCTION_ENTRY("postAdhocLabelChange");

    if (!PaRightsManager::getInstance().hasRight(PaRightsManager::PA_MODIFY_ADHOC_MESSAGE))
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);

        FUNCTION_EXIT;
        return false;
    }

    ThreadGuard sync(m_lock);

    CWaitCursor hourGlassCursor;

    try
    {
        LOG_DEBUG("postAdhocLabelChange(): IPAAgentCorbaDef::setAdhocLabel: %s", nvps(messageKey, label));

        CORBA_CALL(CachedConfig::getInstance()->getAgentObject(), setAdhocLabel, (
            messageKey,
            label.c_str(),
            CachedConfig::getInstance()->getSessionId().c_str()));

        FUNCTION_EXIT;
        return true;
    }
    catch (const TA_Base_Core::BadParameterException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::AD_HOC_UPDATE_LABEL_FAIL);
    }
    catch (const TA_Base_Core::AccessDeniedException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);
    }
    catch (const TA_Base_Core::DatabaseErrorException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::DATABASE_ERROR);
    }
    catch (const TA_Base_Core::ObjectResolutionException&)
    {
        // Comms error (couldn't resolve agent)
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }
    catch (const CORBA::Exception&)
    {
        PaErrorHandler::displayError(PaErrorHandler::AD_HOC_UPDATE_LABEL_FAIL);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::AD_HOC_UPDATE_LABEL_FAIL);
    }

    FUNCTION_EXIT;
    return false;
}

bool StationBroadcastModel::postAdhocTypeChange(unsigned long messageKey, char newMessageType)
{
    LOG_SCOPE("StationBroadcastModel::postAdhocTypeChange");
    FUNCTION_ENTRY("postAdhocTypeChange");

    if (!PaRightsManager::getInstance().hasRight(PaRightsManager::PA_MODIFY_ADHOC_MESSAGE))
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);

        FUNCTION_EXIT;
        return false;
    }

    ThreadGuard sync(m_lock);

    CWaitCursor hourGlassCursor;

    try
    {
        LOG_DEBUG("postAdhocTypeChange(): IPAAgentCorbaDef::setAdhocType: %s", nvps(messageKey, newMessageType));

        // jeffrey++ TD10853
        /*
        CachedConfig::getInstance()->getAgentObject()->setAdhocType(
                                    messageKey,
                                    newMessageType,
                                    CachedConfig::getInstance()->getSessionId().c_str());

        FUNCTION_EXIT;
        return true;
        */
        CORBA::Boolean ret = true;
        CORBA_CALL_RETURN(ret, CachedConfig::getInstance()->getAgentObject(), setAdhocType, (
            messageKey,
            newMessageType,
            CachedConfig::getInstance()->getSessionId().c_str()));
        bool value = static_cast<bool>(ret);

        if (!value)
        {
            // TD14164 ++
            /*AfxMessageBox("Adhoc type does not change.");*/
            TA_Base_Bus::TransActiveMessage userMsg;
            userMsg.showMsgBox(IDS_UE_610018);
            // ++ TD14164
        }

        FUNCTION_EXIT;
        return value;
        // ++jeffrey TD10853
    }
    catch (const TA_Base_Core::BadParameterException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::AD_HOC_UPDATE_TYPE_FAIL);
    }
    catch (const TA_Base_Bus::IPAAgentCorbaDef::FepcCommsFailure&)
    {
        PaErrorHandler::displayError(PaErrorHandler::FEPC_HARDWARE_FAILURE);
    }
    catch (const TA_Base_Core::DatabaseErrorException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::DATABASE_ERROR);
    }
    catch (const TA_Base_Core::AccessDeniedException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);
    }
    catch (const TA_Base_Core::ObjectResolutionException&)
    {
        // Comms error (couldn't resolve agent)
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }
    catch (const CORBA::Exception&)
    {
        PaErrorHandler::displayError(PaErrorHandler::AD_HOC_UPDATE_TYPE_FAIL);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::AD_HOC_UPDATE_TYPE_FAIL);
    }

    FUNCTION_EXIT;
    return false;
}

Keys StationBroadcastModel::getZonesAssociatedWithGroup(const std::string& label)
{
    FUNCTION_ENTRY("getZonesAssociatedWithGroup");

    // Allow exceptions to propogate
    FUNCTION_EXIT;
    return TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneGroupCachedMap().getZonesAssociatedWithGroup(CachedConfig::getInstance()->getEntityLocationKey(),
                                                                                                        label);
}

void StationBroadcastModel::removeSelectedMessagesWithKey(unsigned long key)
{
    FUNCTION_ENTRY("removeSelectedMessagesWithKey");

    Keys keys = getSelectedMessages();

    // Remove all keys with matching type
    keys.remove(key);

    // Expect input corresponds to one of existing selected items
    TA_ASSERT(keys != getSelectedMessages(), "No removal occurred");

    setSelectedMessages(keys, true);

    FUNCTION_EXIT;
}

DWORD StationBroadcastModel::getSelectedMessageFlags()
{
    FUNCTION_ENTRY("getSelectedMessageFlags");

    DWORD resultFlags = 0;

    for (KeysIt it = m_selectedMessages.begin(); it != m_selectedMessages.end(); it++)
    {
        try
        {
            TA_IRS_Bus::StationDvaMessageRecord record =
                TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(*it);

            bool isAdhoc = TA_IRS_Bus::PaStationDvaMessageCachedMap::isAdhocMessageId(
                CachedConfig::getAdhocSet(),
                record.m_id);

            if (DvaListCtrlModel::getEmergencyType() == record.m_type)
            {
                // Emergency category
                resultFlags |= (isAdhoc ? EMERGENCY_ADHOC : EMERGENCY_PRERECORDED);
            }
            else if (DvaListCtrlModel::getNormalType() == record.m_type)
            {
                // Normal category
                resultFlags |= (isAdhoc ? NORMAL_ADHOC : NORMAL_PRERECORDED);
            }
        }
        catch (const TA_Base_Core::TransactiveException&)
        {
            LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Invalid message", "Selected message not found");
        }
    }

    FUNCTION_EXIT;
    return resultFlags;
}

int StationBroadcastModel::getCurrentPriorityEstimationMode()
{
    FUNCTION_ENTRY("getCurrentPriorityEstimationMode");

    // These constants not used in PA Manager
    //    const octet PA_MODE_CD_FCC_NEL                     = 1;
    //    const octet PA_MODE_PCP                            = 2;
    //    const octet PA_MODE_RADIO_EPAX_LIVE                = 9;
    //    const octet PA_MODE_OCC_PSC_BACKGROUND_MUSIC       = 14;
    //    const octet PA_MODE_EVENT_NORMAL_PRERECORDED       = 15;
    //    const octet PA_MODE_EVENT_EMERGENCY_PRERECORDED    = 16;

    using namespace TA_IRS_Bus;

    bool isAtOcc = (CachedConfig::LOCATION_TYPE_OCC ==
                    CachedConfig::getInstance()->getLocationType());

    // TD15312
    bool isAtDepot = (CachedConfig::LOCATION_TYPE_DEPOT ==
                      CachedConfig::getInstance()->getLocationType());
    // TD15312

    if (!m_broadcastType.hasBeenSet())
    {
        // No broadcast type defined, this may correspond to music
        // liuyu++ TD8857
        // if (IPAAgentCorbaDef::NoMusic == m_broadcastMusicSource)

        if ((IPAAgentCorbaDef::NoOccMusic == m_broadcastMusicSource) ||

            (IPAAgentCorbaDef::NoLocalMusic == m_broadcastMusicSource))

            // ++liuyu TD8857
        {
            // When in 'Music OFF' mode, priority is not applicable
            FUNCTION_EXIT;
            return -1;
        }

        else if (isAtDepot)
        {
            FUNCTION_EXIT;
            return IPAAgentCorbaDef::PA_MODE_DCO_BACKGROUND_MUSIC;
        }
        else
        {
            FUNCTION_EXIT;
            return IPAAgentCorbaDef::PA_MODE_OCC_PSC_BACKGROUND_MUSIC;
        }
    }

    switch (m_broadcastType.getValue())
    {
        // Music broadcasts are no longer executed (performed automatically)

        /*

        case IPAAgentCorbaDef::StationMusic:

            break;

        */
        case IPAAgentCorbaDef::StationLive:
            if (isAtOcc)
            {
                FUNCTION_EXIT;
                return IPAAgentCorbaDef::PA_MODE_OCC_LIVE_ANNOUNCEMENT;
            }
            // TD15312
            else if (isAtDepot)
            {
                FUNCTION_EXIT;
                return IPAAgentCorbaDef::PA_MODE_DCO_LIVE_ANNOUNCEMENT;
            }
            // TD15312
            else
            {
                FUNCTION_EXIT;
                return IPAAgentCorbaDef::PA_MODE_PSC_LIVE_ANNOUNCEMENT;
            }

            break;

        case IPAAgentCorbaDef::StationDva:

            // The currently selected messages may be mixed, in which case

            // we take the message representing the most dominant state -

            // order of dominance is as follows:

            // Emergency Adhoc

            // Emergency Prerecorded

            // Normal adhoc

            // Normal Prerecorded

            // - When broadcasting in zone, hardware takes highest calculated priority

            //      (this is where ordering shown above is derived from)

            DWORD selectedMessageTypeFlags;

            selectedMessageTypeFlags = getSelectedMessageFlags();

            if (selectedMessageTypeFlags & EMERGENCY_ADHOC)
            {
                // TD15312
                /*

                FUNCTION_EXIT;
                return isAtOcc ? IPAAgentCorbaDef::PA_MODE_OCC_EMERGENCY_ADHOC :

                                 IPAAgentCorbaDef::PA_MODE_PSC_EMERGENCY_ADHOC;
                */
                if (isAtOcc)
                {
                    FUNCTION_EXIT;
                    return IPAAgentCorbaDef::PA_MODE_OCC_EMERGENCY_ADHOC;
                }
                else if (isAtDepot)
                {
                    FUNCTION_EXIT;
                    return IPAAgentCorbaDef::PA_MODE_DCO_EMERGENCY_ADHOC;
                }
                else
                {
                    FUNCTION_EXIT;
                    return IPAAgentCorbaDef::PA_MODE_PSC_EMERGENCY_ADHOC;
                }

                // TD15312
            }
            else if (selectedMessageTypeFlags & EMERGENCY_PRERECORDED)
            {
                // TD15312
                /*

                FUNCTION_EXIT;
                return isAtOcc ? IPAAgentCorbaDef::PA_MODE_OCC_EMERGENCY_PRERECORDED :

                                 IPAAgentCorbaDef::PA_MODE_PSC_EMERGENCY_PRERECORDED;
                */
                if (isAtOcc)
                {
                    FUNCTION_EXIT;
                    return IPAAgentCorbaDef::PA_MODE_OCC_EMERGENCY_PRERECORDED;
                }
                else if (isAtDepot)
                {
                    FUNCTION_EXIT;
                    return IPAAgentCorbaDef::PA_MODE_DCO_EMERGENCY_PRERECORDED;
                }
                else
                {
                    FUNCTION_EXIT;
                    return IPAAgentCorbaDef::PA_MODE_PSC_EMERGENCY_PRERECORDED;
                }

                // TD15312
            }
            else if (selectedMessageTypeFlags & NORMAL_ADHOC)
            {
                // TD15312
                /*

                FUNCTION_EXIT;
                return isAtOcc ? IPAAgentCorbaDef::PA_MODE_OCC_NORMAL_ADHOC :

                                 IPAAgentCorbaDef::PA_MODE_PSC_NORMAL_ADHOC;
                */
                if (isAtOcc)
                {
                    FUNCTION_EXIT;
                    return IPAAgentCorbaDef::PA_MODE_OCC_NORMAL_ADHOC;
                }
                else if (isAtDepot)
                {
                    FUNCTION_EXIT;
                    return IPAAgentCorbaDef::PA_MODE_DCO_NORMAL_ADHOC;
                }
                else
                {
                    FUNCTION_EXIT;
                    return IPAAgentCorbaDef::PA_MODE_PSC_NORMAL_ADHOC;
                }

                // TD15312
            }
            else if (selectedMessageTypeFlags & NORMAL_PRERECORDED)
            {
                // TD15312
                /*

                FUNCTION_EXIT;
                return isAtOcc ? IPAAgentCorbaDef::PA_MODE_OCC_NORMAL_PRERECORDED :

                                 IPAAgentCorbaDef::PA_MODE_PSC_NORMAL_PRERECORDED;
                */
                if (isAtOcc)
                {
                    FUNCTION_EXIT;
                    return IPAAgentCorbaDef::PA_MODE_OCC_NORMAL_PRERECORDED;
                }
                else if (isAtDepot)
                {
                    FUNCTION_EXIT;
                    return IPAAgentCorbaDef::PA_MODE_DCO_NORMAL_PRERECORDED;
                }
                else
                {
                    FUNCTION_EXIT;
                    return IPAAgentCorbaDef::PA_MODE_PSC_NORMAL_PRERECORDED;
                }

                // TD15312
            }
            else
            {
                // This means there are no selected messages
                // Or none that had a recognized type
                FUNCTION_EXIT;
                return -1;
            }

            break;

        default:

            TA_ASSERT(FALSE, "Invalid getCurrentPriorityEstimationMode request made");

            FUNCTION_EXIT;
            return -1;
    }
}

bool StationBroadcastModel::testCanObtainLock()
{
    FUNCTION_ENTRY("testCanObtainLock");

    TA_Base_Core::ThreadGuard sync(m_lock);

    FUNCTION_EXIT;
    return true;
}
