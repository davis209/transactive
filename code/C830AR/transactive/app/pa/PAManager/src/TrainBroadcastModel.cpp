/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainBroadcastModel.cpp $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * TrainBroadcastModel models behaviour for the train broadcast GUI tab of the PA Manager GUI
 */

#include "app/pa/PAManager/src/stdafx.h"

#include "app/pa/PAManager/src/BroadcastParametersDlg.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/ITrainBroadcastPage.h"
#include "app/pa/PAManager/src/PaManagerModel.h"
#include "app/pa/PAManager/src/PaRightsManager.h"
#include "app/pa/PAManager/src/TrainBroadcastModel.h"
#include "app/pa/PAManager/src/TrainDvaStatusModel.h"
#include "app/pa/PAManager/src/pamanager.h"

#include "bus/trains/TrainAgentCorba/idl/src/ITrainExceptionsCorba.h"
#include "bus/trains/TrainCommonLibrary/src/PaTypes.h"
#include "bus/trains/TrainCommonLibrary/src/InvalidParameterException.h"
#include "bus/trains/TrainSelectionComponent/src/TrainSelectorGUI.h"

#include "core/data_access_interface/pa_4669/src/PaTrainDvaMessageAccessFactory.h"
#include "core/exceptions/src/PasExceptions.h"
#include "core/exceptions/src/ValueNotSetException.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "core/utilities/src/DebugUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <stdlib.h>

TrainBroadcastModel::TrainBroadcastModel(ITrainBroadcastPage& pageView,
                                         TA_IRS_Bus::LocationCache::EStationType initialStationMode)
    : GenericBroadcastModel(pageView),
    m_trainBroadcastPage(pageView),
    m_inboundUpdateBuffer(WM_PA_TRAIN_INBOUND_LIST_UPDATE),
    m_outboundUpdateBuffer(WM_PA_TRAIN_OUTBOUND_LIST_UPDATE),
    m_duration(0),
    m_period(0),
    m_cyclicalEnabled(false),
    m_trainSelectionSubscriber(0),
    m_currentStationMode(initialStationMode),
    m_hasOverrideOption(false),
    m_selectedStaticGroup("")
{
    FUNCTION_ENTRY("TrainBroadcastModel");
    FUNCTION_EXIT;
}

TrainBroadcastModel::~TrainBroadcastModel()
{
    FUNCTION_ENTRY("~TrainBroadcastModel");

    TA_ASSERT(::IsWindow(getPageHwnd()), "Window no longer valid while in destructor");

    TA_ASSERT(0 == m_trainSelectionSubscriber, "Subscriber should be destroyed in enableUpdateListener");

    FUNCTION_EXIT;
}

void TrainBroadcastModel::attemptTrainSubscriberConnection()
{
    FUNCTION_ENTRY("attemptTrainSubscriberConnection");

    TA_ASSERT(0 == m_trainSelectionSubscriber, "Object already exists");

    unsigned long locationKey = CachedConfig::getInstance()->getEntityLocationKey();

    try
    {
        // Add TA_TrainSelectionComponent, and dependencies on this and
        // TrainAgentCorbaDef (which already exists in build all workspace)
        m_trainSelectionSubscriber = new TA_IRS_Bus::TrainSelector(*this, locationKey, m_currentStationMode);
    }
    catch (const TA_Base_Core::DatabaseException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::DATABASE_ERROR);
    }
    catch (const TA_Base_Core::DataException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::INVALID_CONFIG_DATA);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // Usually ObjectResolutionNotSet error
        PaErrorHandler::displayError(PaErrorHandler::INVALID_CONFIG_DATA);
    }
    catch (...)
    {
        // bug fix: manager should be able to run without train agent
    }

    FUNCTION_EXIT;
}

bool TrainBroadcastModel::isTrainSubscriberConnected() const
{
    FUNCTION_ENTRY("isTrainSubscriberConnected");

    // If object exists, assume it's connected
    FUNCTION_EXIT;
    return 0 != m_trainSelectionSubscriber;
}

