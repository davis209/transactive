/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DVAVersionsPage.cpp $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * Controls the DVA version display page
 *
 */

#include "app/pa/PAManager/src/stdafx.h"
#include "app/pa/PAManager/src/pamanager.h"
#include "app/pa/PAManager/src/DvaVersionsPage.h"
#include "app/pa/PAManager/src/PaErrorHandler.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "core/exceptions/src/PasExceptions.h"
#include "app/pa/PAManager/src/PaLayoutHelper.h"
#include "bus/pa/cachedmaps/src/PaDvaVersionCachedMap.h"
#include "core/data_access_interface/src/LocationAccessFactory.h"

#include "bus/pa/cachedmaps/src/PaDvaPublicVersionCachedMap.h"
#include "bus/pa/cachedmaps/src/PaDvaPrivateVersionCachedMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_IRS_Bus;

const DWORD HEADER_TEXT_COLOUR = RGB(255, 255, 255);
const DWORD HEADER_BACKGROUND_COLOUR = RGB(0, 0, 0);
// This is the amount the list control is extended (width) in order to view full version info
// - not required any more, after change made to only view one DVA set (not both private and public)
// - this means none of the scrolling code is used (required) anymore, but left in nonetheless
const DWORD LIST_WIDTH_EXCESS = 0;

IMPLEMENT_DYNCREATE(DvaVersionsPage, CPropertyPage)

DvaVersionsPage::DvaVersionsPage(PAManagerDlg* paManagerDlg)
    :
    CPropertyPage(DvaVersionsPage::IDD)
{
    //{{AFX_DATA_INIT(DvaVersionsPage)
    //}}AFX_DATA_INIT
}

DvaVersionsPage::~DvaVersionsPage()
{
}

void DvaVersionsPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(DvaVersionsPage)
    DDX_Control(pDX, IDC_DVA_HEADER_LST_2, m_secondVersionHeaderList);
    DDX_Control(pDX, IDC_DVA_HEADER_LST, m_firstVersionHeaderList);
    DDX_Control(pDX, IDC_DVA_VERSION_MAIN, m_versionList);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(DvaVersionsPage, CPropertyPage)
    //{{AFX_MSG_MAP(DvaVersionsPage)
    ON_WM_DESTROY()
    ON_MESSAGE(WM_PA_DVA_VERSION_UPDATE, OnDvaVersionUpdate)
    ON_NOTIFY(LVN_ITEMCHANGING, IDC_DVA_HEADER_LST, OnItemchangingDvaHeaderLst)
    ON_NOTIFY(LVN_ITEMCHANGING, IDC_DVA_HEADER_LST_2, OnItemchangingDvaHeaderLst2)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL DvaVersionsPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    ///////////
    // Listen out for updates to any ad-hoc messages
    try
    {
        // m_dvaVersionChangeSubscriber.addListenerWindow(m_hWnd);
        m_dvaPublicVersionChangeSubscriber.addListenerWindow(m_hWnd);
        m_dvaPrivateVersionChangeSubscriber.addListenerWindow(m_hWnd);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // The ad-hoc list items will not be updated (nothing major, just can't view up to date info)
        PaErrorHandler::displayError(PaErrorHandler::INIT_ERROR);
    }

    // The depot and OCC are special locations, when at the OCC (otherwise there
    // are no 'special' listview items, all treated the same)
    if (CachedConfig::getInstance()->getLocationType() ==
        CachedConfig::LOCATION_TYPE_OCC)
    {
        m_versionList.registerHighlightData(
            TA_Base_Core::LocationAccessFactory::getInstance().getOccLocationKey());
        m_versionList.registerHighlightData(
            TA_Base_Core::LocationAccessFactory::getInstance().getDepotLocationKey());
    }

    m_versionList.addHorizontalScrollingChild(m_secondVersionHeaderList);
    m_versionList.addHorizontalScrollingChild(m_firstVersionHeaderList);

    m_secondVersionHeaderList.HideScrollBars(LCSB_NCOVERRIDE);
    m_firstVersionHeaderList.HideScrollBars(LCSB_NCOVERRIDE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void DvaVersionsPage::postDialogCreate()
{
    LOG_SCOPE("DvaVersionsPage::postDialogCreate");

    // set up the primary list of versions
    setupMainVersionList();

    setupSecondHeaderList();

    setupFirstHeaderList();
}

void DvaVersionsPage::setupFirstHeaderList()
{
    // Set up the version header - first column will match width of
    // first column from main list, last column same
    setupListView(m_firstVersionHeaderList, 3, true, LVCFMT_CENTER);

    m_firstVersionHeaderList.SetTextColor(HEADER_TEXT_COLOUR);
    m_firstVersionHeaderList.SetTextBkColor(HEADER_BACKGROUND_COLOUR);
    m_firstVersionHeaderList.SetBkColor(HEADER_BACKGROUND_COLOUR);

    // Adjust widths of header to match the main list
    DWORD firstColumnWidth = m_secondVersionHeaderList.GetColumnWidth(0);
    m_firstVersionHeaderList.SetColumnWidth(0, firstColumnWidth);

    // Second column will match up with the next 7 columns
    DWORD columnWidth = 0;
    int i;

    for (i = 1; i < 8; i++)
    {
        columnWidth += m_secondVersionHeaderList.GetColumnWidth(i);
    }

    m_firstVersionHeaderList.SetColumnWidth(1, columnWidth);

    // Make last column take up the remainder of space
    m_firstVersionHeaderList.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);

    m_firstVersionHeaderList.InsertItem(0, "");

    // Only show single DVA Version Set depending on which location we're at
    switch (CachedConfig::getInstance()->getLocationType())
    {
        case CachedConfig::LOCATION_TYPE_OCC:
            m_firstVersionHeaderList.SetItemText(0, 1, "DVA Set A");
            break;

        case CachedConfig::LOCATION_TYPE_STATION:
            m_firstVersionHeaderList.SetItemText(0, 1, "DVA Set B");
            break;

        case CachedConfig::LOCATION_TYPE_DEPOT:
            m_firstVersionHeaderList.SetItemText(0, 1, "DVA Set D");
            break;

        default:
            TA_ASSERT(false, "Unexpected location type");
    }

    /*
    m_firstVersionHeaderList.SetItemText(0, 1, "DVA Public Set A (OCR & Stations)");
    m_firstVersionHeaderList.SetItemText(0, 2, "DVA Private Set B&D (Stations & Depot)");
    */
}

