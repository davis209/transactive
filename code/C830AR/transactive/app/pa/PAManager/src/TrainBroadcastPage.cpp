/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainBroadcastPage.cpp $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * TrainBroadcastPage handles GUI side of train broadcast tab
 */

#include "app/pa/PAManager/src/StdAfx.h"

#include "app/pa/PAManager/src/CachedMapChangedWindowNotificationHandler.h"
#include "app/pa/PAManager/src/CreateNewGroupDlg.h"
#include "app/pa/PAManager/src/LockControlRedraw.h"
#include "app/pa/PAManager/src/PaErrorHandler.h"
#include "app/pa/PAManager/src/PaLayoutHelper.h"
#include "app/pa/PAManager/src/PaManagerDlg.h"
#include "app/pa/PAManager/src/PaRightsManager.h"
#include "app/pa/PAManager/src/RightsChangedWindowNotificationHandler.h"
#include "app/pa/PAManager/src/TrainBroadcastModel.h"
#include "app/pa/PAManager/src/TrainBroadcastPage.h"
#include "app/pa/PAManager/src/TrainDvaStatusDlg.h"
#include "app/pa/PAManager/src/TrainDvaVersionsPage.h"
#include "app/pa/PAManager/src/TrainLiveStatusDlg.h"
#include "app/pa/PAManager/src/pamanager.h"

#include "bus/mfc_extensions/src/selection_helpers/SelectionHelper.h"
#include "bus/pa/cachedmaps/src/PaTrainDvaMessageCachedMap.h"
#include "bus/trains/TrainSelectionComponent/src/ITrainSelectorGUI.h"
#include "bus/trains/TrainSelectionComponent/src/MFCTrainSelectorGUI.h"

#include "core/utilities/src/DebugUtil.h"

// AZ++ TES 10924

// ++AZ

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_IRS_Bus;

static const COleDateTime       NULL_TIME(2000, 1, 1, 0, 0, 0);
static const COleDateTimeSpan   NULL_TIMESPAN(NULL_TIME);
static const int				MAX_TIME_INTERVAL = 65536;

IMPLEMENT_DYNCREATE(TrainBroadcastPage, CPropertyPage)

const std::string TrainBroadcastPage::s_inServiceTabText = "Mainline";
const std::string TrainBroadcastPage::s_depotTabText = "Depot";

const std::string TrainBroadcastPage::s_enterNonZeroLengthNameMessage =
"Please enter a valid name.";
const std::string TrainBroadcastPage::s_dynamicGroupNameAlreadyExistsMessage =
"The name entered is already in use, overwrite existing group?";
const std::string TrainBroadcastPage::s_defaultDynamicGroupName =
"Unnamed Group";

const std::string TrainBroadcastPage::s_enterDynamicGroupNamePrompt =
"Please enter a name for the new dynamic group";
const std::string TrainBroadcastPage::s_enterDynamicGroupNameWindowTitle =
"PA Manager - Add Dynamic Train Group";

TrainBroadcastPage::TrainBroadcastPage(PAManagerDlg* paManagerDlg)
    : CPropertyPage(TrainBroadcastPage::IDD),
    m_paManagerDlg(paManagerDlg),
    m_anyTrainsSelected(false),
    m_pageActive(false),
    m_hLiveIcon(0),
    m_hDvaIcon(0),
    m_trainDvaUpdateNotificationHandler(0),
    m_rightsChangeHandler(0),
    m_trainSelectorGUI(NULL),
    m_outboundTrainList(0),
    m_inboundTrainList(0),
    m_dvaMessageList(TDM_COL_MAX),
    // AZ++ TES 10924
    m_trainDvaVersionsPage(NULL)
    // ++AZ
{
    FUNCTION_ENTRY("TrainBroadcastPage");

    //{{AFX_DATA_INIT(TrainBroadcastPage)
    //}}AFX_DATA_INIT
    TA_IRS_Bus::LocationCache::EStationType startMode;

    if (CachedConfig::getInstance()->getLocationType() ==
        CachedConfig::LOCATION_TYPE_DEPOT)
    {
        // If at depot, only the depot tab is available
        // (so we must display depot mode)
        startMode = TA_IRS_Bus::LocationCache::Depot;
    }
    else
    {
        // At either OCC or Station, OK to use In Service mode
        startMode = getAppropriateInServiceMode();
    }

    m_trainBroadcastModel = new TrainBroadcastModel(*this, startMode);

    FUNCTION_EXIT;
}

TrainBroadcastPage::~TrainBroadcastPage()
{
    FUNCTION_ENTRY("~TrainBroadcastPage");

    TA_ASSERT(0 == m_rightsChangeHandler, "Rights change handler should be deleted in OnDestroy");
    TA_ASSERT(0 == m_inboundTrainList, "Inbound list should be deleted in OnDestroy");
    TA_ASSERT(0 == m_outboundTrainList, "Outbound list should be deleted in OnDestroy");

    TA_ASSERT(0 == m_trainDvaUpdateNotificationHandler, "Shouldn't be valid still");

    try
    {
        if (m_hLiveIcon)
        {
            DestroyIcon((HICON)m_hLiveIcon);
            m_hLiveIcon = 0;
        }

        if (m_hDvaIcon)
        {
            DestroyIcon((HICON)m_hDvaIcon);
            m_hDvaIcon = 0;
        }
    }
    catch (...)
    {
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "Caught in destructor");
    }

    FUNCTION_EXIT;
}

void TrainBroadcastPage::DoDataExchange(CDataExchange* pDX)
{
    FUNCTION_ENTRY("DoDataExchange");

    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(TrainBroadcastPage)
    DDX_Control(pDX, IDC_BUTTONALLOUTBOUND, m_allOutboundButton);
    DDX_Control(pDX, IDC_BUTTONALLINBOUND, m_allInboundButton);
    DDX_Control(pDX, IDC_BUTTONCLEARALL, m_clearTrainSelectionButton);
    DDX_Control(pDX, IDC_BUTTONALLTRAINS, m_selectAllTrainsButton);
    DDX_Control(pDX, IDC_DYNAMIC_GROUPS, m_dynamicGroupCombo);
    DDX_Control(pDX, IDC_STATIC_GROUPS, m_staticGroupCombo);
    DDX_Control(pDX, IDC_BUTTON_SAVE_GROUP_SEL, m_saveDynamicGroupButton);
    DDX_Control(pDX, IDC_BUTTON_DELETE_GROUP, m_deleteDynamicGroupButton);
    DDX_Control(pDX, IDC_TRN_CYCLICAL_STOP, m_period);
    DDX_Control(pDX, IDC_TRN_CYCLICAL_START, m_duration);
    DDX_Control(pDX, IDC_TRN_DVA_MESSAGES_LST, m_dvaMessageList);
    DDX_Control(pDX, IDC_TRAINLIST_TAB, m_locationTab);
    DDX_Control(pDX, IDC_TRN_MESSAGE_FILTER_TB, m_messageFilterTb);
    DDX_Control(pDX, IDC_TRN_LIVE_BTN, m_liveBtn);
    DDX_Control(pDX, IDC_TRN_DVA_BTN, m_dvaBtn);
    DDX_Control(pDX, IDC_TRN_CYCLIC_CB, m_cyclicCb);
    DDX_Control(pDX, IDC_INBOUND_LABEL, m_inboundLabel);
    DDX_Control(pDX, IDC_OUTBOUND_LABEL, m_outboundLabel);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_STN_OUTBOUND_SELECTION_LST, getOutboundTrainList());
    DDX_Control(pDX, IDC_STN_INBOUND_SELECTION_LST, getInboundTrainList());

    FUNCTION_EXIT;
}

