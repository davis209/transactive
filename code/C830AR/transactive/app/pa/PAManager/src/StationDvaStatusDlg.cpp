/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/StationDvaStatusDlg.cpp $
 * @author Jade Lee
 * @version $Revision: #4 $
 * Last modification: $DateTime: 2025/02/10 22:34:14 $
 * Last modified by: $Author: limin.zhu $
 *
 * StationDvaStatusDlg handles GUI component of DVA station broadcast status dialog
 */

#include "app/pa/PAManager/src/stdafx.h"
#include "app/pa/PAManager/src/pamanager.h"
#include "app/pa/PAManager/src/StationDvaStatusDlg.h"
#include "app/pa/PAManager/src/DvaMessageChangeSubscriber.h"
#include "app/pa/PAManager/src/StationDvaStatusModel.h"
#include "app/pa/PAManager/src/PaLayoutHelper.h"
#include "bus/generic_gui/src/HelpLauncher.h"
#include "bus/generic_gui/src/TransactiveMessage.h" // TD14164

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_Base_Core;

StationDvaStatusDlg::StationDvaStatusDlg(
    // song toan++
    // TD 10316
    bool isOcc,
    // ++song toan
    // TD 10316
    const std::string& broadcastId,
    DvaMessageChangeSubscriber& dvaMessageChangeSubscriber,
    CWnd* pParent)
    :
    DvaStatusPage(
        // song toan++
        // TD 10316
        isOcc,
        // ++song toan
        // TD 10316
        StationDvaStatusDlg::IDD, pParent),
    m_dvaMessageChangeSubscriber(dvaMessageChangeSubscriber),
    m_openingBroadcastId(broadcastId),
    m_messagesToBroadcastLst(false),

    // **********  Added #8854
    // song toan++
    // TD 10316
    m_progressLst(isOcc, false),
    m_failuresLst(isOcc, true)
    // ++song toan
    // TD 10316
    // **********  #8854

{
    //{{AFX_DATA_INIT(StationDvaStatusDlg)
    m_chimeText = _T("");
    m_dwellSecs = 0;
    m_periodSecs = 0;
    // jeffrey++ TES712
    //m_priority = 0;
    // ++jeffrey TES712
    m_startTimeText = _T("");
    m_endTimeText = _T("");
    //}}AFX_DATA_INIT
    // jeffrey++ TD10377
    broadcastStateUnknownflag = false;
    // ++jeffrey TD10377
}

StationDvaStatusDlg::~StationDvaStatusDlg()
{
}

StationDvaStatusModel& StationDvaStatusDlg::getStationDvaStatusModel()
{
    return castPointerToReference<StationDvaStatusModel>(m_stationStatusModel);
}

GenericStatusModel* StationDvaStatusDlg::getStatusModelPointer()
{
    TA_ASSERT(0 != m_stationStatusModel, "Status model null");
    return m_stationStatusModel;
}