void DvaVersionsPage::setupSecondHeaderList()
{
    // These show headings for main list - needs to match precisely
    setupListView(m_secondVersionHeaderList, 8, true, LVCFMT_LEFT);

    m_secondVersionHeaderList.SetTextColor(HEADER_TEXT_COLOUR);
    m_secondVersionHeaderList.SetTextBkColor(HEADER_BACKGROUND_COLOUR);
    m_secondVersionHeaderList.SetBkColor(HEADER_BACKGROUND_COLOUR);

    // Add in the list headings before adding in elements
    int nItem = m_secondVersionHeaderList.InsertItem(0, "Station");
    m_secondVersionHeaderList.SetItemText(nItem, 1, "Adhoc 1");
    m_secondVersionHeaderList.SetItemText(nItem, 2, "Adhoc 2");
    m_secondVersionHeaderList.SetItemText(nItem, 3, "Adhoc 3");
    m_secondVersionHeaderList.SetItemText(nItem, 4, "Adhoc 4");
    m_secondVersionHeaderList.SetItemText(nItem, 5, "Adhoc 5");
    m_secondVersionHeaderList.SetItemText(nItem, 6, "Adhoc 6");
    m_secondVersionHeaderList.SetItemText(nItem, 7, "Pre-recorded");

    // As of latest GUI design notes - only show a single set at
    // any given location (not both sets)
    /*
    m_secondVersionHeaderList.SetItemText(nItem, 8, "A1");
    m_secondVersionHeaderList.SetItemText(nItem, 9, "A2");
    m_secondVersionHeaderList.SetItemText(nItem, 10, "A3");
    m_secondVersionHeaderList.SetItemText(nItem, 11, "A4");
    m_secondVersionHeaderList.SetItemText(nItem, 12, "A5");
    m_secondVersionHeaderList.SetItemText(nItem, 13, "A6");
    m_secondVersionHeaderList.SetItemText(nItem, 14, "P");
    */
}

void DvaVersionsPage::setupMainVersionList()
{
    setupListView(m_versionList, 8, true, LVCFMT_LEFT);

    // Now add in the actual version data
    try
    {
        if (CachedConfig::getInstance()->isAtOcc())
        {
            DvaPublicVersionRecords allVersionData =
                TA_IRS_Bus::CachedMaps::getInstance()->getDvaPublicVersionCachedMap().getAllRecords();

            for (DvaPublicVersionRecordsIt it = allVersionData.begin(); it != allVersionData.end(); it++)
            {
                m_versionList.addRowData(*it);
            }
        }
        else
        {
            DvaPrivateVersionRecords allVersionData =
                TA_IRS_Bus::CachedMaps::getInstance()->getDvaPrivateVersionCachedMap().getAllRecords();

            for (DvaPrivateVersionRecordsIt it = allVersionData.begin(); it != allVersionData.end(); it++)
            {
                m_versionList.addRowData(*it);
            }
        }

        // Add in all the row data
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // Comms error (ObjectResolutionException)
        // - or possibly database error (non unique location keys in version data set)
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }
}