BEGIN_MESSAGE_MAP(TrainBroadcastPage, CPropertyPage)
    //{{AFX_MSG_MAP(TrainBroadcastPage)
    ON_BN_CLICKED(IDC_TRN_CYCLIC_CB, OnTrnCyclicCb)
    ON_BN_CLICKED(IDC_TRN_DVA_BTN, OnTrnDvaBtn)
    ON_BN_CLICKED(IDC_TRN_LIVE_BTN, OnTrnLiveBtn)
    ON_MESSAGE(WM_PA_TRAIN_INBOUND_LIST_UPDATE, OnInboundTrainListUpdate)
    ON_MESSAGE(WM_PA_TRAIN_OUTBOUND_LIST_UPDATE, OnOutboundTrainListUpdate)
    ON_MESSAGE(WM_PA_TRAIN_DYN_LIST_UPDATE, OnDynamicGroupListUpdate)
    ON_MESSAGE(WM_PA_TRAIN_DVA_MESSAGE_UPDATE, OnTrainDvaMessageUpdate)
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_BUTTON_DELETE_GROUP, OnButtonDeleteGroup)
    ON_BN_CLICKED(IDC_BUTTON_SAVE_GROUP_SEL, OnButtonSaveGroupSel)
    ON_BN_CLICKED(IDC_BUTTONALLTRAINS, OnButtonalltrains)
    ON_BN_CLICKED(IDC_BUTTONCLEARALL, OnButtonclearall)
    ON_NOTIFY(TCN_SELCHANGING, IDC_TRAINLIST_TAB, OnSelchangingTrainlistTab)
    ON_EN_CHANGE(IDC_TRN_MESSAGE_FILTER_TB, OnChangeTrnMessageFilterTb)
    ON_NOTIFY(NM_CLICK, IDC_STN_INBOUND_SELECTION_LST, OnClickStnInboundSelectionLst)
    ON_NOTIFY(NM_CLICK, IDC_STN_OUTBOUND_SELECTION_LST, OnClickStnOutboundSelectionLst)
    ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TRN_CYCLICAL_START, OnDurationChange)
    ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TRN_CYCLICAL_STOP, OnPeriodChange)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_TRN_DVA_MESSAGES_LST, OnItemchangedTrnDvaMessagesLst)
    ON_BN_CLICKED(IDC_BUTTONALLOUTBOUND, OnButtonalloutbound)
    ON_BN_CLICKED(IDC_BUTTONALLINBOUND, OnButtonallinbound)
    ON_MESSAGE(WM_PA_RIGHTS_CHANGE, OnRightsChanged)
    // jeffrey++ TD7956
    ON_BN_CLICKED(IDC_RADIO_OVERRIDE, OnRadioOverride)
    ON_BN_CLICKED(IDC_RADIO_NORMAL, OnRadioNormal)
    // ++jeffrey TD7956
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TrainBroadcastPage message handlers

void TrainBroadcastPage::OnTrnLiveBtn()
{
    FUNCTION_ENTRY("OnTrnLiveBtn");

    // TD16862
    OnRadioOverride();
    // TD16862
    // Force always checked
    setSelectedBroadcastButton(&m_liveBtn);

    // enable GOA group selection
    m_trainSelectorGUI->enableRadioGroupCombo(true);

    FUNCTION_EXIT;
}

void TrainBroadcastPage::setSelectedBroadcastButton(CButton* selection)
{
    FUNCTION_ENTRY("setSelectedBroadcastButton");

    if (0 != selection)
    {
        selection->SetCheck(BST_CHECKED);
    }

    if ((&m_liveBtn) != selection)
    {
        m_liveBtn.SetCheck(BST_UNCHECKED);
    }

    if ((&m_dvaBtn) != selection)
    {
        m_dvaBtn.SetCheck(BST_UNCHECKED);
    }

    EnableDvaArea(m_dvaBtn.GetCheck() != 0 ? true : false);

    updateExecuteButtonEnabledState();

    FUNCTION_EXIT;
}

void TrainBroadcastPage::OnTrnDvaBtn()
{
    FUNCTION_ENTRY("OnTrnDvaBtn");

    // TD16862
    OnRadioNormal();
    // TD16862
    setSelectedBroadcastButton(&m_dvaBtn);

    // disable GOA group selection
    m_trainSelectorGUI->enableRadioGroupCombo(false);

    FUNCTION_EXIT;
}

void TrainBroadcastPage::OnTrnCyclicCb()
{
    FUNCTION_ENTRY("OnTrnCyclicCb");

    EnableCyclicArea(m_cyclicCb.GetCheck() != 0 ? true : false);

    FUNCTION_EXIT;
}

void TrainBroadcastPage::EnableDvaArea(bool isEnabled)
{
    FUNCTION_ENTRY("EnableDvaArea");

    EnableCyclicArea(isEnabled);

    m_dvaMessageList.EnableWindow(isEnabled);
    m_messageFilterTb.EnableWindow(isEnabled);
    m_cyclicCb.EnableWindow(isEnabled);

    m_cyclicCb.EnableWindow(isEnabled);

    if (isEnabled == false)
    {
        EnableCyclicArea(false);
    }
    else
    {
        EnableCyclicArea(m_cyclicCb.GetCheck() != 0 ? true : false);
    }

    FUNCTION_EXIT;
}

void TrainBroadcastPage::EnableCyclicArea(bool isEnabled)
{
    FUNCTION_ENTRY("EnableCyclicArea");

    m_duration.EnableWindow(isEnabled);
    m_period.EnableWindow(isEnabled);

    m_trainBroadcastModel->setCyclicalEnabled(isEnabled);

    FUNCTION_EXIT;
}

