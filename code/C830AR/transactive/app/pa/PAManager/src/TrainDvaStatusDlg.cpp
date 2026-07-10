/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainDvaStatusDlg.cpp $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * TrainDvaStatusDlg handles GUI component of DVA train broadcast status dialog
 */

#include "app/pa/PAManager/src/stdafx.h"

#include "app/pa/PAManager/src/CachedMapChangedWindowNotificationHandler.h"
#include "app/pa/PAManager/src/PaErrorHandler.h"
#include "app/pa/PAManager/src/PaLayoutHelper.h"
#include "app/pa/PAManager/src/TrainDvaStatusDlg.h"
#include "app/pa/PAManager/src/TrainDvaStatusModel.h"
#include "app/pa/PAManager/src/pamanager.h"

#include "bus/generic_gui/src/HelpLauncher.h"
#include "bus/pa/cachedmaps/src/PaTrainDvaMessageCachedMap.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_IRS_Bus;

TrainDvaStatusDlg::TrainDvaStatusDlg(const std::string& broadcastId,
                                     CWnd* pParent /*=NULL*/)
    :
    DvaStatusPage(
        // song toan++
        // TD 10316
        false,
        // ++song toan
        // TD 10316
        TrainDvaStatusDlg::IDD, pParent),
    m_openingBroadcastId(broadcastId),
    m_trainDvaUpdateNotificationHandler(0),
    m_dvaMessageKey(TA_Base_Core::DatabaseKey::getInvalidKey()),

    // **********  Added #8854
    // song toan++
    // TD 10316
    m_progressList(false, true),
    m_failuresList(false, false)
    // ++song toan
    // TD 10316

    // **********  #8854
{
    FUNCTION_ENTRY("TrainDvaStatusDlg");

    //{{AFX_DATA_INIT(TrainDvaStatusDlg)
    m_startTime = _T("");
    m_stopTime = _T("");
    m_messageString = _T("");
    // jeffrey++ TD7956
    m_override = false;
    // ++jeffrey TD7956
    //}}AFX_DATA_INIT

    FUNCTION_EXIT;
}

// jeffrey++ TD7956
// TD16865
TrainDvaStatusDlg::TrainDvaStatusDlg(const std::string& broadcastId, bool OverrideFlag,
                                     std::map<unsigned int, std::string> trainIDToServiceNumber, CWnd* pParent /*=NULL*/)
    : DvaStatusPage(false, TrainDvaStatusDlg::IDD, pParent),
    m_openingBroadcastId(broadcastId),
    m_trainDvaUpdateNotificationHandler(0),
    m_dvaMessageKey(TA_Base_Core::DatabaseKey::getInvalidKey()),
    m_progressList(false, true),
    m_failuresList(false, false),
    m_override(OverrideFlag),
    m_trainIdServiceNum(trainIDToServiceNumber)
{
    FUNCTION_ENTRY("TrainDvaStatusDlg");

    m_startTime = _T("");
    m_stopTime = _T("");
    m_messageString = _T("");

    FUNCTION_EXIT;
}

// TD16865
// ++jeffrey TD7956

TrainDvaStatusDlg::~TrainDvaStatusDlg()
{
    FUNCTION_ENTRY("~TrainDvaStatusDlg");

    TA_ASSERT(0 == m_trainDvaUpdateNotificationHandler, "Shouldn't be valid still");

    FUNCTION_EXIT;
}

