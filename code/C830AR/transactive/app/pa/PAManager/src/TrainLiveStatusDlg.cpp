/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainLiveStatusDlg.cpp $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * TrainLiveStatusDlg handles GUI component of live train broadcast status dialog
 *
 */

 // Disable std::deque warnings
#pragma warning(disable: 4018 4146)

#include "app/pa/PAManager/src/stdafx.h"

#include "app/pa/PAManager/src/TrainLiveStatusDlg.h"
#include "app/pa/PAManager/src/TrainRadioPttMonitor.h"
#include "app/pa/PAManager/src/pamanager.h"

#include "bus/generic_gui/src/HelpLauncher.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const DWORD CONTINUE_BUTTON_RED_COLOUR = RGB(255, 0, 0);
static const float CONTINUE_BUTTON_BLINK_FREQUENCY = 2.5f;    // Alternates this many times/second
static const DWORD CONTINUE_BUTTON_INVERT_MESSAGE = 1;

const std::string TrainLiveStatusDlg::s_pressBeginToStart =
"Press \"Begin Live Announcement\" to commence.";
const std::string TrainLiveStatusDlg::s_pressPttToTalk =
"Press PTT on the radio paging console before speaking.";
// TES 647
//const std::string TrainLiveStatusDlg::s_broadcastComplete =
//            "Broadcast complete.";
const std::string TrainLiveStatusDlg::s_broadcastComplete =
"Broadcast Process End.";
// TES 647

// This string must have a %d present
const std::string TrainLiveStatusDlg::s_callRequiredOnTrain =
"Call on train %d requires confirmation to continue broadcast.";
// This string must have a %d present
const std::string TrainLiveStatusDlg::s_callConfirmedOnTrain =
"Call on train %d has been confirmed.";

// These strings taken from Radio Agent design notes,
// probably not suitable for PA Manager (but for consistency they were used)
const std::string TrainLiveStatusDlg::s_operatorPressedPtt =
"Operator talking";
const std::string TrainLiveStatusDlg::s_operatorReleasedPtt =
"Operator listening";

// jeffrey++ TD7956
// TD16865
TrainLiveStatusDlg::TrainLiveStatusDlg(const std::string& broadcastId, bool OverrideFlag,
                                       std::map<unsigned int, std::string> trainIDToServiceNumber,
                                       CWnd* pParent /*=NULL*/)
    :
    LiveStatusPage(
        // song toan++
        // TD 10316
        false,
        // ++song toan
        // TD 10316
        TrainLiveStatusDlg::IDD, pParent),
    m_trainRadioPttMonitor(NULL),
    m_openingBroadcastId(broadcastId),
    m_trainStatusModel(NULL),
    m_override(OverrideFlag),
    m_trainIdServiceNum(trainIDToServiceNumber),
    // TD16865
    // **********  Added #8854
    // song toan++
    // TD 10316
    m_progressList(false, false),
    m_failuresList(false, true)
    // ++song toan
    // TD 10316

    // **********  #8854

{
    FUNCTION_ENTRY("TrainLiveStatusDlg");

    //{{AFX_DATA_INIT(TrainLiveStatusDlg)
    // jeffrey++ TD7956
    m_override = false;
    // ++jeffrey TD7956
    //}}AFX_DATA_INIT

    FUNCTION_EXIT;
}

// ++jeffrey TD7956

TrainLiveStatusDlg::~TrainLiveStatusDlg()
{
    FUNCTION_ENTRY("~TrainLiveStatusDlg");
    FUNCTION_EXIT;
}

