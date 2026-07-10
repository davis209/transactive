/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainDvaVersionsPage.cpp $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * Controls the Train DVA version display page
 *
 */

#include "app/pa/PAManager/src/stdafx.h"

#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/PaLayoutHelper.h"
#include "app/pa/PAManager/src/TrainDvaVersionsPage.h"
#include "app/pa/PAManager/src/pamanager.h"

#include "bus/trains/TrainCorbaProxy/src/TrainAgentProxyFactory.h"

#include "core/data_access_interface/pa_4669/src/PaTrainDvaMessageAccessFactory.h"

#include <iomanip>
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_IRS_Bus;

const DWORD HEADER_TEXT_COLOUR = RGB(255, 255, 255);
const DWORD HEADER_BACKGROUND_COLOUR = RGB(0, 0, 0);

IMPLEMENT_DYNCREATE(TrainDvaVersionsPage, CPropertyPage)

//const DWORD ISCS_ITEM_DATA = 1;
const DWORD TRAIN_ITEM_DATA  = 0;

// AZ++ TES 10924
const std::string TrainDvaVersionsPage::TRAIN_ID_COLUMN = "Train ID";
const std::string TrainDvaVersionsPage::VERSION_COLUMN = "Version";
// ++AZ

TrainDvaVersionsPage::TrainDvaVersionsPage(PAManagerDlg* paManagerDlg)
    : CPropertyPage(TrainDvaVersionsPage::IDD)
{
    FUNCTION_ENTRY("TrainDvaVersionsPage");

    TA_IRS_Bus::TrainAgentProxyFactory::getInstance().guiInitialise(CachedConfig::getInstance()->getSessionId());

    m_trainPaCorbaProxy = TA_IRS_Bus::TrainAgentProxyFactory::getInstance().createTrainPaCorbaProxy();

    //{{AFX_DATA_INIT(TrainDvaVersionsPage)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    FUNCTION_EXIT;
}

TrainDvaVersionsPage::~TrainDvaVersionsPage()
{
    FUNCTION_ENTRY("~TrainDvaVersionsPage");

    m_trainPaCorbaProxy->removeObserver(this);

    FUNCTION_EXIT;
}

void TrainDvaVersionsPage::DoDataExchange(CDataExchange* pDX)
{
    FUNCTION_ENTRY("DoDataExchange");

    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(TrainDvaVersionsPage)
    DDX_Control(pDX, IDC_DVA_HEADER_LST, m_firstVersionHeaderList);
    DDX_Control(pDX, IDC_DVA_VERSION_MAIN, m_versionList);
    //}}AFX_DATA_MAP

    FUNCTION_EXIT;
}

BEGIN_MESSAGE_MAP(TrainDvaVersionsPage, CPropertyPage)
    //{{AFX_MSG_MAP(TrainDvaVersionsPage)
    ON_NOTIFY(LVN_ITEMCHANGING, IDC_DVA_HEADER_LST, OnItemchangingDvaHeaderLst)
    ON_WM_DESTROY()
    ON_MESSAGE(WM_PA_TRAIN_GLOBAL_VERSION_UPDATE, OnGlobalTrainVersionChange)
    ON_MESSAGE(WM_PA_TRAIN_VERSION_UPDATE, OnSingleTrainVersionChange)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void TrainDvaVersionsPage::postDialogCreate()
{
    LOG_SCOPE("TrainDvaVersionsPage::postDialogCreate");
    FUNCTION_ENTRY("postDialogCreate");

    // set up the primary list of versions
    setupFirstHeaderList();

    // AZ++ TES 10924

    // this must be called after the header is set
    setupMainVersionList();

    // Begin listening out for global version updates just before adding to list
    TA_IRS_Bus::CachedMaps::getInstance()->getPaTrainDvaMessageCachedMap().registerGlobalVersionObserver(this);

    refreshISCSVersion();

    // get the PA versions
    populateInitialTrainVersions();

    // set up the train selection component
    // and PA version updates
    startSubscriptions();

    FUNCTION_EXIT;
}

