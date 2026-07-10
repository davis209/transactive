/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/GenericStatusPage.cpp $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * GenericStatusPage provides some basic functionality that can be
 * shared by all Status Pages.  If the MFC based message mapping
 * (within the BEGIN_MESSAGE_MAP blocks) doesn't seem to be working
 * properly, it means someone didn't define the opening of the message
 * map BEGIN_MESSAGE_MAP(thisClass, baseClass) - if 'baseClass' skips
 * a class - then the skipped classes message map will not work
 * For example BEGIN_MESSAGE_MAP(DerivedStatusPage, CDialog) - all message
 * maps in the hierarchy will be ignored, except for DerivedStatusPage
 * (should go BEGIN_MESSAGE_MAP(DerivedStatusPage, GenericStatusPage)
 *
 */

#include "app/pa/PAManager/src/StdAfx.h"

#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/GenericStatusModel.h"
#include "app/pa/PAManager/src/GenericStatusPage.h"
#include "app/pa/PAManager/src/LockControlRedraw.h"
#include "app/pa/PAManager/src/PaLayoutHelper.h"
#include "app/pa/PAManager/src/PaRightsManager.h"
#include "app/pa/PAManager/src/RightsChangedWindowNotificationHandler.h"
#include "app/pa/PAManager/src/StationStatusHelper.h"
#include "app/pa/PAManager/src/TrainStatusHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_IRS_Bus;

// song toan++
// TD 10316
GenericStatusPage::GenericStatusPage(bool isOCC, UINT nIDTemplate, CWnd* pParentWnd)
    :
    CDialog(nIDTemplate, pParentWnd),
    m_rightsChangeHandler(0),
    m_isOCC(isOCC)
{
    FUNCTION_ENTRY("GenericStatusPage");
    FUNCTION_EXIT;
}

// song toan++
// TD 10316

GenericStatusPage::~GenericStatusPage()
{
    FUNCTION_ENTRY("~GenericStatusPage");

    TA_ASSERT(0 == m_rightsChangeHandler, "Rights change handler should be deleted in OnDestroy");

    FUNCTION_EXIT;
}

void GenericStatusPage::DoDataExchange(CDataExchange* pDX)
{
    FUNCTION_ENTRY("DoDataExchange");

    CDialog::DoDataExchange(pDX);

    FUNCTION_EXIT;
}

BEGIN_MESSAGE_MAP(GenericStatusPage, CDialog)
    //{{AFX_MSG_MAP(DvaStatusPage)
    ON_MESSAGE(WM_PA_STN_STATUS_REPORT_UPDATE, OnReportUpdate)
    ON_MESSAGE(WM_PA_RIGHTS_CHANGE, OnRightsChanged)
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

GenericStatusModel& GenericStatusPage::getStatusModel()
{
    FUNCTION_ENTRY("getStatusModel");
    FUNCTION_EXIT;
    return castPointerToReference<GenericStatusModel>(getStatusModelPointer());
}

LRESULT GenericStatusPage::OnReportUpdate(UINT wParam, LONG lParam)
{
    LOG_SCOPE("WM_PA_STN_STATUS_REPORT_UPDATE", "GenericStatusPage::OnReportUpdate");
    FUNCTION_ENTRY("OnReportUpdate");

    TA_ASSERT(getStatusModelPointer() != NULL, "getStatusModelPointer() NULL");

    try
    {
        LOG_INFO("Collecting message for window %d", m_hWnd);

        // This function, once called, will subsequently update this page view
        TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState newBroadcastState = getStatusModel().processProgressUpdate();

        // May need to update state of the retry buttons & abort button
        // after any given progress report - if exists change in broadcast state
        // NOTE: This is done outside function processProgressUpdate to
        // correctly sequence the order of refresh operatiosn (onBroadcastStateChanged
        // MUST be called last, after the progress report has been processed, so don't shift this)
        if (!m_currentBroadcastState.hasBeenSetToValue(newBroadcastState))
        {
            // Update broadcast state value
            m_currentBroadcastState.setValue(newBroadcastState);

            // If the broadcast is complete, it will be in this state
            onBroadcastStateChanged(m_currentBroadcastState.getValue(), false);
        }
    }

    STANDARD_PA_CATCH("GenericStatusPage::OnReportUpdate", PaErrorHandler::UNSPECIFIED_ERROR);

    // Each time there is new report information, must update retry state of buttons
    // (there may be a change to the broadcast state, or items present in failures list)
    updateRetryButtonState();

    FUNCTION_EXIT;
    return 0;
}

void GenericStatusPage::onBroadcastStateChanged(const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState& newState, bool initialisation)
{
    FUNCTION_ENTRY("onBroadcastStateChanged");

    if (!initialisation)
    {
        std::string message = getBroadcastStatusAreaText(newState);
        addStringToStatusArea(message);
    }

    FUNCTION_EXIT;
}

void GenericStatusPage::onGlobalStateChanged(const TA_Base_Bus::IPAAgentCorbaDef::EGlobalState& state)
{
    FUNCTION_ENTRY("onGlobalStateChanged");

    if (TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FAILURE != state)
    {
        // Only display status text if not in the no failure (nothing to report) state
        std::string message = getGlobalStateStatusAreaText(state);

        addStringToStatusArea(message);
    }

    FUNCTION_EXIT;
}

void GenericStatusPage::updateRetryButtonState()
{
    FUNCTION_ENTRY("updateRetryButtonState");

    // Only allow any kind of retry when PendingRemoval state entered
    BOOL broadcastComplete = m_currentBroadcastState.hasBeenSetToValue(
        TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval);

    BOOL hasRightToRetry = PaRightsManager::getInstance().
        hasRightToRetryBroadcast(getStatusModel().getBroadcastId(),
                                 CachedConfig::getInstance()->getAgentObject());

    // Enable retry buttons only when broadcast is complete
    BOOL enableRetryAll = broadcastComplete;

    if (enableRetryAll && !hasRightToRetry)
    {
        // We are not allowed to retry this particular broadcast
        enableRetryAll = FALSE;
    }

    getRetryAllButton().EnableWindow(enableRetryAll);

    BOOL enableRetryFailed = broadcastComplete;

    if (0 == getFailuresListControl().GetItemCount())
    {
        // If there are no items in the failed list, no need to enable
        // the retry failed button (as it won't do anything)
        enableRetryFailed = FALSE;
    }

    if (enableRetryFailed && !hasRightToRetry)
    {
        // We are not allowed to retry this particular broadcast
        enableRetryFailed = FALSE;
    }

    getRetryFailedButton().EnableWindow(enableRetryFailed);

    FUNCTION_EXIT;
}

BOOL GenericStatusPage::OnInitDialog()
{
    FUNCTION_ENTRY("OnInitDialog");

    m_rightsChangeHandler =  new RightsChangedWindowNotificationHandler(m_hWnd);

    CDialog::OnInitDialog();

    // Perform initialisation of report lists
    TA_ASSERT(getStatusModelPointer() != NULL, "getStatusModelPointer() NULL");

    // Enter an initial state (this will update button states relevant
    // to state of broadcast)
    onBroadcastStateChanged(TA_Base_Bus::IPAAgentCorbaDef::Creation, true);

    if (getStatusModel().isStationModel())
    {
        // song toan++
        // TD 10316
        StationStatusHelper::setupReportWindowList(m_isOCC, getFailuresListControl(), true);
        StationStatusHelper::setupReportWindowList(m_isOCC, getProgressListControl(), false);
        // song toan++
        // TD 10316
    }
    else
    {
        TrainStatusHelper::setupReportWindowList(getFailuresListControl());
        TrainStatusHelper::setupReportWindowList(getProgressListControl());
    }

    // Force an initial refresh of progress report - this ensures (to best of our
    // ability) that the Progress and Failures report lists are up to date,
    // without waiting to receive message from PAAgent (which could take a while
    // hence leaving a blank screen)
    // - Additionally
    // AZ++ TD10377
    getStatusModel().forceProgressReportUpdate(true);
    // ++AZ TD10377

    // Just in case initial progress report update failed
    updateRetryButtonState();

    FUNCTION_EXIT;
    return TRUE;
}

void GenericStatusPage::OnDestroy()
{
    FUNCTION_ENTRY("OnDestroy");

    // Delete the rights change notification object before window handle becomes invalid
    delete m_rightsChangeHandler;
    m_rightsChangeHandler = 0;

    CDialog::OnDestroy();

    // Delete the model here, rather than in destructor
    // Because in this way model stops communicating with window handle
    // Which will be invalid post destruction
    destroyStatusModel();

    FUNCTION_EXIT;
}

void GenericStatusPage::updateReportLists(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData& pr)
{
    LOG_SCOPE("GenericStatusPage::updateReportLists");
    FUNCTION_ENTRY("updateReportLists");

    // song toan++
    // TD 10316
    CListCtrlSort& failuresList = getFailuresListControl();
    CListCtrlSort& progressList = getProgressListControl();
    // ++song toan
    // TD 10316

    // Disabling the redraw while appending items
    // stops the scrollbar from flickering (tested and confirmed)
    LockControlRedraw failuresRedrawLock(failuresList);
    LockControlRedraw progressRedrawLock(progressList);

    int pCount = progressList.GetItemCount();
    int fCount = failuresList.GetItemCount();

    // This messes up the scroll bar position...  so add new items FIRST and delete old
    // to avoid this (rather than deleting, then adding)
    //progressList.DeleteAllItems();
    //failuresList.DeleteAllItems();

    // Ensure we attempt to add in all items (without terminating update upon error)
    // - meaning non-use of exceptions for error codes
    bool successfulAddition = true;

    // Add in the data to the lists (without removing old data..yet)
    if (getStatusModel().isStationModel())
    {
        successfulAddition = StationStatusHelper::addToStationReportLists(
            // song toan++
            // TD 10316
            m_isOCC,
            // ++song toan
            // TD 10316
            failuresList,
            progressList,
            pr.localProgresses);
    }
    else
    {
        // TD16865
        std::map<unsigned int, std::string>& trainIdAndServiceNum = getTrainIdAndServiceNumber();
        // TD16865

        successfulAddition = TrainStatusHelper::addToTrainReportLists(
            failuresList,
            progressList,
            pr.localProgresses,
            // TD16865
            trainIdAndServiceNum);
        // TD16865
    }

    // Delete things to long winded way to stop the scroll bar
    // position from being messed up - this is a cheap way to do it
    // (rather than saving the scroll position and restoring it)
    // However it's not as efficient..
    int i;

    // Because items are inserted at the head, keep clipping off the last item
    for (i = 0; i < pCount; i++)
    {
        progressList.DeleteItem(progressList.GetItemCount() - 1);
    }

    for (i = 0; i < fCount; i++)
    {
        failuresList.DeleteItem(failuresList.GetItemCount() - 1);
    }

    // Preferably display errors after fully updating lists (if do half way through
    // will get unpredictable results)
    if (!successfulAddition)
    {
        TA_THROW(TA_Base_Core::TransactiveException("Error adding items to lists"));
    }

    // Check if status area text needs to be updated
    if (!m_globalState.hasBeenSetToValue(pr.globalState))
    {
        m_globalState.setValue(pr.globalState);
        onGlobalStateChanged(m_globalState.getValue());
    }

    FUNCTION_EXIT;
}

afx_msg LRESULT GenericStatusPage::OnRightsChanged(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnRightsChanged");

    try
    {
        // Rights can only affect whether we may retry or not
        updateRetryButtonState();
    }

    STANDARD_PA_CATCH("GenericStatusPage::OnRightsChanged", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

std::string GenericStatusPage::getBroadcastStatusAreaText(const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState& state)
{
    FUNCTION_ENTRY("getBroadcastStatusAreaText");

    switch (state)
    {
        case TA_Base_Bus::IPAAgentCorbaDef::Creation:
            FUNCTION_EXIT;
            return "Preparing broadcast...";

        case TA_Base_Bus::IPAAgentCorbaDef::Execution:
            FUNCTION_EXIT;
            return "Executing broadcast...";

        case TA_Base_Bus::IPAAgentCorbaDef::Runtime:
            FUNCTION_EXIT;
            return "Broadcast in progress.";

        case TA_Base_Bus::IPAAgentCorbaDef::Termination:
            FUNCTION_EXIT;
            return "Terminating broadcast...";

        case TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval:
            // TES647
            //         return "Broadcast complete.";
            FUNCTION_EXIT;
            return "Broadcast Process End.";

            // TES647
        default:
            LOG_ERROR("getBroadcastStatusAreaText: Unrecognized broadcast state (%d)", int(state));

            FUNCTION_EXIT;
            return "???";
    };
}

std::string GenericStatusPage::getGlobalStateStatusAreaText(const TA_Base_Bus::IPAAgentCorbaDef::EGlobalState& state)
{
    FUNCTION_ENTRY("getGlobalStateStatusAreaText");

    switch (state)
    {
        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FAILURE:
            // Enter this state at startup and in-between failures, shouldn't be
            // requesting text for it..
            TA_ASSERT(FALSE, "Requesting GLOBAL_STATE_NO_FAILURE text");

            FUNCTION_EXIT;
            return "";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FREE_ANNOUNCE_ID:
            FUNCTION_EXIT;
            return "Error notification: No free announce ID.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FREE_MESSAGE_SEQ_ID:
            FUNCTION_EXIT;
            return "Error notification: No free message sequence ID.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FREE_DVA_CHANNEL:
            FUNCTION_EXIT;
            return "Error notification: No free DVA channel.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_RECORDING_SOURCE_BUSY:
            FUNCTION_EXIT;
            return "Error notification: Recording source Busy.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_MESSAGE_BUSY_RECORDING:
            FUNCTION_EXIT;
            return "Error notification: Message busy Recording.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_MESSAGE_BUSY_BROADCASTING:
            FUNCTION_EXIT;
            return "Error notification: Message busy broadcasting.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_COMMS_FAILURE_TRAIN_AGENT:
            FUNCTION_EXIT;
            return "Error notification: Train Agent communications failure.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_COMMS_FAILURE_TIS_AGENT:
            FUNCTION_EXIT;
            return "Error notification: TIS Agent communications failure.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_COMMS_FAILURE_AUTHENTICATION_AGENT:
            FUNCTION_EXIT;
            return "Error notification: Authentication Agent communications failure.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_HARDWARE_FAILURE_TRAIN_AGENT:
            FUNCTION_EXIT;
            return "Error notification: Train Agent hardware failure.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_HARDWARE_FAILURE_TIS_AGENT:
            FUNCTION_EXIT;
            return "Error notification: TIS Agent hardware failure.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_HARDWARE_FAILURE_PA_AGENT:
            FUNCTION_EXIT;
            return "Error notification: PA Agent hardware failure.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_PERMISSION_FAILURE_TRAIN_AGENT:
            FUNCTION_EXIT;
            return "Error notification: Train Agent permissions failure.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_PERMISSION_FAILURE_TIS_AGENT:
            FUNCTION_EXIT;
            return "Error notification: TIS Agent permissions failure.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_TRAIN_AGENT:
            FUNCTION_EXIT;
            return "Error notification: Train Agent protocol failure.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_TIS_AGENT:
            FUNCTION_EXIT;
            return "Error notification: TIS Agent protocol failure.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_PA_AGENT:
            FUNCTION_EXIT;
            return "Error notification: PA Agent protocol failure.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_PUBLIC_DVA_SET_VERSION_CONFLICT:
            FUNCTION_EXIT;
            return "Public DVA set version conflict.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_RADIO_CALL_RESET:
            FUNCTION_EXIT;
            return "Audio Call Disconnected.";

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_STATIC_GROUP_IN_USE:
            FUNCTION_EXIT;
            return "Selected GOA group is in use.";

        default:
            LOG_ERROR("getGlobalStateStatusAreaText: Unrecognized state (%d)", int(state));

            FUNCTION_EXIT;
            return "???";
    };
}

#include "TrainDvaStatusDlg.h"
void GenericStatusPage::addStringToStatusArea(const std::string& newLine)
{
    FUNCTION_ENTRY("addStringToStatusArea");

    // This is temporary, until Train DVA Status dialogs have a status area control added..
    if (typeid(*this) == typeid(TrainDvaStatusDlg))
    {
        FUNCTION_EXIT;
        return;
    }

    PaLayoutHelper::appendStringToStatusWindow(
        getStatusTextListBox(),
        newLine);

    FUNCTION_EXIT;
}
