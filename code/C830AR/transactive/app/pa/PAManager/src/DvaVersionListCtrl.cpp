/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DvaVersionListCtrl.cpp $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * Helper class for displaying DVA Message data
 *
 */

#include "app/pa/PAManager/src/stdafx.h"
#include "app/pa/PAManager/src/pamanager.h"
#include "app/pa/PAManager/src/DvaVersionListCtrl.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "core/exceptions/src/PasExceptions.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "app/pa/PAManager/src/LockControlRedraw.h"
#include "app/pa/PAManager/src/CachedConfig.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_IRS_Bus;

DvaVersionListCtrl::DvaVersionListCtrl()
    : m_excessWidth(0)
{
}

DvaVersionListCtrl::~DvaVersionListCtrl()
{
}

BEGIN_MESSAGE_MAP(DvaVersionListCtrl, TA_Base_Bus::HighlightItemListCtrl)
    //{{AFX_MSG_MAP(DvaVersionListCtrl)
    ON_WM_HSCROLL()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void DvaVersionListCtrl::setRowText(int nItem, const DvaVersionRecord& rowData)
{
    // Station name in first column
    SetItemText(nItem, 0, getStationName(rowData.m_locationKey).c_str());

    // When running at OCC, use Set A (public) data
    // At stations and depot, use Set B/D (private) data

    if (CachedConfig::getInstance()->isAtOcc())
    {
        // Show public sets in columns 1-7 at OCC
        SetItemText(nItem, 1, (rowData.m_publicAdhoc1).c_str());
        SetItemText(nItem, 2, (rowData.m_publicAdhoc2).c_str());
        SetItemText(nItem, 3, (rowData.m_publicAdhoc3).c_str());
        SetItemText(nItem, 4, (rowData.m_publicAdhoc4).c_str());
        SetItemText(nItem, 5, (rowData.m_publicAdhoc5).c_str());
        SetItemText(nItem, 6, (rowData.m_publicAdhoc6).c_str());
        SetItemText(nItem, 7, (rowData.m_publicPreRecorded).c_str());
    }
    else
    {
        // Show private sets in columns 1-7 at depot and stations
        SetItemText(nItem, 1, (rowData.m_privateAdhoc1).c_str());
        SetItemText(nItem, 2, (rowData.m_privateAdhoc2).c_str());
        SetItemText(nItem, 3, (rowData.m_privateAdhoc3).c_str());
        SetItemText(nItem, 4, (rowData.m_privateAdhoc4).c_str());
        SetItemText(nItem, 5, (rowData.m_privateAdhoc5).c_str());
        SetItemText(nItem, 6, (rowData.m_privateAdhoc6).c_str());
        SetItemText(nItem, 7, (rowData.m_privatePreRecorded).c_str());
    }

    // Data is display in context sensitive manner in first 7 columns
    /*
    // The private sets are shown in remaining columns
    SetItemText(nItem, 8, (rowData.m_privateAdhoc1).c_str());
    SetItemText(nItem, 9, (rowData.m_privateAdhoc2).c_str());
    SetItemText(nItem, 10, (rowData.m_privateAdhoc3).c_str());
    SetItemText(nItem, 11, (rowData.m_privateAdhoc4).c_str());
    SetItemText(nItem, 12, (rowData.m_privateAdhoc5).c_str());
    SetItemText(nItem, 13, (rowData.m_privateAdhoc6).c_str());
    SetItemText(nItem, 14, (rowData.m_privatePreRecorded).c_str());
    */
}

void DvaVersionListCtrl::setRowText(int nItem, const DvaPublicVersionRecord& rowData)
{
    // Station name in first column
    SetItemText(nItem, 0, getStationName(rowData.m_locationKey).c_str());

    // When running at OCC, use Set A (public) data
    // At stations and depot, use Set B/D (private) data

    SetItemText(nItem, 1, (rowData.m_publicAdhoc1).c_str());
    SetItemText(nItem, 2, (rowData.m_publicAdhoc2).c_str());
    SetItemText(nItem, 3, (rowData.m_publicAdhoc3).c_str());
    SetItemText(nItem, 4, (rowData.m_publicAdhoc4).c_str());
    SetItemText(nItem, 5, (rowData.m_publicAdhoc5).c_str());
    SetItemText(nItem, 6, (rowData.m_publicAdhoc6).c_str());
    SetItemText(nItem, 7, (rowData.m_publicPreRecorded).c_str());
}