void TrainLiveStatusDlg::DoDataExchange(CDataExchange* pDX)
{
    FUNCTION_ENTRY("DoDataExchange");

    LiveStatusPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(TrainLiveStatusDlg)
    DDX_Control(pDX, IDC_BUTTON_CONTINUE, m_continueButton);
    DDX_Control(pDX, IDC_LIVE_STATUS, m_announceStatusControl);
    DDX_Control(pDX, IDC_TRAIN_STATUS_PROGRESS_LST, m_progressList);
    DDX_Control(pDX, IDC_TRAIN_STATUS_FAILURES_LST, m_failuresList);
    DDX_Control(pDX, IDC_BUTTON_END, m_endButton);
    DDX_Control(pDX, IDC_BUTTON_BEGIN, m_beginButton);
    DDX_Control(pDX, ID_TRAIN_STATUS_RETRY_FAILURES_BTN, m_retryFailuresButton);
    DDX_Control(pDX, ID_TRAIN_STATUS_RETRY_ALL_BTN, m_retryAllButton);
    DDX_Control(pDX, ID_TRAIN_STATUS_CLOSE_BTN, m_closeButton);
    //}}AFX_DATA_MAP

    FUNCTION_EXIT;
}

BEGIN_MESSAGE_MAP(TrainLiveStatusDlg, LiveStatusPage)
    //{{AFX_MSG_MAP(TrainLiveStatusDlg)
    ON_BN_CLICKED(IDC_BUTTON_BEGIN, OnButtonBeginLiveAnnouncement)
    ON_BN_CLICKED(IDC_BUTTON_CONTINUE, OnButtonContinueLiveAnnouncement)
    ON_BN_CLICKED(IDC_BUTTON_END, OnButtonEndLiveAnnouncement)
    ON_BN_CLICKED(IDC_RADIO_OVERRIDE, OnRadioOverride)
    ON_BN_CLICKED(IDC_RADIO_NORMAL, OnRadioNormal)
    ON_BN_CLICKED(ID_TRAIN_HELP, OnTrainHelp)
    ON_WM_DESTROY()
    ON_BN_CLICKED(ID_TRAIN_STATUS_CLOSE_BTN, OnTrainStatusCloseBtn)
    ON_MESSAGE(WM_PA_STN_STATUS_REPORT_UPDATE, OnReportUpdate)
    ON_MESSAGE(WM_PA_TRAIN_PTT_CHANGE, OnTrainPttStateChange)
    ON_WM_CLOSE()
    ON_BN_CLICKED(ID_TRAIN_STATUS_RETRY_FAILURES_BTN, OnTrainStatusRetryFailuresBtn)
    ON_BN_CLICKED(ID_TRAIN_STATUS_RETRY_ALL_BTN, OnTrainStatusRetryAllBtn)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

GenericStatusModel* TrainLiveStatusDlg::getStatusModelPointer()
{
    FUNCTION_ENTRY("getStatusModelPointer");

    TA_ASSERT(0 != m_trainStatusModel, "Status model null");

    FUNCTION_EXIT;
    return m_trainStatusModel;
}

void TrainLiveStatusDlg::destroyStatusModel()
{
    FUNCTION_ENTRY("destroyStatusModel");

    delete m_trainStatusModel;
    m_trainStatusModel = 0;

    FUNCTION_EXIT;
}

void TrainLiveStatusDlg::OnButtonBeginLiveAnnouncement()
{
    FUNCTION_ENTRY("OnButtonBeginLiveAnnouncement");

    // Previously the begin live announcement button was left enabled until the
    // notification was received of begin request which would disable the
    // Begin Live Announcement button (see onBroadcastStateChanged)
    // However, that allows the operator to click on the button multiple times
    // which is incorrect. The button will now be disabled as soon as it is clicked,
    // and will be re-enabled on an End live announcement of if an exception occurs.
    try
    {
        m_trainStatusModel->beginLiveAnnouncement();

        m_beginButton.EnableWindow(false);

        // start monitoring PTT - the call has been made
        if (m_trainRadioPttMonitor == NULL)
        {
            m_trainRadioPttMonitor = new TrainRadioPttMonitor(m_hWnd);
        }

        // User can end broadcast
        std::string statusWindowOutput = s_pressPttToTalk;

        if (!m_lastStatusString.hasBeenSetToValue(statusWindowOutput))
        {
            m_lastStatusString.setValue(statusWindowOutput);
            addStringToStatusArea(statusWindowOutput);
        }
    }
    catch (const TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdNotFound&)
    {
        PaErrorHandler::displayError(PaErrorHandler::BROADCAST_ID_NOT_FOUND);
        m_beginButton.EnableWindow(true); // Re-enable the Begin Live Announcement button
        // to ensure the user can try again.
    }
    catch (const TA_Base_Core::BadParameterException& bpe)
    {
        //PaErrorHandler::displayError(PaErrorHandler::INVALID_REQUEST_PARAMS);
        std::string error(bpe.reason.in());
        addStringToStatusArea(error);
        m_beginButton.EnableWindow(true); // Re-enable the Begin Live Announcement button
        // to ensure the user can try again.
    }
    catch (const CORBA::Exception&)
    {
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
        m_beginButton.EnableWindow(true); // Re-enable the Begin Live Announcement button
        // to ensure the user can try again.
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
        m_beginButton.EnableWindow(true); // Re-enable the Begin Live Announcement button
        // to ensure the user can try again.
    }

    FUNCTION_EXIT;
}