void TrainDvaStatusDlg::DoDataExchange(CDataExchange* pDX)
{
    FUNCTION_ENTRY("DoDataExchange");

    DvaStatusPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(TrainDvaStatusDlg)
    DDX_Control(pDX, ID_TRAIN_STATUS_RETRY_FAILURES_BTN, m_retryFailuresButton);
    DDX_Control(pDX, ID_TRAIN_STATUS_RETRY_ALL_BTN, m_retryAllButton);
    DDX_Control(pDX, ID_TRAIN_STATUS_ABORT_BTN, m_abortButton);
    DDX_Control(pDX, IDC_TRAIN_STATUS_PROGRESS_LST, m_progressList);
    DDX_Control(pDX, IDC_TRAIN_STATUS_FAILURES_LST, m_failuresList);
    DDX_Control(pDX, IDC_TRAIN_STATUS_BROADCAST_ID_TB_NEW, m_newStatusNameControl);
    DDX_Control(pDX, IDC_TRAIN_STATUS_BROADCAST_ID_TB, m_currentStatusNameControl);
    DDX_Text(pDX, IDC_TRAIN_STATUS_START_TB, m_startTime);
    DDX_Text(pDX, IDC_TRAIN_STATUS_STOP_TB, m_stopTime);
    DDX_Text(pDX, IDC_TRAIN_MESSAGE_TO_BROADCAST, m_messageString);
    //}}AFX_DATA_MAP

    FUNCTION_EXIT;
}

BEGIN_MESSAGE_MAP(TrainDvaStatusDlg, DvaStatusPage)
    //{{AFX_MSG_MAP(TrainDvaStatusDlg)
    ON_BN_CLICKED(ID_TRAIN_STATUS_CLOSE_BTN, OnButtonClose)
    ON_BN_CLICKED(ID_TRAIN_STATUS_RETRY_FAILURES_BTN, OnButtonRetryFailures)
    ON_BN_CLICKED(ID_TRAIN_STATUS_RETRY_ALL_BTN, OnButtonRetryAll)
    ON_BN_CLICKED(ID_TRAIN_STATUS_ABORT_BTN, OnButtonAbort)
    ON_EN_CHANGE(IDC_TRAIN_STATUS_BROADCAST_ID_TB_NEW, OnChangeNewStatusName)
    ON_BN_CLICKED(IDC_RADIO_NORMAL, OnRadioNormal)
    ON_BN_CLICKED(IDC_RADIO_OVERRIDE, OnRadioOverride)
    ON_WM_DESTROY()
    ON_MESSAGE(WM_PA_TRAIN_DVA_MESSAGE_UPDATE, OnTrainDvaMessageUpdate)
    ON_BN_CLICKED(ID_TRAIN_HELP, OnTrainHelp)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

TrainDvaStatusModel& TrainDvaStatusDlg::getTrainDvaStatusModel()
{
    FUNCTION_ENTRY("getTrainDvaStatusModel");
    FUNCTION_EXIT;
    return castPointerToReference<TrainDvaStatusModel>(m_trainStatusModel);
}

GenericStatusModel* TrainDvaStatusDlg::getStatusModelPointer()
{
    FUNCTION_ENTRY("getStatusModelPointer");

    TA_ASSERT(0 != m_trainStatusModel, "Status model null");

    FUNCTION_EXIT;
    return m_trainStatusModel;
}