void TrainBroadcastModel::enableUpdateListener(bool bEnable)
{
    FUNCTION_ENTRY("enableUpdateListener");

    // Should only be called while the page window is valid (OnInitDialog/DestroyWindow)
    TA_ASSERT(getPageHwnd() != 0, "Internal check failed: Incorrect startup/shutdown ordering");

    if (bEnable)
    {
        // Only attempt to subscribe if not already subscribed
        if (!isTrainSubscriberConnected())
        {
            attemptTrainSubscriberConnection();
        }
    }
    else
    {
        // Shut down listener
        if (m_trainSelectionSubscriber)
        {
            delete m_trainSelectionSubscriber;
            m_trainSelectionSubscriber = 0;
        }
    }

    FUNCTION_EXIT;
}

void TrainBroadcastModel::updateInboundList(TA_IRS_Bus::ITrainSelectorGUI::EAction action,
                                            const TA_IRS_Bus::TrainInformationTypes::TrainDetails& trainDetails)
{
    FUNCTION_ENTRY("updateInboundList");

    // Removed [the debug/testing trainselector causes this assert to trip]
    //TA_ASSERT(trainDetails.isInbound, "Updating in bound list with outbound train");

    m_inboundUpdateBuffer.bufferUpdate(getPageHwnd(), action, trainDetails);

    FUNCTION_EXIT;
}

void TrainBroadcastModel::updateOutboundList(TA_IRS_Bus::ITrainSelectorGUI::EAction action,
                                             const TA_IRS_Bus::TrainInformationTypes::TrainDetails& trainDetails)
{
    FUNCTION_ENTRY("updateOutboundList");

    // Removed [the debug/testing trainselector causes this assert to trip]
    //TA_ASSERT(!trainDetails.isInbound, "Updating outbound list with inbound train");

    m_outboundUpdateBuffer.bufferUpdate(getPageHwnd(), action, trainDetails);

    FUNCTION_EXIT;
}

TrainListUpdateContainer::BufferedItem TrainBroadcastModel::fetchUpdatedInboundTrainData()
{
    FUNCTION_ENTRY("fetchUpdatedInboundTrainData");
    FUNCTION_EXIT;
    return m_inboundUpdateBuffer.fetchUpdate(getPageHwnd());
}

TrainListUpdateContainer::BufferedItem TrainBroadcastModel::fetchUpdatedOutboundTrainData()
{
    FUNCTION_ENTRY("fetchUpdatedOutboundTrainData");
    FUNCTION_EXIT;
    return m_outboundUpdateBuffer.fetchUpdate(getPageHwnd());
}

void TrainBroadcastModel::updateSelectionCombo(const std::vector<std::string>& trainListNames)
{
    FUNCTION_ENTRY("updateSelectionCombo");

    // Post notification of new list group data - only need to store latest update
    TA_Base_Core::ThreadGuard sync(m_dynamicGroupListLock);

    m_dynamicGroupList = trainListNames;

    // Change notification sent out to window
    ::PostMessage(getPageHwnd(), WM_PA_TRAIN_DYN_LIST_UPDATE, 0, 0);

    FUNCTION_EXIT;
}

std::vector<std::string> TrainBroadcastModel::fetchDynamicGroupList()
{
    FUNCTION_ENTRY("fetchDynamicGroupList");

    TA_Base_Core::ThreadGuard sync(m_dynamicGroupListLock);

    FUNCTION_EXIT;
    return m_dynamicGroupList;
}

HWND TrainBroadcastModel::getPageHwnd() const
{
    FUNCTION_ENTRY("getPageHwnd");

    // Shouldn't be requesting handle if page no longer exists
    TA_ASSERT(m_trainBroadcastPage.getWindow(), "Page no longer exists");
    TA_ASSERT(m_trainBroadcastPage.getWindow()->m_hWnd != 0, "Page no longer exists");

    FUNCTION_EXIT;
    return m_trainBroadcastPage.getWindow()->m_hWnd;
}

void TrainBroadcastModel::unsetBroadcastType()
{
    FUNCTION_ENTRY("unsetBroadcastType");

    ThreadGuard sync(m_lock);
    m_broadcastType.resetValue();

    FUNCTION_EXIT;
}

void TrainBroadcastModel::setBroadcastType(TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType broadcastType)
{
    FUNCTION_ENTRY("setBroadcastType");

    ThreadGuard sync(m_lock);
    m_broadcastType.setValue(broadcastType);

    FUNCTION_EXIT;
}