void TrainLiveStatusDlg::OnButtonEndLiveAnnouncement()
{
    FUNCTION_ENTRY("OnButtonEndLiveAnnouncement");

    // Don't actually disable the end live announcement button
    // directly (allow an indirect update - the broadcast should
    // leave a Runtime state once notification received of end request
    // which will disable the End Live Announcement button
    // (see onBroadcastStateChanged)
    try
    {
        m_trainStatusModel->endLiveAnnouncement();
    }
    catch (const TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdNotFound&)
    {
        PaErrorHandler::displayError(PaErrorHandler::BROADCAST_ID_NOT_FOUND);
    }
    catch (const TA_Base_Core::BadParameterException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::INVALID_REQUEST_PARAMS);
    }
    catch (const CORBA::Exception&)
    {
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }

    FUNCTION_EXIT;
}

void TrainLiveStatusDlg::OnButtonContinueLiveAnnouncement()
{
    FUNCTION_ENTRY("OnButtonContinueLiveAnnouncement");

    // User should only be allowed to press this as long as
    // there is an item present in m_continueRequiredQueue
    if (0 == m_continueRequiredQueue.size())
    {
        // Nothing to do
        TA_ASSERT(FALSE, "Invalid internal state");

        FUNCTION_EXIT;
        return;
    }

    int trainId = m_continueRequiredQueue.front();
    bool success = false;

    try
    {
        m_trainStatusModel->requestContinueOnTrain();

        // Now that request has been dispatched to agent, clear item from queue
        m_continueRequiredQueue.clear();

        success = true;
    }
    catch (const TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdNotFound&)
    {
        PaErrorHandler::displayError(PaErrorHandler::BROADCAST_ID_NOT_FOUND);
    }
    catch (const TA_Base_Core::BadParameterException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::INVALID_REQUEST_PARAMS);
    }
    catch (const CORBA::Exception&)
    {
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }

    // Refresh the continue button flashing state
    updateContinueButtonState(m_continueRequiredQueue.size() > 0);

    FUNCTION_EXIT;
}

void TrainLiveStatusDlg::OnRadioOverride()
{
    FUNCTION_ENTRY("OnRadioOverride");

    CheckRadioButton(IDC_RADIO_NORMAL, IDC_RADIO_OVERRIDE, IDC_RADIO_OVERRIDE);
    // Override has been set
    m_trainStatusModel->setHasOverrideOption(true);

    FUNCTION_EXIT;
}

void TrainLiveStatusDlg::OnRadioNormal()
{
    FUNCTION_ENTRY("OnRadioNormal");

    CheckRadioButton(IDC_RADIO_NORMAL, IDC_RADIO_OVERRIDE, IDC_RADIO_NORMAL);
    // Normal mode has been set
    m_trainStatusModel->setHasOverrideOption(false);

    FUNCTION_EXIT;
}