void DvaVersionListCtrl::setRowText(int nItem, const DvaPrivateVersionRecord& rowData)
{
    // Station name in first column
    SetItemText(nItem, 0, getStationName(rowData.m_locationKey).c_str());

    // When running at OCC, use Set A (public) data
    // At stations and depot, use Set B/D (private) data

    // Show private sets in columns 1-7 at depot and stations
    SetItemText(nItem, 1, (rowData.m_privateAdhoc1).c_str());
    SetItemText(nItem, 2, (rowData.m_privateAdhoc2).c_str());
    SetItemText(nItem, 3, (rowData.m_privateAdhoc3).c_str());
    SetItemText(nItem, 4, (rowData.m_privateAdhoc4).c_str());
    SetItemText(nItem, 5, (rowData.m_privateAdhoc5).c_str());
    SetItemText(nItem, 6, (rowData.m_privateAdhoc6).c_str());
    SetItemText(nItem, 7, (rowData.m_privatePreRecorded).c_str());
}

//sequence<octet, 6> DvaVersionListCtrl::

void DvaVersionListCtrl::addRowData(const DvaVersionRecord& rowData)
{
    // Item shouldn't already exist in list if adding
    int existingIndex = findVersionIndex(rowData.m_locationKey);

    if (existingIndex >= 0)
    {
        TA_THROW(TA_Base_Core::InvalidPasPacketException("Row with same key already exists"));
    }

    // Insert item at front (so last item added corresponds to top item)
    int nItem = InsertItem(0, getStationName(rowData.m_locationKey).c_str());

    // Update entire row of data
    setRowText(nItem, rowData);

    SetItemData(nItem, rowData.m_locationKey);
}

void DvaVersionListCtrl::addRowData(const DvaPublicVersionRecord& rowData)
{
    // Item shouldn't already exist in list if adding
    int existingIndex = findVersionIndex(rowData.m_locationKey);

    if (existingIndex >= 0)
    {
        TA_THROW(TA_Base_Core::InvalidPasPacketException("Row with same key already exists"));
    }

    // Insert item at front (so last item added corresponds to top item)
    int nItem = InsertItem(0, getStationName(rowData.m_locationKey).c_str());

    // Update entire row of data
    setRowText(nItem, rowData);

    SetItemData(nItem, rowData.m_locationKey);
}

void DvaVersionListCtrl::addRowData(const DvaPrivateVersionRecord& rowData)
{
    // Item shouldn't already exist in list if adding
    int existingIndex = findVersionIndex(rowData.m_locationKey);

    if (existingIndex >= 0)
    {
        TA_THROW(TA_Base_Core::InvalidPasPacketException("Row with same key already exists"));
    }

    // Insert item at front (so last item added corresponds to top item)
    int nItem = InsertItem(0, getStationName(rowData.m_locationKey).c_str());

    // Update entire row of data
    setRowText(nItem, rowData);

    SetItemData(nItem, rowData.m_locationKey);
}

void DvaVersionListCtrl::updateRowData(const TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecord& rowData)
{
    DvaVersionRecord convertedRowData;

    convertedRowData.m_locationKey = rowData.m_locationKey;

    convertedRowData.m_privateAdhoc1 = convertVersionToString(rowData.m_privateAdhoc1);
    convertedRowData.m_privateAdhoc2 = convertVersionToString(rowData.m_privateAdhoc2);
    convertedRowData.m_privateAdhoc3 = convertVersionToString(rowData.m_privateAdhoc3);
    convertedRowData.m_privateAdhoc4 = convertVersionToString(rowData.m_privateAdhoc4);
    convertedRowData.m_privateAdhoc5 = convertVersionToString(rowData.m_privateAdhoc5);
    convertedRowData.m_privateAdhoc6 = convertVersionToString(rowData.m_privateAdhoc6);
    convertedRowData.m_privatePreRecorded = convertVersionToString(rowData.m_privatePreRecorded);

    convertedRowData.m_publicAdhoc1 = convertVersionToString(rowData.m_publicAdhoc1);
    convertedRowData.m_publicAdhoc2 = convertVersionToString(rowData.m_publicAdhoc2);
    convertedRowData.m_publicAdhoc3 = convertVersionToString(rowData.m_publicAdhoc3);
    convertedRowData.m_publicAdhoc4 = convertVersionToString(rowData.m_publicAdhoc4);
    convertedRowData.m_publicAdhoc5 = convertVersionToString(rowData.m_publicAdhoc5);
    convertedRowData.m_publicAdhoc6 = convertVersionToString(rowData.m_publicAdhoc6);
    convertedRowData.m_publicPreRecorded = convertVersionToString(rowData.m_publicPreRecorded);

    updateRowData(convertedRowData);
}