void TrainBroadcastModel::executeBroadcast(const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trainCoverage)
{
    FUNCTION_ENTRY("executeBroadcast");

    m_trainCoverage = trainCoverage;

    // Make cursor into hour glass for duration of the execution
    // Note: If the function succeeds and brings up the new dialog,
    // the cursor will automatically be restored to normal..
    CWaitCursor hourGlassCursor;

    try
    {
        switch (m_broadcastType.getValue())
        {
            case TA_Base_Bus::IPAAgentCorbaDef::TrainDva:
                executeDva();
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::TrainLive:
                executeLive();
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
    catch (const CORBA::Exception&)
    {
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }
    catch (const TA_Base_Core::InvalidPasConfigurationException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::BROADCAST_TRAIN_VERSION_FAILURE);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }

    FUNCTION_EXIT;
}

void TrainBroadcastModel::executeDva()
{
    LOG_SCOPE("TrainBroadcastModel::executeDva");
    FUNCTION_ENTRY("executeDva");

    std::auto_ptr<ThreadGuard> sync(new ThreadGuard(m_lock));

    short libraryVersion            = 0;

    try
    {
        libraryVersion = (short)TA_Base_Core::PaTrainDvaMessageAccessFactory::getInstance().getLibraryVersion();
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        TA_THROW(TA_Base_Core::InvalidPasConfigurationException());
    }

    TA_Base_Bus::ITrainPaCorba::MessageSchedule timeInterval = {m_period, m_duration};

    if (!m_cyclicalEnabled)
    {
        // Not a cyclical broadcast, so cannot set start / stop time
        timeInterval.period = 0;
        timeInterval.duration = 0;
    }

    if (!PaRightsManager::getInstance().hasRight(PaRightsManager::TRAIN_PA_PRERECORDED_ANNOUNCEMENT))
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);

        FUNCTION_EXIT;
        return;
    }

    std::string broadcastId;
    CORBA_CALL_RETURN(broadcastId, CachedConfig::getInstance()->getAgentObject(), broadcastTrainDva, (m_trainCoverage,
                                                                                                      m_selectedMessage.getValue(),
                                                                                                      libraryVersion,
                                                                                                      timeInterval,
                                                                                                      m_hasOverrideOption,
                                                                                                      CachedConfig::getInstance()->getSessionId().c_str()));
    // We must release the model lock before displaying broadcast - since
    // there may be interactions while the TrainBroadcastPage modal dialog displayed
    delete (sync.release());

    // Bring up the status dialog
    m_trainBroadcastPage.displayDvaStatusDialog(broadcastId);

    FUNCTION_EXIT;
}

void TrainBroadcastModel::executeLive()
{
    LOG_SCOPE("TrainBroadcastModel::executeLive");
    FUNCTION_ENTRY("executeLive");

    if (!PaRightsManager::getInstance().hasRightToBroadcastTrainLive())
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);

        FUNCTION_EXIT;
        return;
    }

    // find the static group TSI from the name
    std::string groupTsi = "";

    try
    {
        TA_IRS_Bus::RadioGroupTypes::TrainRadioGroup selectedGroup = m_trainSelectionSubscriber->getRadioGroupForName(m_selectedStaticGroup);

        groupTsi = selectedGroup.groupTsi;
    }
    catch (TA_IRS_Bus::InvalidParameterException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::InvalidParameterException", e.what());
    }

    std::string broadcastId;
    {
        // We must release the model lock before displayLiveStatusDialog - since
        // there may be interactions while the TrainBroadcastPage modal dialog displayed
        // (so generate lock within this scope so it's destroyed)
        ThreadGuard sync(m_lock);

        CORBA_CALL_RETURN(broadcastId, CachedConfig::getInstance()->getAgentObject(), broadcastTrainLive, (m_trainCoverage,
                                                                                                           m_hasOverrideOption,
                                                                                                           groupTsi.c_str(),
                                                                                                           CachedConfig::getInstance()->getSessionId().c_str()));
    }

    // Bring up the status dialog
    m_trainBroadcastPage.displayLiveStatusDialog(broadcastId);

    FUNCTION_EXIT;
}

