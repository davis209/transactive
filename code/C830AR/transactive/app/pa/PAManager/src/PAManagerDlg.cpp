/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAManagerDlg.cpp $
 * @author:  Ripple
 * @version: $Revision: #3 $
 *
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
 // PAManagerDlg.cpp : implementation file
 //

#include "app/pa/PAManager/src/stdafx.h"

#include "app/pa/PAManager/src/DvaVersionsPage.h"
#include "app/pa/PAManager/src/GraphworxComms.h"
#include "app/pa/PAManager/src/PAManagerDlg.h"
#include "app/pa/PAManager/src/PaErrorHandler.h"
#include "app/pa/PAManager/src/PaLayoutHelper.h"
#include "app/pa/PAManager/src/PaManager.h"
#include "app/pa/PAManager/src/PaManagerModel.h"
#include "app/pa/PAManager/src/PaRightsManager.h"
#include "app/pa/PAManager/src/RightsChangedWindowNotificationHandler.h"
#include "app/pa/PAManager/src/RunParamListener.h"
#include "app/pa/PAManager/src/SplashScreenEx.h"
#include "app/pa/PAManager/src/StationBroadcastPage.h"
#include "app/pa/PAManager/src/TAPropertySheet.h"
#include "app/pa/PAManager/src/TrainBroadcastPage.h"
#include "app/pa/PAManager/src/TrainDvaVersionsPage.h"
#include "app/pa/PAManager/src/TrainDvaVersionsPage.h"

#include "bus/generic_gui/src/GenericGuiConstants.h"
#include "bus/generic_gui/src/HelpLauncher.h"
#include "bus/generic_gui/src/TransActiveMessage.h"
#include "bus/mfc_extensions/src/selection_helpers/SelectionHelper.h"

#include "core/data_access_interface/entity_access/src/EntityAccessFactory.h"
#include "core/data_access_interface/pa/src/PaDvaMessageAccessFactory.h"
#include "core/data_access_interface/pa/src/PaDvaMessageVersionAccessFactory.h"
#include "core/data_access_interface/pa/src/PaTrainDvaMessageAccessFactory.h"
#include "bus/generic_gui/src/AppLauncher.h"	//zhongjie++
#include "core/utilities/src/RunParamsEx.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RPARAM_NO_TRAIN_PAGE                    "NoTrainPage"
#define RPARAM_NO_TRAIN_DVA_VERSION_PAGE        "NoTrainDvaVersionPage"

#define SM_CXVIRTUALSCREEN      78		//zhongjie++
#define MINVALUE				2400	//zhongjie++
#define NUMSCREENS				3		//zhongjie++

using namespace TA_Base_Core;

char DEPOT_BROADCAST_PAGE_TITLE[] = "Depot Broadcast";
char DEPOT_DVA_SET_VERSIONS_PAGE_TITLE[] = "Depot DVA Set Versions";
// TD17372
const std::string PAManagerDlg::MANAGER_LAUNCHING_BROADCAST_ID = "HardwareConnectionTest";
// TD17372

TA_Base_Core::PrimitiveWrapper<int> PAManagerDlg::s_restartTargetPosition;

/////////////////////////////////////////////////////////////////////////////
// PAManagerDlg dialog

PAManagerDlg::PAManagerDlg(TA_Base_Bus::IGUIAccess& controlClass) :
    TransActiveDialog(controlClass, PAManagerDlg::IDD, NULL),
    m_mainTab(NULL),
    m_stationPage(NULL),
    m_trainPage(NULL),
    m_dvaVersionsPage(NULL),
    m_trainDvaVersionsPage(NULL),
    m_managerModel(NULL),
    m_overridingOnMove(false),
    m_wantToShow(false),
    m_initialDisplay(false),
    m_rightsChangeHandler(0)
    // liuyu++ TD8839
    , m_pSplash(NULL)
    // ++liuyu TD8839
    //Maochun Sun++
    //TD14509
    , m_schematicId("")
    //++Maochun Sun
    //TD14509
{
    FUNCTION_ENTRY("PAManagerDlg");

    //{{AFX_DATA_INIT(PAManagerDlg)
    //}}AFX_DATA_INIT
    // liuyu++ TD8839
    m_pSplash = new CSplashScreenEx();
    m_pSplash->Create(this, "Loading", 0, CSS_FADE | CSS_CENTERSCREEN | CSS_SHADOW);
    m_pSplash->SetBitmap(IDB_SPLASH, 255, 0, 255);
    m_pSplash->Show();
    // ++liuyu TD8839
    m_mainTab = new TAPropertySheet("Nothing Important", this, 0);

    FUNCTION_EXIT;
}

PAManagerDlg::~PAManagerDlg()
{
    FUNCTION_ENTRY("~PAManagerDlg");

    //libo  before ~PAManageDlg,Call Windows Destroy.
    //Launch 2 PA schematics, after one of the PA schematic
    //shows ACTIVE, click on "Launch PA Manager" on the
    //other PA schematic, PA Manager will crash.

    if (m_rightsChangeHandler != 0)
    {
        DestroyWindow();
        OnDestroy();
    }

    // libo
    TA_ASSERT(0 == m_rightsChangeHandler, "Rights change handler should be deleted in OnDestroy");

    try
    {
        // Avoid any destruction here - core framework is
        // mostly destroyed
    }
    catch (...)
    {
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "Caught in destructor");
    }

    FUNCTION_EXIT;
}

void PAManagerDlg::DoDataExchange(CDataExchange* pDX)
{
    FUNCTION_ENTRY("DoDataExchange");

    TransActiveDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(PAManagerDlg)
    DDX_Control(pDX, IDC_ABOUTBUTTON, m_aboutButton);
    DDX_Control(pDX, IDC_HELP_BTN, m_helpButton);
    DDX_Control(pDX, IDC_EXECUTE_BTN, m_executeButton);
    DDX_Control(pDX, IDC_CLOSE_BTN, m_closeButton);
    DDX_Control(pDX, IDC_KILL_ALL_STATUS_BTN, m_killAllStatusButton);
    DDX_Control(pDX, IDC_KILL_STATUS_BTN, m_killStatusButton);
    DDX_Control(pDX, IDC_RESTORE_STATUS_BTN, m_restoreStatusButton);
    DDX_Control(pDX, IDC_STATUS_CB, m_statusNameList);
    DDX_Control(pDX, IDC_KILL_ALL_GOING_DVA, m_killAllGoingDVA);
    //}}AFX_DATA_MAP

    FUNCTION_EXIT;
}