BOOL TrainLiveStatusDlg::OnInitDialog()
{
    FUNCTION_ENTRY("OnInitDialog");

    m_trainStatusModel = new TrainLiveStatusModel(*this, m_openingBroadcastId);

    LiveStatusPage::OnInitDialog();

    // TD16862
    // jeffrey++ TD7956
    // Default to the override retry mode
    OnRadioOverride();
    //    if(m_override)
    //        OnRadioOverride();
    //    else
    //        OnRadioNormal();
    // ++jeffrey TD7956
    // TD16862

    setContinueButtonEnabledState(false);

    // Set up the flashing button parameters
    UINT delay = UINT(1000.0f / CONTINUE_BUTTON_BLINK_FREQUENCY);
    m_continueButton.SetFlashColour(CONTINUE_BUTTON_RED_COLOUR);
    m_continueButton.SetFlashPeriod(delay);

    FUNCTION_EXIT;
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void TrainLiveStatusDlg::updateContinueButtonState(bool bFlashRed)
{
    FUNCTION_ENTRY("updateContinueButtonState");

    if (bFlashRed)
    {
        TA_ASSERT(::IsWindow(m_hWnd), "Window not initialised");

        // Ensure button enabled if flashing
        setContinueButtonEnabledState(true);

        // Start up timer (or reset if already started)
        m_continueButton.StartFlashing();
    }
    else
    {
        m_continueButton.StopFlashing();
        // While the button not flashing, we may as well disable it -
        // because it won't actually be doing anything if user clicks if
        setContinueButtonEnabledState(false);
    }

    FUNCTION_EXIT;
}

void TrainLiveStatusDlg::onBroadcastStateChanged(const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState& newState, bool initialisation)
{
    FUNCTION_ENTRY("onBroadcastStateChanged");

    LiveStatusPage::onBroadcastStateChanged(newState, initialisation);

    updateButtonStatesAndFeedbackArea(newState);

    FUNCTION_EXIT;
}

void TrainLiveStatusDlg::updateButtonStatesAndFeedbackArea(const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState& newState)
{
    FUNCTION_ENTRY("updateButtonStatesAndFeedbackArea");

    /*
    EXECUTION: "Press Begin Live Announcement to start"
    Initially the broadcast state will be in Creation..Execution, at any stage the
    operator can click the begin broadcast button (any unconnected trains will be aborted at
    that time).  Once the user clicks BEGIN broadcast, the broadcastState enters Runtime.
    RUNTIME: "Press PTT to talk"
    When broadcastState enters RunTime, continue button enabled, may flash, end button available
    User clicks the end live announcement, broadcast begins termination
    TERMINATION: "Broadcast complete" message displayed (retry still unavailable, begin unavailable)

    * NOTES: - Can only retry anything when in PendingRemoval state
             - The Begin Live Announcement button only available when in Creation or Execution state
    */

    // All buttons we control disabled by default
    bool enableBeginButton = false;
    bool enableEndButton = false;

    std::string statusWindowOutput;

    switch (newState)
    {
        case TA_Base_Bus::IPAAgentCorbaDef::Creation:
            // Wait for creation stage to complete before allowing actions
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::Execution:

            break;

        case TA_Base_Bus::IPAAgentCorbaDef::Runtime:

            // User can begin announcement provided *at least* one train has indicated
            // that it is ready for the live announcement
            if (m_trainStatusModel->existsTrainsWaitingForBroadcast())
            {
                statusWindowOutput = s_pressBeginToStart;
                enableBeginButton = true;
            }

            enableEndButton = true; // user can end broadcast for all trains that have been set up so far
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::Termination:

            // While terminating cannot do anything except close dialog
            // FALLTHROUGH
        case TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval:
            statusWindowOutput = s_broadcastComplete;

            // Termination successful

            // clean up PTT
            if (m_trainRadioPttMonitor != NULL)
            {
                delete m_trainRadioPttMonitor;
                m_trainRadioPttMonitor = NULL;
            }

            break;

        default:
            TA_ASSERT(FALSE, "Unhandled broadcast state");
            break;
    }

    // Because two states may contribute to the same message (and just
    // in case we skip certain short term states like Termination)
    // We usually get two consecutive notifications for same message -
    // we only respond to the first of any such consecutive request
    if (!m_lastStatusString.hasBeenSetToValue(statusWindowOutput))
    {
        m_lastStatusString.setValue(statusWindowOutput);
        addStringToStatusArea(statusWindowOutput);
    }

    if (TA_Base_Bus::IPAAgentCorbaDef::Runtime != newState)
    {
        // When leave runtime, no longer require confirmations from operator
        resetContinueRequiredButton();
        // May clear the continue required state tracking map
        // of train model once finished
        m_trainStatusModel->resetContinueRequiredStates();
    }

    // Close buttons should always be enabled, in the event of failure (ie if we're
    // stuck in a bad state, shouldn't prohibit operator from closing dialog)
    m_closeButton.EnableWindow(TRUE);
    m_beginButton.EnableWindow(enableBeginButton);
    m_endButton.EnableWindow(enableEndButton);

    FUNCTION_EXIT;
}

void TrainLiveStatusDlg::resetContinueRequiredButton()
{
    FUNCTION_ENTRY("resetContinueRequiredButton");

    // No more need for items in continue confirmation required queue
    // (that is, broadcast has ended, and can discard
    // all pending continue required items)
    m_continueRequiredQueue.clear();
    // Must update continue button state whenever messing around with continue required queue
    updateContinueButtonState(m_continueRequiredQueue.size() > 0);

    FUNCTION_EXIT;
}

void TrainLiveStatusDlg::OnTrainHelp()
{
    FUNCTION_ENTRY("OnTrainHelp");

    TA_Base_Bus::HelpLauncher::getInstance().displayHelp();

    FUNCTION_EXIT;
}

void TrainLiveStatusDlg::setContinueButtonEnabledState(bool enableContinueButton)
{
    FUNCTION_ENTRY("setContinueButtonEnabledState");

    if (enableContinueButton == (m_continueButton.IsWindowEnabled() == TRUE))
    {
        // No change, do nothing
        FUNCTION_EXIT;
        return;
    }

    DWORD currentStyle = m_continueButton.GetButtonStyle();

    if (enableContinueButton)
    {
        // If enable, we want owner draw (to draw coloured)
        m_continueButton.SetButtonStyle(currentStyle | BS_OWNERDRAW);
    }
    else
    {
        // If disabled, take out the owner draw (we just want it to be
        // drawn normally)
        m_continueButton.SetButtonStyle(currentStyle & ~BS_OWNERDRAW);
    }

    m_continueButton.EnableWindow(enableContinueButton);

    FUNCTION_EXIT;
}

void TrainLiveStatusDlg::OnDestroy()
{
    FUNCTION_ENTRY("OnDestroy");

    try
    {
        if (m_trainRadioPttMonitor != NULL)
        {
            delete m_trainRadioPttMonitor;
            m_trainRadioPttMonitor = NULL;
        }
    }
    catch (...)
    {
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "Caught in DestroyWindow");
    }

    LiveStatusPage::OnDestroy();

    FUNCTION_EXIT;
}