void DvaVersionsPage::setupListView(CListCtrl& list,
                                    DWORD columnCount,
                                    bool accountForScroll,
                                    DWORD format)
{
    list.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

    // Width is default width as seen in dialog editor
    int width = PaLayoutHelper::getListHeaderWidth(list, false);

    // add in excess amount
    width += LIST_WIDTH_EXCESS;

    int averageWidth = width / columnCount;

    for (DWORD nCol = 0; nCol < columnCount; nCol++)
    {
        // Keep inserting at the back (so the formatting works
        // - formatting is always left justified on first column no
        // matter what you specify)
        int nItem = list.InsertColumn(nCol, "", format);

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

    list.SetScrollPos(SB_HORZ, 0);
}

void DvaVersionsPage::OnDestroy()
{
    CPropertyPage::OnDestroy();

    try
    {
        // m_dvaVersionChangeSubscriber.removeListenerWindow(m_hWnd);
        m_dvaPublicVersionChangeSubscriber.removeListenerWindow(m_hWnd);
        m_dvaPrivateVersionChangeSubscriber.removeListenerWindow(m_hWnd);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // Cannot unsubscribe ad hoc message change listener
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }
}

afx_msg LRESULT DvaVersionsPage::OnDvaVersionUpdate(UINT wParam, LONG lParam)
{
    //    if (!m_dvaVersionChangeSubscriber.isConnectedListenerWindow(m_hWnd))
    //    {
    //        // Probably in the closing down stage, receiving last few messages
    //        // while unsubscribed - do NOT respond to this notification in this case
    //        return 0;
    //    }

    if (!m_dvaPublicVersionChangeSubscriber.isConnectedListenerWindow(m_hWnd))
    {
        // Probably in the closing down stage, receiving last few messages
        // while unsubscribed - do NOT respond to this notification in this case
        return 0;
    }

    if (!m_dvaPrivateVersionChangeSubscriber.isConnectedListenerWindow(m_hWnd))
    {
        // Probably in the closing down stage, receiving last few messages
        // while unsubscribed - do NOT respond to this notification in this case
        return 0;
    }

    try
    {
        if (CachedConfig::getInstance()->isAtOcc())
        {
            const TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecords allVersionData
                = m_dvaPublicVersionChangeSubscriber.getMessageData(wParam, m_hWnd);

            for (int i = 0; i < allVersionData.length(); i++)
            {
                m_versionList.updateRowData(allVersionData[i]);
            }
        }
        else
        {
            const TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecords allVersionData
                = m_dvaPrivateVersionChangeSubscriber.getMessageData(wParam, m_hWnd);

            for (int i = 0; i < allVersionData.length(); i++)
            {
                m_versionList.updateRowData(allVersionData[i]);
            }
        }

        // Send the updates to the relevant list control
        //        for (int i = 0; i < allVersionData.length(); i ++)
        //        {
        //            m_versionList.updateRowData(allVersionData[i]);
        //        }
    }
    catch (const TA_Base_Core::ValueNotSetException&)
    {
        // Also could mean this window not recognized by subscriber - adhoc won't be updated
        // But it really should be impossible for this to happen..
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }
    catch (const TA_Base_Core::InvalidPasPacketException&)
    {
        // Means one of the version keys didn't match existing key in list (corrupt data)
        PaErrorHandler::displayError(PaErrorHandler::DATABASE_ERROR);
    }

    STANDARD_PA_CATCH("DvaVersionsPage::OnDvaVersionUpdate", PaErrorHandler::UNSPECIFIED_ERROR);

    return 0;
}

void DvaVersionsPage::OnItemchangingDvaHeaderLst(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    // Only interested in state changes
    if (pNMListView->uChanged != LVIF_STATE)
    {
        return;
    }

    // Reject the change in state (no selections allowed on header controls)
    *pResult = TRUE;
}

void DvaVersionsPage::OnItemchangingDvaHeaderLst2(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    // Only interested in state changes
    if (pNMListView->uChanged != LVIF_STATE)
    {
        return;
    }

    // Reject the change in state (no selections allowed on header controls)
    *pResult = TRUE;
}