BEGIN_MESSAGE_MAP(PAManagerDlg, TransActiveDialog)
    //{{AFX_MSG_MAP(PAManagerDlg)
    ON_WM_PAINT()
    ON_BN_CLICKED(IDC_CLOSE_BTN, OnCloseBtn)
    ON_BN_CLICKED(IDC_EXECUTE_BTN, OnExecuteBtn)
    ON_BN_CLICKED(IDC_HELP_BTN, OnHelpBtn)
    ON_MESSAGE(WM_PA_STN_STATUS_NAME_UPDATE, OnStatusNameChange)
    ON_MESSAGE(WM_PA_HMI_DISPLAYSEL_CHANGED, OnDisplayModeChange)
    ON_MESSAGE(WM_PA_ERROR_MESSAGE, OnErrorMessage)
    ON_CBN_SELCHANGE(IDC_STATUS_CB, OnSelchangeStatusCb)
    ON_BN_CLICKED(IDC_RESTORE_STATUS_BTN, OnRestoreStatusBtn)
    ON_BN_CLICKED(IDC_KILL_STATUS_BTN, OnKillStatusBtn)
    ON_BN_CLICKED(IDC_KILL_ALL_STATUS_BTN, OnKillAllStatusBtn)
    ON_WM_DESTROY()
    ON_WM_SHOWWINDOW()
    ON_WM_SIZE()
    ON_BN_CLICKED(IDC_ABOUTBUTTON, OnAboutbutton)
    ON_WM_MOVE()
    ON_WM_ACTIVATEAPP()
    ON_MESSAGE(WM_PA_RIGHTS_CHANGE, OnRightsChanged)
    ON_MESSAGE(WM_PA_ATTEMPT_APPLICATION_RESTART, OnAttemptRestart)
    ON_REGISTERED_MESSAGE(TA_Base_Bus::TA_GenericGui::WM_SET_WINDOW_POSITION, OnSetWindowPosition)
    ON_MESSAGE(WM_PA_MANAGER_ACTIVE, OnActivateSchematic)
    ON_BN_CLICKED(IDC_KILL_ALL_GOING_DVA, OnKillAllGoingDVA)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL PAManagerDlg::OnInitDialog()
{
    FUNCTION_ENTRY("OnInitDialog");

    RunParamListener::getInstance().setRestartApplicationCallback(this);

    m_rightsChangeHandler =  new RightsChangedWindowNotificationHandler(m_hWnd);

    //
    // C830 SPECIFIC!!!
    // Resize the window to width:1280, height: 803
    //
    SetWindowPos(NULL, 0, 0, 1280, 803, SWP_NOMOVE | SWP_NOZORDER);

    TA_Base_Bus::ResizingProperties rp;
    rp.canMaximise = false;
    rp.maxWidth = 1280;
    rp.maxHeight = 1024;

    // The minimum allowed dimensions are initial (startup) dimensions
    CRect rc;
    GetClientRect(rc);
    rp.minWidth = rc.Width();
    rp.minHeight = rc.Height();

    setResizingProperties(rp);

    // Go and create dialog
    TransActiveDialog::OnInitDialog();

    ModifyWindowPos(); //zhongjie++
    m_wantToShow = false;
    //TD18095, jianghp, to fix the performance of showing manager application

    //wenbo,cl-21355
    std::string cmdline(GetCommandLine());

    if (cmdline.find("select") != std::string::npos)
    {
        ShowWindow(SW_MINIMIZE);
    }

    FUNCTION_EXIT;
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void PAManagerDlg::init()
{
    LOG_SCOPE("PAManagerDlg::init");
    FUNCTION_ENTRY("init");

    if (m_managerModel == NULL)
    {
        m_managerModel = new PaManagerModel();
    }

    if (m_stationPage == NULL)
    {
        m_stationPage = new StationBroadcastPage(m_managerModel->getDvaMessageChangeSubscriber(), this);
    }

    if (m_trainPage == NULL)
    {
        if (!RunParamsEx::is_optional_true(RPARAM_NO_TRAIN_PAGE))
        {
            m_trainPage = new TrainBroadcastPage(this);
        }
    }

    if (m_dvaVersionsPage == NULL)
    {
        m_dvaVersionsPage = new DvaVersionsPage(this);
    }

    if (m_trainDvaVersionsPage == NULL)
    {
        if (!RunParamsEx::is_optional_true(RPARAM_NO_TRAIN_DVA_VERSION_PAGE))
        {
            m_trainDvaVersionsPage = new TrainDvaVersionsPage(this);
        }
    }

    if (m_stationPage)
    {
        m_mainTab->AddPage(m_stationPage);
    }

    if (m_trainPage)
    {
        m_mainTab->AddPage(m_trainPage);
        // AZ++ TES 10924 - AdamR TD 12158
        // set up the train selection component for the train version page
        m_trainPage->setTrainDvaVersionsPage(m_trainDvaVersionsPage);
        // ++AZ
    }

    if (m_dvaVersionsPage)
    {
        m_mainTab->AddPage(m_dvaVersionsPage);
    }

    if (m_trainDvaVersionsPage)
    {
        m_mainTab->AddPage(m_trainDvaVersionsPage);
    }

    layoutMainTab();

    refreshStatusNameCombo(true);

    // Perform final stage initialization for the tabs (MUST call from here)
    if (m_stationPage)
    {
        m_stationPage->postDialogCreate();
    }

    if (m_trainPage)
    {
        m_trainPage->postDialogCreate();
    }

    if (m_dvaVersionsPage)
    {
        m_dvaVersionsPage->postDialogCreate();
    }

    if (m_trainDvaVersionsPage)
    {
        m_trainDvaVersionsPage->postDialogCreate();
    }

    // Listen out to the Display tag (special system level command)
    // Should listen out for this AFTER posting startup hide request (otherwise
    // the request to display may be overriden by startup hide request)
    RunParamListener::getInstance().registerListener(RunParamListener::DISPLAY_TAG, *this);
    // TD 15349
    RunParamListener::getInstance().registerListener(RunParamListener::SELECTED_STATION_MSG_TAG, *this);
    RunParamListener::getInstance().registerListener(RunParamListener::SELECTED_TRAIN_MSG_TAG, *this);
    // TD 15349

    // Ensure controls layout out properly
    refreshControlPositions();

    // This causes the schematic to activate as soon as the PA Manager has completed
    // loading (this may not be desirable, as application loads in background)
    // It may be more appropriate to call this whenever a SHOW command is processed
    // (user wants to launch PA Manager)

    if (CachedConfig::getInstance()->GetActiveSchematicFlag())
    {
        LOG_INFO("PamManager  Show");
        activateSchematic();
    }

    // Set window handle AFTER responding to any display tags - if
    // an AfxMessageBox is present while we areminimized (result from
    // Display=HIDE message), the message box will (has in past) disappear
    // off the screen (and shift into non-screen area at top left).
    // Probably as a result of the minimize call being dispatched to
    // the currently active message box
    PaErrorHandler::setPrimaryWindowHandle(m_hWnd);

    // liuyu++ TD8839
    m_pSplash->Hide();

    if (m_pSplash)
    {
        //delete m_pSplash;
        m_pSplash = NULL;
    }

    // ++liuyu TD8839

    // TD17372
    CORBA::Boolean ret = true;
    CORBA_CALL(CachedConfig::getInstance()->getAgentObject(), hardwareLinkTest, (MANAGER_LAUNCHING_BROADCAST_ID.c_str(), CachedConfig::getInstance()->getSessionId().c_str()));

    if (!ret)
    {
        PaErrorHandler::displayError(PaErrorHandler::FEPC_HARDWARE_FAILURE);
    }

    // TD17372

    FUNCTION_EXIT;
}

void PAManagerDlg::activateSchematic()
{
    FUNCTION_ENTRY("activateSchematic");

    bool schematicUpdated = false;

    // Now we've finished loading, we may activate the graphworx display we're operating for
    try
    {
        //Maochun Sun++
        //TD14509
        //std::string schematicId = RunParamListener::getInstance().getSchematicIdentifier();
        //schematicUpdated = TA_App::GraphworxComms::getInstance().activateGraphworxDisplay(schematicId);

        m_schematicId = RunParamListener::getInstance().getSchematicIdentifier();
        schematicUpdated = TA_App::GraphworxComms::getInstance().activateGraphworxDisplay(m_schematicId);
        //++Maochun Sun
        //TD14509
    }
    catch (const TA_Base_Core::ValueNotSetException& e)
    {
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "ValueNotSetException", e.what());
    }
    catch (...)
    {
        // Silently log any error communicating with the actual schematic, may still run
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "While activating schematic");
    }