void DvaVersionListCtrl::updateRowData(const TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecord& rowData)
{
    DvaPublicVersionRecord convertedRowData;

    convertedRowData.m_locationKey = rowData.m_locationKey;

    convertedRowData.m_publicAdhoc1 = convertVersionToString(rowData.m_publicAdhoc1);
    convertedRowData.m_publicAdhoc2 = convertVersionToString(rowData.m_publicAdhoc2);
    convertedRowData.m_publicAdhoc3 = convertVersionToString(rowData.m_publicAdhoc3);
    convertedRowData.m_publicAdhoc4 = convertVersionToString(rowData.m_publicAdhoc4);
    convertedRowData.m_publicAdhoc5 = convertVersionToString(rowData.m_publicAdhoc5);
    convertedRowData.m_publicAdhoc6 = convertVersionToString(rowData.m_publicAdhoc6);
    convertedRowData.m_publicPreRecorded = convertVersionToString(rowData.m_publicPreRecorded);

    updateRowData(convertedRowData);
}

void DvaVersionListCtrl::updateRowData(const TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecord& rowData)
{
    DvaPrivateVersionRecord convertedRowData;

    convertedRowData.m_locationKey = rowData.m_locationKey;

    convertedRowData.m_privateAdhoc1 = convertVersionToString(rowData.m_privateAdhoc1);
    convertedRowData.m_privateAdhoc2 = convertVersionToString(rowData.m_privateAdhoc2);
    convertedRowData.m_privateAdhoc3 = convertVersionToString(rowData.m_privateAdhoc3);
    convertedRowData.m_privateAdhoc4 = convertVersionToString(rowData.m_privateAdhoc4);
    convertedRowData.m_privateAdhoc5 = convertVersionToString(rowData.m_privateAdhoc5);
    convertedRowData.m_privateAdhoc6 = convertVersionToString(rowData.m_privateAdhoc6);
    convertedRowData.m_privatePreRecorded = convertVersionToString(rowData.m_privatePreRecorded);

    updateRowData(convertedRowData);
}

void DvaVersionListCtrl::updateRowData(const DvaVersionRecord& rowData)
{
    // To update the item, it must exist in list already
    int existingIndex = findVersionIndex(rowData.m_locationKey);

    if (existingIndex < 0)
    {
        TA_THROW(TA_Base_Core::InvalidPasPacketException("Item to update not present in list"));
    }

    // Refresh the data in this row
    setRowText(existingIndex, rowData);
}

void DvaVersionListCtrl::updateRowData(const DvaPublicVersionRecord& rowData)
{
    // To update the item, it must exist in list already
    int existingIndex = findVersionIndex(rowData.m_locationKey);

    if (existingIndex < 0)
    {
        TA_THROW(TA_Base_Core::InvalidPasPacketException("Item to update not present in list"));
    }

    // Refresh the data in this row
    setRowText(existingIndex, rowData);
}

void DvaVersionListCtrl::updateRowData(const DvaPrivateVersionRecord& rowData)
{
    // To update the item, it must exist in list already
    int existingIndex = findVersionIndex(rowData.m_locationKey);

    if (existingIndex < 0)
    {
        TA_THROW(TA_Base_Core::InvalidPasPacketException("Item to update not present in list"));
    }

    // Refresh the data in this row
    setRowText(existingIndex, rowData);
}

int DvaVersionListCtrl::findVersionIndex(unsigned long locationKey) const
{
    LVFINDINFO searchParams;
    searchParams.flags = LVFI_PARAM;
    searchParams.lParam = locationKey;

    return FindItem(&searchParams);
}

std::string DvaVersionListCtrl::getStationName(unsigned long locationKey) const
{
    try
    {
        std::string name = TA_IRS_Bus::CachedMaps::getInstance()->getLocationNameFromKey(locationKey);

        return name;
    }
    catch (const TA_Base_Core::CachedMappingNotFoundException&)
    {
        return std::string("UNKNOWN");
    }
}

std::string DvaVersionListCtrl::convertVersionToString(const char* version)
{
    return std::string(version);
}

void DvaVersionListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);

    int scrollPos = GetScrollPos(SB_HORZ);

    int max = GetScrollLimit(SB_HORZ);

    float proportion = float(scrollPos) / float(max);
    CRect rc;
    GetClientRect(rc);

    // Determine point we should scroll up to
    // It will be capped at 'max' - when we scroll to 'max' full right side in view
    int absWidthDistance = (int) (proportion * max);

    for (unsigned int i = 0; i < m_hScrollChildren.size(); i ++)
    {
        CListCtrl& child = *(m_hScrollChildren[i].listControl);

        // absWidthDistance is our target
        int relativeOffset = absWidthDistance - m_hScrollChildren[i].wOffset;
        m_hScrollChildren[i].wOffset += relativeOffset;

        CSize scrollAmount(relativeOffset, 0);

        child.Scroll(scrollAmount);
    }
}

void DvaVersionListCtrl::addHorizontalScrollingChild(CListCtrl& child)
{
    ListContainer lc;
    lc.listControl = &child;
    lc.wOffset = 0;
    m_hScrollChildren.push_back(lc);
}