void StationDvaStatusDlg::DoDataExchange(CDataExchange* pDX)
{
    DvaStatusPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(StationDvaStatusDlg)
    DDX_Control(pDX, ID_STN_STATUS_RETRY_FAILURES_BTN, m_retryFailuresButton);
    DDX_Control(pDX, ID_STN_STATUS_RETRY_ALL_BTN, m_retryAllButton);
    DDX_Control(pDX, IDC_STATION_STATUS_BROADCAST_ID_TB, m_currentBroadcastIdTb);
    DDX_Control(pDX, IDC_STATION_STATUS_BROADCAST_ID_TB_NEW, m_newBroadcastIdTb);
    DDX_Control(pDX, IDC_STN_STATUS_PROGRESS_LST, m_progressLst);
    DDX_Control(pDX, IDC_STN_STATUS_FAILURES_LST, m_failuresLst);
    DDX_Control(pDX, IDC_STATION_SELECTED_MESSAGES_LST, m_messagesToBroadcastLst);
    DDX_Control(pDX, ID_STN_STATUS_CLOSE_BTN, m_closeButton);
    DDX_Control(pDX, ID_STN_STATUS_ABORT_BTN, m_abortButton);
    DDX_Text(pDX, IDC_STATION_STATUS_CHIME_TB, m_chimeText);
    DDX_Text(pDX, IDC_STATION_STATUS_DWELL_TB, m_dwellSecs);
    DDX_Text(pDX, IDC_STATION_STATUS_PERIOD_TB, m_periodSecs);
    // jeffrey++ TES712
    //DDX_Text(pDX, IDC_STATION_STATUS_PRIORITY_TB, m_priority);
    // ++jeffrey TES712
    DDX_Text(pDX, IDC_STATION_STATUS_START_TB, m_startTimeText);
    DDX_Text(pDX, IDC_STATION_STATUS_STOP_TB, m_endTimeText);
    DDX_Control(pDX, IDC_STATUS_TEXT, m_statusAreaText);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(StationDvaStatusDlg, DvaStatusPage)
    //{{AFX_MSG_MAP(StationDvaStatusDlg)
    ON_MESSAGE(WM_PA_DVA_MESSAGE_UPDATE, OnDvaMessageUpdate)
    ON_BN_CLICKED(ID_STN_STATUS_CLOSE_BTN, OnButtonClose)
    ON_BN_CLICKED(ID_STN_HELP, OnButtonHelp)
    ON_BN_CLICKED(ID_STN_STATUS_RETRY_FAILURES_BTN, OnButtonRetryFailures)
    ON_BN_CLICKED(ID_STN_STATUS_RETRY_ALL_BTN, OnButtonRetryAll)
    ON_BN_CLICKED(ID_STN_STATUS_ABORT_BTN, OnButtonAbort)
    ON_EN_CHANGE(IDC_STATION_STATUS_BROADCAST_ID_TB_NEW, OnChangeNewStatusName)
    ON_WM_DESTROY()
    // jeffrey++ TD10377
    ON_WM_TIMER()
    // ++jeffrey TD10377
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL StationDvaStatusDlg::OnInitDialog()
{
    m_stationStatusModel = new StationDvaStatusModel(*this, m_openingBroadcastId);

    DvaStatusPage::OnInitDialog();

    PaLayoutHelper::formatDvaMessageList(m_messagesToBroadcastLst, "Message", false);

    ///////////
    // Listen out for updates to any ad-hoc messages
    try
    {
        m_dvaMessageChangeSubscriber.addListenerWindow(getHWnd());
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // The ad-hoc list items will not be updated (nothing major, just can't view up to date info)
        PaErrorHandler::displayError(PaErrorHandler::INIT_ERROR);
    }

    // Fill out contents of the messages to broadcast list once off - must be done AFTER
    // the subscription to ad-hoc updates (above) - to ensure up to date
    // (this means fillOutBroadcastConfiguration needs to be done last)
    fillOutBroadcastConfiguration();
    // jeffrey++ TD10377
    SetTimer(1, 1000, NULL);	// enable and set timer interval to 1 second
    // ++jeffrey TD10377

    // TD 16863
    m_newBroadcastIdTb.SetLimitText(40);
    // TD 16863

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void StationDvaStatusDlg::fillOutBroadcastConfiguration()
{
    try
    {
        std::auto_ptr<TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig> bc =
            m_stationStatusModel->getBroadcastConfig();

        TA_Base_Bus::IPAAgentCorbaDef::StationDvaConfig& stationDvaConfig =
            bc->theBroadcastConfigSpecifics.theStationDvaConfig();

        m_chimeText = stationDvaConfig.hasChime ? "On" : "Off";
        // jeffrey++ TES712
        //m_priority = stationDvaConfig.priority;
        // ++jeffrey TES712
        m_dwellSecs = stationDvaConfig.dwellInSecs;

        m_periodSecs = stationDvaConfig.periodInSecs;

        if (0 == stationDvaConfig.startTime &&
            0 == stationDvaConfig.stopTime)
        {
            // Not a cyclical broadcast, no start / stop time defined
            // TD #3838, use "0" instead of empty string
            m_startTimeText = "0";
            m_endTimeText = "0";
        }
        else
        {
            // Agent stores UTC times, won't need any adjustment PaLayoutHelper::convertFromUTC
            time_t localStartTime = stationDvaConfig.startTime;
            time_t localStopTime = stationDvaConfig.stopTime;

            m_startTimeText = GenericStatusModel::formatDateIntoString(localStartTime);
            m_endTimeText = GenericStatusModel::formatDateIntoString(localStopTime);
        }

        // Convert message into type 'Keys'
        Keys messageKeys;

        for (int i = 0; i < stationDvaConfig.messages.length(); i++)
        {
            // TD 15843
            // Traverse the selected messages list in reverse order, because items
            // at bottom of list (visually) are located at the head of the array (because
            // they were added to the list most recently, and the old items pop off the top
            // of the list) - and we want items in the list to display correctly

            // messageKeys.push_back(stationDvaConfig.messages[i]);
            messageKeys.push_front(stationDvaConfig.messages[i]);
            // TD 15843
        }

        if (!m_messagesToBroadcastLst.refreshContents(messageKeys))
        {
            // Some of the messages no longer exist, warn the user
            // (this should rarely, if at all happen)
            PaErrorHandler::displayError(PaErrorHandler::DVA_MESSAGES_NO_LONGER_EXIST);
        }

        // Dispatch internal variable updates to UI
        UpdateData(FALSE);
    }
    catch (const TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdNotFound&)
    {
        PaErrorHandler::displayError(PaErrorHandler::BROADCAST_ID_NOT_FOUND);
        return;
    }
    catch (const CORBA::Exception&)
    {
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
        return;
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
        return;
    }
}

void StationDvaStatusDlg::OnButtonClose()
{
    bool shouldCloseDialog = tryUpdateStatusNameOnClose();

    if (shouldCloseDialog)
    {
        // May shut down window as per normal
        CDialog::OnCancel();
    }
}

void StationDvaStatusDlg::OnButtonHelp()
{
    TA_Base_Bus::HelpLauncher::getInstance().displayHelp();
}

void StationDvaStatusDlg::OnButtonRetryFailures()
{
    attemptOperation(RETRY_FAILURES);
}

void StationDvaStatusDlg::OnButtonRetryAll()
{
    attemptOperation(RETRY_ALL);
}

void StationDvaStatusDlg::OnButtonAbort()
{
    // Notify the PA Agent to stop playing messages on broadcast ID,
    // This does not exit the system, still must hit close to do that
    attemptOperation(TERMINATE_BROADCAST);
}

void StationDvaStatusDlg::OnChangeNewStatusName()
{
    // Just pass on to generic implementation
    DvaStatusPage::OnChangeNewStatusName();
}

void StationDvaStatusDlg::onCurrentStatusNameDeleted()
{
    DvaStatusPage::onCurrentStatusNameDeleted();

    // Disable all but close window
    m_retryAllButton.EnableWindow(FALSE);
    m_retryFailuresButton.EnableWindow(FALSE);
    m_newBroadcastIdTb.EnableWindow(FALSE);
    m_abortButton.EnableWindow(FALSE);
}

void StationDvaStatusDlg::destroyStatusModel()
{
    delete m_stationStatusModel;
    m_stationStatusModel = 0;
}

afx_msg LRESULT StationDvaStatusDlg::OnDvaMessageUpdate(UINT wParam, LONG lParam)
{
    LOG_SCOPE("WM_PA_DVA_MESSAGE_UPDATE", "StationDvaStatusDlg::OnDvaMessageUpdate");

    if (!m_dvaMessageChangeSubscriber.isConnectedListenerWindow(getHWnd()))
    {
        // Probably in the closing down stage, receiving last few messages
        // while unsubscribed - do NOT respond to this notification in this case
        return 0;
    }

    try
    {
        const DvaMessageUpdateData updatedKeys
            = m_dvaMessageChangeSubscriber.getMessageData(wParam, getHWnd());

        DvaMessageUpdateData modifiedKeys;
        DvaMessageUpdateData deletedKeys;

        bool typeChangesDetected = false;
        bool labelChangesDetected = false;

        // Need to check for changes to selected messages list
        if (m_messagesToBroadcastLst.collectMatchingKeys(updatedKeys,
                                                         modifiedKeys,
                                                         deletedKeys,
                                                         &typeChangesDetected,
                                                         &labelChangesDetected))
        {
            if (typeChangesDetected && deletedKeys.size() > 0)
            {
                // Warn the user of the dynamic change - one of messages, an ad-hoc message,
                // has *just* been deleted
                // (but don't literally delete the message from the control, just display
                // whitespace for that item - still showing ID, that way operator
                // knows a deleted message is present in this broadcast)
                PaErrorHandler::displayError(PaErrorHandler::DVA_MESSAGES_NO_LONGER_EXIST);
            }

            // Process ALL updates (so a deleted message becomes visually cleared)
            m_messagesToBroadcastLst.processDvaMessageUpdates(updatedKeys);
        }
    }
    catch (const TA_Base_Core::ValueNotSetException&)
    {
        // This window not recognized by subscriber - adhoc won't be updated
        // But it really should be impossible for this to happen..
        PaErrorHandler::displayError(PaErrorHandler::DVA_MESSAGE_UPDATE_FAILURE);
        return 0;
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        TA_ASSERT(FALSE, "isConnectedListenerWindow should stop this assertion");
    }

    return 0;
}

void StationDvaStatusDlg::OnDestroy()
{
    try
    {
        m_dvaMessageChangeSubscriber.removeListenerWindow(m_hWnd);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // Cannot unsubscribe ad hoc message change listener
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }

    DvaStatusPage::OnDestroy();
}

// AZ++ TD10377 -- added check for broadcast ID not found
// jeffrey++ TD10377
// Test link status with PAAgent. If agent is down, throw exception.
// Then update broadcast status to "Unknown". Once the agent is back,
// request progress report update from agent.
void StationDvaStatusDlg::OnTimer(UINT nIDEvent)
{
    LOG_SCOPE("StationDvaStatusDlg::OnTimer");

    try
    {
        {
            LOG_NONE();
            CORBA_CALL(CachedConfig::getInstance()->getAgentObject(), agentLinkTest, ());
        }

        if (broadcastStateUnknownflag)
        {
            try
            {
                getStatusModel().forceProgressReportUpdate(false);
                broadcastStateUnknownflag = false;
            }
            catch (const TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdNotFound&)
            {
                broadcastStateUnknownflag = false;
                // TD14164 ++
                /*MessageBox("Broadcast ID was not recognised");*/
                TA_Base_Bus::TransActiveMessage userMsg;
                userMsg.showMsgBox(IDS_UE_610019);
                // ++ TD14164
                CDialog::OnCancel();
            }
            catch (...)
            {
            }
        }
    }
    catch (...)
    {
        if (CachedConfig::getInstance()->isAtOcc())
        {
            for (int i = 0; i < m_progressLst.GetItemCount(); i++)
            {
                m_progressLst.SetItemText(i, 2, "Unknown");
            }
        }
        else
        {
            for (int i = 0; i < m_progressLst.GetItemCount(); i++)
            {
                m_progressLst.SetItemText(i, 1, "Unknown");
            }
        }

        broadcastStateUnknownflag = true;
    }

    DvaStatusPage::OnTimer(nIDEvent);
}

// ++jeffrey TD10377
// ++AZ TD10377

// TD16865
std::map<unsigned int, std::string>& StationDvaStatusDlg::getTrainIdAndServiceNumber()
{
    TA_ASSERT(false, "This method is not used in the station broadcast");
    std::map<unsigned int, std::string> temp;
    return temp;
}

// TD16865