#ifdef _DEBUG

    // Perform an override if a debug session
    if (TA_Base_Core::RunParams::getInstance().isSet("Debug"))
    {
        schematicUpdated = true;
    }

#endif

    if (!schematicUpdated)
    {
        PaErrorHandler::displayModalError(PaErrorHandler::ERROR_SCHEMATIC_COMMS);
        PostQuitMessage(0);
    }

    FUNCTION_EXIT;
}

void PAManagerDlg::offsetWindowPos(CWnd& wnd, long x, long y)
{
    FUNCTION_ENTRY("offsetWindowPos");

    CRect rc;
    wnd.GetWindowRect(rc);

    // Convert into format required for MoveWindow call (if required)
    if (0 != wnd.GetParent())
    {
        wnd.GetParent()->ScreenToClient(rc);
    }

    wnd.MoveWindow(rc.left + x, rc.top + y, rc.Width(), rc.Height());

    FUNCTION_EXIT;
}

void PAManagerDlg::OnActivateApp(BOOL bActive, DWORD hTask)
{
    FUNCTION_ENTRY("OnActivateApp");

    if (!bActive)
    {
        // De-activation, no longer want to show the application when it's de-activated
        if (m_initialDisplay)
        {
            m_wantToShow = false;
        }
    }

    TransActiveDialog::OnActivateApp(bActive, (DWORD)hTask);

    FUNCTION_EXIT;
}

void PAManagerDlg::refreshStatusNameCombo(bool bInitialization)
{
    LOG_SCOPE("PAManagerDlg::refreshStatusNameCombo");
    FUNCTION_ENTRY("refreshStatusNameCombo");

    // First off clear the list
    m_statusNameList.ResetContent();
    m_statusNameList.SetCurSel(-1); // Deselect all

    // Perform initial update of buttons linked to selection in this combo
    // The buttons will be disabled initially until valid selection made
    OnSelchangeStatusCb();

    // Before fill out selection, choose type of messages
    // we respond to (when a status name comes through, we need to
    // filter out the ones we don't care about)
    m_currentStatusNameBroadcastType.resetValue();

    if (getActiveTabPage() == m_stationPage)
    {
        m_currentStatusNameBroadcastType.setValue(TA_Base_Bus::IPAAgentCorbaDef::StationDva);
    }
    else if (getActiveTabPage() == m_trainPage)
    {
        m_currentStatusNameBroadcastType.setValue(TA_Base_Bus::IPAAgentCorbaDef::TrainDva);
    }
    else
    {
        // Any unrecognized windows do not respond to any status name changes
        // so may exit early
        m_statusNameList.EnableWindow(FALSE);

        FUNCTION_EXIT;
        return;
    }

    if (0 == m_managerModel)
    {
        TA_ASSERT(FALSE, "m_managerModel NULL");

        FUNCTION_EXIT;
        return;
    }

    // Establish connection to name change subscriber before filling
    // out the status name combo box (important to minimize risk of out
    // of sync issues)
    if (bInitialization)
    {
        m_managerModel->initializeStatusNameSubscriber(m_hWnd);
    }

    std::vector<std::string> broadcastZones;

    bool success = false;

    try
    {
        if (getActiveTabPage() == m_stationPage)
        {
            broadcastZones = m_managerModel->getStationDvaBroadcastZones();
        }
        else if (getActiveTabPage() == m_trainPage)
        {
            broadcastZones = m_managerModel->getTrainDvaBroadcastZones();
        }
        else
        {
            TA_ASSERT(FALSE, "Unrecognized page");
        }

        success = true;
    }
    catch (TA_Base_Core::ObjectResolutionException&)
    {
    }
    catch (const CORBA::Exception&)
    {
    }

    if (!success)
    {
        // Unable to resolve CORBA object, no update available (combo remains empty)
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);

        // The combo can't be used
        m_statusNameList.EnableWindow(false);

        FUNCTION_EXIT;
        return;
    }

    // Enable list iff items present
    m_statusNameList.EnableWindow(broadcastZones.size() > 0);

    // Add all the names to the status name combo box
    for (unsigned int i = 0; i < broadcastZones.size(); i++)
    {
        m_statusNameList.AddString(broadcastZones[i].c_str());
    }

    // Now all items present in status name list, perform another refresh
    // to keep up to date (typically results in clear all button being enabled
    // if applicable)
    OnSelchangeStatusCb();

    FUNCTION_EXIT;
}

afx_msg LRESULT PAManagerDlg::OnErrorMessage(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnErrorMessage");

    try
    {
        // This is called by PaErrorHandler class, to allow modal error
        // windows to be displayed from the main loop.
        // Serious potential problems exist if a modal error dialog pops up
        // (with notifications continuing to be processed), since while
        // the modal error message is display, further execution may lead
        // to the same position in code, breaking NonReEntrantThreadlockable locks..
        PaErrorHandler::displayModalError(PaErrorHandler::EErrorId(wParam));
        // Confirm afterwards (to avoid repetitive displaying of errors)
        PaErrorHandler::confirmReceiptOfRequest(wParam);
    }
    catch (...)
    {
    }

    FUNCTION_EXIT;
    return 0;
}