void TrainBroadcastModel::setSelectedMessage(int id)
{
    FUNCTION_ENTRY("setSelectedMessage");

    TA_Base_Core::ThreadGuard sync(m_lock);

    if (id >= 0)
    {
        m_selectedMessage.setValue(id);
    }
    else
    {
        m_selectedMessage.resetValue();
    }

    FUNCTION_EXIT;
}

void TrainBroadcastModel::setDuration(time_t start)
{
    FUNCTION_ENTRY("setDuration");

    TA_Base_Core::ThreadGuard sync(m_lock);
    m_duration = start;

    FUNCTION_EXIT;
}

void TrainBroadcastModel::setPeriod(time_t stop)
{
    FUNCTION_ENTRY("setPeriod");

    TA_Base_Core::ThreadGuard sync(m_lock);
    m_period = stop;

    FUNCTION_EXIT;
}

void TrainBroadcastModel::setCyclicalEnabled(bool enabled)
{
    FUNCTION_ENTRY("setCyclicalEnabled");

    TA_Base_Core::ThreadGuard sync(m_lock);
    m_cyclicalEnabled = enabled;

    FUNCTION_EXIT;
}

const std::string TrainBroadcastModel::getSessionId() const
{
    FUNCTION_ENTRY("getSessionId");
    FUNCTION_EXIT;
    return CachedConfig::getInstance()->getSessionId();
}

void TrainBroadcastModel::switchTrainListingMode(TA_IRS_Bus::LocationCache::EStationType newMode)
{
    FUNCTION_ENTRY("switchTrainListingMode");

    if (m_currentStationMode == newMode)
    {
        FUNCTION_EXIT;
        return;
    }

    if (0 == m_trainSelectionSubscriber)
    {
        // jeffrey++ TD4405
        //TA_THROW(TA_Base_Core::PasConnectionException("No train subscriber available"));
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "No train subscriber available");

        FUNCTION_EXIT;
        return;
        // ++jeffrey TD4405
    }

    m_currentStationMode = newMode;

    unsigned long locationKey = CachedConfig::getInstance()->getEntityLocationKey();

    m_trainSelectionSubscriber->setStationMode(m_currentStationMode, locationKey);

    FUNCTION_EXIT;
}

bool TrainBroadcastModel::isDuplicateDynamicGroupName(const std::string& selectionName) const
{
    FUNCTION_ENTRY("isDuplicateDynamicGroupName");

    CWaitCursor hourGlassCursor;

    if (0 == m_trainSelectionSubscriber)
    {
        TA_THROW(TA_Base_Core::PasConnectionException("No train subscriber available"));
    }

    // There are two ways to approach this - check the local m_dynamicGroupList cache
    // against the input string, however this relies on the TrainAgent having
    // sent out the update notification (so that m_dynamicGroupList is up to date)
    // We could rely on this...  but instead perform a manual query of the train
    // selector to retrieve the selection by input name - and if request fails,
    // we know the group can't exist yet
    try
    {
        m_trainSelectionSubscriber->getTrainSelection(selectionName);
    }
    catch (const TA_Base_Bus::ITrainExceptionsCorba::InvalidSelectionListException&)
    {
        // Selection list doesn't exist - must mean not a duplicate
        FUNCTION_EXIT;
        return false;
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // Comms error
        TA_THROW(TA_Base_Core::PasConnectionException("COMMS ERROR"));
    }
    catch (const CORBA::Exception&)
    {
        // Comms error
        TA_THROW(TA_Base_Core::PasConnectionException("COMMS ERROR"));
    }

    // Must already exist if no exceptions thrown - it is a duplicate
    FUNCTION_EXIT;
    return true;
}

