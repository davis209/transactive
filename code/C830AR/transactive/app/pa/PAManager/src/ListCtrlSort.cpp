/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/ListCtrlSort.cpp $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
// ListCtrlSort.cpp: implementation of the CListCtrlSort class.
//
//////////////////////////////////////////////////////////////////////

// song toan++
// TD 10316
#include "app/pa/PAManager/src/stdafx.h"
#include "app/pa/PAManager/src/pamanager.h"
#include "bus/pa/cachedmaps/src/CachedMapDefinitions.h"
#include "app/pa/PAManager/src/ListCtrlSort.h"
#include "app/pa/PAManager/src/PaLayoutHelper.h"
// ++song toan
// TD 10316

#ifdef _DEBUG
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
    #define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// song toan++
// TD 10316
// Note: The width of Zone column will be determined at runtime
static const int STATION_LOC_WIDTH_PIXELS = 50;
static const int STATUS_WIDTH_PIXELS      = 110;
// jeffrey++ TD8850
//static const PRIORITY_WIDTH_PIXELS    = 65;
static const int PRIORITY_WIDTH_PIXELS    = 75;
// ++jeffrey TD8850

static const CString STATION_LOC_HEADER  = "Loc";
static const CString STATION_ZONE_HEADER = "Zone";
static const CString STATUS_HEADER       = "Status";
static const CString PRIORITY_HEADER     = "Priority";

// song toan++
// TD 10316

CListCtrlSort::CListCtrlSort( bool isOCC, bool isFailure )
    :
    m_bIsOCC(isOCC),
    m_bIsFailure(isFailure),
    m_colSortingInfo(4)
    // ++song toan
    // TD 10316
{
    // We want LVN_ITEMCHANGING notifications to be passed back to parent window
    // (and give it the opportunity to handle the notifications)
    setItemChangeRequestForwarding(TRUE);

    // song toan++
    // TD 10316

    // determine column indexes
    m_iColLoc = 0;
    m_iColZone = isOCC ? 1 : 0;
    m_iColStatus = m_iColZone + 1;
    m_iColPriority = m_iColStatus + 1;

    m_colSortingInfo.setCallback(this);

    // define sorting semantic for all columns
    if ( m_bIsOCC )
        m_colSortingInfo.defineSortingSemantic( m_iColLoc, TA_Base_Bus::AutoSortListCtrl::BY_CALLBACK);

    m_colSortingInfo.defineSortingSemantic( m_iColZone, TA_Base_Bus::AutoSortListCtrl::BY_CALLBACK);
    m_colSortingInfo.defineSortingSemantic( m_iColStatus, TA_Base_Bus::AutoSortListCtrl::BY_CALLBACK);

    if ( m_bIsFailure )
        m_colSortingInfo.defineSortingSemantic( m_iColPriority, TA_Base_Bus::AutoSortListCtrl::BY_CALLBACK);

    // Make 1st column the active sorting one
    m_colSortingInfo.setCurrentSort( m_bIsOCC ? m_iColLoc : m_iColZone, TA_Base_Bus::AutoSortListCtrl::ASCENDING );
    m_colSortingInfo.activateSort( m_bIsOCC ? m_iColLoc : m_iColZone );

    // ++song toan
    // TD 10316
}

CListCtrlSort::~CListCtrlSort()
{
}

LRESULT CListCtrlSort::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    // ++song toan
    // TD 10316
    TA_Base_Bus::AutoSortListCtrl::WindowProc(message, wParam, lParam, *this, m_colSortingInfo);
    // ++song toan
    // TD 10316
    return ListCtrlSelNoFocus::WindowProc(message, wParam, lParam);
}

/*
void CListCtrlSort::PreSubclassWindow()
{
    ListCtrlSelNoFocus::PreSubclassWindow();

    m_fixedHeaderCtrl.subclassHeader(GetHeaderCtrl());
}
*/
// song toan++
// TD 10316

BOOL CListCtrlSort::DeleteAllItems()
{
    int i = 0;

    while ( GetItemCount() > 0)
    {
        // get the item data
        TA_IRS_Bus::PaZoneRecord* itemData = reinterpret_cast<TA_IRS_Bus::PaZoneRecord*>(GetItemData(0));
        delete itemData;
        // delete the item
        DeleteItem(0);
    }

    return CListCtrl::DeleteAllItems();
}