afx_msg LRESULT PAManagerDlg::OnStatusNameChange(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnStatusNameChange");

    try
    {
        if (!updateComboBoxOnNameChange(wParam))
        {
            // Failed to update the combo box, so refresh it entirely
            // - because the name list may now not be up to date, because failed to
            //   process the received update

            // Attempt to update the name change properly
            refreshStatusNameCombo(false);
        }

        // Update enabled state of status name list dependent on how
        // many items are present there
        m_statusNameList.EnableWindow(m_statusNameList.GetCount() > 0);

        // Now that combo box has changed, update selection options
        OnSelchangeStatusCb();
    }

    STANDARD_PA_CATCH("PAManagerDlg::OnStatusNameChange", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

bool PAManagerDlg::updateComboBoxOnNameChange(UINT id)
{
    FUNCTION_ENTRY("updateComboBoxOnNameChange");

    if (0 == m_managerModel)
    {
        TA_ASSERT(FALSE, "m_managerModel NULL");

        FUNCTION_EXIT;
        return false;
    }

    // When a Status Name change occurs, we must intelligently respond so
    // that the current selection (in the Combo Box) remains consistent
    // There are a couple of possibilities:
    //  a) Selected item renamed - the appropriate item in list renamed
    //  b) Selected item deleted - the selection in list gets zeroed out
    // If there are any invalid notifications received (referencing non existent
    // status names etc), they are simply ignored..
    std::string from;
    std::string to;

    try
    {
        TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType broadcastType =
            m_managerModel->collectStatusNameChange(id, m_hWnd, from, to);

        if (!m_currentStatusNameBroadcastType.hasBeenSetToValue(broadcastType))
        {
            // If the current status name type we want doesn't match
            // the broadcast type of the change we're responding to -
            // we may discard the change (return true to indicate
            // processing successful).  It is quite normal to receive
            // changes not relevant to us (ie not an error condition)
            FUNCTION_EXIT;
            return true;
        }
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // The status (broadcast) name list may not be up to date, because
        // failed to collect the name change information
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);

        FUNCTION_EXIT;
        return false;
    }

    // Check if the target index already exists in combo box
    if (LB_ERR != TA_Base_Bus::SelectionHelper::findStringInComboCaseSensitive(m_statusNameList, to))
    {
        // This happens if the PA agent and local combo box are out of sync
        FUNCTION_EXIT;
        return false;
    }

    if (0 == from.length())
    {
        TA_ASSERT(to.length() > 0, "Internal error");

        // Source string is empty - this means we're adding to the combo box
        // Perform a final check to confirm we're allowed to display this new status name
        // (this should already have been handled at a higher level)
        if (PaRightsManager::getInstance().hasRightToDisplayBroadcast(
            to,
            CachedConfig::getInstance()->getAgentObject()))
        {
            m_statusNameList.AddString(to.c_str());
        }

        FUNCTION_EXIT;
        return true;
    }

    int nIndex = TA_Base_Bus::SelectionHelper::findStringInComboCaseSensitive(m_statusNameList, from);

    if (LB_ERR == nIndex)
    {
        // The 'from' field wasn't found in the list, we must be a little out of sync with
        // the PA Agent - ignore this update, but refresh combo list with latest items
        FUNCTION_EXIT;
        return false;
    }

    // Set a flag to indicate if the current selection corresponds to item being modified
    bool removedCurrentlySelectedItem = (m_statusNameList.GetCurSel() == nIndex);

    if (m_statusNameList.GetDroppedState() &&
        1 == m_statusNameList.GetCount() &&
        0 == to.length())
    {
        // Clear dropdown list just before removing the final string
        // To fix TestDirector Defect #2274
        m_statusNameList.ShowDropDown(FALSE);
    }

    // From item was found, determine whether we need to rename or delete an item from
    // the list - when we delete an item that's currently selected, the selection
    // is automatically cleared
    m_statusNameList.DeleteString(nIndex);

    if (0 == to.length())
    {
        // We removed the item, but nothing to replace it with - ensure the current selection is cleared
        if (removedCurrentlySelectedItem)
        {
            m_statusNameList.SetCurSel(-1);
        }
    }
    else
    {
        // Insert in the new string - it's position will depend on alphabetical ordering...
        nIndex = m_statusNameList.AddString(to.c_str());

        // Ensure newly added item is selected - but only if we just removed an item
        // that was currently selected
        if (removedCurrentlySelectedItem)
        {
            int confirm = m_statusNameList.SetCurSel(nIndex);

            TA_ASSERT(confirm == nIndex, "Internal error");
        }
    }

    FUNCTION_EXIT;
    return true;
}

void PAManagerDlg::layoutMainTab()
{
    LOG_SCOPE("PAManagerDlg::layoutMainTab");
    FUNCTION_ENTRY("layoutMainTab");

    CWnd* marker = this->GetDlgItem(IDC_TABMARKER);
    CRect markerRect;
    marker->GetWindowRect(markerRect);
    marker->DestroyWindow();
    this->ScreenToClient(markerRect);

    m_mainTab->Create(this, WS_CHILD | WS_EX_CONTROLPARENT | WS_VISIBLE | WS_TABSTOP);
    // We wioll want to receive notifications messages from this tab control
    m_mainTab->setParentToReceiveNotifyMessages(true);

    markerRect.left -= 7;
    markerRect.top -= 7;
    m_mainTab->MoveWindow(markerRect);

    // 4669FAT TD #709: Usually the station broadcasts page has text "Station Broadcast" in tab
    // change to "Depot Broadcast" if we're located at depot station
    if (CachedConfig::LOCATION_TYPE_DEPOT == CachedConfig::getInstance()->getLocationType())
    {
        int pageIndex = m_mainTab->GetPageIndex(m_stationPage);
        CTabCtrl* tabCtrl = m_mainTab->GetTabControl();

        if (pageIndex >= 0 && 0 != tabCtrl)
        {
            TCITEM tabData;
            tabData.mask = TCIF_TEXT;
            tabData.pszText = DEPOT_BROADCAST_PAGE_TITLE;
            tabCtrl->SetItem(pageIndex, &tabData);
        }

        // TD #10427
        // Set the title of the "Station DVA Set Versions" to "Depot DVA Set Versions"
        // if we're located at the depot station.
        pageIndex = m_mainTab->GetPageIndex(m_dvaVersionsPage);
        tabCtrl = m_mainTab->GetTabControl();

        if (pageIndex >= 0 && 0 != tabCtrl)
        {
            TCITEM tabData;
            tabData.mask = TCIF_TEXT;
            tabData.pszText = DEPOT_DVA_SET_VERSIONS_PAGE_TITLE;
            tabCtrl->SetItem(pageIndex, &tabData);

            // Also set the caption of the group box to the same text.
            CWnd* groupBox = m_dvaVersionsPage->GetDlgItem(IDC_STATIC_DVA_LABEL);
            groupBox->SetWindowText(DEPOT_DVA_SET_VERSIONS_PAGE_TITLE);
        }
    }

    FUNCTION_EXIT;
}

BOOL PAManagerDlg::DestroyWindow()
{
    FUNCTION_ENTRY("DestroyWindow");

    /*
    Do not destroy pages individually on exit
    Allow m_mainTab->DestroyWindow to do this indirectly
    Otherwise as we (for example) delete the station page
    the train page is activated automatically, causing
    more processing (which we don't want, we just want to quit)
    if (m_stationPage)
    {
        m_mainTab->RemovePage(m_stationPage);
    }
    if (m_trainPage)
    {
        m_mainTab->RemovePage(m_trainPage);
    }
    if (m_dvaVersionsPage)
    {
        m_mainTab->RemovePage(m_dvaVersionsPage);
    }
    if (m_trainDvaVersionsPage)
    {
        m_mainTab->RemovePage(m_trainDvaVersionsPage);
    }*/

    if (m_mainTab)
    {
        m_mainTab->DestroyWindow();
    }

    if (m_stationPage)
    {
        m_stationPage->DestroyWindow();
    }

    if (m_trainPage)
    {
        m_trainPage->DestroyWindow();
    }

    if (m_dvaVersionsPage)
    {
        m_dvaVersionsPage->DestroyWindow();
    }

    if (m_trainDvaVersionsPage)
    {
        m_trainDvaVersionsPage->DestroyWindow();
    }

    FUNCTION_EXIT;
    return TransActiveDialog::DestroyWindow();
}

void PAManagerDlg::OnPaint()
{
    FUNCTION_ENTRY("OnPaint");

    CPaintDC dc(this); // device context for painting

    // Do not call TransActiveDialog::OnPaint() for painting messages

    FUNCTION_EXIT;
}

void PAManagerDlg::OnCloseBtn()
{
    FUNCTION_ENTRY("OnCloseBtn");

    // Force closure//TD18095, jianghp, to fix the performance of showing manager application
    TransActiveDialog::DestroyWindow();

    FUNCTION_EXIT;
}

void PAManagerDlg::enableExecuteButton(bool isEnabled)
{
    FUNCTION_ENTRY("enableExecuteButton");

    CWnd* executeButtonWnd = this->GetDlgItem(IDC_EXECUTE_BTN);
    executeButtonWnd->EnableWindow(isEnabled);

    FUNCTION_EXIT;
}

void PAManagerDlg::OnHelpBtn()
{
    FUNCTION_ENTRY("OnHelpBtn");

    TA_Base_Bus::HelpLauncher::getInstance().displayHelp();

    FUNCTION_EXIT;
}

void PAManagerDlg::OnExecuteBtn()
{
    FUNCTION_ENTRY("OnExecuteBtn");

    int index = m_mainTab->GetActiveIndex();

    switch (index)
    {
        case 0:
        {
            // Station Broadcast
            m_stationPage->executeBroadcast();
        }
        break;

        case 1:
        {
            // Train Broadcast
            m_trainPage->executeBroadcast();
            break;
        }

        case 2:

            // DVA Versions
        default:
            TA_ASSERT(false, "Execute button pressed in invalid context.");
            break;
    }

    FUNCTION_EXIT;
}

void PAManagerDlg::OnSelchangeStatusCb()
{
    FUNCTION_ENTRY("OnSelchangeStatusCb");

    int curSel = m_statusNameList.GetCurSel();

    // User selected a new DVA broadcast group (aka Status Name)
    BOOL validSelection = (curSel >= 0);

    std::vector<std::string> statusNames;

    for (int i = 0; i < m_statusNameList.GetCount(); i++)
    {
        CString broadcastId;
        m_statusNameList.GetLBText(i, broadcastId);
        statusNames.push_back(broadcastId.GetBuffer(0));
    }

    ///////////////////////
    // Check if clear selected broadcast should be enabled
    BOOL allowedToClearSelectedBroadcast = validSelection;

    if (allowedToClearSelectedBroadcast)
    {
        // Confirm if about to enable the kill status button
        // that the user has the required access privileges to do so
        allowedToClearSelectedBroadcast = PaRightsManager::getInstance().hasRightToClearBroadcast(
            statusNames[curSel],
            CachedConfig::getInstance()->getAgentObject());

        // TES #703: Cannot clear if currently in progress
        if (PaRightsManager::getInstance().isBroadcastInProgress(
            statusNames[curSel],
            CachedConfig::getInstance()->getAgentObject()))
        {
            allowedToClearSelectedBroadcast = false;
        }
    }

    m_killStatusButton.EnableWindow(allowedToClearSelectedBroadcast);
    ////////////////////////

    ////////////////////////
    // Check if restore button should be enabled
    m_restoreStatusButton.EnableWindow(validSelection);
    ////////////////////////

    //////////////////////////////
    // Determine whether clear all button should be enabled
    bool enableKillAll = (m_statusNameList.GetCount() > 0);

    // May use clear all provided there is one or more item in list
    // NOTE: May not be able to clear the broadcasts if privileges not present (feedback provided to operator)
    m_killAllStatusButton.EnableWindow(enableKillAll);
    //////////////////////////////

    FUNCTION_EXIT;
}

void PAManagerDlg::onTabChanged()
{
    FUNCTION_ENTRY("onTabChanged");

    // Must refresh the combo each time change tabs (because
    // the status name combo box is context sensitive)
    refreshStatusNameCombo(false);

    FUNCTION_EXIT;
}

void PAManagerDlg::OnRestoreStatusBtn()
{
    FUNCTION_ENTRY("OnRestoreStatusBtn");

    CString broadcastId;
    int sel = m_statusNameList.GetCurSel();

    if (sel < 0)
    {
        // Despite all sorts of attempts, this condition can still occur when a combo box
        // changes while it is 'dropped down' (the onSelChanged function is not called when it should be)
        // So silently ignore
        OnSelchangeStatusCb();
        //TA_ASSERT(FALSE, "Invalid current selection");
        FUNCTION_EXIT;
        return;
    }

    m_statusNameList.GetLBText(sel, broadcastId);

    // Check if this broadcast ID still exists before attempting restore
    if (!m_managerModel->confirmStatusNameExists(broadcastId.GetBuffer(0)))
    {
        // No longer exists - we're out of sync, perform full refresh of combo
        refreshStatusNameCombo(false);

        FUNCTION_EXIT;
        return;
    }

    if (!m_currentStatusNameBroadcastType.hasBeenSet())
    {
        TA_ASSERT(FALSE, "Unknown broadcast ID context");

        FUNCTION_EXIT;
        return;
    }

    // If we don't have the right to display this status name (that we're
    // attempting to restore), then don't allow it) - we shouldn't get here
    // normally (because the status name shouldn't exist to be selectable
    // in the first place..)
    if (!PaRightsManager::getInstance().hasRightToDisplayBroadcast(
        broadcastId.GetBuffer(0),
        CachedConfig::getInstance()->getAgentObject()))
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);

        FUNCTION_EXIT;
        return;
    }

    if (getActiveTabPage() == m_stationPage)
    {
        m_stationPage->displayDvaStatusDialog(broadcastId.GetBuffer(0));
    }
    else if (getActiveTabPage() == m_trainPage)
    {
        m_trainPage->displayDvaStatusDialog(broadcastId.GetBuffer(0));
    }
    else
    {
        TA_ASSERT(FALSE, "PAManagerDlg::OnRestoreStatusBtn: Invalid mode for restore operation");
    }

    FUNCTION_EXIT;
}