std::vector<unsigned int> TrainBroadcastModel::getSelectedTrainsFromGroup(const std::string& selectionName) const
{
    FUNCTION_ENTRY("getSelectedTrainsFromGroup");

    if (0 == m_trainSelectionSubscriber)
    {
        TA_THROW(TA_Base_Core::PasConnectionException("No train subscriber available"));
    }

    try
    {
        TA_IRS_Bus::CommonTypes::TrainIdList selection = m_trainSelectionSubscriber->getTrainSelection(selectionName);

        //convert "unsigned char" to "unsigned int"
        std::vector<unsigned int> ret;
        ret.reserve(selection.size());
        std::copy(selection.begin(), selection.end(), std::back_inserter(ret));

        FUNCTION_EXIT;
        return ret;
    }
    catch (const TA_Base_Bus::ITrainExceptionsCorba::InvalidSelectionListException&)
    {
        // Selection list doesn't exist

        FUNCTION_EXIT;
        throw;
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // Comms error, recast to PasConnectionException
        TA_THROW(TA_Base_Core::PasConnectionException("COMMS ERROR"));
    }
    catch (const CORBA::Exception&)
    {
        // Comms error
        TA_THROW(TA_Base_Core::PasConnectionException("COMMS ERROR"));
    }
}

TA_IRS_Bus::TrainDvaMessageRecords TrainBroadcastModel::getMessages(const std::string& filterKeyword)
{
    FUNCTION_ENTRY("getMessages");
    FUNCTION_EXIT;
    return TA_IRS_Bus::CachedMaps::getInstance()->getTrainDvaMessageRecords(filterKeyword);
}

bool TrainBroadcastModel::getHasOverrideOption()
{
    FUNCTION_ENTRY("getHasOverrideOption");
    FUNCTION_EXIT;
    return m_hasOverrideOption;
}

void TrainBroadcastModel::setSelectedStaticGroup(const std::string& groupName)
{
    FUNCTION_ENTRY("setSelectedStaticGroup");

    m_selectedStaticGroup = groupName;

    FUNCTION_EXIT;
}

void TrainBroadcastModel::updateFallbackStatus(bool inFallback)
{
    FUNCTION_ENTRY("updateFallbackStatus");

    m_trainBroadcastPage.getTrainSelectorGUI().updateFallbackStatus(inFallback);

    FUNCTION_EXIT;
}

bool TrainBroadcastModel::hasRadioGroupSupport()
{
    FUNCTION_ENTRY("hasRadioGroupSupport");
    FUNCTION_EXIT;
    return m_trainBroadcastPage.getTrainSelectorGUI().hasRadioGroupSupport();
}

void TrainBroadcastModel::updateRadioGroupCombo(const std::vector<std::string>& trainListNames)
{
    FUNCTION_ENTRY("updateRadioGroupCombo");

    m_trainBroadcastPage.getTrainSelectorGUI().updateRadioGroupCombo(trainListNames);

    FUNCTION_EXIT;
}

TA_IRS_Bus::TrainInformationTypes::TrainDetailsList TrainBroadcastModel::getSelectedTrainData() const
{
    FUNCTION_ENTRY("getSelectedTrainData");
    FUNCTION_EXIT;
    return m_trainBroadcastPage.getTrainSelectorGUI().getSelectedTrainData();
}

bool TrainBroadcastModel::testCanObtainLock()
{
    FUNCTION_ENTRY("testCanObtainLock");

    TA_Base_Core::ThreadGuard sync(m_lock);

    FUNCTION_EXIT;
    return true;
}

/*
    //////////////////////////
    // Add in a set of dummy trains
    std::vector<unsigned int> result;
    for (int p = 0; p < 5; p ++) { result.push_back(10 + rand() % 20); }
    return result;
    ///////////////////////////

    /////!!!
    //////////////////////////
    // Add in a set of dummy trains
    TA_IRS_Bus::TrainInformationTypes::TrainDetails td;
    int i;
    for (i = 0; i < 20; i ++)
    {
        td.currentLocation = 50;
        td.trainID = 10 + i;
        td.serviceNumber = CORBA::string_dup("Test");

        if (i % 2 == 0)
        {
            td.isInbound = false;
            updateOutboundList(TA_IRS_Bus::ITrainSelectorGUI::Add, td);
        }
        else
        {
            td.isInbound = true;
            updateInboundList(TA_IRS_Bus::ITrainSelectorGUI::Add, td);
        }
    }

    std::string name;
    if (rand() % 2 == 0) name = ("Train Group 1");
    else                 name = ("Train Group 2");

    std::vector<std::string> trainNames(10, name);
    updateSelectionCombo(trainNames);
    //// END DEBUG
*/