void TrainDvaVersionsPage::setupFirstHeaderList()
{
    FUNCTION_ENTRY("setupFirstHeaderList");

    setupListView(m_firstVersionHeaderList, 2, true, LVCFMT_LEFT);

    m_firstVersionHeaderList.SetTextColor(HEADER_TEXT_COLOUR);
    m_firstVersionHeaderList.SetTextBkColor(HEADER_BACKGROUND_COLOUR);
    m_firstVersionHeaderList.SetBkColor(HEADER_BACKGROUND_COLOUR);

    // AZ++ TES 10924
    m_firstVersionHeaderList.InsertItem(0, TRAIN_ID_COLUMN.c_str());
    m_firstVersionHeaderList.SetItemText(0, 1, VERSION_COLUMN.c_str());

    // ++AZ

    FUNCTION_EXIT;
}

void TrainDvaVersionsPage::setupMainVersionList()
{
    FUNCTION_ENTRY("setupMainVersionList");

    // AZ++ TES 10924

    // set the data provider (for version info)
    m_versionList.setCustomDataProvider(this);

    // hide all the standard columns
    m_versionList.setStandardColumnSizes(0, 0, 0);

    // set up custom columns with same width as the header
    int widths[2];
    widths[0] = m_firstVersionHeaderList.GetColumnWidth(0);
    widths[1] = m_firstVersionHeaderList.GetColumnWidth(1);

    m_trainIdColumnIndex = m_versionList.addCustomColumn(TRAIN_ID_COLUMN, widths[0]);
    m_versionColumnIndex = m_versionList.addCustomColumn(VERSION_COLUMN, widths[1]);

    // ++AZ

    FUNCTION_EXIT;
}

void TrainDvaVersionsPage::setupListView(CListCtrl& list, DWORD columnCount, bool accountForScroll, DWORD format)
{
    FUNCTION_ENTRY("setupListView");

    list.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

    // jeffrey++ TD8850
    int width       = PaLayoutHelper::getListHeaderWidth(list, false);
    // ++jeffrey TD8850
    int averageWidth = width / columnCount;

    for (unsigned int nCol = 0; nCol < columnCount; nCol++)
    {
        // Keep inserting at the back (so the formatting works
        // - formatting is always left justified on first column no
        // matter what you specify)
        int nItem   = list.InsertColumn(nCol, "", format);

        if (0 == nCol)
        {
            // Make the first item expand to fill out any remainder
            list.SetColumnWidth(nItem, width - (averageWidth * (columnCount - 1)));
        }
        else
        {
            list.SetColumnWidth(nItem, averageWidth);
        }
    }

    FUNCTION_EXIT;
}

void TrainDvaVersionsPage::OnItemchangingDvaHeaderLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    FUNCTION_ENTRY("OnItemchangingDvaHeaderLst");

    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    // Only interested in state changes
    if (pNMListView->uChanged != LVIF_STATE)
    {
        FUNCTION_EXIT;
        return;
    }

    // Reject the change in state (no selections allowed on header controls)
    *pResult = TRUE;

    FUNCTION_EXIT;
}

void TrainDvaVersionsPage::OnDestroy()
{
    FUNCTION_ENTRY("OnDestroy");

    TA_IRS_Bus::CachedMaps::getInstance()->getPaTrainDvaMessageCachedMap().deRegisterGlobalVersionObserver(this);

    FUNCTION_EXIT;
}

void TrainDvaVersionsPage::onGlobalTrainVersionChanged()
{
    FUNCTION_ENTRY("onGlobalTrainVersionChanged");

    PostMessage(WM_PA_TRAIN_GLOBAL_VERSION_UPDATE, 0, 0);

    FUNCTION_EXIT;
}

LRESULT TrainDvaVersionsPage::OnGlobalTrainVersionChange(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnGlobalTrainVersionChange");

    try
    {
        // ISCS global train version has changed, update list
        refreshISCSVersion();
    }

    STANDARD_PA_CATCH("TrainDvaVersionsPage::OnGlobalTrainVersionChange", PaErrorHandler::UNSPECIFIED_ERROR);

    FUNCTION_EXIT;
    return 0;
}

void TrainDvaVersionsPage::refreshISCSVersion()
{
    FUNCTION_ENTRY("refreshISCSVersion");

    std::ostringstream iscsVersion;
    iscsVersion << TA_Base_Core::PaTrainDvaMessageAccessFactory::getInstance().getLibraryVersion();

    CWnd* pVersion  = GetDlgItem(IDC_ISCS_VERSION_FIELD);

    if (0 != pVersion)
    {
        pVersion->SetWindowText((LPCTSTR)iscsVersion.str().c_str());
    }

    FUNCTION_EXIT;
}