void PAManagerDlg::OnKillStatusBtn()
{
    FUNCTION_ENTRY("OnKillStatusBtn");

    if (0 == m_managerModel)
    {
        TA_ASSERT(FALSE, "m_managerModel NULL");

        FUNCTION_EXIT;
        return;
    }

    int sel = m_statusNameList.GetCurSel();

    if (sel < 0)
    {
        // Despite all sorts of attempts, this condition can still occur when a combo box
        // changes while it is 'dropped down' (the onSelChanged function is not called when it should be)
        // So silently ignore
        OnSelchangeStatusCb();

        //TA_ASSERT(FALSE, "Invalid current selection");
        FUNCTION_EXIT;
        return;
    }

    // Kill off a single item
    CString broadcastId;
    m_statusNameList.GetLBText(sel, broadcastId);

    std::vector<std::string> broadcastArray;
    broadcastArray.push_back(broadcastId.GetBuffer(0));

    try
    {
        if (m_managerModel->terminateAndRemoveBroadcastIds(broadcastArray)
            != broadcastArray.size())
        {
            // Not all broadcasts passed in could be removed (maybe some already
            // removed..) - meaning we're not up to date
            // Force a full update, to ensure we're in sync with agent
            refreshStatusNameCombo(false);
        }
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

void PAManagerDlg::OnKillAllStatusBtn()
{
    FUNCTION_ENTRY("OnKillAllStatusBtn");

    if (0 == m_managerModel)
    {
        TA_ASSERT(FALSE, "m_managerModel NULL");

        FUNCTION_EXIT;
        return;
    }

    std::vector<std::string> broadcastArray;

    // Array of all broadcasts we don't have a right to terminate
    std::vector<std::string> invalidRights;
    // Array of all broadcasts still in progress
    std::vector<std::string> inProgressBroadcasts;

    // Progressively go through and kill each item currently in listbox..
    for (int i = 0; i < m_statusNameList.GetCount(); i++)
    {
        CString broadcastId;
        m_statusNameList.GetLBText(i, broadcastId);

        // Only add those items we're allowed to remove to the list
        // (but if rights change before actual call, secondary test performed
        // there also, which will pop up error dialog)
        if (PaRightsManager::getInstance().hasRightToClearBroadcast(
            broadcastId.GetBuffer(0),
            CachedConfig::getInstance()->getAgentObject()))
        {
            if (!PaRightsManager::getInstance().isBroadcastInProgress(
                broadcastId.GetBuffer(0),
                CachedConfig::getInstance()->getAgentObject()))
            {
                // TES #703: Can only clear inactive broadcasts
                broadcastArray.push_back(broadcastId.GetBuffer(0));
            }
            else
            {
                inProgressBroadcasts.push_back(broadcastId.GetBuffer(0));
            }
        }
        else
        {
            invalidRights.push_back(broadcastId.GetBuffer(0));
        }
    }

    try
    {
        // This could feasibly be empty if no right to clear any in the list
        if (broadcastArray.size() > 0)
        {
            if (m_managerModel->terminateAndRemoveBroadcastIds(broadcastArray) !=
                broadcastArray.size())
            {
                // Not all broadcasts passed in could be removed (maybe some already
                // removed..) - meaning we're not up to date
                // Force a full update, to ensure we're in sync with agent
                refreshStatusNameCombo(false);
            }
        }

        std::ostringstream reportStr;

        // Display a report showing the broadcasts that could not be cleared
        if (invalidRights.size() > 0)
        {
            // UI-610002
            reportStr << "The following broadcasts could not be cleared due to insufficient operator privileges:" << std::endl;

            for (unsigned int i = 0; i < invalidRights.size(); i++)
            {
                reportStr << "\t" << invalidRights[i] << std::endl;
            }
        }

        // TES #703 indicate which broadcasts couldn't be cleared because they were in progress
        if (inProgressBroadcasts.size() > 0)
        {
            // UI-610002
            reportStr << "The following broadcasts are currently active, and cannot be cleared:" << std::endl;

            for (unsigned int i = 0; i < inProgressBroadcasts.size(); i++)
            {
                reportStr << "\t" << inProgressBroadcasts[i] << std::endl;
            }
        }

        if (reportStr.str().length() > 0)
        {
            //TD14164 yezh++
            TA_Base_Bus::TransActiveMessage userMsg;
            userMsg.showMsgBox(IDS_UI_610002);
            //AfxMessageBox(reportStr.str().c_str());
            //TD14164 yezh++
        }
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

BOOL PAManagerDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnNotify");

    BOOL result =  TransActiveDialog::OnNotify(wParam, lParam, pResult);

    if (wParam == (WPARAM)(m_mainTab))
    {
        NMHDR* pnmh = (LPNMHDR)lParam;

        if (TCN_SELCHANGE == pnmh->code)
        {
            // Selected page just changed - must result in a refresh
            // Must do this after OnNotify (above) to ensure
            // getActivePage returns correct page
            onTabChanged();
            // The execute button state is maintained externally,
            // as windows lose focus they
        }
    }

    FUNCTION_EXIT;
    return result;
}

LRESULT PAManagerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    FUNCTION_ENTRY("WindowProc");

    LRESULT result = 0;

    // We shouldn't get any CORBA exceptions at this level, not necessarily
    // true for TransactiveExceptions though..
    try
    {
        result = TransActiveDialog::WindowProc(message, wParam, lParam);
    }

    STANDARD_PA_CATCH("PAManagerDlg::WindowProc", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return result;
}

afx_msg LRESULT PAManagerDlg::OnDisplayModeChange(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnDisplayModeChange");

    if (1 == wParam)
    {
        ShowWindow(SW_RESTORE);
        RunParamListener::getInstance().setIsCurrentlyVisible(true);

        m_initialDisplay = true;
    }
    else
    {
        // This causes the dialog to minimize(??) - on a normal dev system
        // it's handled OK, but on the MFTs the modal dialog disappears
        // (off the top left of the screen - a SW_MINIMIZE message posts out
        // a OnMove notification, to -32000, -32000)
        if (!PaErrorHandler::isDisplayingModalError())
        {
            ShowWindow(SW_MINIMIZE);
            RunParamListener::getInstance().setIsCurrentlyVisible(false);
        }
        else
        {
            PaErrorHandler::displayModalError(
                PaErrorHandler::CANNOT_MINIMIZE_PA_MANAGER_WHILE_MSGBOX_ACTIVE);
        }
    }

    FUNCTION_EXIT;
    return 0;
}

void PAManagerDlg::processRunParamChange(const std::string& name, const std::string& value)
{
    FUNCTION_ENTRY("processRunParamChange");

    LOG_INFO("PamManageDlg  RunParamChange name = '%s',ParamValue ='%s'", name, value);

    if (stricmp(name.c_str(), RunParamListener::DISPLAY_TAG.c_str()) == 0)
    {
        if (stricmp(value.c_str(), "SHOW") == 0)
        {
            ::PostMessage(m_hWnd, WM_PA_HMI_DISPLAYSEL_CHANGED, 1, 0);
            //ShowWindow(SW_RESTORE);
            //td5027  Each time the state of a PA zone is change the PA Manager shouldn't gain focus.
            //m_wantToShow = true;
        }
        else
        {
            //ShowWindow(SW_MINIMIZE);
            // ShowWindow(SW_HIDE);

            // Removed as per Propweb case #3429
            // If we hide window whenever we get a HIDE commmand, when
            // the pamanager is displayed, and user leaves then goes back to
            // the schematic, we DON'T want the PA Manager hiding as the schematic
            // is openened - the PA Manager is hidden on startup by default
            // so the Display=HIDE command is now redundant..
            /*
            m_wantToShow = false;
            ::PostMessage(m_hWnd, WM_PA_HMI_DISPLAYSEL_CHANGED, 0, 0);
            */
        }
    }
    // TD 15349
    else if (stricmp(name.c_str(), RunParamListener::SELECTED_STATION_MSG_TAG.c_str()) == 0)
    {
        m_stationPage->selectMessageNameInLists(value);
    }
    else if (stricmp(name.c_str(), RunParamListener::SELECTED_TRAIN_MSG_TAG.c_str()) == 0)
    {
        m_mainTab->SetActivePage(1);
        m_trainPage->selectMessageNameInList(value);
    }
    // TD 15349
    else
    {
        TA_ASSERT(FALSE, "Unrecognized run param change");
    }

    FUNCTION_EXIT;
}

void PAManagerDlg::OnDestroy()
{
    FUNCTION_ENTRY("OnDestroy");

    RunParamListener::getInstance().setRestartApplicationCallback(0);

    // Delete the rights change notification object before window handle becomes invalid
    delete m_rightsChangeHandler;
    m_rightsChangeHandler = 0;

    m_managerModel->removeStatusNameSubscriber(m_hWnd);

    TransActiveDialog::OnDestroy();

    // Deregister all listeners attached to window on cleanup
    RunParamListener::getInstance().deRegisterListener(*this);

    // Reset error handler window reference before we're destroyed
    PaErrorHandler::setPrimaryWindowHandle(0);

    try
    {
        if (m_mainTab)
        {
            delete m_mainTab;
            m_mainTab = NULL;
        }

        if (m_stationPage)
        {
            delete m_stationPage;
            m_stationPage = NULL;
        }

        if (m_trainPage)
        {
            delete m_trainPage;
            m_trainPage = NULL;
        }

        if (m_dvaVersionsPage)
        {
            delete m_dvaVersionsPage;
            m_dvaVersionsPage = NULL;
        }

        if (m_trainDvaVersionsPage)
        {
            delete m_trainDvaVersionsPage;
            m_trainDvaVersionsPage = NULL;
        }

        delete m_managerModel;
        m_managerModel = 0;

        // Must attempt cleanup of singletons here before
        // core servants are de-activated
        PAManagerApp::cleanupSingletons();

        // When we're shutting down, should also de-activate all the graphworx displays
        TA_App::GraphworxComms::getInstance().resetGraphworxDisplaysToInactive();
    }
    catch (...)
    {
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "Caught in OnDestroy");
    }

    FUNCTION_EXIT;
}

CPropertyPage* PAManagerDlg::getActiveTabPage()
{
    FUNCTION_ENTRY("getActiveTabPage");

    // Sometimes GetActivePage is out of date, while GetActiveIndex isn't??
    int curSel = m_mainTab->GetActiveIndex();

    FUNCTION_EXIT;
    return m_mainTab->GetPage(curSel);
}

void PAManagerDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    FUNCTION_ENTRY("OnShowWindow");

    TransActiveDialog::OnShowWindow(bShow, nStatus);

    // TODO: Add your message handler code here

    FUNCTION_EXIT;
}

void PAManagerDlg::OnSize(UINT nType, int cx, int cy)
{
    FUNCTION_ENTRY("OnSize");

    if (0 == m_mainTab)
    {
        TA_ASSERT(FALSE, "m_mainTab NULL");

        FUNCTION_EXIT;
        return;
    }

    // Tab control not created yet
    if (!IsWindow(m_mainTab->m_hWnd))
    {
        FUNCTION_EXIT;
        return;
    }

    refreshControlPositions();

    // If the 'Launch...' button has been pressed
    // only then should the app be
    if ((m_wantToShow) && (nType == 0))
    {
        TransActiveDialog::OnSize(nType, cx, cy);

        // Each time the window moves, must reposition main tab display
        // and items aligned below tab (the buttons)
    }
    else
    {
        if (nType == 1) // Minimise
        {
            m_wantToShow = false;  // make sure the window doesn't pop up on PID selections now it is minimised
            FUNCTION_EXIT;
            return;
        }
    }

    FUNCTION_EXIT;
}

void PAManagerDlg::refreshControlPositions()
{
    FUNCTION_ENTRY("refreshControlPositions");

    if (0 == m_mainTab)
    {
        TA_ASSERT(FALSE, "m_mainTab NULL");

        FUNCTION_EXIT;
        return;
    }

    CRect originalTabPos;
    //m_mainTab->GetWindowRect(originalTabPos);
    m_mainTab->GetClientRect(originalTabPos);
    m_mainTab->ClientToScreen(&originalTabPos);
    // Centre the main tab in the window
    m_mainTab->CenterWindow(this);

    CRect helpRect;
    //m_helpButton.GetWindowRect(helpRect);
    m_helpButton.GetClientRect(helpRect);
    m_helpButton.ClientToScreen(&helpRect);

    // Now shift up slightly (to account for lower set of buttons
    // & center properly)
    long buttonArea = (helpRect.bottom - originalTabPos.bottom);
    offsetWindowPos(*m_mainTab, 0, -(long)ceil((long double)(buttonArea / 2)));

    CRect newTabPos;
    //m_mainTab->GetWindowRect(newTabPos);
    m_mainTab->GetClientRect(newTabPos);
    m_mainTab->ClientToScreen(&newTabPos);

    // Take offset representing movement from original tab to new tab pos
    long offsetX = newTabPos.right - originalTabPos.right;
    long offsetY = newTabPos.bottom - originalTabPos.bottom;

    // Now shift all the buttons + combo box to beneath main tab
    offsetWindowPos(m_helpButton, offsetX, offsetY);
    offsetWindowPos(m_aboutButton, offsetX, offsetY);
    offsetWindowPos(m_restoreStatusButton, offsetX, offsetY);
    offsetWindowPos(m_killStatusButton, offsetX, offsetY);
    offsetWindowPos(m_killAllStatusButton, offsetX, offsetY);
    offsetWindowPos(m_statusNameList, offsetX, offsetY);
    offsetWindowPos(m_executeButton, offsetX, offsetY);
    offsetWindowPos(m_closeButton, offsetX, offsetY);
    offsetWindowPos(m_killAllGoingDVA, offsetX, offsetY); //wuzhongyi CL-19136

    FUNCTION_EXIT;
}

void PAManagerDlg::OnAboutbutton()
{
    FUNCTION_ENTRY("OnAboutbutton");

    TA_Base_Bus::HelpLauncher::getInstance().displayAboutBox();

    FUNCTION_EXIT;
}

void PAManagerDlg::OnMove(int x, int y)
{
    FUNCTION_ENTRY("OnMove");

    TransActiveDialog::OnMove(x, y);

    LOG_INFO("Window moved to (%d,%d)", x, y);

    if (x == -32000 && y == -32000)
    {
        // This is a special case situation that occurs in response
        // to a WM_MINIMIZE notification
        FUNCTION_EXIT;
        return;
    }

    CRect screen;
    ::GetClientRect(::GetDesktopWindow(), screen);
    CRect rc;
    GetWindowRect(rc);

    enum { BUFFER_DISTANCE = 8 };

    int adjustedX = x;
    int adjustedY = y;

    if (adjustedY < BUFFER_DISTANCE)
    {
        // Don't allow movement into negative on Y (at all)
        adjustedY = BUFFER_DISTANCE;
    }

    // Don't allow to move completely below bottom of screen
    if (adjustedY > screen.Height() - BUFFER_DISTANCE)
    {
        adjustedY = screen.Height() - BUFFER_DISTANCE;
    }

    // Allow at least BUFFER_DISTANCE pixel inside of screen either side
    if (adjustedX + rc.Width() < BUFFER_DISTANCE)
    {
        adjustedX = -rc.Width() + BUFFER_DISTANCE;
    }

    if (adjustedX > screen.Width() - BUFFER_DISTANCE)
    {
        adjustedX = screen.Width() - BUFFER_DISTANCE;
    }

    // To avoid recursion, confirm this OnMove request is not in response
    // To an onMove override
    if (!m_overridingOnMove)
    {
        if (x != adjustedX || y != adjustedY)
        {
            CRect client;
            GetClientRect(client);
            ClientToScreen(client);
            adjustedY -= (client.top - rc.top);     // Subtract height of menu
            adjustedX -= (client.left - rc.left);   // Subtract width of left border

            m_overridingOnMove = true;

            // Reposition as necessary
            ::MoveWindow(m_hWnd, adjustedX, adjustedY, rc.Width(), rc.Height(), TRUE);

            m_overridingOnMove = false;

            LOG_INFO("Window outside screen, repositioned to (%d,%d)", adjustedX, adjustedY);
        }
    }

    FUNCTION_EXIT;
}

afx_msg LRESULT PAManagerDlg::OnRightsChanged(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnRightsChanged");

    try
    {
        // Refresh contents of the status name combo fully (may be changes in those we can / can not view)
        refreshStatusNameCombo(false);
    }

    STANDARD_PA_CATCH("PAManagerDlg::OnRightsChanged", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

void PAManagerDlg::restartApplication(int leftPos)
{
    FUNCTION_ENTRY("restartApplication");

    ::PostMessage(m_hWnd, WM_PA_ATTEMPT_APPLICATION_RESTART, (DWORD)leftPos, 0);

    FUNCTION_EXIT;
}

afx_msg LRESULT PAManagerDlg::OnAttemptRestart(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnAttemptRestart");

    // Before we can restart - we must confirm there are no modal dialogs
    // active (otherwise users changes will be lost), if there are any open
    // we display an appropriate error message
    // We can tell if there are modal dialog boxes if the main dialog window
    // is disabled

    if (!IsWindowEnabled())
    {
        // This means a modal dialog is active..
        PaErrorHandler::displayModalError(PaErrorHandler::ERROR_AUTO_SHUTDOWN);
    }
    else
    {
        s_restartTargetPosition.setValue((int)(wParam));

        // Otherwise it shouldn't be a problem restarting, runparams should transfer to
        // new instance no problems, except for SynchroniseSelected (which spans several runparams)
        PostQuitMessage(0);
    }

    FUNCTION_EXIT;
    return 0;
}

bool PAManagerDlg::isApplicationRestartInProgress()
{
    FUNCTION_ENTRY("isApplicationRestartInProgress");
    FUNCTION_EXIT;
    return s_restartTargetPosition.hasBeenSet();
}

int PAManagerDlg::getAppplicationRestartTargetPosition()
{
    FUNCTION_ENTRY("getAppplicationRestartTargetPosition");

    ASSERT(isApplicationRestartInProgress());

    FUNCTION_EXIT;
    return s_restartTargetPosition.getValue();
}

LRESULT PAManagerDlg::OnSetWindowPosition(WPARAM wParam, LPARAM lParam)
{
    FUNCTION_ENTRY("OnSetWindowPosition");

    // TES #720
    // don't show it if we don't want it to be shown
    ACE_GUARD_RETURN(ACE_Thread_Mutex, ace_mon, RunParamListener::Tag_mutex, -1);

    if (RunParamListener::DONTGOFRONT_TAG == 0)
    {
        FUNCTION_EXIT;
        return TransActiveDialog::OnSetWindowPosition(wParam, lParam);
    }

    --RunParamListener::DONTGOFRONT_TAG;
    // TD 16522
    //++TES230
}

// TD 15349
const std::string PAManagerDlg::getSelectedStationMessage()
{
    FUNCTION_ENTRY("getSelectedStationMessage");
    FUNCTION_EXIT;
    return RunParamListener::getInstance().getSelectedStationMessage();
}

const std::string PAManagerDlg::getSelectedTrainMessage()
{
    FUNCTION_ENTRY("getSelectedTrainMessage");
    FUNCTION_EXIT;
    return RunParamListener::getInstance().getSelectedTrainMessage();
}

// TD 15349

afx_msg LRESULT PAManagerDlg::OnActivateSchematic(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnActivateSchematic");

    LOG_INFO("PamManager  OnActivateSchematic");

    activateSchematic();

    FUNCTION_EXIT;
    return 0;
}

void PAManagerDlg::ModifyWindowPos()  //zhongjie++ this function to modify the PAManager Dialog position in three monitor
{
    int left, top, width, height;
    int screenWidth, currentScreen;
    int numScreens = 1;
    int x_pos;

    CPoint   pt;
    GetCursorPos(&pt);

    RECT boundary = TA_Base_Bus::AppLauncher::getInstance().getRect(TA_Base_Bus::TA_GenericGui::SCREEN_CURRENT,
                                                                    TA_Base_Bus::TA_GenericGui::AREA_SCHEMATIC,
                                                                    pt.x);
    x_pos = pt.x;
    screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);

    if (screenWidth >= MINVALUE)
        numScreens = NUMSCREENS;

    currentScreen = x_pos / (screenWidth / numScreens);
    left	= currentScreen * (screenWidth / numScreens) + 1;
    width	= screenWidth / numScreens;
    top		= boundary.top;
    height	= boundary.bottom - boundary.top;

    this->MoveWindow(left, top, width, height, NULL);
}

void PAManagerDlg::OnKillAllGoingDVA()
{
    LOG_SCOPE("PAManagerDlg::OnKillAllGoingDVA");
    FUNCTION_ENTRY("OnKillAllGoingDVA");

    std::string confirmMessage = "Are you sure to stop all ongoing DVA ?";
    std::string MessageBoxCaption = "PA Manager";

    if (IDYES == MessageBox(confirmMessage.c_str(), MessageBoxCaption.c_str(), MB_YESNO + MB_DEFBUTTON2 + MB_ICONQUESTION))
    {
        try
        {
            CORBA_CALL(CachedConfig::getInstance()->getAgentObject(), forceTerminateAllExistPA, ());
        }
        catch (TA_Base_Core::TransactiveException& err)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TransactiveException", err.what());
        }
        catch (...)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "Unknown", "Unknown exception.");
        }
    }
    else
    {
        //do nothing
    }

    FUNCTION_EXIT;
}