//-----------------------------------------------------------------------------
void CListCtrlSort::setupReportWindowList()
{
    // Determine total width, accounting for scroll bar
    // jeffrey++ TD8850
    //int headerWidth = PaLayoutHelper::getListHeaderWidth(*this, true );
    int headerWidth = PaLayoutHelper::getListHeaderWidth(*this, false );
    // ++jeffrey TD8850

    // song toan++
    // TD 10316
    //
    // First column does not display combined station / zone string anymore.
    // Below is a summary of possible columns of both Progress/Failure list controls
    // at different locations
    //
    //        \  List|                      |
    // Location\ con-|       Progress       |            Failure
    //          \trol|                      |
    // ----------\---+----------------------+-------------------------------
    // OCC           | Location,Zone,Status | Location,Zone,Status,Priority
    // --------------+----------------------+-------------------------------
    // Others        |          Zone,Status |          Zone,Status,Priority

    int remainWidth = headerWidth - STATUS_WIDTH_PIXELS;

    if ( m_bIsFailure )
    {
        // priority column is only at failures list
        remainWidth -= PRIORITY_WIDTH_PIXELS;
    }

    if (m_bIsOCC)
    {
        // Location column is available at OCC
        remainWidth -= STATION_LOC_WIDTH_PIXELS;
    }

    int stationZoneWidthPixels = remainWidth;

    if ( m_bIsOCC )
    {
        // Location is shown at first column of both lists
        InsertColumn(   m_iColLoc,
                        STATION_LOC_HEADER,
                        LVCFMT_LEFT,
                        STATION_LOC_WIDTH_PIXELS );
    }

    InsertColumn(   m_iColZone,
                    STATION_ZONE_HEADER,
                    LVCFMT_LEFT,
                    stationZoneWidthPixels);

    // Status header
    InsertColumn(   m_iColStatus,
                    STATUS_HEADER,
                    LVCFMT_LEFT,
                    STATUS_WIDTH_PIXELS);

    if (m_bIsFailure)
    {
        // Priority shown on end of failures list,
        // it has one additional column..
        InsertColumn(   m_iColPriority,
                        PRIORITY_HEADER,
                        LVCFMT_LEFT,
                        PRIORITY_WIDTH_PIXELS);
    }

    // ++AZ TES 9613, tooltips set in ListCtrlSelNoFocus
    //    SetExtendedStyle( GetExtendedStyle() | LVS_EX_INFOTIP );
    // AZ++
}

//-----------------------------------------------------------------------------
std::string CListCtrlSort::getItemColumnLabel( const TA_IRS_Bus::PaZoneRecord* pZoneRecord, int iColumn )
{
    LVFINDINFO lvFind;
    lvFind.flags = LVFI_PARAM;
    lvFind.lParam = (LPARAM) pZoneRecord;
    int iItem = FindItem(&lvFind );

    const int MAX_LENGTH = 128;
    char szStatus[MAX_LENGTH];

    LVITEM item;
    item.mask = LVIF_TEXT;
    item.iItem = iItem;
    item.iSubItem = iColumn;
    item.pszText = szStatus;
    item.cchTextMax = MAX_LENGTH;
    GetItem(&item);

    return std::string(szStatus);
}

//-----------------------------------------------------------------------------
std::string CListCtrlSort::getStatusStr( const TA_IRS_Bus::PaZoneRecord* pZoneRecord )
{
    return getItemColumnLabel( pZoneRecord, m_iColStatus );
}

//-----------------------------------------------------------------------------
std::string CListCtrlSort::getPriorityStr( const TA_IRS_Bus::PaZoneRecord* pZoneRecord )
{
    return getItemColumnLabel( pZoneRecord, m_iColPriority );
}

//-----------------------------------------------------------------------------
int CListCtrlSort::sortCallback(LPARAM lParam1,
                                LPARAM lParam2,
                                int columnIndex,
                                TA_Base_Bus::AutoSortListCtrl::ECurrentSort currentSort)
{
    TA_IRS_Bus::PaZoneRecord* first = reinterpret_cast<TA_IRS_Bus::PaZoneRecord*>(lParam1);
    TA_IRS_Bus::PaZoneRecord* second = reinterpret_cast<TA_IRS_Bus::PaZoneRecord*>(lParam2);

    if (currentSort != TA_Base_Bus::AutoSortListCtrl::ASCENDING)
    {
        // descending
        std::swap( first, second );
    }

    if ( m_bIsOCC )
    {
        if ( columnIndex == m_iColLoc )
        {
            // geographical order
            int ret = first->m_orderId - second->m_orderId;

            if (ret == 0)
            {
                // same location -> order by zone name
                ret = first->m_label.compare(second->m_label);
            }

            return ret;
        }
    }

    if ( columnIndex == m_iColZone )
    {
        int ret = first->m_label.compare(second->m_label);

        if (ret == 0)
        {
            // same zone name -> order by location
            ret = first->m_orderId - second->m_orderId;
        }

        return ret;
    }
    else if ( columnIndex == m_iColStatus )
    {
        return ( getStatusStr(first).compare(getStatusStr(second)) );
    }
    else if (columnIndex == m_iColPriority )
    {
        return ( getPriorityStr(first).compare(getPriorityStr(second)) );
    }
    else
    {
        TA_ASSERT(false, "Unsupported column given to sort by callback");
    }

    // ++song toan
    // Bug #276

    return 0;
}

// song toan++
// TD 10316