LRESULT TrainLiveStatusDlg::OnTrainPttStateChange(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnTrainPttStateChange");

    try
    {
        // Display feedback to user whenever the state of the train PTT mic changes
        if (0 == wParam)
        {
            addStringToStatusArea(s_operatorReleasedPtt.c_str());
        }
        else
        {
            addStringToStatusArea(s_operatorPressedPtt.c_str());
        }
    }

    STANDARD_PA_CATCH("TrainLiveStatusDlg::OnTrainPttStateChange", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

LRESULT TrainLiveStatusDlg::OnReportUpdate(UINT wParam, LONG lParam)
{
    LOG_SCOPE("WM_PA_STN_STATUS_REPORT_UPDATE", "TrainLiveStatusDlg::OnReportUpdate");
    FUNCTION_ENTRY("OnReportUpdate");

    LRESULT result = 0;

    try
    {
        // Each time a new report comes through check if there is a state
        // change in the trains waiting for broadcast state
        // - if there is a state change, we need to refresh window controls
        bool trainsWaiting = m_trainStatusModel->existsTrainsWaitingForBroadcast();

        result = LiveStatusPage::OnReportUpdate(wParam, lParam);

        if (m_trainStatusModel->existsTrainsWaitingForBroadcast() != trainsWaiting)
        {
            // Each time the state of whether there are trains waiting for a broadcast
            // changes, we must invalidate the button and status feedback area
            // Sometimes this may be a redundant action, but no special checks in place
            // to avoid redundant refreshes
            try
            {
                updateButtonStatesAndFeedbackArea(getCurrentBroadcastState());
            }
            catch (const TA_Base_Core::ValueNotSetException&)
            {
                // This shouldn't occur (from getCurrentBroadcastState)
                // provided GenericStatusPage::OnReportUpdate is doing its job

                FUNCTION_EXIT;
                throw;
            }
        }

        // Process the update as usual, then query our status model
        // To see if there are any changes to the 'continue required' states
        // of the trains (if so, display the changes on screen)
        while (m_trainStatusModel->doesNewContinueRequiredTrainExist())
        {
            processContinueRequiredStateChange(
                m_trainStatusModel->popFrontNewContinueRequiredTrain());
        }
    }

    STANDARD_PA_CATCH("TrainLiveStatusDlg::OnReportUpdate", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return result;
}

void TrainLiveStatusDlg::processContinueRequiredStateChange(const TrainLiveStatusModel::TrainContinueState& state)
{
    FUNCTION_ENTRY("processContinueRequiredStateChange");

    // Any time the new state is a continue required one, we add it
    // to a continueRequired queue (which upon emptying will stop
    // the "Continue Required" button from flashing)
    if (state.continueRequired)
    {
        if (std::find(m_continueRequiredQueue.begin(),
                      m_continueRequiredQueue.end(),
                      state.trainId)
            == m_continueRequiredQueue.end())
        {
            // Add it to queue only if not present (if somehow the PA Agent sent us two
            // state changes indicating require continue button, we still only
            // need to send one through)
            m_continueRequiredQueue.push_back(state.trainId);

            // Provide feedback to user, a train requires confirmation
            CString msg;
            TA_ASSERT(s_callRequiredOnTrain.find("%d") != std::string::npos, "No variable field found");

            msg.Format(s_callRequiredOnTrain.c_str(), state.trainId);

            addStringToStatusArea(msg.GetBuffer(0));
        }
    }
    else
    {
        // if state cleared, we can acknowledge the confirmation
        // reached the agent, and continue is no longer required

        // Provide feedback to user, train has been confirmed -
        CString msg;
        TA_ASSERT(s_callConfirmedOnTrain.find("%d") != std::string::npos, "No variable field found");
        msg.Format(s_callConfirmedOnTrain.c_str(), state.trainId);
        addStringToStatusArea(msg.GetBuffer(0));
    }

    // Make the button flash red so long as there are continue required items
    updateContinueButtonState(m_continueRequiredQueue.size() > 0);

    FUNCTION_EXIT;
}

void TrainLiveStatusDlg::OnTrainStatusCloseBtn()
{
    FUNCTION_ENTRY("OnTrainStatusCloseBtn");

    // End a live announcement if it is possible to do so
    if (m_endButton.IsWindowEnabled())
    {
        OnButtonEndLiveAnnouncement();
    }

    CDialog::OnCancel();

    FUNCTION_EXIT;
}

void TrainLiveStatusDlg::OnTrainStatusRetryFailuresBtn()
{
    FUNCTION_ENTRY("OnTrainStatusRetryFailuresBtn");

    attemptOperation(RETRY_FAILURES);

    FUNCTION_EXIT;
}

void TrainLiveStatusDlg::OnTrainStatusRetryAllBtn()
{
    FUNCTION_ENTRY("OnTrainStatusRetryAllBtn");

    attemptOperation(RETRY_ALL);

    FUNCTION_EXIT;
}