void TrainBroadcastPage::postDialogCreate()
{
    LOG_SCOPE("TrainBroadcastPage::postDialogCreate");
    FUNCTION_ENTRY("postDialogCreate");

    initialiseButtonIcons();

    /////////////////////////
    // Setup Prerecorded List
    /////////////////////////
    formatTrainDvaMessageSelectionList(m_dvaMessageList);

    m_dvaFixedHeaderCtrl.subclassHeader(m_dvaMessageList.GetHeaderCtrl());

    // Adjust dimensions of list controls
    static const DWORD LOCN_WIDTH_PIXELS = 60;
    static const DWORD TRAIN_ID_WIDTH_PIXELS = 120;

    // Account for the scroll bar
    // jeffrey++ TD8850
    //int headerTotalWidth = PaLayoutHelper::getListHeaderWidth(getInboundTrainList(), true);
    int headerTotalWidth = PaLayoutHelper::getListHeaderWidth(getInboundTrainList(), false);
    // ++jeffrey TD8850
    getInboundTrainList().setStandardColumnSizes(LOCN_WIDTH_PIXELS,
                                                 TRAIN_ID_WIDTH_PIXELS,
                                                 headerTotalWidth - LOCN_WIDTH_PIXELS - TRAIN_ID_WIDTH_PIXELS);

    // jeffrey++ TD8850
    //headerTotalWidth = PaLayoutHelper::getListHeaderWidth(getOutboundTrainList(), true);
    headerTotalWidth = PaLayoutHelper::getListHeaderWidth(getOutboundTrainList(), false);
    // ++jeffrey TD8850
    getOutboundTrainList().setStandardColumnSizes(LOCN_WIDTH_PIXELS,
                                                  TRAIN_ID_WIDTH_PIXELS,
                                                  headerTotalWidth - LOCN_WIDTH_PIXELS - TRAIN_ID_WIDTH_PIXELS);

    // This will force initial refresh of train messages..
    // Now that list is set up
    OnChangeTrnMessageFilterTb();

    /////////////////////////
    // Setup Train Lists
    /////////////////////////
    // Only initialise the update listener once the window has been initialised
    m_trainBroadcastModel->enableUpdateListener(true);

    // Apply train selector to shared train selection component
    m_trainSelectorGUI->setTrainSelector(m_trainBroadcastModel->getTrainSelector());

    FUNCTION_EXIT;
}

