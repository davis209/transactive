/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/StationBroadcastPage.cpp $
 * @author Sean Liew
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 */

#include "app/pa/PAManager/src/stdafx.h"

#include <afxdtctl.h>

#include "app/pa/PAManager/src/AdhocMessageRecordPage.h"
#include "app/pa/PAManager/src/CachedMapChangedWindowNotificationHandler.h"
#include "app/pa/PAManager/src/CreateNewGroupDlg.h"
#include "app/pa/PAManager/src/DvaMessageChangeSubscriber.h"
#include "app/pa/PAManager/src/HmiSelectionListener.h"
#include "app/pa/PAManager/src/LockControlRedraw.h"
#include "app/pa/PAManager/src/OperatorTerminalDvaPlaybackModel.h"
#include "app/pa/PAManager/src/PAManagerDlg.h"
#include "app/pa/PAManager/src/PaLayoutHelper.h"
#include "app/pa/PAManager/src/PaRightsManager.h"
#include "app/pa/PAManager/src/PaStatusNameChangeSubscriber.h"
#include "app/pa/PAManager/src/RightsChangedWindowNotificationHandler.h"
#include "app/pa/PAManager/src/StationBroadcastModel.h"
#include "app/pa/PAManager/src/StationBroadcastPage.h"
#include "app/pa/PAManager/src/StationDvaStatusDlg.h"
#include "app/pa/PAManager/src/StationLiveStatusDlg.h"

#include "bus/generic_gui/src/TransActiveMessage.h"
#include "bus/pa/cachedmaps/src/ICachedMapObserverRegistry.h"
#include "bus/pa/cachedmaps/src/PaZoneCachedMap.h"
#include "bus/pa/cachedmaps/src/PaZoneGroupCachedMap.h"

#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/stdutil/strings.h"
#include <boost/range/algorithm.hpp>

#include <assert.h>
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_IRS_Bus;

static const COleDateTime       NULL_TIME(2000, 1, 1, 0, 0, 0);
static const COleDateTimeSpan   NULL_TIMESPAN(NULL_TIME);

static const COleDateTime       DEFAULT_DWELL_TIME(2000, 1, 1, 0, 0, 3); //3 seconds
static const COleDateTime       MAX_DWELL_TIME(2000, 1, 1, 0, 0, 10);    // 10 seconds

static const COleDateTime       DEFAULT_INTERVAL_TIME(2000, 1, 1, 0, 1, 0); // 1 minute

const std::string StationBroadcastPage::s_enterNonZeroLengthNameMessage =
"Please enter a valid label.";
const std::string StationBroadcastPage::s_zoneGroupNameAlreadyExistsMessage =
"The label entered is already in use, overwrite existing zone group?";

// UW-610009
const std::string StationBroadcastPage::s_zoneGroupNameSystemReservedMessage =
"The label entered is reserved for system use, please enter a different label.";

const std::string StationBroadcastPage::s_defaultZoneGroupName =
"Unnamed Zone Group";

const std::string StationBroadcastPage::s_enterZoneGroupNamePrompt =
"Please enter a label for the new zone group";
const std::string StationBroadcastPage::s_enterZoneGroupNameWindowTitle =
"PA Manager - Add Zone Group";

const std::string ADHOCLIST_TITLE = "Adhoc Messages";
const std::string ADHOCLIST_FULL_TITLE = "E  " + ADHOCLIST_TITLE;

static const DWORD COLOR_RED = RGB(255, 0, 0);

//IMPLEMENT_DYNCREATE(StationBroadcastPage, CPropertyPage)

StationBroadcastPage::StationBroadcastPage(DvaMessageChangeSubscriber& adhocSubscriber, PAManagerDlg* paManagerDlg)
    : CPropertyPage(StationBroadcastPage::IDD),
    m_paManagerDlg(paManagerDlg),
    m_stationBroadcastModel(NULL),
    m_dvaMessageChangeSubscriber(adhocSubscriber),
    m_adhocWindowDisabled(0x0),
    m_pageActive(false),
    m_dvaPlaybackObject(new OperatorTerminalDvaPlaybackModel()),
    m_hmiZoneSelChangeListener(0),
    m_freezePlayStopRecordButtonUpdate(false),
    m_rightsChangeHandler(0),
    m_adhocStyleChangeInProgress(false),
    // TD #10335
    // We don't want the Ad-Hoc Message list to be sorted, otherwise the ordering
    // won't correspond to the ad-hoc messages on the DVA Set Version page.
    m_adhocLst(false),
    m_normalLst(true),
    m_emergencyLst(true),
    m_selectedLst(false)
{
    FUNCTION_ENTRY("StationBroadcastPage");

    m_stationBroadcastModel = new StationBroadcastModel(*this);
    // song toan++
    // TD 10316
    m_LocationType = m_stationBroadcastModel->getLocationType();
    // ++song toan
    // TD 10316
    //{{AFX_DATA_INIT(StationBroadcastPage)
    //}}AFX_DATA_INIT

    FUNCTION_EXIT;
}

StationBroadcastPage::~StationBroadcastPage()
{
    FUNCTION_ENTRY("~StationBroadcastPage");

    TA_ASSERT(0 == m_rightsChangeHandler, "Rights change handler should be deleted in OnDestroy");

    // This should be deleted in OnDestroy()
    TA_ASSERT(m_hmiZoneSelChangeListener == 0, "m_hmiZoneSelChangeListener still present");

    TA_ASSERT(m_cachedMapObservers.size() == 0, "Observers should be cleared out in DestroyWindow");

    try
    {
        destroyIcons();

        delete m_dvaPlaybackObject;
        m_dvaPlaybackObject = NULL;

        delete m_stationBroadcastModel;
        m_stationBroadcastModel = NULL;
    }
    catch (...)
    {
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "Caught in destructor");
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::DoDataExchange(CDataExchange* pDX)
{
    FUNCTION_ENTRY("DoDataExchange");

    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(StationBroadcastPage)
    DDX_Control(pDX, IDC_STN_CLEAR_ONE_BTN, m_clearOneBtn);
    DDX_Control(pDX, IDC_STN_CLEAR_ALL_BTN, m_clearAllBtn);
    DDX_Control(pDX, IDC_STN_MUSIC_BTN, m_stnMusicButton);
    DDX_Control(pDX, IDC_OCC_MUSIC_BTN, m_occMusicButton);
    DDX_Control(pDX, IDC_STN_DELETE_BTN, m_deleteZoneGroupButton);
    DDX_Control(pDX, IDC_STN_SAVE_BTN, m_saveZoneGroupButton);
    DDX_Control(pDX, IDC_STN_ZONE_GROUP_COMBO, m_zoneCombo);
    DDX_Control(pDX, IDC_CYCLICAL_START_DATE, m_cyclicalStartDate);
    DDX_Control(pDX, IDC_CYCLICAL_STOP_DATE, m_cyclicalStopDate);
    DDX_Control(pDX, IDC_STN_MB_DWELL_SEC, m_multiBroadcastDwellTimeSeconds);
    DDX_Control(pDX, IDC_STN_CYCLICAL_INTERVAL, m_cyclicalInterval);
    DDX_Control(pDX, IDC_STN_CYCLICAL_STOP_TIME, m_cyclicalStopTime);
    DDX_Control(pDX, IDC_STN_CYCLICAL_START_TIME, m_cyclicalStartTime);
    DDX_Control(pDX, IDC_STN_ZONE_SELECTION_LST, m_zoneLst);
    DDX_Control(pDX, IDC_STN_ADHOC_MESSAGES_LST, m_adhocLst);
    DDX_Control(pDX, IDC_STN_STIS_CB, m_stisCb);
    DDX_Control(pDX, IDC_STN_CHIME_CB, m_chimeCb);
    DDX_Control(pDX, IDC_STN_STOP_BTN, m_stopBtn);
    DDX_Control(pDX, IDC_STN_EMERGENCY_FILTER_TB, m_emergencyFilterTb);
    DDX_Control(pDX, IDC_STN_NORMAL_FILTER_TB, m_normalFilterTb);
    DDX_Control(pDX, IDC_STN_SELECTED_MESSAGES_LST, m_selectedLst);
    DDX_Control(pDX, IDC_STN_RECORD_BTN, m_recordBtn);
    DDX_Control(pDX, IDC_STN_PLAY_BTN, m_playBtn);
    DDX_Control(pDX, IDC_STN_NORMAL_MESSAGES_LST, m_normalLst);
    DDX_Control(pDX, IDC_STN_EMERGENCY_MESSAGES_LST, m_emergencyLst);
    DDX_Control(pDX, IDC_STN_LIVE_BTN, m_liveBtn);
    DDX_Control(pDX, IDC_STN_DVA_BTN, m_dvaBtn);
    DDX_Control(pDX, IDC_STN_CYCLIC_CB, m_cyclicCb);
    DDX_Control(pDX, IDC_STN_MULTI_CB, m_multiCb);
    //}}AFX_DATA_MAP

    FUNCTION_EXIT;
}

BEGIN_MESSAGE_MAP(StationBroadcastPage, CPropertyPage)
    //{{AFX_MSG_MAP(StationBroadcastPage)
    ON_BN_CLICKED(IDC_STN_CYCLIC_CB, OnCyclicCb)
    ON_BN_CLICKED(IDC_STN_MULTI_CB, OnMultiCb)
    ON_BN_CLICKED(IDC_STN_DVA_BTN, OnDvaBtn)
    ON_BN_CLICKED(IDC_STN_LIVE_BTN, OnLiveBtn)
    ON_EN_CHANGE(IDC_STN_NORMAL_FILTER_TB, OnChangeNormalFilterTb)
    ON_EN_CHANGE(IDC_STN_EMERGENCY_FILTER_TB, OnChangeEmergencyFilterTb)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_STN_SELECTED_MESSAGES_LST, OnItemchangedSelectedMessagesLst)
    ON_BN_CLICKED(IDC_STN_CLEAR_ALL_BTN, OnClearAllBtn)
    ON_BN_CLICKED(IDC_STN_CHIME_CB, OnChimeCb)
    ON_BN_CLICKED(IDC_STN_STIS_CB, OnStisCb)
    ON_MESSAGE(WM_PA_DVA_MESSAGE_UPDATE, OnDvaMessageUpdate)
    ON_MESSAGE(WM_PA_STN_STATUS_NAME_UPDATE, OnStatusNameChange)
    ON_MESSAGE(WM_PA_STN_STATUS_REPORT_UPDATE, OnProgressReportUpdate)
    ON_MESSAGE(WM_PA_STN_ZONE_GROUP_UPDATE, OnZoneGroupUpdate)
    ON_MESSAGE(WM_PA_STN_ZONE_UPDATE, OnZoneUpdate)
    ON_MESSAGE(WM_PA_STN_HMI_ZONE_SEL_CHANGE, OnHMIZoneSelectionChange)
    ON_NOTIFY(LVN_ENDLABELEDIT, IDC_STN_ADHOC_MESSAGES_LST, OnEndlabeleditAdhocMessagesLst)
    ON_NOTIFY(DTN_DATETIMECHANGE, IDC_CYCLICAL_STOP_DATE, OnDatetimechangeStopDtp)
    ON_NOTIFY(DTN_DATETIMECHANGE, IDC_CYCLICAL_START_DATE, OnDatetimechangeStartDtp)
    ON_BN_CLICKED(IDC_STN_CLEAR_ONE_BTN, OnClearOneBtn)
    ON_NOTIFY(NM_DBLCLK, IDC_STN_NORMAL_MESSAGES_LST, OnDblclkNormalMessagesLst)
    ON_NOTIFY(NM_DBLCLK, IDC_STN_EMERGENCY_MESSAGES_LST, OnDblclkEmergencyMessagesLst)
    ON_NOTIFY(NM_DBLCLK, IDC_STN_ADHOC_MESSAGES_LST, OnDblclkAdhocMessagesLst)
    ON_NOTIFY(DTN_DATETIMECHANGE, IDC_STN_MB_DWELL_SEC, OnDatetimechangeStnDwellDtp)
    ON_WM_MOUSEMOVE()
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_CTLCOLOR()
    ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_STN_ADHOC_MESSAGES_LST, OnBeginlabeleditStnAdhocMessagesLst)
    ON_NOTIFY(LVN_ITEMCHANGING, IDC_STN_ADHOC_MESSAGES_LST, OnItemchangingStnAdhocMessagesLst)
    ON_BN_CLICKED(IDC_STN_SAVE_BTN, OnZoneGroupSaveBtn)
    ON_BN_CLICKED(IDC_STN_DELETE_BTN, OnZoneGroupDeleteBtn)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_STN_ZONE_SELECTION_LST, OnItemchangedStnZoneSelectionLst)
    ON_CBN_SELCHANGE(IDC_STN_ZONE_GROUP_COMBO, OnSelchangeStnZoneGroupCombo)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_STN_NORMAL_MESSAGES_LST, OnItemchangedStnNormalMessagesLst)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_STN_EMERGENCY_MESSAGES_LST, OnItemchangedStnEmergencyMessagesLst)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_STN_ADHOC_MESSAGES_LST, OnItemchangedStnAdhocMessagesLst)
    ON_BN_CLICKED(IDC_STN_MUSIC_BTN, OnStnMusicBtn)
    ON_BN_CLICKED(IDC_OCC_MUSIC_BTN, OnOccMusicBtn)
    ON_BN_CLICKED(IDC_STN_PLAY_BTN, OnStnPlayBtn)
    ON_BN_CLICKED(IDC_STN_RECORD_BTN, OnStnRecordBtn)
    ON_BN_CLICKED(IDC_STN_STOP_BTN, OnStnStopBtn)
    ON_WM_DESTROY()
    ON_NOTIFY(DTN_DATETIMECHANGE, IDC_STN_CYCLICAL_INTERVAL, OnDatetimechangeStnIntervalDtp)
    ON_NOTIFY(DTN_DATETIMECHANGE, IDC_STN_CYCLICAL_START_TIME, OnDatetimechangeStartDtp)
    ON_NOTIFY(DTN_DATETIMECHANGE, IDC_STN_CYCLICAL_STOP_TIME, OnDatetimechangeStopDtp)
    ON_NOTIFY(LVN_ITEMCHANGING, IDC_STN_ZONE_SELECTION_LST, OnItemchangingStnZoneSelectionLst)
    ON_MESSAGE(WM_PA_RIGHTS_CHANGE, OnRightsChanged)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// StationBroadcastPage message handlers