// AZ++ TES 10924

std::string TrainDvaVersionsPage::getCustomData(unsigned int trainId, const std::string& columnName)
{
    FUNCTION_ENTRY("getCustomData");

    // get the version for this train from train agent
    std::stringstream value;

    if (columnName == VERSION_COLUMN)
    {
        try
        {
            // get it from the cached internal values rather than asking the agent every time
            std::map< unsigned char, int >::iterator findIter = m_trainLibraryVersions.find(trainId);

            if (findIter != m_trainLibraryVersions.end())
            {
                value << std::setw(3) << std::setfill('0') << findIter->second;
            }
            else
            {
                // that failed for some reason, check the agent directly
                // missed a message?
                int version = 0;
                version = m_trainPaCorbaProxy->getTrainPaLibraryVersion(trainId);
                m_trainLibraryVersions[trainId] = version;

                value << std::setw(3) << std::setfill('0') << version;
            }
        }
        catch (TA_Base_Core::TransactiveException& e)
        {
            LOG_WARN(e.what());
        }
        catch (...)
        {
            // ignore all errors
        }
    }
    else if (columnName == TRAIN_ID_COLUMN)
    {
        value << trainId;
    }

    FUNCTION_EXIT;
    return value.str();
}

void TrainDvaVersionsPage::updateTrainList(const TA_IRS_Bus::ITrainSelectorGUI::EAction& action,
                                           const TA_IRS_Bus::TrainInformationTypes::TrainDetails& trainDetails)
{
    FUNCTION_ENTRY("updateTrainList");

    m_versionList.updateTrainList(action, trainDetails);
    // AdamR TD 12158 - removed redundant code

    FUNCTION_EXIT;
}

// ++AZ

// AdamR TD 12158

void TrainDvaVersionsPage::populateInitialTrainVersions()
{
    LOG_SCOPE("TrainDvaVersionsPage::populateInitialTrainVersions");
    FUNCTION_ENTRY("populateInitialTrainVersions");

    try
    {
        TA_IRS_Bus::PaTypes::TrainLibraryInfoList versionData = m_trainPaCorbaProxy->getTrainPaLibraryVersionList();

        for (UINT i = 0; i < versionData.size(); ++i)
        {
            m_trainLibraryVersions[versionData[i].trainId] = versionData[i].libraryVersion;
        }
    }
    catch (const TA_Base_Bus::ITrainExceptionsCorba::InvalidParameterException&)
    {
        // We didn't pass any parameters..
        PaErrorHandler::displayError(PaErrorHandler::TRAIN_AGENT_FAIL);
    }
    catch (const TA_Base_Bus::ITrainExceptionsCorba::TransactionTimeoutException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::TRAIN_AGENT_TIMEOUT);
    }
    catch (const TA_Base_Core::ObjectResolutionException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::OBJECT_RESOLUTION_EXCEPTION);
    }
    catch (const CORBA::Exception&)
    {
        // CORBA subsystem error
        PaErrorHandler::displayError(PaErrorHandler::CORBA_COMM_FAIL);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::TRAIN_AGENT_FAIL);
    }

    FUNCTION_EXIT;
}

void TrainDvaVersionsPage::startSubscriptions()
{
    FUNCTION_ENTRY("startSubscriptions");

    m_trainPaCorbaProxy->addObserver(this);

    FUNCTION_EXIT;
}

void TrainDvaVersionsPage::processTrainLibraryInfo(const TA_IRS_Bus::PaTypes::TrainLibraryInfo& event)
{
    LOG_INFO("Train Library version update received version %d for train %d", event.libraryVersion, event.trainId);

    m_trainLibraryVersions[event.trainId] = event.libraryVersion;

    // post a message to update
    PostMessage(WM_PA_TRAIN_VERSION_UPDATE, event.trainId, 0);
}

LRESULT TrainDvaVersionsPage::OnSingleTrainVersionChange(UINT wParam, LONG lParam)
{
    FUNCTION_ENTRY("OnSingleTrainVersionChange");

    // refresh the column
    m_versionList.refreshCustomColumn(wParam, VERSION_COLUMN);

    FUNCTION_EXIT;
    return 0;
}