BOOL TrainDvaStatusDlg::OnInitDialog()
{
    FUNCTION_ENTRY("OnInitDialog");

    // Create status model as soon as dialog present
    m_trainStatusModel = new TrainDvaStatusModel(*this, m_openingBroadcastId);

    DvaStatusPage::OnInitDialog();

    // Create a handler so we can receive zone change notifications from config editor
    m_trainDvaUpdateNotificationHandler =
        new CachedMapChangedWindowNotificationHandler(
            m_hWnd,
            WM_PA_TRAIN_DVA_MESSAGE_UPDATE,
            TA_IRS_Bus::CachedMaps::getInstance()->getPaTrainDvaMessageCachedMap());

        // TD16862
        // jeffrey++ TD7956
        // Default to the normal retry mode
    OnRadioNormal();
    //    if(m_override)
    //        OnRadioOverride();
    //    else
    //        OnRadioNormal();
    // ++jeffrey TD7956
    // TD16862

    // Display details about the current broadcast
    fillOutBroadcastConfiguration();

    FUNCTION_EXIT;
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void TrainDvaStatusDlg::fillOutBroadcastConfiguration()
{
    FUNCTION_ENTRY("fillOutBroadcastConfiguration");

    try
    {
        std::auto_ptr<TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig> bc =
            m_trainStatusModel->getBroadcastConfig();

        TA_Base_Bus::IPAAgentCorbaDef::TrainDvaConfig& trainDvaConfig =
            bc->theBroadcastConfigSpecifics.theTrainDvaConfig();

        m_dvaMessageKey = trainDvaConfig.messageKey;
        m_messageString = getMessageStringFromKey(m_dvaMessageKey).c_str();

        const TA_Base_Bus::ITrainPaCorba::MessageSchedule& period = trainDvaConfig.announcementPeriod;

        if (0 == period.period && 0 == period.duration)
        {
            // When all time values zeroed out, means not a cyclical broadcast, so leave boxes empty
            // TD #3838, use "0" instead of empty
            m_startTime = "0";
            m_stopTime = "0";
        }
        else
        {
            // Temporary fix to get the period and duration from the time interval structure
            CString result;

            // Period is in the second half of the structure
            // Duration is in the first half of the structure
            // The hour is the low order and the minute is the high order
            int periodTime = period.period;
            int durationTime = period.duration;

            int periodHour = periodTime / 3600;
            int periodMinute = (periodTime - periodHour * 3600) / 60;
            int periodSecond = periodTime - periodHour * 3600 - periodMinute * 60;

            int durationHour = durationTime / 3600;
            int durationMinute = (durationTime - durationHour * 3600) / 60;
            int durationSecond = durationTime - durationHour * 3600 - durationMinute * 60;

            result.Format("%02d:%02d:%02d", periodHour, periodMinute, periodSecond);
            m_startTime = result;
            result.Format("%02d:%02d:%02d", durationHour, durationMinute, durationSecond);
            m_stopTime = result;
        }

        // Dispatch internal variable updates to UI
        UpdateData(FALSE);
    }
    catch (const TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdNotFound&)
    {
        PaErrorHandler::displayError(PaErrorHandler::BROADCAST_ID_NOT_FOUND);

        FUNCTION_EXIT;
        return;
    }
    catch (const CORBA::Exception&)
    {
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);

        FUNCTION_EXIT;
        return;
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);

        FUNCTION_EXIT;
        return;
    }

    FUNCTION_EXIT;
}

std::string TrainDvaStatusDlg::getMessageStringFromKey(unsigned long messageKey)
{
    FUNCTION_ENTRY("getMessageStringFromKey");

    try
    {
        TA_IRS_Bus::TrainDvaMessageRecord trainMessage =
            TA_IRS_Bus::CachedMaps::getInstance()->getTrainDvaMessageRecordFromKey(messageKey);

        FUNCTION_EXIT;
        return trainMessage.m_label;
    }
    catch (const TA_Base_Core::CachedMappingNotFoundException&)
    {
        // Couldn't find this message in the database - someone has deleted it
        PaErrorHandler::displayError(PaErrorHandler::DVA_MESSAGES_NO_LONGER_EXIST);

        FUNCTION_EXIT;
        return CachedConfig::getInstance()->getDeletedTrainDvaMessageName();
    }
}

void TrainDvaStatusDlg::destroyStatusModel()
{
    FUNCTION_ENTRY("destroyStatusModel");

    delete m_trainStatusModel;
    m_trainStatusModel = 0;

    FUNCTION_EXIT;
}

void TrainDvaStatusDlg::OnButtonClose()
{
    FUNCTION_ENTRY("OnButtonClose");

    bool shouldCloseDialog = tryUpdateStatusNameOnClose();

    if (shouldCloseDialog)
    {
        // May shut down window as per normal
        CDialog::OnCancel();
    }

    FUNCTION_EXIT;
}