BOOL TrainBroadcastPage::OnInitDialog()
{
    FUNCTION_ENTRY("OnInitDialog");

    m_rightsChangeHandler =  new RightsChangedWindowNotificationHandler(m_hWnd);

    CPropertyPage::OnInitDialog();

    // Create a handler so we can receive zone change notifications from config editor
    m_trainDvaUpdateNotificationHandler =
        new CachedMapChangedWindowNotificationHandler(
            m_hWnd,
            WM_PA_TRAIN_DVA_MESSAGE_UPDATE,
            TA_IRS_Bus::CachedMaps::getInstance()->getPaTrainDvaMessageCachedMap());

        // Only want to receive a single notification no matter how many
        // changes are buffered up
    m_trainDvaUpdateNotificationHandler->setMustAcknowledgeBeforeResendFlag(true);

    initialiseTrainListTabSelector();

    /////////////////////////
    // Setup Cyclic Broadcast
    /////////////////////////
    setupCyclicalBroadcastArea();

    OnTrnCyclicCb();

    refreshBroadcastButtonEnabledState();
    // Enter some suitable default broadcast mode
    enterDefaultBroadcastMode();

    // Refresh dynamic group combo contents
    OnDynamicGroupListUpdate(0, 0);

    updateExecuteButtonEnabledState();

    m_messageFilterTb.SetLimitText(MAX_FILTER_TEXT);

    // jeffrey++ TD7956
    OnRadioNormal();
    // ++jeffrey TD7956

    // TD 15349
    std::string selectedTrainMessage = m_paManagerDlg->getSelectedTrainMessage();

    if (selectedTrainMessage != "" && m_dvaBtn.GetCheck() == BST_CHECKED)
    {
        selectMessageNameInList(selectedTrainMessage);
    }

    // TD 15349
    FUNCTION_EXIT;
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void TrainBroadcastPage::setupCyclicalBroadcastArea()
{
    FUNCTION_ENTRY("setupCyclicalBroadcastArea");

    // set the time format
    m_duration.SetFormat("    HH:mm:ss");
    m_period.SetFormat("    HH:mm:ss");

    // set the maximum time - the time can be 0 to 65536 seconds
    CTime lowerTime(1971, 1, 1, 0, 0, 1, 0);
    unsigned short minutes = MAX_TIME_INTERVAL / 60;
    unsigned short hours = minutes / 60;
    minutes -= (hours * 60);
    unsigned short seconds = MAX_TIME_INTERVAL - (hours * 60 * 60) - (minutes * 60);
    CTime upperTime(1971, 1, 1, hours, minutes, seconds, 0);
    m_duration.SetRange(&lowerTime, &upperTime);
    m_period.SetRange(&lowerTime, &upperTime);

    // put a couple of values in for defaults
    CTime defaultPeriod(1971, 1, 1, 0, 1, 0, 0);
    CTime defaultDuration(1971, 1, 1, 0, 5, 0, 0);
    m_period.SetTime(&defaultPeriod);
    m_duration.SetTime(&defaultDuration);

    // Perform initial refresh taking value from controls
    processStartStopTimeChange();

    FUNCTION_EXIT;
}

void TrainBroadcastPage::initialiseButtonIcons()
{
    FUNCTION_ENTRY("initialiseButtonIcons");

    HINSTANCE hInstance = AfxGetInstanceHandle();
    CRect tempRect;

    /////////////////////////
    // Load PA Mode icons
    /////////////////////////

    m_liveBtn.GetWindowRect(tempRect);
    m_hLiveIcon = LoadImage(hInstance,
                            MAKEINTRESOURCE(IDI_LIVE_ICON),
                            IMAGE_ICON,
                            tempRect.Width(),
                            tempRect.Height(),
                            LR_DEFAULTCOLOR);
    m_liveBtn.SetIcon((HICON)m_hLiveIcon);

    m_dvaBtn.GetWindowRect(tempRect);
    m_hDvaIcon = LoadImage(hInstance,
                           MAKEINTRESOURCE(IDI_DVA_ICON),
                           IMAGE_ICON,
                           tempRect.Width(),
                           tempRect.Height(),
                           LR_DEFAULTCOLOR);
    m_dvaBtn.SetIcon((HICON)m_hDvaIcon);

    FUNCTION_EXIT;
}

afx_msg LRESULT TrainBroadcastPage::OnInboundTrainListUpdate(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnInboundTrainListUpdate");

    TA_ASSERT(m_trainBroadcastModel != NULL, "m_trainBroadcastModel NULL");

    try
    {
        // Data waiting at the model, collect it for processing
        TrainListUpdateContainer::BufferedItem data =
            m_trainBroadcastModel->fetchUpdatedInboundTrainData();

        getTrainSelectorGUI().updateInboundList(data.action, data.trainDetails);

        // AZ++ TES 10924
        if (m_trainDvaVersionsPage != NULL)
        {
            m_trainDvaVersionsPage->updateTrainList(data.action, data.trainDetails);
        }

        // ++AZ
    }

    STANDARD_PA_CATCH("TrainBroadcastPage::OnInboundTrainListUpdate", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

afx_msg LRESULT TrainBroadcastPage::OnOutboundTrainListUpdate(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnOutboundTrainListUpdate");

    TA_ASSERT(m_trainBroadcastModel != NULL, "m_trainBroadcastModel NULL");

    try
    {
        // Data waiting at the model, collect it for processing
        TrainListUpdateContainer::BufferedItem data =
            m_trainBroadcastModel->fetchUpdatedOutboundTrainData();

        getTrainSelectorGUI().updateOutboundList(data.action, data.trainDetails);

        // AZ++ TES 10924
        if (m_trainDvaVersionsPage != NULL)
        {
            m_trainDvaVersionsPage->updateTrainList(data.action, data.trainDetails);
        }

        // ++AZ
    }

    STANDARD_PA_CATCH("TrainBroadcastPage::OnOutboundTrainListUpdate", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

afx_msg LRESULT TrainBroadcastPage::OnTrainDvaMessageUpdate(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnTrainDvaMessageUpdate");

    if (0 == m_trainDvaUpdateNotificationHandler)
    {
        FUNCTION_EXIT;
        return 0;
    }

    try
    {
        // Acknowledge the message prior to update (if a whole bunch of updates
        // are received, we will only get the one notification - to avoid
        // excessive waiting times calling OnChangeTrnMessageFilterTb() many times)
        m_trainDvaUpdateNotificationHandler->acknowledgeMessage();

        DWORD changedItemKey = wParam;

        // Calling this next function will refresh list fully (and deselect
        // any existing selections)
        OnChangeTrnMessageFilterTb();

        // Train messages are not local to a station - indicate to user there
        // has been an update
        PaErrorHandler::displayError(PaErrorHandler::DVA_MESSAGE_UPDATE_RECEIVED_EXTERNALLY);
    }

    STANDARD_PA_CATCH("TrainBroadcastPage::OnTrainDvaMessageUpdate", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

afx_msg LRESULT TrainBroadcastPage::OnDynamicGroupListUpdate(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnDynamicGroupListUpdate");

    TA_ASSERT(m_trainBroadcastModel != NULL, "m_trainBroadcastModel NULL");

    try
    {
        TA_IRS_Bus::CommonTypes::TrainIdList previousSelection = getTrainSelectorGUI().getSelectedTrains();

        getTrainSelectorGUI().updateSelectionCombo(m_trainBroadcastModel->fetchDynamicGroupList());

        // TD #3315 - if an external source causes the current selection to change in any way
        // (whether a group was deleted, or selection updated for currently selected group)
        // we must inform the user (not safe to silently update selection without them knowing it)
        TA_IRS_Bus::CommonTypes::TrainIdList newSelection = getTrainSelectorGUI().getSelectedTrains();

        if (newSelection != previousSelection)
        {
            PaErrorHandler::displayError(PaErrorHandler::EXTERNAL_TRAIN_DYNAMIC_GROUP_UPDATE_RECEIVED);
        }
    }

    STANDARD_PA_CATCH("TrainBroadcastPage::OnDynamicGroupListUpdate", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

void TrainBroadcastPage::OnDestroy()
{
    FUNCTION_ENTRY("OnDestroy");

    if (m_trainSelectorGUI != NULL)
    {
        delete m_trainSelectorGUI;
        m_trainSelectorGUI = NULL;
    }

    // Delete the rights change notification object before window handle becomes invalid
    delete m_rightsChangeHandler;
    m_rightsChangeHandler = 0;

    // Cannot delete update notification objects after window made
    // invalid, so must do here
    delete m_trainDvaUpdateNotificationHandler;
    m_trainDvaUpdateNotificationHandler = 0;

    TA_ASSERT(m_hWnd != 0, "Internal check failed");

    // Shut down the listener before we destroy this window (otherwise
    // it will attempt to post to non-existent window, and assert)
    m_trainBroadcastModel->enableUpdateListener(false);

    // Delete train broadcast model before window destroyed
    delete m_trainBroadcastModel;
    m_trainBroadcastModel = 0;

    delete m_outboundTrainList;
    m_outboundTrainList = 0;

    delete m_inboundTrainList;
    m_inboundTrainList = 0;

    CPropertyPage::OnDestroy();

    FUNCTION_EXIT;
}

void TrainBroadcastPage::formatTrainDvaMessageSelectionList(CListCtrl& listToUpdate)
{
    FUNCTION_ENTRY("formatTrainDvaMessageSelectionList");

    // ++AZ TES 9613, tooltips set in ListCtrlSelNoFocus
    listToUpdate.SetExtendedStyle(LVS_EX_FULLROWSELECT);
    //listToUpdate.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
    // AZ++

    static const DWORD MESSAGE_ID_PIXELS = 50;

    // jeffrey++ TD8850
    //bool accountForScroll = true;
    bool accountForScroll = false;
    // ++jeffrey TD8850

    int headerTotalWidth = PaLayoutHelper::getListHeaderWidth(listToUpdate, accountForScroll);

    listToUpdate.InsertColumn(0, "Message", LVCFMT_LEFT,
                              headerTotalWidth);
    /*
    listToUpdate.InsertColumn(1, "ID", LVCFMT_LEFT,
                                    MESSAGE_ID_PIXELS);
    */

    FUNCTION_EXIT;
}

void TrainBroadcastPage::OnSelchangingTrainlistTab(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnSelchangingTrainlistTab");

    if (!refreshTrainListState())
    {
        // Failed to switch modes - don't allow tab to change
        *pResult = TRUE;
    }
    else
    {
        // Success
        *pResult = 0;

        LockControlRedraw lock(*this);

        // Remember TAB current selection hasn't changed yet
        if (m_locationTab.GetCurSel() == 1)       // Switching from Depot (1) mode
        {
            // set the GUI to mainline mode
            getTrainSelectorGUI().setMainlineMode();
        }
        else if (m_locationTab.GetCurSel() == 0)      // Switching from Mainline(0) mode
        {
            getTrainSelectorGUI().setDepotMode();
        }
    }

    FUNCTION_EXIT;
}

TA_IRS_Bus::LocationCache::EStationType TrainBroadcastPage::getAppropriateInServiceMode() const
{
    FUNCTION_ENTRY("getAppropriateInServiceMode");

    if (CachedConfig::getInstance()->getLocationType() ==
        CachedConfig::LOCATION_TYPE_OCC)
    {
        // We're at OCC, display all in service trains on circle line
        FUNCTION_EXIT;
        return TA_IRS_Bus::LocationCache::InService;
    }
    else
    {
        // Stations aren't allowed to view all in service, only local trains
        FUNCTION_EXIT;
        return TA_IRS_Bus::LocationCache::Station;
    }
}

bool TrainBroadcastPage::switchToTrainDisplayMode(TA_IRS_Bus::LocationCache::EStationType mode)
{
    FUNCTION_ENTRY("switchToTrainDisplayMode");

    try
    {
        CWaitCursor hourGlassCursor;

        m_trainBroadcastModel->switchTrainListingMode(mode);

        FUNCTION_EXIT;
        return true;
    }
    catch (const TA_Base_Core::PasConnectionException&)
    {
        // Not connected
        PaErrorHandler::displayError(PaErrorHandler::ENTITY_NAME_NOT_FOUND);

        FUNCTION_EXIT;
        return false;
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // CORBA error
        PaErrorHandler::displayError(PaErrorHandler::ENTITY_NAME_NOT_FOUND);

        FUNCTION_EXIT;
        return false;
    }
    catch (const CORBA::Exception&)
    {
        // Comms error
        TA_THROW(TA_Base_Core::PasConnectionException("COMMS ERROR"));

        FUNCTION_EXIT;
        return false;
    }
}

void TrainBroadcastPage::updateExecuteButtonEnabledState()
{
    FUNCTION_ENTRY("updateExecuteButtonEnabledState");

    bool enable = false;

    BOOL anyButtonPressed = m_dvaBtn.GetCheck() ||
        m_liveBtn.GetCheck();

// Need to update this in the following cases:
//- Selection change of trains (selection cleared / modified)
//- Change of message selection (when a message initially selected)

// At least one button must be pressed to enable "Execute" button
    if (anyButtonPressed)
    {
        // If there are no trains selected, cannot enable
        if (m_anyTrainsSelected)
        {
            if (m_dvaBtn.GetCheck())
            {
                // If DVA, must select a message to go with it
                if (TA_Base_Core::DatabaseKey::isValidKey(getSelectedDvaMessageKey()))
                {
                    enable = true;
                    m_trainBroadcastModel->setBroadcastType(TA_Base_Bus::IPAAgentCorbaDef::TrainDva);
                }
            }
            else if (m_liveBtn.GetCheck())
            {
                enable = true;
                m_trainBroadcastModel->setBroadcastType(TA_Base_Bus::IPAAgentCorbaDef::TrainLive);
            }
        }
    }

    if (!enable)
    {
        m_trainBroadcastModel->unsetBroadcastType();
    }

    if (m_pageActive)
    {
        m_paManagerDlg->enableExecuteButton(enable);
    }

    FUNCTION_EXIT;
}

void TrainBroadcastPage::executeBroadcast()
{
    FUNCTION_ENTRY("executeBroadcast");

    CWaitCursor hourGlassCursor;

    TA_ASSERT(m_trainBroadcastModel != NULL, "m_trainBroadcastModel NULL");

    // Collect the train coverage data just before execution
    std::vector<unsigned int> selTrains = getSelectedTrains();
    TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage trainCoverage;
    trainCoverage.length(selTrains.size());

    // Transfer selected trains into appropriate structure
    for (unsigned int i = 0; i < selTrains.size(); i++)
    {
        trainCoverage[i] = selTrains[i];
    }

    m_trainBroadcastModel->setSelectedStaticGroup(m_staticGroupCombo.getCurrentGroupName());

    m_trainBroadcastModel->executeBroadcast(trainCoverage);

    FUNCTION_EXIT;
}

void TrainBroadcastPage::setTimeStartDtp(const CTime& time)
{
    FUNCTION_ENTRY("setTimeStartDtp");

    m_duration.SetTime(&time);

    FUNCTION_EXIT;
}

void TrainBroadcastPage::setTimeStopDtp(const CTime& time)
{
    FUNCTION_ENTRY("setTimeStopDtp");

    m_period.SetTime(&time);

    FUNCTION_EXIT;
}

CTime TrainBroadcastPage::getTimeStartDtp() const
{
    FUNCTION_ENTRY("getTimeStartDtp");

    time_t seconds = PaLayoutHelper::getSecondsFromControls(0,
                                                            &m_duration,
                                                            &m_duration,
                                                            &m_duration);

    FUNCTION_EXIT;
    return CTime(seconds);
}

CTime TrainBroadcastPage::getTimeStopDtp() const
{
    FUNCTION_ENTRY("getTimeStopDtp");

    time_t seconds = PaLayoutHelper::getSecondsFromControls(0,
                                                            &m_period,
                                                            &m_period,
                                                            &m_period);

    FUNCTION_EXIT;
    return CTime(seconds);
}

void TrainBroadcastPage::processStartStopTimeChange()
{
    FUNCTION_ENTRY("processPeriodDurationChange");

    CTime startTime;
    CTime stopTime;

    m_duration.GetTime(startTime);
    m_period.GetTime(stopTime);

    int actualStartTime = (startTime.GetHour() * 3600) + (startTime.GetMinute() * 60) + startTime.GetSecond();
    int actualEndTime = (stopTime.GetHour() * 3600) + (stopTime.GetMinute() * 60) + stopTime.GetSecond();

    m_trainBroadcastModel->setDuration(actualStartTime);
    m_trainBroadcastModel->setPeriod(actualEndTime);

    FUNCTION_EXIT;
}

void TrainBroadcastPage::OnDurationChange(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnDurationChange");

    processStartStopTimeChange();

    *pResult = 0;

    FUNCTION_EXIT;
}

void TrainBroadcastPage::OnPeriodChange(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnPeriodChange");

    processStartStopTimeChange();

    *pResult = 0;

    FUNCTION_EXIT;
}

void TrainBroadcastPage::trainSelectionChanged(bool trainsAreSelected)
{
    FUNCTION_ENTRY("trainSelectionChanged");

    // User changed current selection of trains
    if (trainsAreSelected != m_anyTrainsSelected)
    {
        m_anyTrainsSelected = trainsAreSelected;

        // Must update various items whenever there is a state change in current selection
        updateExecuteButtonEnabledState();
    }

    FUNCTION_EXIT;
}

void TrainBroadcastPage::OnButtonDeleteGroup()
{
    FUNCTION_ENTRY("OnButtonDeleteGroup");

    getTrainSelectorGUI().deleteTrainSelection();

    FUNCTION_EXIT;
}

void TrainBroadcastPage::OnButtonSaveGroupSel()
{
    FUNCTION_ENTRY("OnButtonSaveGroupSel");

    getTrainSelectorGUI().saveTrainSelection();

    FUNCTION_EXIT;
}

std::vector<unsigned int> TrainBroadcastPage::getSelectedTrains()
{
    FUNCTION_ENTRY("getSelectedTrains");

    TA_ASSERT(m_trainSelectorGUI != NULL, "Train selection component is NULL");
    std::vector<TA_IRS_Bus::TrainInformationTypes::TrainDetails> selectedTrains = m_trainSelectorGUI->getSelectedTrainData();

    // Also create a vector of strings of the trains that are already in the dymaic group (can't be in the group
    // more than once).
    std::vector<unsigned int> trains;

    for (std::vector<TA_IRS_Bus::TrainInformationTypes::TrainDetails>::iterator iter = selectedTrains.begin();
         iter != selectedTrains.end(); iter++)
    {
        // Work out the ID of this train
        trains.push_back(iter->trainId);
        // TD16865
        unsigned int trainId = iter->trainId;
        std::string serviceNumber = iter->serviceNumber;
        m_trainIdToServiceNumber[trainId] = serviceNumber;
        // TD16865
    }

    FUNCTION_EXIT;
    return trains;
}

void TrainBroadcastPage::OnButtonalloutbound()
{
    FUNCTION_ENTRY("OnButtonalloutbound");

    getTrainSelectorGUI().selectAllOutbound();

    FUNCTION_EXIT;
}

void TrainBroadcastPage::OnButtonallinbound()
{
    FUNCTION_ENTRY("OnButtonallinbound");

    getTrainSelectorGUI().selectAllInbound();

    FUNCTION_EXIT;
}

void TrainBroadcastPage::OnButtonalltrains()
{
    FUNCTION_ENTRY("OnButtonalltrains");

    getTrainSelectorGUI().selectAllTrains();

    FUNCTION_EXIT;
}

void TrainBroadcastPage::OnButtonclearall()
{
    FUNCTION_ENTRY("OnButtonclearall");

    getTrainSelectorGUI().clearSelection();

    FUNCTION_EXIT;
}

void TrainBroadcastPage::OnChangeTrnMessageFilterTb()
{
    FUNCTION_ENTRY("OnChangeTrnMessageFilterTb");

    std::string filter = PaLayoutHelper::fetchStringFromControl(m_messageFilterTb);

    TA_IRS_Bus::TrainDvaMessageRecords trainMessages = m_trainBroadcastModel->getMessages(filter);

    updateDvaMessageList(trainMessages);

    FUNCTION_EXIT;
}

void TrainBroadcastPage::updateDvaMessageList(const TA_IRS_Bus::TrainDvaMessageRecords& messages)
{
    FUNCTION_ENTRY("updateDvaMessageList");

    LockControlRedraw redrawLock(m_dvaMessageList);

    // Refresh message list with input items
    m_dvaMessageList.DeleteAllItems();

    for (TA_IRS_Bus::TrainDvaMessageRecords::const_iterator it = messages.begin();
         it != messages.end();
         it++)
    {
        int nInsertionPoint = m_dvaMessageList.GetItemCount();
        std::string messageLabel = std::string((*it).m_label);

        // Keep adding items to end of CListCtrl (to maintain
        // a natural ordering)
        int nItem = m_dvaMessageList.InsertItem(nInsertionPoint, messageLabel.c_str());

        /*
        CString identifierString;
        identifierString.Format(_T("%d"), (*it).m_pKeyOrId);
        m_dvaMessageList.SetItemText(nItem, TDM_COL, identifierString);
        */

        m_dvaMessageList.SetItemData(nItem, (*it).m_pKeyOrId);
    }

    FUNCTION_EXIT;
}

void TrainBroadcastPage::OnClickStnInboundSelectionLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnClickStnInboundSelectionLst");

    LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pNMHDR;

    // If user clicks on inbound list without holding ctrl down, clear outbound list selection
    if (!(lpnmitem->uKeyFlags & LVKF_CONTROL))
    {
        getOutboundTrainList().deselectAll();
    }

    m_dynamicGroupCombo.SetCurSel(-1);                //TD17987
    m_dynamicGroupCombo.SetWindowText("");            //TD17987

    *pResult = 0;

    FUNCTION_EXIT;
}

void TrainBroadcastPage::OnClickStnOutboundSelectionLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnClickStnOutboundSelectionLst");

    LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)pNMHDR;

    // If user clicks on outbound list without holding ctrl down, clear inbound list selection
    if (!(lpnmitem->uKeyFlags & LVKF_CONTROL))
    {
        getInboundTrainList().deselectAll();
    }

    m_dynamicGroupCombo.SetCurSel(-1);                //TD17987
    m_dynamicGroupCombo.SetWindowText("");            //TD17987

    *pResult = 0;

    FUNCTION_EXIT;
}

BOOL TrainBroadcastPage::OnKillActive()
{
    FUNCTION_ENTRY("OnKillActive");

    m_pageActive = false;

    // Disable execute button on the main dialog when lose focus
    m_paManagerDlg->enableExecuteButton(false);

    FUNCTION_EXIT;
    return CPropertyPage::OnKillActive();
}

BOOL TrainBroadcastPage::OnSetActive()
{
    FUNCTION_ENTRY("OnSetActive");

    m_pageActive = true;

    CWaitCursor hourGlassCursor;

    // Every time page re-activated, attempt a reconnection if required
    if (!m_trainBroadcastModel->isTrainSubscriberConnected())
    {
        m_trainBroadcastModel->attemptTrainSubscriberConnection();
    }

    // Should call this after activation (CPropertyPage::OnSetActive)
    // otherwise won't correctly update execute button state on parent page
    updateExecuteButtonEnabledState();

    FUNCTION_EXIT;
    return CPropertyPage::OnSetActive();
}

void TrainBroadcastPage::displayDvaStatusDialog(const std::string& broadcastId)
{
    FUNCTION_ENTRY("displayDvaStatusDialog");

    TA_ASSERT(m_trainBroadcastModel->testCanObtainLock(), "This thread shouldn't own model lock");

    // jeffrey++ TD7956
    //TrainDvaStatusDlg trainDvaStatusDialog(broadcastId.c_str(), this);
    // TrainDvaStatusDlg trainDvaStatusDialog(broadcastId.c_str(), m_trainBroadcastModel->getHasOverrideOption(), this);
    // ++jeffrey TD7956

    // TD16865
    TrainDvaStatusDlg trainDvaStatusDialog(broadcastId.c_str(), m_trainBroadcastModel->getHasOverrideOption(), m_trainIdToServiceNumber, this);
    // TD16865

    trainDvaStatusDialog.DoModal();

    FUNCTION_EXIT;
}

void TrainBroadcastPage::displayLiveStatusDialog(const std::string& broadcastId)
{
    FUNCTION_ENTRY("displayLiveStatusDialog");

    TA_ASSERT(m_trainBroadcastModel->testCanObtainLock(), "This thread shouldn't own model lock");

    // jeffrey++ TD7956
    //TrainLiveStatusDlg trainLiveStatusDialog(broadcastId.c_str(), this);
    // TrainLiveStatusDlg trainLiveStatusDialog(broadcastId.c_str(), m_trainBroadcastModel->getHasOverrideOption(), this);
    // ++jeffrey TD7956

    // TD16865
    TrainLiveStatusDlg trainLiveStatusDialog(broadcastId.c_str(), m_trainBroadcastModel->getHasOverrideOption(), m_trainIdToServiceNumber, this);
    // TD16865

    trainLiveStatusDialog.DoModal();

    FUNCTION_EXIT;
}

void TrainBroadcastPage::OnItemchangedTrnDvaMessagesLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnItemchangedTrnDvaMessagesLst");

    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    // Only interested in state changes
    if (pNMListView->uChanged != LVIF_STATE)
    {
        FUNCTION_EXIT;
        return;
    }

    OnMessageSelectionChanged();

    *pResult = 0;

    FUNCTION_EXIT;
}

int TrainBroadcastPage::getSelectedDvaMessageKey()
{
    FUNCTION_ENTRY("getSelectedDvaMessageKey");

    std::vector<unsigned int> selection =
        TA_Base_Bus::SelectionHelper::getSelectedItemIndices(m_dvaMessageList);

    TA_ASSERT(selection.size() <= 1, "Multiple selections not allowed");

    if (0 != selection.size())
    {
        FUNCTION_EXIT;
        return m_dvaMessageList.GetItemData(selection[0]);
    }

    FUNCTION_EXIT;
    return TA_Base_Core::DatabaseKey::getInvalidKey();
}

void TrainBroadcastPage::OnMessageSelectionChanged()
{
    FUNCTION_ENTRY("OnMessageSelectionChanged");

    m_trainBroadcastModel->setSelectedMessage(getSelectedDvaMessageKey());

    updateExecuteButtonEnabledState();

    FUNCTION_EXIT;
}

void TrainBroadcastPage::refreshBroadcastButtonEnabledState()
{
    FUNCTION_ENTRY("refreshBroadcastButtonEnabledState");

    BOOL enableBroadcastLive = PaRightsManager::getInstance().
        hasRightToBroadcastTrainLive();

    BOOL enableBroadcastDva = PaRightsManager::getInstance().hasRight(
        PaRightsManager::TRAIN_PA_PRERECORDED_ANNOUNCEMENT);

    m_liveBtn.EnableWindow(enableBroadcastLive);
    m_dvaBtn.EnableWindow(enableBroadcastDva);

    FUNCTION_EXIT;
}

void TrainBroadcastPage::enterDefaultBroadcastMode()
{
    FUNCTION_ENTRY("enterDefaultBroadcastMode");

    if (m_dvaBtn.IsWindowEnabled())
    {
        // Attempt to default to DVA Button pressed, by default
        OnTrnDvaBtn();
    }
    else if (m_liveBtn.IsWindowEnabled())
    {
        OnTrnLiveBtn();
    }
    else
    {
        // No selection possible..
        setSelectedBroadcastButton(0);
    }

    FUNCTION_EXIT;
}

bool TrainBroadcastPage::refreshTrainListState()
{
    FUNCTION_ENTRY("refreshTrainListState");

    TA_IRS_Bus::LocationCache::EStationType newMode;

    // There should be a better way of identifying which tab we're changing
    // the selection to - but while we only have two tabs total, it will be
    // sufficient to use the current tab selection to find this info out
    if (m_locationTab.GetCurSel() == 1)
    {
        // Currently on Depot tab - can only possibly be switching to In Service tab
        newMode = getAppropriateInServiceMode();
    }
    else if (m_locationTab.GetCurSel() == 0)
    {
        // Current on In Service tab, can only possible be switching to Depot tab
        newMode = TA_IRS_Bus::LocationCache::Depot;
    }

    bool result = switchToTrainDisplayMode(newMode);

    if (result)
    {
        // Switch went OK
        // Clear all current train selections when switching tabs
        OnButtonclearall();
    }

    FUNCTION_EXIT;
    return result;
}

void TrainBroadcastPage::initialiseTrainListTabSelector()
{
    FUNCTION_ENTRY("initialiseTrainListTabSelector");

    // this does the GUI logic
    m_trainSelectorGUI = new TA_IRS_Bus::TrainSelectorGUI(getInboundTrainList(),  // pass all controls
                                                          &getOutboundTrainList(),
                                                          &m_allInboundButton,
                                                          &m_allOutboundButton,
                                                          &m_selectAllTrainsButton,
                                                          &m_clearTrainSelectionButton,
                                                          &m_inboundLabel,
                                                          &m_outboundLabel,
                                                          &m_dynamicGroupCombo,
                                                          &m_saveDynamicGroupButton,
                                                          &m_deleteDynamicGroupButton,
                                                          &m_staticGroupCombo,
                                                          this);  // this listens for train selection changes

   // for ease of debugging - have it so you can enable all train selection tabs
    if (!TA_Base_Core::RunParams::getInstance().isSet("EnableAllTabs"))
    {
        if (m_trainBroadcastModel->getCurrentListingMode() != TA_IRS_Bus::LocationCache::InService)
        {
            // if this is at a station, disable the depot tab
            // if at the depot, disable the mainline tab
            // if this is the occ, dont disable any tabs
            if (m_trainBroadcastModel->getCurrentListingMode() == TA_IRS_Bus::LocationCache::Depot)
            {
                m_locationTab.SetActiveTab(1);
                getTrainSelectorGUI().setDepotMode();
                m_locationTab.EnableTabIndex(0, false);

                // the train selection component should already be in depot mode
            }
            else
            {
                // already on mainline tab
                m_locationTab.EnableTabIndex(1, false);
            }
        }
        else if (CachedConfig::getInstance()->isAtOcc())
        {
            // TES #628: Depot tab only available at depot (since a change in requirements, there
            // is never a situation when both tabs are enabled, making two tabs a bit redundant)
            m_locationTab.EnableTabIndex(1, false);
        }
    }

    // Add the required tab items
    m_locationTab.InsertItem(0, s_inServiceTabText.c_str());
    m_locationTab.InsertItem(1, s_depotTabText.c_str());

    // Synchronize the current train mode with the tab selection
    // (ie select the tab that reflects current train selection)
    if (TA_IRS_Bus::LocationCache::Depot ==
        m_trainBroadcastModel->getCurrentListingMode())
    {
        // Select the depot (second) tab initially
        m_locationTab.SetCurSel(1);
    }
    else if (getAppropriateInServiceMode() ==
             m_trainBroadcastModel->getCurrentListingMode())
    {
        // Select the in service (first) tab
        m_locationTab.SetCurSel(0);
    }
    else
    {
        TA_ASSERT(false, "Unexpected initial state");
    }

    // Enable the owner draw style for tab control (so can properly
    // display enabled / disabled tabs)
    m_locationTab.ModifyStyle(0, TCS_OWNERDRAWFIXED);

    // Now, depending on which location we're running at, may need
    // to disable some of the tabs
    switch (CachedConfig::getInstance()->getLocationType())
    {
        case CachedConfig::LOCATION_TYPE_DEPOT:
            // The depot can only display depot items
            // Disable in service tab at first index
            m_locationTab.EnableTabIndex(0, false);
            break;

        case CachedConfig::LOCATION_TYPE_STATION:
            // Stations can't view the depot trains (at second index)
            m_locationTab.EnableTabIndex(1, false);
            break;

        case CachedConfig::LOCATION_TYPE_OCC:
            // Both tabs enabled, nothing to disable
            break;

        default:
            TA_ASSERT(false, "Unexpected location");
    }

    // Confirm initially selected tab is an enabled tab (this should be
    // set appropriately in constructor)
    TA_ASSERT(m_locationTab.IsTabEnabled(m_locationTab.GetCurSel()), "Invalid initial selection");

    // Set up initial rights settings..
    refreshTrainSelectionComponentRights();

    FUNCTION_EXIT;
}

afx_msg LRESULT TrainBroadcastPage::OnRightsChanged(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnRightsChanged");

    try
    {
        // Refresh the enabled state of the DVA / Live broadcast buttons
        refreshBroadcastButtonEnabledState();

        // If the currently checked button happens to become disabled, we need to
        // have the active selection change (to something..anything..that's valid)
        if (BST_CHECKED == m_dvaBtn.GetCheck() && !m_dvaBtn.IsWindowEnabled() ||
            BST_CHECKED == m_liveBtn.GetCheck() && !m_liveBtn.IsWindowEnabled())
        {
            enterDefaultBroadcastMode();
        }

        refreshTrainSelectionComponentRights();
    }

    STANDARD_PA_CATCH("TrainBroadcastPage::OnRightsChanged", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

void TrainBroadcastPage::refreshTrainSelectionComponentRights()
{
    FUNCTION_ENTRY("refreshTrainSelectionComponentRights");

    // Because the train selection component is shared between apps,
    // use the Radio Manager privelege aca_RADIO_MANAGE_TRAIN_SELECTIONS to
    // determine appropriate right settings
    bool enableTrainGroups = PaRightsManager::getInstance().hasRight(PaRightsManager::RADIO_MANAGE_TRAIN_SELECTIONS);

    // update the rights in the train selection component
    //getTrainSelectorGUI().setRights( true,    // Train list always enabled
    //getTrainSelectorGUI().setRights( enableTrainGroups,    // TD17648 lichao
    getTrainSelectorGUI().setRights(true,    // hongzhi, temporarily revert it, since rights have some problem, always disabled
                                    enableTrainGroups,           // train group combo
                                    enableTrainGroups,           // save train groups
                                    enableTrainGroups);          // delete train groups

    FUNCTION_EXIT;
}

TA_IRS_Bus::TrainSelectorGUI& TrainBroadcastPage::getTrainSelectorGUI()
{
    FUNCTION_ENTRY("getTrainSelectorGUI");

    TA_ASSERT(m_trainSelectorGUI != NULL, "Train selection component is NULL");

    FUNCTION_EXIT;
    return *m_trainSelectorGUI;
}

TA_IRS_Bus::TrainListCtrl& TrainBroadcastPage::getOutboundTrainList()
{
    FUNCTION_ENTRY("getOutboundTrainList");

    if (0 == m_outboundTrainList)
    {
        m_outboundTrainList = new TA_IRS_Bus::TrainListCtrl();
    }

    FUNCTION_EXIT;
    return *m_outboundTrainList;
}

TA_IRS_Bus::TrainListCtrl& TrainBroadcastPage::getInboundTrainList()
{
    FUNCTION_ENTRY("getInboundTrainList");

    if (0 == m_inboundTrainList)
    {
        m_inboundTrainList = new TA_IRS_Bus::TrainListCtrl();
    }

    FUNCTION_EXIT;
    return *m_inboundTrainList;
}

// jeffrey++ TD7956
void TrainBroadcastPage::OnRadioOverride()
{
    FUNCTION_ENTRY("OnRadioOverride");

    CheckRadioButton(IDC_RADIO_NORMAL, IDC_RADIO_OVERRIDE, IDC_RADIO_OVERRIDE);
    // Override has been set
    m_trainBroadcastModel->setHasOverrideOption(true);

    FUNCTION_EXIT;
}

void TrainBroadcastPage::OnRadioNormal()
{
    FUNCTION_ENTRY("OnRadioNormal");

    CheckRadioButton(IDC_RADIO_NORMAL, IDC_RADIO_OVERRIDE, IDC_RADIO_NORMAL);
    // Normal mode has been set
    m_trainBroadcastModel->setHasOverrideOption(false);

    FUNCTION_EXIT;
}

// ++jeffrey TD7956

// AZ++ TES 10924
void TrainBroadcastPage::setTrainDvaVersionsPage(TrainDvaVersionsPage* page)
{
    FUNCTION_ENTRY("setTrainDvaVersionsPage");

    m_trainDvaVersionsPage = page;

    FUNCTION_EXIT;
}

// ++AZ

// TD 15349
bool TrainBroadcastPage::selectMessageNameInList(const std::string messageName)
{
    FUNCTION_ENTRY("selectMessageNameInList");
    FUNCTION_EXIT;
    return findAndSelectMessageNameInList(messageName);
}

bool TrainBroadcastPage::findAndSelectMessageNameInList(const std::string messageName)
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

    // compare selected message with the messages in list
    for (int nItem = 0; nItem < m_dvaMessageList.GetItemCount(); nItem++)
    {
        CString nameInList = m_dvaMessageList.GetItemText(nItem, 0);

        if (stricmp(nameInList, paramValue.c_str()) == 0)
        {
            if (clearMessageSelectionInList() && m_dvaMessageList.SetItemState(nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED))
            {
                isSelectMessageNameInList = true;
                break;
            }
            else
            {
                LOG_ERROR("Cannot clear the exist selected message and add the pre-select message into message list");
            }
        }
    }

    FUNCTION_EXIT;
    return isSelectMessageNameInList;
}

bool TrainBroadcastPage::clearMessageSelectionInList()
{
    FUNCTION_ENTRY("clearMessageSelectionInList");

    for (int nItem = 0; nItem < m_dvaMessageList.GetItemCount(); nItem++)
    {
        if (m_dvaMessageList.SetItemState(nItem, 0, LVIS_SELECTED | LVIS_FOCUSED) == false)
        {
            FUNCTION_EXIT;
            return false;
        }
    }

    FUNCTION_EXIT;
    return true;
}

// TD 15349