BOOL StationBroadcastPage::OnInitDialog()
{
    LOG_SCOPE("StationBroadcastPage::OnInitDialog");
    FUNCTION_ENTRY("OnInitDialog");

    m_rightsChangeHandler =  new RightsChangedWindowNotificationHandler(m_hWnd);

    CPropertyPage::OnInitDialog();

    // Stop multiple active selections on these three lists
    m_dvaMessageListSelController.registerList(&m_adhocLst);
    m_dvaMessageListSelController.registerList(&m_normalLst);
    m_dvaMessageListSelController.registerList(&m_emergencyLst);

    // Begin subscription to ad hoc message changes first off - need to do this before
    // filling out the ad hoc list (to ensure cachedMaps updated - hence we're up to date)
    try
    {
        m_dvaMessageChangeSubscriber.addListenerWindow(m_hWnd);
        PaStatusNameChangeSubscriber::getInstance().addListenerWindow(m_hWnd);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // The ad-hoc list items will not be updated (nothing major, just can't view up to date info)
        PaErrorHandler::displayError(PaErrorHandler::INIT_ERROR);
    }

    createCachedMapObservers();

    m_zoneLst.initialise(CachedConfig::getInstance()->getAgentObject());
    // Map all items with an itemData setting of zero to be coloured red
    // (meaning HMI is coloured as red)
    m_zoneCombo.mapItemDataToColour(0, COLOR_RED);
    refreshZoneComboData();
    refreshZoneList();

    // Set up the formats for various input controls
    setupDateTimeEntryControls();

    /////////////////////////
    // Setup Misc.
    /////////////////////////

    OnMultiCb();
    OnCyclicCb();

    refreshBroadcastButtonEnabledState();
    // Enter a default initial broadcast state (may vary depending
    // on the operator priveleges set up)
    enterDefaultBroadcastMode();

    m_chimeCb.SetCheck(1);
    m_stisCb.SetCheck(0);
    OnChimeCb();
    OnStisCb();

    updateExecuteButtonEnabledState();

    m_normalFilterTb.SetLimitText(MAX_FILTER_TEXT);
    m_emergencyFilterTb.SetLimitText(MAX_FILTER_TEXT);

    m_hmiZoneSelChangeListener = new HmiSelectionListener(m_hWnd);

    // HMI listener will be up to date as soon as constructed
    setHMIZones(m_hmiZoneSelChangeListener->getZoneList());

    // Send initial start / stop time state to model
    processStartStopTimeChange();

    // TD 15349
    // check whether the parameters contain pre-select message
    std::string selectedStationMessage = m_paManagerDlg->getSelectedStationMessage();

    if (selectedStationMessage != "" && m_dvaBtn.GetCheck() == BST_CHECKED)
    {
        selectMessageNameInLists(selectedStationMessage);
    }

    // TD 15349
    FUNCTION_EXIT;
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void StationBroadcastPage::setupDateTimeEntryControls()
{
    FUNCTION_ENTRY("setupDateTimeEntryControls");

    /////////////////////////
    // Setup Cyclic Broadcast
    /////////////////////////

    PaLayoutHelper::setUpDateTimeControls(&m_cyclicalStartDate,
                                          &m_cyclicalStartTime);
    PaLayoutHelper::setUpDateTimeControls(&m_cyclicalStopDate,
                                          &m_cyclicalStopTime);

    // This time is restricted to 60 minutes, which occurs by default
    m_cyclicalInterval.SetFormat("mm:ss");

    // Apply default starting time
    m_cyclicalInterval.SetTime(DEFAULT_INTERVAL_TIME);

    /////////////////////////
    // Setup Multi-Broadcast
    /////////////////////////

    m_multiBroadcastDwellTimeSeconds.SetFormat("ss");

    m_multiBroadcastDwellTimeSeconds.SetTime(DEFAULT_DWELL_TIME);
    m_multiBroadcastDwellTimeSeconds.SetRange(&NULL_TIME, &MAX_DWELL_TIME);

    FUNCTION_EXIT;
}

void StationBroadcastPage::postDialogCreate()
{
    LOG_SCOPE("StationBroadcastPage::postDialogCreate");
    FUNCTION_ENTRY("postDialogCreate");

    // Fetching size of buttons doesn't work in OnInitDialog
    setupPaModeButtons();
    setupAdhocBitmaps();

    // Setup Message Lists
    /////////////////////////
    // jeffrey++ TD8850
    PaLayoutHelper::formatDvaMessageList(m_normalLst, "Normal Messages", false);

    PaLayoutHelper::formatDvaMessageList(m_emergencyLst, "Emergency Messages", false);
    // ++jeffrey TD8850

    PaLayoutHelper::formatDvaMessageList(m_adhocLst, ADHOCLIST_FULL_TITLE, false, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
    refreshAdhocListStyle();

    PaLayoutHelper::formatDvaMessageList(m_selectedLst, "Selected Messages", false);

    /// Fill lists with all unfiltered data

    // Important to perform ad hoc update last - to ensure it keeps
    // up to date by beginning listening for ad hoc message changes before getting
    // most up to date set of data
    fillOutAdhocList();
    fillOutEmergencyList();
    fillOutNormalList();

    CFont* pFont = GetDlgItem(IDC_STIS_TEXT)->GetFont();
    LOGFONT lf;

    if (0 == pFont->GetLogFont(&lf))
    {
        TA_ASSERT(FALSE, "Failed to retrieve dialog font");
    }

    lf.lfUnderline = TRUE;

    if (0 == m_underlineFont.CreateFontIndirect(&lf))
    {
        TA_ASSERT(FALSE, "Failed to create underline font");
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnCyclicCb()
{
    FUNCTION_ENTRY("OnCyclicCb");

    enableCyclicArea(m_cyclicCb.GetCheck() != 0 ? true : false);

    updateStisCheckboxEnabled();

    FUNCTION_EXIT;
}

void StationBroadcastPage::updateStisCheckboxEnabled()
{
    FUNCTION_ENTRY("updateStisCheckboxEnabled");

    bool enableStis = false;

    if (m_cyclicCb.GetCheck() == 0)
    {
        if (m_multiCb.GetCheck() == 0)
        {
            // Non cyclic, single broadcast, may enable STIS

            // But only enable stis if there is a STIS message
            // selected in the selected messages list
            if (1 == m_selectedLst.GetItemCount())
            {
                TA_IRS_Bus::StationDvaMessageRecord* message =
                    reinterpret_cast<TA_IRS_Bus::StationDvaMessageRecord*>(m_selectedLst.GetItemData(0));

                if (DvaListCtrlModel::isTisEnabled(*message))
                {
                    // The single message in the selected list has associated TIS functionality
                    enableStis = true;
                }
            }
        }
    }

    if (!enableStis)
    {
        // Uncheck if ever disabling window
        m_stisCb.SetCheck(0);

        // Must update model (via OnStisCb)
        OnStisCb();
    }

    m_stisCb.EnableWindow(enableStis);

    // Whenever changing enabled state of the STIS checkbox, must also
    // update the associated text fields (which had to be treated separately
    // in order to underline the word STIS)
    if (0 != GetDlgItem(IDC_STN_STIS_CB_TEXT1))
    {
        GetDlgItem(IDC_STN_STIS_CB_TEXT1)->EnableWindow(enableStis);
    }

    if (0 != GetDlgItem(IDC_STIS_TEXT))
    {
        GetDlgItem(IDC_STIS_TEXT)->EnableWindow(enableStis);
    }

    if (0 != GetDlgItem(IDC_STN_STIS_CB_TEXT2))
    {
        GetDlgItem(IDC_STN_STIS_CB_TEXT2)->EnableWindow(enableStis);
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnMultiCb()
{
    FUNCTION_ENTRY("OnMultiCb");

    bool mbEnabled = m_multiCb.GetCheck() != 0;

    if (mbEnabled)
    {
        // TES642: Set to default dwell time each time multi-broadcast is selected
        m_multiBroadcastDwellTimeSeconds.SetTime(DEFAULT_DWELL_TIME);
    }

    enableMultiArea(mbEnabled);

    updateStisCheckboxEnabled();

    FUNCTION_EXIT;
}

void StationBroadcastPage::refreshZoneList()
{
    LOG_SCOPE("StationBroadcastPage::refreshZoneList");
    FUNCTION_ENTRY("refreshZoneList");

    int curSel = m_zoneCombo.GetCurSel();
    Keys zoneGroupKeys;

    // Should always have at least one selection..
    if (curSel >= 0)
    {
        if (m_zoneCombo.GetItemData(curSel) == 0)
        {
            // This is the special HMI selection
            zoneGroupKeys = m_hmiZones;
        }
        else
        {
            CString zoneGroupLabel;
            m_zoneCombo.GetLBText(curSel, zoneGroupLabel);

            try
            {
                zoneGroupKeys = m_stationBroadcastModel->getZonesAssociatedWithGroup(zoneGroupLabel.GetBuffer(0));
            }
            catch (const TA_Base_Core::TransactiveException&)
            {
                // Error fetching data, the zone list will be emptied
                PaErrorHandler::displayError(PaErrorHandler::DATABASE_ERROR);
            }
        }
    }

    m_zoneLst.updateZoneList(zoneGroupKeys);

    updateExecuteButtonEnabledState();

    FUNCTION_EXIT;
}

void StationBroadcastPage::refreshZoneComboData()
{
    LOG_SCOPE("StationBroadcastPage::refreshZoneComboData");
    FUNCTION_ENTRY("refreshZoneComboData");

    m_zoneCombo.saveSelection();
    m_zoneCombo.SetCurSel(-1);
    int hmiIndex = -1;

    // Place all the existing zone strings into zone combo drop list
    try
    {
        // Clear everything in combo
        m_zoneCombo.ResetContent();
        // Label changed as per TES #644
        // TD #10389
        // Label changed from "Current Schematic Selected Zones" to "Active Schematic Selected Zones"
        hmiIndex = m_zoneCombo.AddString(_T("Active Schematic Selected Zones"));
        m_zoneCombo.SetItemData(hmiIndex, 0);

        // Only obtain the zone groups that are NOT event groups
        // (these are system groups that user should see or tamper with)
        TA_IRS_Bus::PaZoneGroupRecords zoneData = TA_IRS_Bus::CachedMaps::getInstance()->
            getPaZoneGroupCachedMap().getPaZoneGroupsByEventFlag(false);

        for (TA_IRS_Bus::PaZoneGroupRecordsIt it = zoneData.begin();
             it != zoneData.end();
             it++)
        {
            int nItem = m_zoneCombo.AddString(it->m_label.c_str());
            m_zoneCombo.SetItemData(nItem, it->m_pKey);
        }
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // Consequences of this: the broadcast selection zone list will be empty
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }
    catch (const CORBA::Exception&)
    {
        // Comms error??
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }

    if (!m_zoneCombo.restoreSelection(hmiIndex))
    {
        // If couldn't restore selection to currently selected
        // group, means that group was deleted, refresh
        // the zone list (since new selection present)
        OnSelchangeStnZoneGroupCombo();
    }

    // This doesn't get called automatically (updates buttons associated with selection)
    onZoneListOrZoneComboSelectionChange();

    FUNCTION_EXIT;
}

void StationBroadcastPage::setupAdhocBitmaps()
{
    FUNCTION_ENTRY("setupAdhocBitmaps");

    CRect tempRect;

    // Buttons are 64x30
    m_playBtn.GetWindowRect(tempRect);
    m_playBtn.SetIcon(loadIcon(IDI_PLAY_ICON, tempRect));

    // TD16864
    //    m_stopBtn.GetWindowRect(tempRect);
    //    m_stopBtn.SetIcon(loadIcon(IDI_STOP_ICON, tempRect));
    CRect rc(0, 0, 20, 20);
    m_stopBtn.SetIcon(loadIcon(IDI_STOP_ICON, rc));
    // TD16864

    FUNCTION_EXIT;
}

HICON StationBroadcastPage::loadIcon(DWORD resourceId, CRect rc)
{
    FUNCTION_ENTRY("loadIcon");

    std::map<DWORD, HANDLE>::iterator it = m_hIconBitmaps.find(resourceId);

    if (it == m_hIconBitmaps.end())
    {
        HANDLE icon = LoadImage(AfxGetInstanceHandle(),
                                MAKEINTRESOURCE(resourceId),
                                IMAGE_ICON,
                                rc.Width(),
                                rc.Height(),
                                LR_DEFAULTCOLOR);

        TA_ASSERT(icon, "Icon not found");

        m_hIconBitmaps[resourceId] = icon;

        FUNCTION_EXIT;
        return (HICON)icon;
    }
    else
    {
        FUNCTION_EXIT;
        return (HICON)it->second;
    }
}

void StationBroadcastPage::setupPaModeButtons()
{
    FUNCTION_ENTRY("setupPaModeButtons");

    CRect tempRect;

    /////////////////////////
    // Load PA Mode Bitmaps (63x68)
    /////////////////////////
    m_occMusicButton.GetWindowRect(tempRect);

    m_occMusicButton.SetIcon(loadIcon(IDI_OCCMUSIC_ICON, tempRect));

    // If located at depot, display depot specific icon for station music broadcast
    if (CachedConfig::getInstance()->getLocationType() ==
        CachedConfig::LOCATION_TYPE_DEPOT)
    {
        m_stnMusicButton.SetIcon(loadIcon(IDI_DEPOTMUSIC_ICON, tempRect));
    }
    else
    {
        m_stnMusicButton.SetIcon(loadIcon(IDI_STNMUSIC_ICON, tempRect));
    }

    m_liveBtn.GetWindowRect(tempRect);
    m_liveBtn.SetIcon(loadIcon(IDI_LIVE_ICON, tempRect));

    m_dvaBtn.GetWindowRect(tempRect);
    m_dvaBtn.SetIcon(loadIcon(IDI_DVA_ICON, tempRect));

    // Station music button not allowed at occ
    if (CachedConfig::LOCATION_TYPE_OCC == m_stationBroadcastModel->getLocationType())
    {
        m_stnMusicButton.EnableWindow(FALSE);
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::enableDvaArea(bool isEnabled)
{
    FUNCTION_ENTRY("enableDvaArea");

    EnableAdhocWindow(isEnabled, ADHOC_AREA_ENABLE);

    m_emergencyLst.EnableWindow(isEnabled);
    m_normalLst.EnableWindow(isEnabled);

    // TD17286
    if (!isEnabled)
    {
        m_emergencyLst.SetTextColor(RGB(128, 128, 128));
        m_normalLst.SetTextColor(RGB(128, 128, 128));
    }

    // TD17286
    m_emergencyFilterTb.EnableWindow(isEnabled);
    m_normalFilterTb.EnableWindow(isEnabled);
    m_selectedLst.EnableWindow(isEnabled);

    // TD17286
    if (!isEnabled)
    {
        m_selectedLst.SetTextColor(RGB(128, 128, 128));
    }

    // TD17286

    updatePlayStopRecordButtons(isEnabled);

    updateSelectedListButtons(isEnabled);

    m_cyclicCb.EnableWindow(isEnabled);
    m_multiCb.EnableWindow(isEnabled);
    m_chimeCb.EnableWindow(isEnabled);

    if (isEnabled == false)
    {
        enableCyclicArea(false);
        enableMultiArea(false);
    }
    else
    {
        enableCyclicArea(m_cyclicCb.GetCheck() != 0 ? true : false);
        enableMultiArea(m_multiCb.GetCheck() != 0 ? true : false);
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::updatePlayStopRecordButtons(bool isDvaAreaEnabled)
{
    LOG_SCOPE("StationBroadcastPage::updatePlayStopRecordButtons");
    FUNCTION_ENTRY("updatePlayStopRecordButtons");

    if (m_freezePlayStopRecordButtonUpdate)
    {
        // Return early if class has frozen updating of buttons
        FUNCTION_EXIT;
        return;
    }

    bool recordEnabled = false;
    bool playEnabled = false;

    if (isDvaAreaEnabled)
    {
        unsigned long key = getSelectedDvaMessageKey();

        // Check which buttons should be enabled when DVA area active
        if (TA_Base_Core::DatabaseKey::isValidKey(key))
        {
            // There is a message selected, play/stop should be enabled
            // (unless the message type is 'Deleted')
            try
            {
                char messageType = TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(key).m_type;

                // Could be a deleted message type, in which case we can't play it back
                if (!DvaListCtrlModel::isDeletedMessage(messageType))
                {
                    playEnabled = true;
                }

                // Additionally, if we've selected an ad-hoc message, enable record
                if (m_adhocLst.GetSelectedCount() > 0)
                {
                    recordEnabled = true;
                }
            }
            catch (const TA_Base_Core::CachedMappingNotFoundException&)
            {
                // Error finding selected message, button states will be disabled
                // When in this undefined state - this is a valid control flow path
                // - when a record is deleted (and currently selected), we may
                // get into this function before it's deselected, and it won't exist in
                // the cached maps...
                //PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
            }
        }
    }

    if (playEnabled &&
        !PaRightsManager::getInstance().hasRightToPlaybackDvaMessage())
    {
        // If no right to play back dva messages, disable the button
        playEnabled = false;
    }

    m_playBtn.EnableWindow(playEnabled);

    // If the play button is checked, record button will be forcibly disabled
    // (cannot begin recording session while any DVA message is playing back)
    if (m_playBtn.GetCheck() != 0)
    {
        recordEnabled = false;
    }

    if (!PaRightsManager::getInstance().hasRightToRecordDvaMessage())
    {
        recordEnabled = false;
    }

    m_recordBtn.EnableWindow(recordEnabled);

    // The stop button will be enabled if there is a play in progress
    // - if not a play in progress, never enabled (won't do anything)
    m_stopBtn.EnableWindow(m_playBtn.GetCheck() != 0);

    FUNCTION_EXIT;
}

void StationBroadcastPage::enableCyclicArea(bool isEnabled)
{
    FUNCTION_ENTRY("enableCyclicArea");

    if (isEnabled && !m_stationBroadcastModel->getCyclicalEnabled())
    {
        // TES #641: If re-enabling the cyclical area set the start/stop
        // time to the current system time
        time_t now = time(NULL);
        CTime nowInCTime(now);

        setTimeStartDtp(nowInCTime);
        setTimeStopDtp(nowInCTime);
    }

    m_cyclicalStartDate.EnableWindow(isEnabled);
    m_cyclicalStartTime.EnableWindow(isEnabled);

    m_cyclicalStopDate.EnableWindow(isEnabled);
    m_cyclicalStopTime.EnableWindow(isEnabled);

    m_cyclicalInterval.EnableWindow(isEnabled);

    m_stationBroadcastModel->setCyclicalEnabled(isEnabled);

    if (isEnabled)
    {
        // update the model with whatever is in the control
        processPeriod();
    }
    else
    {
        m_stationBroadcastModel->setPeriod(0);
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::enableMultiArea(bool isEnabled)
{
    FUNCTION_ENTRY("enableMultiArea");

    m_multiBroadcastDwellTimeSeconds.EnableWindow(isEnabled);

    m_stationBroadcastModel->enableMultiArea(isEnabled);

    if (isEnabled)
    {
        // update the model with whatever is in the control
        processDwell();
    }
    else
    {
        m_stationBroadcastModel->setDwell(0);
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnDvaBtn()
{
    LOG_SCOPE("StationBroadcastPage::OnDvaBtn");
    FUNCTION_ENTRY("OnDvaBtn");

    processPAModeChange(&m_dvaBtn);

    if (m_dvaBtn.GetCheck() != 0)
    {
        m_stationBroadcastModel->setBroadcastType(TA_Base_Bus::IPAAgentCorbaDef::StationDva);
    }
    else
    {
        m_stationBroadcastModel->unsetBroadcastType();
    }

    updateExecuteButtonEnabledState();

    onBroadcastModeChange();

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnLiveBtn()
{
    LOG_SCOPE("StationBroadcastPage::OnLiveBtn");
    FUNCTION_ENTRY("OnLiveBtn");

    processPAModeChange(&m_liveBtn);

    if (m_liveBtn.GetCheck() != 0)
    {
        m_stationBroadcastModel->setBroadcastType(TA_Base_Bus::IPAAgentCorbaDef::StationLive);
    }
    else
    {
        m_stationBroadcastModel->unsetBroadcastType();
    }

    updateExecuteButtonEnabledState();

    onBroadcastModeChange();

    FUNCTION_EXIT;
}

void StationBroadcastPage::processPAModeChange(CButton* modifiedButton)
{
    FUNCTION_ENTRY("processPAModeChange");

    // FIRST: update state of other checkboxes (BEFORE processing later in function
    // responding to updated state)
    // The button behaviour is as follows:
    // DVA / Live buttons are grouped
    // Station / OCC buttons are grouped
    // Executing a DVA / Live broadcast will automatically uncheck any checked music buttons
    bool pressingButtonIn = false;

    if (0 != modifiedButton)
    {
        pressingButtonIn = (modifiedButton->GetCheck() != 0);
    }

    if (pressingButtonIn)
    {
        // Check if any buttons need to pop out as a result of pressing button in
        if ((&m_stnMusicButton) == modifiedButton)
        {
            m_occMusicButton.SetCheck(BST_UNCHECKED);
        }

        if ((&m_occMusicButton) == modifiedButton)
        {
            m_stnMusicButton.SetCheck(BST_UNCHECKED);
        }

        if ((&m_liveBtn) == modifiedButton)
        {
            m_dvaBtn.SetCheck(BST_UNCHECKED);
        }

        if ((&m_dvaBtn) == modifiedButton)
        {
            m_liveBtn.SetCheck(BST_UNCHECKED);
        }
    }

    ///////
    // Now check updated button states and respond accordingly
    //////

    // Enable DVA area only if DVA button pressed
    if (m_dvaBtn.GetCheck())
    {
        enableDvaArea(true);
    }
    else
    {
        enableDvaArea(false);
    }

    // Only enable the zone controls if the user is setting live or dva stuff
    bool bEnableZoneArea = m_dvaBtn.GetCheck() || m_liveBtn.GetCheck();

    m_zoneLst.EnableWindow(bEnableZoneArea);

    if (m_zoneCombo.GetCount() == 0)
    {
        // No items in combo, can't be enabled (this shouldn't happen
        // unless something has gone wrong internally, HMI selection
        // should always be present)
        m_zoneCombo.EnableWindow(false);
    }
    else
    {
        m_zoneCombo.EnableWindow(bEnableZoneArea);
    }

    onZoneListOrZoneComboSelectionChange();

    FUNCTION_EXIT;
}

void StationBroadcastPage::executeBroadcast()
{
    LOG_SCOPE("StationBroadcastPage::executeBroadcast");
    FUNCTION_ENTRY("executeBroadcast");

    // Whenever executing a broadcast, must cancel any station music
    // that is currently playing (if any)
    if (BST_CHECKED == m_stnMusicButton.GetCheck())
    {
        m_stnMusicButton.SetCheck(BST_UNCHECKED);
        OnStnMusicBtn();
    }

    if (BST_CHECKED == m_occMusicButton.GetCheck())
    {
        m_occMusicButton.SetCheck(BST_UNCHECKED);
        OnOccMusicBtn();
    }

    Keys selectedZones = getSelectedZones();
    m_stationBroadcastModel->executeBroadcast(selectedZones, getSelectedZoneGroupName());

    FUNCTION_EXIT;
}

Keys StationBroadcastPage::getSelectedZones()
{
    FUNCTION_ENTRY("getSelectedZones");
    FUNCTION_EXIT;
    return m_zoneLst.getAllKeys();
}

std::string StationBroadcastPage::getSelectedZoneGroupName()
{
    FUNCTION_ENTRY("getSelectedZoneGroupName");

    int curSel = m_zoneCombo.GetCurSel();

    // Should always have at least one selection..
    if (curSel >= 0)
    {
        CString zoneGroupLabel;
        m_zoneCombo.GetLBText(curSel, zoneGroupLabel);

        FUNCTION_EXIT;
        return std::string(zoneGroupLabel.GetBuffer(0));
    }

    // return an empty string
    FUNCTION_EXIT;
    return std::string("");
}

void StationBroadcastPage::setHMIZones(const Keys& keys)
{
    LOG_SCOPE("StationBroadcastPage::setHMIZones");
    FUNCTION_ENTRY("setHMIZones");
    LOG_DEBUG("setHMIZones(): %s", nvps(keys));

    // song toan++
    // TD4080
    if (m_LocationType == CachedConfig::LOCATION_TYPE_OCC)
    {
        // this is OCC, simply take all changed zones/groups
        m_hmiZones = keys;
    }
    else
    {
        std::vector<int> zoneIds;

        // We have to filter out the zones not belong to our location if this
        // is non-OCC station. It is because after the fix PA Manager will
        // listen to changes of all zones/groups.
        m_hmiZones.clear();
        unsigned long thislocationkey = CachedConfig::getInstance()->getEntityLocationKey();

        for (Keys::const_iterator it = keys.begin();
             it != keys.end(); ++it)
        {
            unsigned long key = *it;
            unsigned long locationkey = CachedMaps::getInstance()->getPaZoneLocationKeyFromKey(key);

            if (locationkey == thislocationkey)
            {
                m_hmiZones.push_back(key);
                zoneIds.emplace_back(CachedMaps::getInstance()->getPaZoneIdFromKey(key));
            }
        }

        boost::sort(zoneIds);
        LOG_DEBUG("setHMIZones(): %s", nvps(zoneIds));
    }

    // ++song toan
    // TD4080

    // Must refresh zone list whenever the HMI set changes
    // (only if HMI selected)
    if (m_zoneCombo.GetCurSel() >= 0 &&
        0 == m_zoneCombo.GetItemData(m_zoneCombo.GetCurSel()))
    {
        refreshZoneList();
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnChangeNormalFilterTb()
{
    FUNCTION_ENTRY("OnChangeNormalFilterTb");

    m_stationBroadcastModel->onChangeNormalFilterTb(
        PaLayoutHelper::fetchStringFromControl(m_normalFilterTb));

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnChangeEmergencyFilterTb()
{
    FUNCTION_ENTRY("OnChangeEmergencyFilterTb");

    m_stationBroadcastModel->onChangeEmergencyFilterTb(
        PaLayoutHelper::fetchStringFromControl(m_emergencyFilterTb));

    FUNCTION_EXIT;
}

void StationBroadcastPage::fillOutNormalList()
{
    FUNCTION_ENTRY("fillOutNormalList");

    OnChangeNormalFilterTb();

    FUNCTION_EXIT;
}

void StationBroadcastPage::fillOutEmergencyList()
{
    FUNCTION_ENTRY("fillOutEmergencyList");

    OnChangeEmergencyFilterTb();

    FUNCTION_EXIT;
}

void StationBroadcastPage::fillOutAdhocList()
{
    FUNCTION_ENTRY("fillOutAdhocList");

    m_stationBroadcastModel->onAdhocListUpdate();

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnChimeCb()
{
    FUNCTION_ENTRY("OnChimeCb");

    m_stationBroadcastModel->setChime(m_chimeCb.GetCheck() != 0 ? true : false);

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnStisCb()
{
    FUNCTION_ENTRY("OnStisCb");

    m_stationBroadcastModel->setStisBroadcast(m_stisCb.GetCheck() != 0 ? true : false);

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnItemchangedSelectedMessagesLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnItemchangedSelectedMessagesLst");

    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    // Each time selected messages selection changes, must update associated buttons
    updateSelectedListButtons(m_dvaBtn.GetCheck() == BST_CHECKED);

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnEndlabeleditAdhocMessagesLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnEndlabeleditAdhocMessagesLst");

    // We allow the users to change labels to whatever they like
    // (Because we can always uniquely identify them by their record name)
    // Only thing we're responsible for is dispatching the rename
    // notification to the PA Agent - however we can set *pResult = FALSE
    // to reject the text change
    LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
    LVITEM& item = pDispInfo->item;

    if (!(item.mask & LVIF_TEXT))
    {
        // We don't care about any updates that don't affect the text - ie cancelled edits
        FUNCTION_EXIT;
        return;
    }

    LPSTR newLabel = item.pszText;

    // TES741: Reject any updates causing empty string in list control
    if (0 == strlen(newLabel))
    {
        *pResult = false;
        PaErrorHandler::displayError(PaErrorHandler::EMPTY_DESCRIPTION_NOT_ALLOWED);

        FUNCTION_EXIT;
        return;
    }

    // Reject any updates causing a null string in listbox (not sure if possible event anyway)
    if (NULL == newLabel)
    {
        *pResult = false;

        FUNCTION_EXIT;
        return;
    }

    // Validate the new label string (may be truncated)
    m_stationBroadcastModel->validateNewAdhocLabel(newLabel);

    TA_IRS_Bus::StationDvaMessageRecord* message = reinterpret_cast<TA_IRS_Bus::StationDvaMessageRecord*>(pDispInfo->item.lParam);
    TA_ASSERT(message, "NULL StationDvaMessageRecord pointer in Adhoc Messages List");

    // - DO NOT update the local record struct -
    // instead wait for update to be received back from ad hoc message change subscriber
    // (and in the meantime, disable the window)
    //message->m_label = pDispInfo->item.pszText;
    userInitiatedAdhocLabelChange(message->m_pKey, newLabel);

    // Reject the change - label must be updated over network
    *pResult = FALSE;

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnBeginlabeleditStnAdhocMessagesLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnBeginlabeleditStnAdhocMessagesLst");

    LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
    TA_IRS_Bus::StationDvaMessageRecord* message =
        reinterpret_cast<TA_IRS_Bus::StationDvaMessageRecord*>(pDispInfo->item.lParam);

    if (!PaRightsManager::getInstance().hasRight(PaRightsManager::PA_MODIFY_ADHOC_MESSAGE))
    {
        *pResult = TRUE;        // Don't allow edit
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);

        FUNCTION_EXIT;
        return;
    }

    if (DvaListCtrlModel::isDeletedMessage(message->m_type))
    {
        // Don't allow editing of deleted adhoc messages (however, user can clear
        //  label from a non deleted adhoc message, without actually deleting it)
        *pResult = TRUE;
        PaErrorHandler::displayError(PaErrorHandler::ADHOC_IS_DELETED);

        FUNCTION_EXIT;
        return;
    }

    // Modify editable length of (edit) control to restrict user editing labels
    // GetEditControl does not work if called somewhere in OnInitDialog - so must be done here
    CEdit* pEdit = m_adhocLst.GetEditControl();
    TA_ASSERT(pEdit != NULL, "No edit control attached to ad hoc list");

    pEdit->SetLimitText(StationBroadcastModel::MAX_ADHOC_LABEL_LEN);

    *pResult = 0;

    FUNCTION_EXIT;
}

void StationBroadcastPage::EnableAdhocWindow(bool bEnable, eAdhocWindowFlags bitFlags)
{
    FUNCTION_ENTRY("EnableAdhocWindow");

    if (bEnable)
    {
        // Otherwise zero out flagged bits
        m_adhocWindowDisabled &= ~bitFlags;
    }
    else
    {
        // If disabling, we set the relevant bits
        m_adhocWindowDisabled |= bitFlags;
    }

    // If any of the disabled bits are present - must disable the window
    if (m_adhocWindowDisabled != 0)
    {
        LOG_DEBUG("Disabling window");

        m_adhocLst.EnableWindow(FALSE);
        // TD17286
        m_adhocLst.SetTextColor(RGB(128, 128, 128));
        // TD17286
    }
    else
    {
        LOG_DEBUG("Enabling window");

        // All bits set to enabled
        m_adhocLst.EnableWindow(TRUE);
    }

    FUNCTION_EXIT;
}

CTime StationBroadcastPage::getTimeStartDtp()
{
    FUNCTION_ENTRY("getTimeStartDtp");

    time_t seconds = PaLayoutHelper::getSecondsFromControls(&m_cyclicalStartDate,
                                                            &m_cyclicalStartTime,
                                                            &m_cyclicalStartTime,
                                                            &m_cyclicalStartTime);

    FUNCTION_EXIT;
    return CTime(seconds);
}

CTime StationBroadcastPage::getTimeStopDtp()
{
    FUNCTION_ENTRY("getTimeStopDtp");

    time_t seconds = PaLayoutHelper::getSecondsFromControls(&m_cyclicalStopDate,
                                                            &m_cyclicalStopTime,
                                                            &m_cyclicalStopTime,
                                                            &m_cyclicalStopTime);

    FUNCTION_EXIT;
    return CTime(seconds);
}

void StationBroadcastPage::OnDatetimechangeStopDtp(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnDatetimechangeStopDtp");

    processStartStopTimeChange();

    *pResult = 0;

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnDatetimechangeStartDtp(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnDatetimechangeStartDtp");

    processStartStopTimeChange();

    *pResult = 0;

    FUNCTION_EXIT;
}

void StationBroadcastPage::processStartStopTimeChange()
{
    FUNCTION_ENTRY("processStartStopTimeChange");

    CTime startTime = getTimeStartDtp();
    CTime stopTime = getTimeStopDtp();

    m_stationBroadcastModel->validateAndUpdateStartAndStop(startTime, stopTime);

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnDatetimechangeStnDwellDtp(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnDatetimechangeStnDwellDtp");

    processDwell();

    *pResult = 0;

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnDatetimechangeStnIntervalDtp(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnDatetimechangeStnIntervalDtp");

    processPeriod();

    *pResult = 0;

    FUNCTION_EXIT;
}

void StationBroadcastPage::processDwell()
{
    FUNCTION_ENTRY("processDwell");

    CString rString;
    m_multiBroadcastDwellTimeSeconds.GetWindowText(rString);

    long dwellInSecs = atol(rString.GetBuffer(0));
    m_stationBroadcastModel->setDwell((time_t)dwellInSecs);

    FUNCTION_EXIT;
}

void StationBroadcastPage::processPeriod()
{
    FUNCTION_ENTRY("processPeriod");

    // get the time from the control
    // this is the current date
    // and the MM:SS from the control
    time_t timeFromControl = PaLayoutHelper::getSecondsFromControls(0,
                                                                    0,
                                                                    &m_cyclicalInterval,
                                                                    &m_cyclicalInterval);

    // Need to subtract the date from the time that is returned,
    // so that it is just the minutes and seconds.
    long periodInSecs = timeFromControl - PaLayoutHelper::getSecondsFromControls(0, 0, 0, 0);

    m_stationBroadcastModel->setPeriod((time_t)periodInSecs);

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnClearOneBtn()
{
    FUNCTION_ENTRY("OnClearOneBtn");

    POSITION pos = m_selectedLst.GetFirstSelectedItemPosition();

    if (pos == NULL)
    {
        FUNCTION_EXIT;
        return;
    }

    int nItem = m_selectedLst.GetNextSelectedItem(pos);

    // Don't simply remove item with the specified key - because may duplicate
    // items, and need to only remove the selected one
    m_stationBroadcastModel->onClearOneBtn(nItem);

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnClearAllBtn()
{
    FUNCTION_ENTRY("OnClearAllBtn");

    m_stationBroadcastModel->onClearAllBtn();

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnDblclkNormalMessagesLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnDblclkNormalMessagesLst");

    addToSelectedMessagesList(m_normalLst);
    *pResult = 0;

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnDblclkEmergencyMessagesLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnDblclkEmergencyMessagesLst");

    addToSelectedMessagesList(m_emergencyLst);
    *pResult = 0;

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnDblclkAdhocMessagesLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnDblclkAdhocMessagesLst");

    addToSelectedMessagesList(m_adhocLst);
    *pResult = 0;

    FUNCTION_EXIT;
}

void StationBroadcastPage::addToSelectedMessagesList(const DvaListCtrl& theControl)
{
    FUNCTION_ENTRY("addToSelectedMessagesList");

    POSITION pos = theControl.GetFirstSelectedItemPosition();

    if (pos == NULL)
    {
        FUNCTION_EXIT;
        return;
    }

    int nItem = theControl.GetNextSelectedItem(pos);
    TA_IRS_Bus::StationDvaMessageRecord* record = reinterpret_cast<TA_IRS_Bus::StationDvaMessageRecord*>(theControl.GetItemData(nItem));

    if (record == NULL ||
        DvaListCtrlModel::isDeletedMessage(record->m_type))
    {
        // User not allowed to add deleted messages to the selected messages list
        // (under normal circumstances, this can only be attempted from adhoc list)
        FUNCTION_EXIT;
        return;
    }

    m_stationBroadcastModel->addToSelectedMessagesList(record->m_pKey, m_multiCb.GetCheck() == 0 ? false : true);

    FUNCTION_EXIT;
}

unsigned long StationBroadcastPage::getSelectedDvaMessageKey()
{
    FUNCTION_ENTRY("getSelectedDvaMessageKey");

    if (m_adhocLst.GetSelectedCount() > 0)
    {
        FUNCTION_EXIT;
        return m_adhocLst.getSelectedMessageKey();
    }
    else if (m_emergencyLst.GetSelectedCount() > 0)
    {
        FUNCTION_EXIT;
        return m_emergencyLst.getSelectedMessageKey();
    }
    else if (m_normalLst.GetSelectedCount() > 0)
    {
        FUNCTION_EXIT;
        return m_normalLst.getSelectedMessageKey();
    }
    else
    {
        // No selection made
        FUNCTION_EXIT;
        return TA_Base_Core::DatabaseKey::getInvalidKey();
    }
}

void StationBroadcastPage::updateExecuteButtonEnabledState()
{
    FUNCTION_ENTRY("updateExecuteButtonEnabledState");

    // Default behaviour is off
    bool enable = false;

    if (BST_CHECKED == m_liveBtn.GetCheck())
    {
        // Live broadcast only available if there are some zones selected
        enable = (m_zoneLst.GetItemCount() > 0);
    }
    else if (BST_CHECKED == m_dvaBtn.GetCheck())
    {
        // DVA Broadcast only available if there are some zones selected
        // and there are some DVA messages selected for the broadcast
        enable = m_zoneLst.GetItemCount() > 0 &&
            m_selectedLst.GetItemCount() > 0;
    }

    if (m_pageActive)
    {
        m_paManagerDlg->enableExecuteButton(enable);
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::setTimeStartDtp(const CTime& time)
{
    FUNCTION_ENTRY("setTimeStartDtp");

    m_cyclicalStartDate.SetTime(&time);
    m_cyclicalStartTime.SetTime(&time);

    // Ensure the change went through correctly
    TA_ASSERT(getTimeStartDtp() == time, "Internal error");

    FUNCTION_EXIT;
}

void StationBroadcastPage::setTimeStopDtp(const CTime& time)
{
    FUNCTION_ENTRY("setTimeStopDtp");

    m_cyclicalStopDate.SetTime(&time);
    m_cyclicalStopTime.SetTime(&time);

    // Ensure the change went through correctly
    TA_ASSERT(getTimeStopDtp() == time, "Internal error");

    FUNCTION_EXIT;
}

void StationBroadcastPage::updateNormalMessageList(TA_IRS_Bus::StationDvaMessageRecordPointers& records)
{
    LOG_SCOPE("StationBroadcastPage::updateNormalMessageList");
    FUNCTION_ENTRY("updateNormalMessageList");

    LOG_DEBUG("updateNormalMessageList(): %s", nvps(records));

    LockControlRedraw redrawLock(m_normalLst);

    // When performing batch updates, must inhibit updatePlayStopRecordButtons button
    // to avoid system slowdown (then call once off after update)
    freezePlayStopRecordUpdate(true);

    {
        m_normalLst.DeleteAllItems();
        TA_IRS_Bus::StationDvaMessageRecordPointersIt it = records.begin();

        for (; it != records.end(); ++it)
        {
            m_normalLst.SetItemData(*it);
        }
    }

    // This is a bit of a hack to force the list to re-sort itself *before*
    // the PlayStopRecord lock is released (because it automatically resorts itself -
    // in turn causing enough calls to refresh Play/Stop/Record buttons to seriously
    // slow down the system)
    MSG msg;

    // Purge all waiting messages in the queue
    while (0 != ::PeekMessage(&msg, m_normalLst.m_hWnd, 0, 0, PM_REMOVE))
    {
        // Process the message here
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    freezePlayStopRecordUpdate(false);

    FUNCTION_EXIT;
}

void StationBroadcastPage::updateEmergencyMessageList(TA_IRS_Bus::StationDvaMessageRecordPointers& records)
{
    LOG_SCOPE("StationBroadcastPage::updateEmergencyMessageList");
    FUNCTION_ENTRY("updateEmergencyMessageList");

    LOG_DEBUG("updateEmergencyMessageList(): %s", nvps(records));

    LockControlRedraw redrawLock(m_emergencyLst);

    // When performing batch updates, must inhibit updatePlayStopRecordButtons button
    // to avoid system slowdown (then call once off after update)
    freezePlayStopRecordUpdate(true);

    {
        m_emergencyLst.DeleteAllItems();
        TA_IRS_Bus::StationDvaMessageRecordPointersIt it = records.begin();

        for (; it != records.end(); ++it)
        {
            m_emergencyLst.SetItemData(*it);
        }
    }

    // This is a bit of a hack to force the list to re-sort itself *before*
    // the PlayStopRecord lock is released (because it automatically resorts itself -
    // in turn causing enough calls to refresh Play/Stop/Record buttons to seriously
    // slow down the system)
    MSG msg;

    // Purge all waiting messages in the queue
    while (0 != ::PeekMessage(&msg, m_emergencyLst.m_hWnd, 0, 0, PM_REMOVE))
    {
        // Process the message here
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    freezePlayStopRecordUpdate(false);

    FUNCTION_EXIT;
}

void StationBroadcastPage::freezePlayStopRecordUpdate(bool freeze)
{
    FUNCTION_ENTRY("freezePlayStopRecordUpdate");

    TA_ASSERT(freeze != m_freezePlayStopRecordButtonUpdate, "Mismatched request");

    m_freezePlayStopRecordButtonUpdate = freeze;

    if (!freeze)
    {
        // Restore state of buttons upon un-freezing
        updatePlayStopRecordButtons(m_dvaBtn.GetCheck() != 0);
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::updateAdhocMessageList(TA_IRS_Bus::StationDvaMessageRecordPointers& records)
{
    LOG_SCOPE("StationBroadcastPage::updateAdhocMessageList");
    FUNCTION_ENTRY("updateAdhocMessageList");

    LOG_DEBUG("updateAdhocMessageList(): %s", nvps(records));

    LockControlRedraw redrawLock(m_adhocLst);

    m_adhocLst.DeleteAllItems();

    // TD #10335
    // We need to iterate through the list backwards since each item gets inserted
    // at index 0. If we don't do this, the Ad Hoc messages will be displayed in reverse order.
    TA_IRS_Bus::StationDvaMessageRecordPointers::reverse_iterator rIt = records.rbegin();

    //TA_IRS_Bus::StationDvaMessageRecordPointersIt it = records.begin();
    for (; rIt != records.rend(); ++rIt)
    {
        /// TD Defect #2648: Sort ad-hoc list alphabetically, not ID
        // TD #10335
        // Sort ad-hoc list by ID, not alphabetically!
        m_adhocLst.SetItemData(*rIt, 0, false);
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::updateSelectedMessagesList(TA_IRS_Bus::StationDvaMessageRecordPointers& records)
{
    FUNCTION_ENTRY("updateSelectedMessagesList");

    m_selectedLst.DeleteAllItems();
    TA_IRS_Bus::StationDvaMessageRecordPointersIt it = records.begin();

    for (; it != records.end(); ++it)
    {
        m_selectedLst.SetItemData(*it);
    }

    // TES #706
    // every time a new message is selected re-check the chime check box
    if (m_chimeCb.GetCheck() == FALSE)
    {
        m_chimeCb.SetCheck(TRUE);
        OnChimeCb();
    }

    // STIS state is dependent on items in selection list, update its state
    updateStisCheckboxEnabled();

    // Whenever items added / removed from selected list, update associated buttons
    updateSelectedListButtons(m_dvaBtn.GetCheck() == BST_CHECKED);

    // The type of currently selected messages affect the current broadcast mode
    onBroadcastModeChange();

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnMouseMove(UINT nFlags, CPoint point)
{
    FUNCTION_ENTRY("OnMouseMove");

    CPropertyPage::OnMouseMove(nFlags, point);

    FUNCTION_EXIT;
}

afx_msg LRESULT StationBroadcastPage::OnDvaMessageUpdate(UINT wParam, LONG lParam)
{
    LOG_SCOPE("WM_PA_DVA_MESSAGE_UPDATE", "StationBroadcastPage::OnDvaMessageUpdate");
    FUNCTION_ENTRY("OnDvaMessageUpdate");

    if (!m_dvaMessageChangeSubscriber.isConnectedListenerWindow(m_hWnd))
    {
        // Probably in the closing down stage, receiving last few messages
        // while unsubscribed - do NOT respond to this notification in this case
        FUNCTION_EXIT;
        return 0;
    }

    try
    {
        DvaMessageUpdateData updatedKeys = m_dvaMessageChangeSubscriber.getMessageData(wParam, m_hWnd);

        processDvaMessageUpdates(updatedKeys);
    }

    STANDARD_PA_CATCH("StationBroadcastPage::OnDvaMessageUpdate", PaErrorHandler::DVA_MESSAGE_UPDATE_FAILURE);

    FUNCTION_EXIT;
    return 0;
}

void StationBroadcastPage::processDvaMessageUpdates(const DvaMessageUpdateData& updatedKeys)
{
    LOG_SCOPE("StationBroadcastPage::processDvaMessageUpdates");
    FUNCTION_ENTRY("processDvaMessageUpdates");

    DvaMessageUpdateData modifiedKeys;
    DvaMessageUpdateData deletedKeys;

    if (m_selectedLst.collectMatchingKeys(updatedKeys,
                                          modifiedKeys,
                                          deletedKeys))
    {
        LOG_DEBUG("processDvaMessageUpdates(): %s", nvps(modifiedKeys, deletedKeys));

        // Clear all deleted keys from selection
        for (DvaMessageUpdateDataIt itr = deletedKeys.begin();
             itr != deletedKeys.end();
             itr++)
        {
            m_stationBroadcastModel->removeSelectedMessagesWithKey(*itr);
        }

        if (modifiedKeys.size() > 0)
        {
            m_selectedLst.processDvaMessageUpdates(updatedKeys);
        }

        // Respond to a possible change in STIS type (when selected list changes)
        updateStisCheckboxEnabled();

        // Items in selected list have changed - as a result broadcast mode needs
        // to be recalculated
        onBroadcastModeChange();
    }

    bool preRecordedMessageChangesMade = true;

    // The ad-hoc list doesn't need any special treatment, just dispatch update
    // directly to the list
    if (m_adhocLst.doesMessageKeyExist(updatedKeys))
    {
        if (m_adhocLst.doAllMessageKeysExist(updatedKeys))
        {
            // If the ad-hoc list contains all the input update keys
            // It means they're all of type ad-hoc, which means
            // there are no updates for pre-recorded messages
            preRecordedMessageChangesMade = false;
        }

        m_adhocLst.processDvaMessageUpdates(updatedKeys);
    }

    if (preRecordedMessageChangesMade)
    {
        // Whenever there is a change to any pre-recorded DVA Message
        // Must udpate both normal and emergency lists entirely (because
        // with the filters, we can't determine definitively whether
        // this update affects us)
        OnChangeNormalFilterTb();
        OnChangeEmergencyFilterTb();
    }

    // Check if this update needs to re-enable the ad hoc list
    if (!m_adhocLst.IsWindowEnabled())
    {
        if (m_dvaMessageChangeSubscriber.hasTypeChangeWatchBeenCleared())
        {
            // Enable the window based on type update received
            EnableAdhocWindow(true, ADHOC_TYPE_ENABLE);
        }

        if (m_dvaMessageChangeSubscriber.hasLabelChangeWatchBeenCleared())
        {
            // Enable the window based on label update received
            EnableAdhocWindow(true, ADHOC_LABEL_ENABLE);
        }
    }

    FUNCTION_EXIT;
}

int StationBroadcastPage::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    FUNCTION_ENTRY("OnCreate");

    if (CPropertyPage::OnCreate(lpCreateStruct) == -1)
    {
        FUNCTION_EXIT;
        return -1;
    }

    FUNCTION_EXIT;
    return 0;
}

void StationBroadcastPage::OnPaint()
{
    FUNCTION_ENTRY("OnPaint");

    CPaintDC dc(this); // device context for painting

    // Do not call CPropertyPage::OnPaint() for painting messages

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnItemchangingStnAdhocMessagesLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnItemchangingStnAdhocMessagesLst");

    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    // Only interested in state changes
    if (pNMListView->uChanged != LVIF_STATE)
    {
        FUNCTION_EXIT;
        return;
    }

    // Looking for checkbox click...
    if (pNMListView->uNewState & 0x0002)
    {
        // Message selected - can ignore
        FUNCTION_EXIT;
        return;
    }

    TA_IRS_Bus::StationDvaMessageRecord* message = reinterpret_cast<TA_IRS_Bus::StationDvaMessageRecord*>(pNMListView->lParam);

    if (message == NULL)
    {
        // Ignore - possible in initial state
        FUNCTION_EXIT;
        return;
    }

    char newItemState = 0;
    // If the existing record data doesn't match the target state
    // It means it's a user applied change (ie user clicked on control)
    bool userAppliedUpdate = false;

    if (pNMListView->uNewState & 0x2000)
    {
        // Checked - Emergency
        newItemState = DvaListCtrlModel::getEmergencyType();

        if (!DvaListCtrlModel::isEmergencyMessage(message->m_type))
        {
            // Item is shifting from non-emergency to emergency
            userAppliedUpdate = true;
        }
    }
    else if (pNMListView->uNewState & 0x1000)
    {
        // Unchecked - Normal
        newItemState = DvaListCtrlModel::getNormalType();

        if (DvaListCtrlModel::isEmergencyMessage(message->m_type))
        {
            // Item is shifting from emergency to non-emergency (Normal or Deleted)
            userAppliedUpdate = true;
        }
    }

    if (m_adhocStyleChangeInProgress)
    {
        // TD #3152
        userAppliedUpdate = false;
    }

    if (0 == newItemState)
    {
        // Item isn't being checked or unchecked
        // (happens when unselecting items)
        FUNCTION_EXIT;
        return;
    }

    if (userAppliedUpdate)
    {
        // If the user is applying the update - then reject the udpate (post it to the agent)
        // We must disable the window while waiting for the update to be
        // received back from the ad hoc message change subscriber
        *pResult = TRUE;

        if (!DvaListCtrlModel::isDeletedMessage(message->m_type))
        {
            // Only actually process a user initiated type change request if the
            // adhoc message is not deleted (if it is deleted, just ignore)
            userInitiatedAdhocTypeChange(message->m_pKey, newItemState);
        }
    }
    else
    {
        // Automated update, accept it
        *pResult = FALSE;
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::userInitiatedAdhocTypeChange(unsigned long key, unsigned char newState)
{
    FUNCTION_ENTRY("userInitiatedAdhocTypeChange");

    if (!m_dvaMessageChangeSubscriber.hasTypeChangeWatchBeenCleared())
    {
        // This shouldn't happen
        assert(false);

        FUNCTION_EXIT;
        return;
    }

    // Initialise watch before the call made (in case update received
    // before postAdhocTypeChange returns)
    m_dvaMessageChangeSubscriber.registerWatchOnTypeChange(key, newState);

    if (m_stationBroadcastModel->postAdhocTypeChange(key, newState))
    {
        // Operation a success, diable the adhoc window based on type
        EnableAdhocWindow(false, ADHOC_TYPE_ENABLE);
    }
    else
    {
        // Operation failed, we should clear out the re-enabling watch
        // we set earlier
        m_dvaMessageChangeSubscriber.clearTypeChangeWatch();
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::userInitiatedAdhocLabelChange(unsigned long key, const std::string& newLabel)
{
    FUNCTION_ENTRY("userInitiatedAdhocLabelChange");

    if (!m_dvaMessageChangeSubscriber.hasLabelChangeWatchBeenCleared())
    {
        // This shouldn't happen
        assert(false);

        FUNCTION_EXIT;
        return;
    }

    // Initialise watch before the call made (in case update received
    // before postAdhocTypeChange returns)
    m_dvaMessageChangeSubscriber.registerWatchOnLabelChange(key, newLabel);

    // Send out the label change notification
    if (m_stationBroadcastModel->postAdhocLabelChange(key, newLabel))
    {
        // Operation a success, diable the adhoc window based on type
        EnableAdhocWindow(false, ADHOC_LABEL_ENABLE);
    }
    else
    {
        // Operation failed, we should clear out the re-enabling watch
        // we set earlier
        m_dvaMessageChangeSubscriber.clearLabelChangeWatch();
    }

    FUNCTION_EXIT;
}

BOOL StationBroadcastPage::OnSetActive()
{
    FUNCTION_ENTRY("OnSetActive");

    m_pageActive = true;

    // Should call this after activation (CPropertyPage::OnSetActive)
    // otherwise won't correctly update execute button state on parent page
    updateExecuteButtonEnabledState();

    // Refresh adhoc disables state on activation (this will
    // force the adhoc control to become enabled, if currently disabled)
    refreshAdhocDisabledState();

    FUNCTION_EXIT;
    return CPropertyPage::OnSetActive();
}

void StationBroadcastPage::refreshAdhocDisabledState()
{
    FUNCTION_ENTRY("refreshAdhocDisabledState");

    m_dvaMessageChangeSubscriber.clearAllChangeWatches();

    // Manually enable the windows
    EnableAdhocWindow(true, ADHOC_TYPE_ENABLE);
    EnableAdhocWindow(true, ADHOC_LABEL_ENABLE);

    FUNCTION_EXIT;
}

BOOL StationBroadcastPage::OnKillActive()
{
    FUNCTION_ENTRY("OnKillActive");

    m_pageActive = false;

    // Disable execute button on the main dialog when lose focus
    m_paManagerDlg->enableExecuteButton(false);

    FUNCTION_EXIT;
    return CPropertyPage::OnKillActive();
}

void StationBroadcastPage::displayDvaStatusDialog(const std::string& broadcastId)
{
    FUNCTION_ENTRY("displayDvaStatusDialog");

    TA_ASSERT(m_stationBroadcastModel->testCanObtainLock(), "This thread shouldn't own model lock");

    ///// Bring up Dva broadcast status window
    StationDvaStatusDlg stationDvaStatusDlg(
        // song toan++
        // TD 10316
        m_LocationType == CachedConfig::LOCATION_TYPE_OCC,
        // ++song toan
        // TD 10316
        broadcastId.c_str(),
        getDvaMessageChangeSubscriber(),
        this);

    // Pop up the dialog
    stationDvaStatusDlg.DoModal();

    FUNCTION_EXIT;
}

void StationBroadcastPage::displayLiveStatusDialog(const std::string& broadcastId)
{
    FUNCTION_ENTRY("displayLiveStatusDialog");

    TA_ASSERT(m_stationBroadcastModel->testCanObtainLock(), "This thread shouldn't own model lock");

    StationLiveStatusDlg stationLiveStatusDlg(
        // song toan++
        // TD 10316
        m_LocationType == CachedConfig::LOCATION_TYPE_OCC,
        // ++song toan
        // TD 10316
        broadcastId.c_str(),
        this);

    stationLiveStatusDlg.DoModal();

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnZoneGroupSaveBtn()
{
    FUNCTION_ENTRY("OnZoneGroupSaveBtn");

    if (!PaRightsManager::getInstance().hasRight(PaRightsManager::PA_MODIFY_ZONE_GROUPS))
    {
        // Perform secondary check
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);

        FUNCTION_EXIT;
        return;
    }

    CString initialName(s_defaultZoneGroupName.c_str());

    int curSel = m_zoneCombo.GetCurSel();

    if (curSel >= 0)
    {
        if (m_zoneCombo.GetItemData(curSel) != 0)
        {
            // Name the intial name correspond to currently selected
            // group (but not if it's the HMI group)
            m_zoneCombo.GetLBText(curSel, initialName);
        }
    }

    // Obtain selected zones before popping up any dialogs (because
    // asynchronous subscriber updates may clear selection while
    // the user is being prompted for a name..)
    std::list<unsigned long> selectedZoneKeys = getSelectedZones();

    if (0 == selectedZoneKeys.size())
    {
        TA_ASSERT(FALSE, "No current zone selection");

        FUNCTION_EXIT;
        return;
    }

    CreateNewGroupDlg nameEntryDialog(s_enterZoneGroupNameWindowTitle.c_str(),
                                      s_enterZoneGroupNamePrompt.c_str(),
                                      initialName,
                                      this);

    while (IDOK == nameEntryDialog.DoModal())
    {
        if (0 == nameEntryDialog.getName().GetLength())
        {
            // TD14164 ++
            TA_Base_Bus::TransActiveMessage userMsg;
            CString fieldName = "a valid label";
            userMsg << fieldName;
            CString errMsg = userMsg.constructMessage(IDS_UW_611002);
            /*AfxMessageBox(s_enterNonZeroLengthNameMessage.c_str());*/
            AfxMessageBox(errMsg); // "Please enter a valid label."
            // ++ TD14164
            continue;
        }

        // Process the request - may need to try again if user entered duplicate name
        std::string name(nameEntryDialog.getName().GetBuffer(0));

        try
        {
            CWaitCursor hourGlassCursor;

            TA_Base_Core::PrimitiveWrapper<TA_IRS_Bus::PaZoneGroupRecord> existingGroup =
                TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneGroupCachedMap().
                getKeyFromLocationAndLabel(
                    CachedConfig::getInstance()->getEntityLocationKey(),
                    name);

            // keyOverride will default to unset (so will generate a new key)
            TA_Base_Core::PrimitiveWrapper<unsigned long> keyOverride;

            // Existing item with same label / location key
            if (existingGroup.hasBeenSet())
            {
                if (existingGroup.getValue().m_isEventGroup)
                {
                    // This is an event group, user may not overwrite (otherwise
                    // will cause internal problems) - typically group names like "All" are reserved

                    // TD14164 yezh++
                    //AfxMessageBox(s_zoneGroupNameSystemReservedMessage.c_str());
                    TA_Base_Bus::TransActiveMessage usrMsg;
                    usrMsg.showMsgBox(IDS_UW_610009);
                    // TD14164 ++ yezh

                    continue;
                }

                const CString confirmationTitle("Zone Group Overwrite");

                // Selected name already exists - query user if they want to overwrite

                // TD14164 ++
                TA_Base_Bus::TransActiveMessage userMsg;
                CString errMsg = userMsg.constructMessage(IDS_UW_611003);
                // ++ TD14164

                // TD14164 ++
                /*if (IDNO == MessageBox(s_zoneGroupNameAlreadyExistsMessage.c_str(),
                                            confirmationTitle,
                                            MB_YESNO))*/
                if (IDNO == MessageBox(errMsg,
                                       confirmationTitle,
                                       MB_YESNO))
                    // ++ TD14164
                {
                    // User doesn't want to overwrite, allow them to choose another name
                    continue;
                }

                // User wants to overwrite - proceed as normal with with duplicate name
                // (set the keyOverride so we overwrite existing group)
                keyOverride.setValue(existingGroup.getValue().m_pKey);
            }

            createZoneGroupWithSelection(name, keyOverride, selectedZoneKeys);

            // Break out now that change made
            break;
        }
        catch (const TA_Base_Core::DatabaseException&)
        {
            PaErrorHandler::displayError(PaErrorHandler::DATABASE_ERROR);
        }
        catch (const TA_Base_Core::TransactiveException&)
        {
            // Name change didn't go through properly - could be
            // someone changed their name to this group before us..
            PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
        }
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnZoneGroupDeleteBtn()
{
    FUNCTION_ENTRY("OnZoneGroupDeleteBtn");

    int curSel = m_zoneCombo.GetCurSel();

    if (curSel < 0)
    {
        FUNCTION_EXIT;
        return;
    }

    unsigned long locationKey = CachedConfig::getInstance()->getEntityLocationKey();
    CString zoneLabel;
    m_zoneCombo.GetLBText(curSel, zoneLabel);

    if (!PaRightsManager::getInstance().hasRight(PaRightsManager::PA_MODIFY_ZONE_GROUPS))
    {
        // Perform secondary check
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);

        FUNCTION_EXIT;
        return;
    }

    try
    {
        std::ostringstream ostr;
        // TD14164 ++
        /*ostr << "Are you sure you want to delete Zone Group \"" << zoneLabel.GetBuffer(0) << "\"?";*/
        ostr << "Zone Group \"" << zoneLabel.GetBuffer(0) << "\"";
        TA_Base_Bus::TransActiveMessage userMsg;
        userMsg << ostr.str();
        UINT selectedButton = userMsg.showMsgBox(IDS_UW_610023);
        // ++ TD14164

        // TES #645, confirm action prior to deletion
        // TD14164 ++
        /*if (IDNO == AfxMessageBox((LPCTSTR)ostr.str().c_str(), MB_YESNO))*/
        if (IDNO == selectedButton)
            // ++ TD14164
        {
            // User cancelled delete operation
            FUNCTION_EXIT;
            return;
        }

        TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneGroupCachedMap().removePaZoneGroup(
            locationKey,
            zoneLabel.GetBuffer(0),
            CachedConfig::getInstance()->getEntityKey(),
            CachedConfig::getInstance()->getSessionId());

        // Refresh the zone group combo after removal of a Pa Zone Group
        refreshZoneComboData();
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // Error performing removal (perhaps someone else removed this group first?)
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::createZoneGroupWithSelection
(
    const std::string& name,
    TA_Base_Core::PrimitiveWrapper<unsigned long> existingKey,
    std::list<unsigned long>& selectedZoneKeys
)
{
    FUNCTION_ENTRY("createZoneGroupWithSelection");

    if (!PaRightsManager::getInstance().hasRight(PaRightsManager::PA_MODIFY_ZONE_GROUPS))
    {
        // Perform third stage check
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);

        FUNCTION_EXIT;
        return;
    }

    unsigned long locationKey = CachedConfig::getInstance()->getEntityLocationKey();

    unsigned long groupKey = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneGroupCachedMap().
        createNewPaZoneGroup(locationKey,
                             name,
                             false,
                             selectedZoneKeys,
                             existingKey,
                             CachedConfig::getInstance()->getEntityKey(),
                             CachedConfig::getInstance()->getSessionId());

    // When creating a group, don't wait for changes to feedback through notification
    // service - perform immediate refresh
    OnZoneGroupUpdate(groupKey, 0);

    // Select the newly created group
    for (DWORD i = 0; i < m_zoneCombo.GetCount(); i++)
    {
        if (groupKey == m_zoneCombo.GetItemData(i))
        {
            m_zoneCombo.SetCurSel(i);
            // This doesn't get called automatically (notify selection changes)
            OnSelchangeStnZoneGroupCombo();
            break;
        }
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnItemchangedStnZoneSelectionLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnItemchangedStnZoneSelectionLst");

    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    onZoneListOrZoneComboSelectionChange();
    *pResult = 0;

    FUNCTION_EXIT;
}

void StationBroadcastPage::onZoneListOrZoneComboSelectionChange()
{
    FUNCTION_ENTRY("onZoneListOrZoneComboSelectionChange");

    // Disable both buttons by default
    bool saveZoneEnabled = false;
    bool removeZoneEnabled = false;

    // We may only consider enabling these buttons if right exists to do so
    if (PaRightsManager::getInstance().hasRight(PaRightsManager::PA_MODIFY_ZONE_GROUPS))
    {
        if (m_zoneCombo.IsWindowEnabled())
        {
            // If the zone list isn't enabled then we should disable the
            // associated buttons also - so only test whether to enable if is enabled

            // Check the selection in zone combo box
            int curSel = m_zoneCombo.GetCurSel();

            if (curSel >= 0)
            {
                // Zone Group selected - may delete it (provided it's not the special
                // HMI group - which has a null data attachment)
                if (0 != m_zoneCombo.GetItemData(curSel))
                {
                    removeZoneEnabled = true;
                }
            }

            // Enable the add zone button if any zones exist in m_zoneLst
            if (m_zoneLst.GetItemCount() > 0)
            {
                saveZoneEnabled = true;
            }
        }
    }

    m_saveZoneGroupButton.EnableWindow(saveZoneEnabled);
    m_deleteZoneGroupButton.EnableWindow(removeZoneEnabled);

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnSelchangeStnZoneGroupCombo()
{
    LOG_SCOPE("StationBroadcastPage::OnSelchangeStnZoneGroupCombo");
    FUNCTION_ENTRY("OnSelchangeStnZoneGroupCombo");

    CWaitCursor waitCursor;

    // Refresh zone list with new data taken from combo
    refreshZoneList();

    onZoneListOrZoneComboSelectionChange();

    FUNCTION_EXIT;
}

void StationBroadcastPage::createCachedMapObservers()
{
    FUNCTION_ENTRY("createCachedMapObservers");

    TA_ASSERT(IsWindow(m_hWnd), "Called create too early");

    CachedMapChangedWindowNotificationHandler* pHandler;

    // Create a handler so we can receive zone change notifications from config editor
    pHandler = new CachedMapChangedWindowNotificationHandler(
        m_hWnd,
        WM_PA_STN_ZONE_UPDATE,
        TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneCachedMap());
    m_cachedMapObservers.push_back(pHandler);

    // Create a handler so we get zone group change notifications from config editor
    pHandler = new CachedMapChangedWindowNotificationHandler(
        m_hWnd,
        WM_PA_STN_ZONE_GROUP_UPDATE,
        TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneGroupCachedMap());

    m_cachedMapObservers.push_back(pHandler);

    FUNCTION_EXIT;
}

void StationBroadcastPage::destroyCachedMapObservers()
{
    FUNCTION_ENTRY("destroyCachedMapObservers");

    // Should clean up before window destroyed (otherwise unsafe)
    TA_ASSERT(IsWindow(m_hWnd), "Called destroy too late");

    for (int i = 0; i < m_cachedMapObservers.size(); i++)
    {
        delete m_cachedMapObservers[i];
    }

    m_cachedMapObservers.clear();

    FUNCTION_EXIT;
}

afx_msg LRESULT StationBroadcastPage::OnZoneGroupUpdate(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnZoneGroupUpdate");

    int curSel = m_zoneCombo.GetCurSel();

    if (curSel < 0)
    {
        // No groups selected, nothing to change
        FUNCTION_EXIT;
        return 0;
    }

    try
    {
        Keys prevZoneSelection = getSelectedZones();

        unsigned long currentGroupKey = m_zoneCombo.GetItemData(curSel);
        CString oldGroupName;
        m_zoneCombo.GetLBText(curSel, oldGroupName);

        // Update item in combo (may delete and revert to HMI selection, or just update)
        // - User may have dropdown list 'open', so even if item isn't selected the
        // changes still need to be propogated
        refreshZoneComboData();

        // If the currently selected zone group doesn't match wParam, nothing
        // to do (changes can't be visualised)
        if (wParam == currentGroupKey)
        {
            // In case there were changes to keys defining zone list
            refreshZoneList();

            Keys newZoneSelection = getSelectedZones();
            // Ensure the comparison is done independent of ordering
            prevZoneSelection.sort();
            newZoneSelection.sort();

            CString newGroupName;
            curSel = m_zoneCombo.GetCurSel();

            if (curSel >= 0)
            {
                m_zoneCombo.GetLBText(curSel, newGroupName);
            }

            // TD #3315: Notify user of any changes to the currently selected group
            // (if a different group changes, no notification required) - this can either
            // be in the form of the group name changing, or contents (zones) changing
            if (newZoneSelection != prevZoneSelection || oldGroupName != newGroupName)
            {
                PaErrorHandler::displayError(PaErrorHandler::EXTERNAL_PA_ZONE_GROUP_UPDATE_RECEIVED);
            }
        }
    }

    STANDARD_PA_CATCH("StationBroadcastPage::OnZoneGroupUpdate", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

afx_msg LRESULT StationBroadcastPage::OnZoneUpdate(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnZoneUpdate");

    // wParam denotes primary key of the zone that's changed - check
    // if there is a need to update display
    unsigned long zoneKey = wParam;

    std::ostringstream s;
    s << "StationBroadcastPage::OnZoneUpdate(" << zoneKey << ")";
    OutputDebugString(s.str().c_str());

    try
    {
        // song toan++
        // TD4080
        // We always have to refresh the zone list because:
        //
        // For non-OCC:
        //      1-If a key is in the zone list:
        //          -Details of the zone associated with it could be changed
        //          by Configuration Editor. They should be updated in this case.
        //
        //          -It may have to be removed from the zone list because it
        //          could be associated to another location by Configuration Editor.
        //
        //      2-If a key is not in the zone list, it may have to be inserted
        //      into the zone list because its location could be changed to this
        //      location by Configuration Editor.
        //
        // For OCC:
        //      We can refresh the zone list only when the given key is in the
        //      zone list. However, a newly created zone would not be added into
        //      the list.
        //
        // song toan++
        // TD4080
        {
            // Invalidate zone list (because zone keys may not change,
            // but zone info may have)
            m_zoneLst.invalidateZoneData();

            refreshZoneList();

            // Change in PA Zone always comes from configuration editor
            PaErrorHandler::displayError(PaErrorHandler::EXTERNAL_PA_ZONE_UPDATE_RECEIVED);
        }
    }

    STANDARD_PA_CATCH("StationBroadcastPage::OnZoneUpdate", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

afx_msg LRESULT StationBroadcastPage::OnHMIZoneSelectionChange(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnHMIZoneSelectionChange");

    TA_ASSERT(0 != m_hmiZoneSelChangeListener, "Zone listener null");

    try
    {
        // Respond by updating HMI zone selection, if HMI group selected
        // then setHMIZones will automatically refresh
        setHMIZones(m_hmiZoneSelChangeListener->getZoneList());
    }

    STANDARD_PA_CATCH("StationBroadcastPage::OnHMIZoneSelectionChange", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

void StationBroadcastPage::OnItemchangedStnNormalMessagesLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnItemchangedStnNormalMessagesLst");

    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    m_dvaMessageListSelController.onListItemStateChange(pNMListView, &m_normalLst);
    updatePlayStopRecordButtons(m_dvaBtn.GetCheck() != 0);

    *pResult = 0;

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnItemchangedStnEmergencyMessagesLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnItemchangedStnEmergencyMessagesLst");

    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    m_dvaMessageListSelController.onListItemStateChange(pNMListView, &m_emergencyLst);
    // DVA Area must be enabled if changing selection
    updatePlayStopRecordButtons(m_dvaBtn.GetCheck() != 0);

    *pResult = 0;

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnItemchangedStnAdhocMessagesLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnItemchangedStnAdhocMessagesLst");

    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    m_dvaMessageListSelController.onListItemStateChange(pNMListView, &m_adhocLst);
    // DVA Area must be enabled if changing selection
    updatePlayStopRecordButtons(m_dvaBtn.GetCheck() != 0);

    *pResult = 0;

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnStnMusicBtn()
{
    FUNCTION_ENTRY("OnStnMusicBtn");

    processPAModeChange(&m_stnMusicButton);

    // Immediately update music broadcasting state on button state change
    if (m_stnMusicButton.GetCheck())
    {
        m_stationBroadcastModel->executeMusic(TA_Base_Bus::IPAAgentCorbaDef::LocalMusic);
    }
    else
    {
        // liuyu++ TD8857
        // m_stationBroadcastModel->executeMusic(TA_Base_Bus::IPAAgentCorbaDef::NoMusic);
        m_stationBroadcastModel->executeMusic(TA_Base_Bus::IPAAgentCorbaDef::NoLocalMusic);
        // ++liuyu TD8857
    }

    onBroadcastModeChange();

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnOccMusicBtn()
{
    FUNCTION_ENTRY("OnOccMusicBtn");

    processPAModeChange(&m_occMusicButton);

    // Immediately update music broadcasting state on button state change
    if (m_occMusicButton.GetCheck())
    {
        m_stationBroadcastModel->executeMusic(TA_Base_Bus::IPAAgentCorbaDef::OccMusic);
    }
    else
    {
        // liuyu++ TD8857
        // m_stationBroadcastModel->executeMusic(TA_Base_Bus::IPAAgentCorbaDef::NoMusic);
        m_stationBroadcastModel->executeMusic(TA_Base_Bus::IPAAgentCorbaDef::NoOccMusic);
        // ++liuyu TD8857
    }

    onBroadcastModeChange();

    FUNCTION_EXIT;
}

void StationBroadcastPage::destroyIcons()
{
    FUNCTION_ENTRY("destroyIcons");

    for (std::map<DWORD, HANDLE>::iterator it = m_hIconBitmaps.begin();
         it != m_hIconBitmaps.end();
         it++)
    {
        BOOL result = DestroyIcon((HICON)it->second);
        TA_ASSERT(result, "Icon destruction failed");
    }

    m_hIconBitmaps.clear();

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnStnPlayBtn()
{
    FUNCTION_ENTRY("OnStnPlayBtn");

    // Do nothing while the model is still playing the last
    // message (user must stop first)
    if (m_dvaPlaybackObject->isPlayingDvaMessage())
    {
        // Silently ignore (user clicking on depressed play button)
        onDvaPlayStateChanged(true);        // - Keep play button pressed
        FUNCTION_EXIT;
        return;
    }
    else
    {
        // Ensure button remains unchecked if not currently
        // playing back - until a broadcast is successful
        onDvaPlayStateChanged(false);
    }

    unsigned long selectedMessageKey = getSelectedDvaMessageKey();

    if (!TA_Base_Core::DatabaseKey::isValidKey(selectedMessageKey))
    {
        TA_ASSERT(FALSE, "No messages selected");

        FUNCTION_EXIT;
        return;
    }

    try
    {
        m_dvaPlaybackObject->playDvaMessage(selectedMessageKey, *this);

        // Indicate the message is playing back
        onDvaPlayStateChanged(true);
    }
    catch (const TA_Base_Bus::IPAAgentCorbaDef::BroadcastLimitReached&)
    {
        PaErrorHandler::displayError(PaErrorHandler::MAXIMUM_BROADCAST_COUNT_REACHED);
    }
    catch (const TA_Base_Core::BadParameterException&)
    {
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
    catch (const TA_Base_Core::ObjectResolutionException&)
    {
        // Comms error
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }
    catch (const TA_Base_Core::CachedMappingNotFoundException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::INVALID_CONFIG_DATA);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnStnRecordBtn()
{
    FUNCTION_ENTRY("OnStnRecordBtn");

    if (!PaRightsManager::getInstance().hasRightToRecordDvaMessage())
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);

        FUNCTION_EXIT;
        return;
    }

    unsigned long adhocKey = getSelectedDvaMessageKey();

    // Shouldn't be bringing up recording dialog while something playing in background
    if (m_dvaPlaybackObject->isPlayingDvaMessage())
    {
        TA_ASSERT(FALSE, "Invalid state");

        FUNCTION_EXIT;
        return;
    }

    if (!TA_Base_Core::DatabaseKey::isValidKey(adhocKey))
    {
        TA_ASSERT(FALSE, "Invalid message key");

        FUNCTION_EXIT;
        return;
    }

    // The selction should correspond to a single item in ad-hoc list
    if (1 != m_adhocLst.findItemIndices(adhocKey).size())
    {
        TA_ASSERT(FALSE, "Invalid message for record operation");

        FUNCTION_EXIT;
        return;
    }

    TA_IRS_Bus::StationDvaMessageRecord record =
        TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(adhocKey);

    AdhocMessageRecordPage recordMessage(record, m_dvaPlaybackObject, this);

    if (IDOK == recordMessage.DoModal())
    {
        // Post a type / label change regardless of whether user
        // made changes locally (in case external changes made)
        // However only attempt to do so if the correct privilege exists
        if (PaRightsManager::getInstance().hasRight(PaRightsManager::PA_MODIFY_ADHOC_MESSAGE))
        {
            // Since we can pop up this dialog while the ad-hoc control is disabled
            // must re-enable control BEFORE attempting some more change requests
            refreshAdhocDisabledState();

            userInitiatedAdhocTypeChange(adhocKey,
                                         recordMessage.getMessageType());
            userInitiatedAdhocLabelChange(adhocKey,
                                          recordMessage.getMessageDescription());
        }
        else
        {
            // Check if the operator had changed any of the type or label settings
            // The only way this could have happened is if the right to modify the
            // adhoc messages was removed AFTER the dialog had been activated
            // (if they didn't change anything, no need to mention this error
            // because they didn't attempt to make any type / label changes)
            if (recordMessage.hasMessageTypeOrLabelBeenModified())
            {
                // Indicate to the user that while they were able to modify
                // the type / label, the privileges have changed (so changes won't be saved)
                PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);
            }
        }
    }
    else
    {
        // User may choose to close dialog, in which case type/label changes
        // won't be applied (but overwriting recording, if had been done, will
        // remain as this change can't be rolled back)
    }

    // To fix IFAT issue where ad-hoc list was no re-enabled.
    // Post a type / label change regardless of whether user
    // made changes locally (in case external changes made)
    // However only attempt to do so if the correct privilege exists
    if (PaRightsManager::getInstance().hasRight(PaRightsManager::PA_MODIFY_ADHOC_MESSAGE))
    {
        // Since we can pop up this dialog while the ad-hoc control is disabled
        // must re-enable control BEFORE attempting some more change requests
        refreshAdhocDisabledState();
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnStnStopBtn()
{
    FUNCTION_ENTRY("OnStnStopBtn");

    // Whether the stop was successful or not, we indicate visually
    // the playback has stopped (if no message playing, just ignore processing
    // of the request - but still update the button state)
    if (m_dvaPlaybackObject->isPlayingDvaMessage())
    {
        if (!m_dvaPlaybackObject->stopDvaPlayback())
        {
            // Failed to stop (PAAgent requests denied) - broadcast remains..
            PaErrorHandler::displayError(PaErrorHandler::BROADCAST_STILL_ACTIVE);
        }
    }

    onDvaPlayStateChanged(false);

    FUNCTION_EXIT;
}

void StationBroadcastPage::onDvaPlayStateChanged(bool bPlayStarted)
{
    LOG_SCOPE("StationBroadcastPage::onDvaPlayStateChanged");
    FUNCTION_ENTRY("onDvaPlayStateChanged");

    if (bPlayStarted)
    {
        m_playBtn.SetCheck(BST_CHECKED);
    }
    else
    {
        // Upon stop, can release the play button
        m_playBtn.SetCheck(BST_UNCHECKED);
    }

    // Need to update button states to bring us up to date
    // (particularly disabling the stop button as required)
    updatePlayStopRecordButtons(m_dvaBtn.GetState() != 0);

    FUNCTION_EXIT;
}

afx_msg LRESULT StationBroadcastPage::OnProgressReportUpdate(UINT wParam, LONG lParam)
{
    LOG_SCOPE("WM_PA_STN_STATUS_REPORT_UPDATE", "StationBroadcastPage::OnProgressReportUpdate");
    FUNCTION_ENTRY("OnProgressReportUpdate");

    // Check if this progress report update is owner by the model,
    // if so, pass it along
    if (lParam == reinterpret_cast<LONG>(m_dvaPlaybackObject))
    {
        try
        {
            if (m_dvaPlaybackObject->processPlayDvaNotification(wParam))
            {
                // Broadcast model has now removed the play broadcast (ie it's complete)
                onDvaPlayStateChanged(false);
            }
        }

        STANDARD_PA_CATCH("StationBroadcastPage::OnProgressReportUpdate", PaErrorHandler::UNSPECIFIED_ERROR);
    }

    FUNCTION_EXIT;
    return 0;
}

void StationBroadcastPage::OnDestroy()
{
    FUNCTION_ENTRY("OnDestroy");

    // Delete the rights change notification object before window handle becomes invalid
    delete m_rightsChangeHandler;
    m_rightsChangeHandler = 0;

    // Must destroy subscribers attached to this window before
    // DestroyWindow completes (otherwise they'll be sending updates
    // to a now destroyed window)

    try
    {
        m_dvaMessageChangeSubscriber.removeListenerWindow(m_hWnd);
        PaStatusNameChangeSubscriber::getInstance().removeListenerWindow(m_hWnd);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // Cannot unsubscribe ad hoc message change listener
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }

    // Clean out the cached map observers (before window handle destroyed)
    destroyCachedMapObservers();

    // Must stop any active DVA playback before we destroy the window
    if (m_dvaPlaybackObject->isPlayingDvaMessage())
    {
        m_dvaPlaybackObject->stopDvaPlayback();
    }

    delete m_hmiZoneSelChangeListener;
    m_hmiZoneSelChangeListener = NULL;

    CPropertyPage::OnDestroy();

    FUNCTION_EXIT;
}

void StationBroadcastPage::refreshBroadcastButtonEnabledState()
{
    LOG_SCOPE("StationBroadcastPage::refreshBroadcastButtonEnabledState");
    FUNCTION_ENTRY("refreshBroadcastButtonEnabledState");

    // Checking the rights, select a suitable default button (fall back to
    // the station music button as a last resort)
    bool canBroadcastDva = PaRightsManager::getInstance().hasRight(PaRightsManager::PA_BROADCAST_STATION_DVA);
    bool canBroadcastLive = PaRightsManager::getInstance().hasRightToBroadcastStationLive();
    bool canBroadcastMusic = PaRightsManager::getInstance().hasRight(PaRightsManager::PA_BROADCAST_STATION_MUSIC);
    // song toan++
    // TD10409

    // station music button must be disabled at OCC
    bool bStationMusicAllowed = canBroadcastMusic && (m_LocationType != CachedConfig::LOCATION_TYPE_OCC);

    // OCC music button must be disabled at OCC and depot
    bool bOccMusicAllowed = canBroadcastMusic &&
        (m_LocationType != CachedConfig::LOCATION_TYPE_OCC) &&
        (m_LocationType != CachedConfig::LOCATION_TYPE_DEPOT);

    LOG_DEBUG("refreshBroadcastButtonEnabledState(): %s", nvps(canBroadcastDva, canBroadcastLive, canBroadcastMusic, bStationMusicAllowed, bOccMusicAllowed));

    m_dvaBtn.EnableWindow(canBroadcastDva);
    m_liveBtn.EnableWindow(canBroadcastLive);
    m_stnMusicButton.EnableWindow(bStationMusicAllowed);
    m_occMusicButton.EnableWindow(bOccMusicAllowed);

    try
    {
        PAAgentNamedObject& objPAAgent = CachedConfig::getInstance()->getAgentObject();
        const char* lpszSessionId = CachedConfig::getInstance()->getSessionId().c_str();

        // what kind of music is playing?
        TA_Base_Bus::IPAAgentCorbaDef::EMusicType musicType;
        CORBA_CALL_RETURN(musicType, objPAAgent, getMusicStatus, (lpszSessionId));

        // we update the OCC/STN music buttons accordingly
        if (bStationMusicAllowed)
        {
            m_stnMusicButton.SetCheck((musicType == TA_Base_Bus::IPAAgentCorbaDef::LocalMusic) ? BST_CHECKED : BST_UNCHECKED);
        }

        if (bOccMusicAllowed)
        {
            m_occMusicButton.SetCheck((musicType == TA_Base_Bus::IPAAgentCorbaDef::OccMusic) ? BST_CHECKED : BST_UNCHECKED);
        }
    }

    STANDARD_PA_CATCH("StationBroadcastPage::refreshBroadcastButtonEnabledState", PaErrorHandler::UNSPECIFIED_ERROR);

    // ++song toan
    // TD10409

    FUNCTION_EXIT;
}

void StationBroadcastPage::enterDefaultBroadcastMode()
{
    FUNCTION_ENTRY("enterDefaultBroadcastMode");

    if (m_dvaBtn.IsWindowEnabled())
    {
        // May default to DVA button provided enabled
        m_dvaBtn.SetCheck(BST_CHECKED);
        OnDvaBtn();
    }
    else if (m_liveBtn.IsWindowEnabled())
    {
        // Fall back to live window if not
        m_liveBtn.SetCheck(BST_CHECKED);
        OnLiveBtn();
    }
    else
    {
        // No default mode entered
        processPAModeChange(0);
        m_stationBroadcastModel->unsetBroadcastType();
        updateExecuteButtonEnabledState();
        onBroadcastModeChange();
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::refreshAdhocListStyle()
{
    FUNCTION_ENTRY("refreshAdhocListStyle");

    DWORD style = m_adhocLst.GetExtendedStyle();

    if (PaRightsManager::getInstance().hasRight(PaRightsManager::PA_MODIFY_ADHOC_MESSAGE))
    {
        // May modify ad-hoc messages, so display checkboxes
        style |= LVS_EX_CHECKBOXES;

        changeAdhocListTitle(ADHOCLIST_FULL_TITLE);
    }
    else
    {
        // Cannot modify message label or type
        style &= ~LVS_EX_CHECKBOXES;

        changeAdhocListTitle(ADHOCLIST_TITLE);
    }

    // TD #3152: When applying the LVS_EXCHECKBOXES style to ad-hoc list,
    // all emergency ad-hoc messages suddenly believe they've
    // just been clicked on (because state changes from normal to emergency
    // - because when no checkboxes present, reverts to normal state)
    // Therefore we must suppress this (otherwise an assert will be tripped later)
    m_adhocStyleChangeInProgress = true;

    try
    {
        m_adhocLst.SetExtendedStyle(style);

        // Each time applying the LVS_EX_CHECKBOXES style, ensure the
        // checkboxes are all set accordingly (to match the state)
        for (int nItem = 0; nItem < m_adhocLst.GetItemCount(); ++nItem)
        {
            TA_IRS_Bus::StationDvaMessageRecord* pRec = m_adhocLst.getRecordFromIndex(nItem);
            m_adhocLst.updateCheckedState(nItem, pRec);
        }
    }
    catch (...)
    {
        m_adhocStyleChangeInProgress = false;

        FUNCTION_EXIT;
        throw;
    }

    m_adhocStyleChangeInProgress = false;

    FUNCTION_EXIT;
}

void StationBroadcastPage::changeAdhocListTitle(std::string newTitle)
{
    FUNCTION_ENTRY("changeAdhocListTitle");

    enum { BUF_SIZE = 128 };
    char text[BUF_SIZE];

    LVCOLUMN columnData;

    columnData.mask = LVCF_TEXT;
    columnData.pszText = text;
    columnData.cchTextMax = BUF_SIZE;

    // Change label of first column header
    if (m_adhocLst.GetColumn(0, &columnData))
    {
        strcpy(columnData.pszText, newTitle.c_str());
        m_adhocLst.SetColumn(0, &columnData);
    }

    FUNCTION_EXIT;
}

void StationBroadcastPage::OnItemchangingStnZoneSelectionLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnItemchangingStnZoneSelectionLst");

    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    // Only interested in state changes
    if (pNMListView->uChanged != LVIF_STATE)
    {
        FUNCTION_EXIT;
        return;
    }

    // Reject the change in state (no selections allowed for PA Zones)
    *pResult = TRUE;

    FUNCTION_EXIT;
}

void StationBroadcastPage::updateSelectedListButtons(bool isDvaAreaEnabled)
{
    FUNCTION_ENTRY("updateSelectedListButtons");

    bool enableClearOne = isDvaAreaEnabled;
    bool enableClearAll = isDvaAreaEnabled;

    // Only enable the clear one button if there is an item selected
    if (0 == m_selectedLst.GetSelectedCount())
    {
        // No items selected
        enableClearOne = false;
    }

    m_clearOneBtn.EnableWindow(enableClearOne);

    if (0 == m_selectedLst.GetItemCount())
    {
        // No items to clear
        enableClearAll = false;
    }

    m_clearAllBtn.EnableWindow(enableClearAll);

    FUNCTION_EXIT;
}

void StationBroadcastPage::onBroadcastModeChange()
{
    LOG_SCOPE("StationBroadcastPage::onBroadcastModeChange");
    FUNCTION_ENTRY("onBroadcastModeChange");

    int currentBroadcastMode = m_stationBroadcastModel->getCurrentPriorityEstimationMode();

    m_zoneLst.setPriorityEstimationMode(currentBroadcastMode);

    FUNCTION_EXIT;
}

afx_msg LRESULT StationBroadcastPage::OnRightsChanged(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnRightsChanged");

    try
    {
        // Make necessary calls to refresh enabled state of buttons (those buttons
        // whose enabled states are dependent on rights anyway)..
        onZoneListOrZoneComboSelectionChange();

        // Refresh buttons associated to music, live and DVA broadcasts
        refreshBroadcastButtonEnabledState();

        // Refresh whether checkboxes present or not on the ad-hoc list control
        refreshAdhocListStyle();

        updatePlayStopRecordButtons(m_dvaBtn.GetCheck() != 0);
    }

    STANDARD_PA_CATCH("StationBroadcastPage::OnHMIZoneSelectionChange", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

afx_msg LRESULT StationBroadcastPage::OnStatusNameChange(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnStatusNameChange");

    try
    {
        const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData nameChange =
            PaStatusNameChangeSubscriber::getInstance().getMessageData(wParam, m_hWnd);

        // PW3819 respond to StationMusic name changes to determine push-button states
        if (TA_Base_Bus::IPAAgentCorbaDef::StationMusic == nameChange.broadcastType)
        {
            onStationMusicStatusNameChange(nameChange);
        }
        else
        {
            // Forward to our DVA playback model, so it can remain up to date
            if (0 != m_dvaPlaybackObject)
            {
                m_dvaPlaybackObject->onStatusNameChange(nameChange.fromBroadcastId.in(),
                                                        nameChange.toBroadcastId.in());
            }
        }
    }

    STANDARD_PA_CATCH("StationBroadcastPage::OnStatusNameChange", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

void StationBroadcastPage::onStationMusicStatusNameChange(const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData& nameChange)
{
    LOG_SCOPE("StationBroadcastPage::onStationMusicStatusNameChange");
    FUNCTION_ENTRY("onStationMusicStatusNameChange");

    // song toan++
    // TD10409
    try
    {
        if (nameChange.broadcastType == TA_Base_Bus::IPAAgentCorbaDef::StationMusic)
        {
            bool canBroadcastMusic = PaRightsManager::getInstance().hasRight(
                PaRightsManager::PA_BROADCAST_STATION_MUSIC);

            // station music button must be disabled at OCC
            bool bStationMusicAllowed = canBroadcastMusic && (CachedConfig::LOCATION_TYPE_OCC != m_LocationType);

            // OCC music button must be disabled at both OCC and depot
            bool bOccMusicAllowed = canBroadcastMusic &&
                (CachedConfig::LOCATION_TYPE_OCC != m_LocationType) &&
                (CachedConfig::LOCATION_TYPE_DEPOT != m_LocationType);

            // what kind of music is playing?
            PAAgentNamedObject& objPAAgent = CachedConfig::getInstance()->getAgentObject();
            const char* lpszSessionId = CachedConfig::getInstance()->getSessionId().c_str();
            TA_Base_Bus::IPAAgentCorbaDef::EMusicType musicType;
            CORBA_CALL_RETURN(musicType, objPAAgent, getMusicStatus, (lpszSessionId));

            // we update the OCC/STN music buttons accordingly
            if (bStationMusicAllowed)
            {
                m_stnMusicButton.SetCheck((musicType == TA_Base_Bus::IPAAgentCorbaDef::LocalMusic) ? BST_CHECKED : BST_UNCHECKED);
            }

            if (bOccMusicAllowed)
            {
                m_occMusicButton.SetCheck((musicType == TA_Base_Bus::IPAAgentCorbaDef::OccMusic) ? BST_CHECKED : BST_UNCHECKED);
            }
        }
    }

    STANDARD_PA_CATCH("StationBroadcastPage::onStationMusicStatusNameChange", PaErrorHandler::UNSPECIFIED_ERROR);
    // ++song toan
    // TD10409

    // This is pending implementation - as currently the PA Agent does not treat station music
    // broadcasts as persistent state broadcasts (they are executed then deleted immediately).
    // Waiting on changes to PAAgent to define how to determine if station music is currently
    // playing, as well as when the playing status changes..
    /*
    try
    {
        // Check what type of Station Music the name change represents (Station or OCC music)
        std::auto_ptr<TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig> bc =
                                    CachedConfig::getInstance()->getAgentObject().getBroadcastConfig();

        TA_Base_Bus::IPAAgentCorbaDef::StationMusicConfig& stationMusicConfig =
                        bc->theBroadcastConfigSpecifics.theStationMusicConfig();

        switch (stationMusicConfig.musicType)
        {
        case TA_Base_Bus::IPAAgentCorbaDef::NoMusic:
            break;
        case TA_Base_Bus::IPAAgentCorbaDef::LocalMusic:
            break;
        case TA_Base_Bus::IPAAgentCorbaDef::OccMusic:
            break;
        default:
            TA_ASSERT(FALSE, "Unhandled switch case");
        }
    }
    STANDARD_PA_CATCH("onStationMusicStatusNameChange", PaErrorHandler::UNSPECIFIED_ERROR);
    */

    FUNCTION_EXIT;
}

HBRUSH StationBroadcastPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    FUNCTION_ENTRY("OnCtlColor");

    HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

    if (IDC_STIS_TEXT == pWnd->GetDlgCtrlID())
    {
        pDC->SelectObject(&m_underlineFont);
    }

    FUNCTION_EXIT;
    return hbr;
}

// TD 15349
bool StationBroadcastPage::selectMessageNameInLists(const std::string messageName)
{
    FUNCTION_ENTRY("selectMessageNameInLists");

    // Find the selected message in DVA message lists by order :
    // EmergencyList, AdhocList, NormalList
    if (findAndSelectMessageNameInList(messageName, m_emergencyLst))
    {
        FUNCTION_EXIT;
        return true;
    }
    else if (findAndSelectMessageNameInList(messageName, m_adhocLst))
    {
        FUNCTION_EXIT;
        return true;
    }
    else if (findAndSelectMessageNameInList(messageName, m_normalLst))
    {
        FUNCTION_EXIT;
        return true;
    }
    else
    {
        LOG_ERROR("Cannot find the message in any DVA list, message name: %s", messageName);

        FUNCTION_EXIT;
        return false;
    }
}

bool StationBroadcastPage::findAndSelectMessageNameInList(const std::string messageName, DvaListCtrl& DvaList)
{
    FUNCTION_ENTRY("findAndSelectMessageNameInList");

    bool isSelectMessageNameInList = false;

    // To represent space between words or characters in selected message, use underscore ( _ ).
    // So underscore is reserved for parsing the command-line and substituted with space.
    // It must not be used as a character in the name.
    std::string::size_type index = 0;
    std::string::size_type pindex = 0;
    std::string paramValue("");

    pindex = messageName.find("_", index);

    while (pindex != std::string::npos)
    {
        paramValue.append(messageName.substr(index, pindex - index));
        paramValue.append(" ");
        index = pindex + 1;
        pindex = messageName.find("_", index);
    }

    paramValue.append(messageName.substr(index, messageName.size() - index));

    // Compare selected message with the messages in list
    for (int nItem = 0; nItem < DvaList.GetItemCount(); nItem++)
    {
        TA_IRS_Bus::StationDvaMessageRecord* pRecord = DvaList.getRecordFromIndex(nItem);

        if (paramValue == pRecord->m_label)
        {
            OnClearAllBtn();
            isSelectMessageNameInList = DvaList.selectItemInList(nItem);
            addToSelectedMessagesList(DvaList);
        }
    }

    FUNCTION_EXIT;
    return isSelectMessageNameInList;
}

// TD 15349