void TrainDvaStatusDlg::OnButtonRetryFailures()
{
    FUNCTION_ENTRY("OnButtonRetryFailures");

    attemptOperation(RETRY_FAILURES);

    FUNCTION_EXIT;
}

void TrainDvaStatusDlg::OnButtonRetryAll()
{
    FUNCTION_ENTRY("OnButtonRetryAll");

    attemptOperation(RETRY_ALL);

    FUNCTION_EXIT;
}

void TrainDvaStatusDlg::OnButtonAbort()
{
    FUNCTION_ENTRY("OnButtonAbort");

    // Notify the PA Agent to stop playing messages on broadcast ID,
    // This does not exit the system, still must hit close to do that
    attemptOperation(TERMINATE_BROADCAST);

    FUNCTION_EXIT;
}

void TrainDvaStatusDlg::OnChangeNewStatusName()
{
    FUNCTION_ENTRY("OnChangeNewStatusName");

    // Just pass on to generic implementation
    DvaStatusPage::OnChangeNewStatusName();

    FUNCTION_EXIT;
}

void TrainDvaStatusDlg::onCurrentStatusNameDeleted()
{
    FUNCTION_ENTRY("onCurrentStatusNameDeleted");

    DvaStatusPage::onCurrentStatusNameDeleted();

    // Disable all but close window
    m_retryAllButton.EnableWindow(FALSE);
    m_retryFailuresButton.EnableWindow(FALSE);
    m_newStatusNameControl.EnableWindow(FALSE);
    m_abortButton.EnableWindow(FALSE);

    // Disable radio buttons
    GetDlgItem(IDC_RADIO_NORMAL)->EnableWindow(FALSE);
    GetDlgItem(IDC_RADIO_OVERRIDE)->EnableWindow(FALSE);

    FUNCTION_EXIT;
}

void TrainDvaStatusDlg::OnRadioOverride()
{
    FUNCTION_ENTRY("OnRadioOverride");

    CheckRadioButton(IDC_RADIO_NORMAL, IDC_RADIO_OVERRIDE, IDC_RADIO_OVERRIDE);

    // Override has been set
    m_trainStatusModel->setHasOverrideOption(true);

    FUNCTION_EXIT;
}

void TrainDvaStatusDlg::OnRadioNormal()
{
    FUNCTION_ENTRY("OnRadioNormal");

    CheckRadioButton(IDC_RADIO_NORMAL, IDC_RADIO_OVERRIDE, IDC_RADIO_NORMAL);

    // Normal mode has been set
    m_trainStatusModel->setHasOverrideOption(false);

    FUNCTION_EXIT;
}

void TrainDvaStatusDlg::OnDestroy()
{
    FUNCTION_ENTRY("OnDestroy");

    // Cannot delete update notification objects after window made
    // invalid, so must do here
    delete m_trainDvaUpdateNotificationHandler;
    m_trainDvaUpdateNotificationHandler = 0;

    DvaStatusPage::OnDestroy();

    FUNCTION_EXIT;
}

afx_msg LRESULT TrainDvaStatusDlg::OnTrainDvaMessageUpdate(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnTrainDvaMessageUpdate");

    if (0 == m_trainDvaUpdateNotificationHandler)
    {
        FUNCTION_EXIT;
        return 0;
    }

    try
    {
        DWORD changedItemKey = wParam;

        if (m_dvaMessageKey == changedItemKey)
        {
            // This update needs to be responded to (the key matches
            // the message key of
            m_messageString = getMessageStringFromKey(m_dvaMessageKey).c_str();
            // Display the changes
            UpdateData(FALSE);
        }
    }

    STANDARD_PA_CATCH("TrainDvaStatusDlg::OnTrainDvaMessageUpdate", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

void TrainDvaStatusDlg::OnTrainHelp()
{
    FUNCTION_ENTRY("OnTrainHelp");

    TA_Base_Bus::HelpLauncher::getInstance().displayHelp();

    FUNCTION_EXIT;
}
