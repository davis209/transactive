/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DvaListCtrl.cpp $
 * @author Sean Liew
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * Maintains a list control for DVA messages
 *
 */

#include "app/pa/PAManager/src/stdafx.h"

#include <limits>
#ifdef max
#undef max
#endif

#include "app/pa/PAManager/src/pamanager.h"
#include "app/pa/PAManager/src/DvaListCtrl.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "core/data_access_interface/src/DatabaseKey.h"
#include "app/pa/PAManager/src/LockControlRedraw.h"
//#include "app/pa/PAManager/src/SortedListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const long EMERGENCYFGCOLOR = RGB(255, 0, 0);

using namespace TA_IRS_Bus;

static int CALLBACK
SortAlphabetAscending(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    TA_IRS_Bus::StationDvaMessageRecord* record1 =
        reinterpret_cast<TA_IRS_Bus::StationDvaMessageRecord*>(lParam1);
    TA_IRS_Bus::StationDvaMessageRecord* record2 =
        reinterpret_cast<TA_IRS_Bus::StationDvaMessageRecord*>(lParam2);

    int result = stricmp(record1->m_label.c_str(), record2->m_label.c_str());

    if (0 == result)
    {
        result = strcmp(record1->m_label.c_str(), record2->m_label.c_str());
    }

    return result;
}

/////////////////////////////////////////////////////////////////////////////
// DvaListCtrl

DvaListCtrl::DvaListCtrl(bool sortingEnabled)
    :
    m_sortingEnabled(sortingEnabled),
    m_colSortingInfo(DVA_COL_MAX)
{
    // We want LVN_ITEMCHANGING notifications to be passed back to parent window
    // (and give it the opportunity to handle the notifications)
    setItemChangeRequestForwarding(TRUE);

    if (m_sortingEnabled)
    {
        // Sort columns based on label
        m_colSortingInfo.setCurrentSort(DVA_COL_LABEL, TA_Base_Bus::AutoSortListCtrl::ASCENDING);
        // Make label column the active sorting column
        m_colSortingInfo.activateSort(DVA_COL_LABEL);
    }
}

DvaListCtrl::~DvaListCtrl()
{
}

void DvaListCtrl::drawUnderline(int rowIndex, LPNMLVCUSTOMDRAW lplvcd)
{
    RECT rc;
    CString itemMsg = GetItemText(rowIndex, 0);   //TD17646
    CSize size;

    if (GetItemRect(lplvcd->nmcd.dwItemSpec, &rc, LVIR_LABEL))
    {
        DWORD lineColour = lplvcd->clrText;

        CDC dc;
        dc.Attach(lplvcd->nmcd.hdc);
        size = dc.GetTextExtent(itemMsg);    //TD17646
        CPen pen(PS_SOLID, 1, lineColour);
        CPen* oldPen = dc.SelectObject(&pen);

        int horzClearance = 2;
        int vertClearance = 3;

        dc.MoveTo(rc.left + horzClearance, rc.bottom - vertClearance);
        dc.LineTo(rc.left + horzClearance + size.cx, rc.bottom - vertClearance); //TD17646

        dc.SelectObject(oldPen);
        dc.Detach();
    }
}

void DvaListCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;

    if (CDDS_ITEMPOSTPAINT == lplvcd->nmcd.dwDrawStage)
    {
        TA_IRS_Bus::StationDvaMessageRecord* record =
            reinterpret_cast<TA_IRS_Bus::StationDvaMessageRecord*>(GetItemData(lplvcd->nmcd.dwItemSpec));

        // If this is a TIS enabled message, it needs to be underlined
        // It is not sufficient to simply provide an underlined font because
        // as defined in the MSDN documentation, selected items cannot
        // have a custom font defined by NM_CUSTOMDRAW
        if (0 != record && DvaListCtrlModel::isTisEnabled(*record))
        {
            // Underline the message
            drawUnderline(lplvcd->nmcd.dwItemSpec, lplvcd);
        }
    }

    // To make sure the selected item draws properly when focus is lost..
    ListCtrlSelNoFocus::OnCustomDraw(pNMHDR, pResult);

    // The ListCtrlSelNoFocus ensures each draw stage is handled
    if (CDDS_ITEMPREPAINT == lplvcd->nmcd.dwDrawStage)
    {
        // Always request post-paint, to allow opportunity to draw underline
        *pResult |= CDRF_NOTIFYPOSTPAINT;
    }
}

DWORD DvaListCtrl::getForegroundColourForItem(int nItem)
{
    TA_IRS_Bus::StationDvaMessageRecord* record =
        reinterpret_cast<TA_IRS_Bus::StationDvaMessageRecord*>(GetItemData(nItem));

    if (0 != record)
    {
        if (DvaListCtrlModel::isEmergencyMessage(record->m_type))
        {
            return EMERGENCYFGCOLOR;
        }
    }

    return ListCtrlSelNoFocus::getForegroundColourForItem(nItem);
}

void DvaListCtrl::CleanupItemData()
{
    for (int i = 0; i < GetItemCount(); ++i)
    {
        deleteItemData(i);
    }
}

BOOL DvaListCtrl::DeleteAllItems()
{
    CleanupItemData();
    return ListCtrlSelNoFocus::DeleteAllItems();
}

BOOL DvaListCtrl::DeleteItem(int nItem)
{
    deleteItemData(nItem);

    return ListCtrlSelNoFocus::DeleteItem(nItem);
}

TA_IRS_Bus::StationDvaMessageRecord* DvaListCtrl::getRecordFromIndex(int index) const
{
    if (index >= 0 && index < GetItemCount())
    {
        return reinterpret_cast<TA_IRS_Bus::StationDvaMessageRecord*>(GetItemData(index));
    }
    else
    {
        TA_ASSERT(FALSE, "Invalid index specified");
        return NULL;
    }
}

void DvaListCtrl::deleteItemData(int nItem)
{
    TA_IRS_Bus::StationDvaMessageRecord* record =
        reinterpret_cast<TA_IRS_Bus::StationDvaMessageRecord*>(GetItemData(nItem));

    if (record)
    {
        m_listModel.clearMapping(record->m_pKey, GetItemData(nItem));

        delete record;
        ListCtrlSelNoFocus::SetItemData(nItem, NULL); // 2B safe
    }
    else
    {
        // No record exists here (dummy entry)
    }
}

unsigned long DvaListCtrl::getSelectedMessageKey()
{
    if (GetSelectedCount() > 0)
    {
        int itemIndex = GetNextItem(-1, LVNI_SELECTED);

        // Invalid - no items selected
        if (itemIndex < 0)
        {
            return TA_Base_Core::DatabaseKey::getInvalidKey();
        }

        DWORD itemData = GetItemData(itemIndex);

        if (0 == itemData)
        {
            // This occurs during cleanup of a list (when
            // window receives notification that an item
            // has been deleted)
            return TA_Base_Core::DatabaseKey::getInvalidKey();
        }

        TA_IRS_Bus::StationDvaMessageRecord* pRecord =
            reinterpret_cast<TA_IRS_Bus::StationDvaMessageRecord*>(itemData);

        TA_ASSERT(0 != pRecord, "Null selection made");

        return pRecord->m_pKey;
    }
    else
    {
        // No selection made
        return TA_Base_Core::DatabaseKey::getInvalidKey();
    }
}

int DvaListCtrl::findSortedIdInsertionPoint(TA_IRS_Bus::StationDvaMessageRecord& record) const
{
    int i = 0;

    for (; i < GetItemCount(); i++)
    {
        TA_IRS_Bus::StationDvaMessageRecord* pRecord = getRecordFromIndex(i);

        if (record.m_id < pRecord->m_id)
        {
            // we can insert here (above this item) - our ID is smaller
            return i;
        }
    }

    // Insert right at end of list (our ID is greater than all others)
    return i;
}

BOOL DvaListCtrl::SetItemData(TA_IRS_Bus::StationDvaMessageRecord* record,
                              const unsigned long* existingKey,
                              bool sortNewItemsById)
{
    LOG_SCOPE("DvaListCtrl::SetItemData");

    TA_ASSERT(record != 0, "Record is null");

    if (0 == existingKey)
    {
        int insertionPoint = 0;

        if (sortNewItemsById)
        {
            insertionPoint = findSortedIdInsertionPoint(*record);
        }

        // Adding item in new
        int nextIndex = InsertItem(insertionPoint, record->m_label.c_str());
        LOG_DEBUG("SetItemData(): %s", nvps(*record));

        // This happens when adding the same item in twice (into selected messages list)...
        //TA_ASSERT(!m_listModel.doesMappingExist(record->m_pKey), "Mapping already exists?");

        internalCompleteItemUpdate(nextIndex, record);
    }
    else
    {
        // Find what entries in list we have to update - may be multiple
        // Each key could have multiple entries in list (each will have a unique
        // DWORD address associated with it though..)
        std::vector<int> items = findItemIndices(*existingKey);

        TA_IRS_Bus::StationDvaMessageRecord* recordToUse;

        // Now loop through all items - if there is more than 1, we need to
        // replicate record (and allocate more copies) - because we CANNOT
        // set objects with same pointer (they're all deleted on exit..)
        for (unsigned int i = 0; i < items.size(); i++)
        {
            if (i >= 1)
            {
                // Once pass the first element, continue re-allocating new record objects
                recordToUse = new TA_IRS_Bus::StationDvaMessageRecord();
                *recordToUse = *record;
            }
            else
            {
                // First insertion can use original record pointer
                recordToUse = record;
            }

            // Begin updating the item
            SetItemText(items[i], DVA_COL_LABEL, recordToUse->m_label.c_str());
            // Remember to use recordToUse - and not send in same record pointer more than once
            internalCompleteItemUpdate(items[i], recordToUse);
        }
    }

    return TRUE;
}

void DvaListCtrl::internalCompleteItemUpdate(int nItem, TA_IRS_Bus::StationDvaMessageRecord* record)
{
    // Retrieve old address of data
    DWORD oldDataAddress = GetItemData(nItem);

    if (0 != oldDataAddress)
    {
        // For correct memory cleanup, must delete item we're replacing
        // If an old address exists, means it must be deleted..
        deleteItemData(nItem);
    }

    // Track record key with associated item data
    // Note this must be done AFTER deleteItemData (which deletes mappings)
    m_listModel.updateMapping(record->m_pKey, reinterpret_cast<DWORD>(record));

    BOOL result = ListCtrlSelNoFocus::SetItemData(nItem, reinterpret_cast<DWORD>(record));

    updateCheckedState(nItem, record);
}

void DvaListCtrl::updateCheckedState(int nItem, TA_IRS_Bus::StationDvaMessageRecord* record)
{
    // We must update the state right at the end - because
    // the state change handler has no means of knowing where the
    // state change request originated from (user or internally) -
    // so the way it currently does this is checking if the internal
    // record state (from GetItemData) corresponds to the target state
    // If not - it knows it's a user state change
    if (0 != record &&
        DvaListCtrlModel::isEmergencyMessage(record->m_type))
    {
        SetCheck(nItem, true);
    }
    else
    {
        SetCheck(nItem, false);
    }
}

std::vector<int> DvaListCtrl::findItemIndices(unsigned long key)
{
    std::vector<int> itemArray;
    // Addresses of all records matching this key (using this
    // we need to them find their list indices)
    std::vector<DWORD> addresses = m_listModel.getMessageData(key);

    TA_ASSERT(addresses.size() > 0, "Couldn't find any matching items");

    for (int i = 0; i < GetItemCount(); i++)
    {
        if (std::find(addresses.begin(), addresses.end(), GetItemData(i)) !=
            addresses.end())
        {
            itemArray.push_back(i);
        }
    }

    TA_ASSERT(itemArray.size() > 0, "DvaListCtrl::findItemIndices: Index not found");

    return itemArray;
}

// TD 15349
bool DvaListCtrl::selectItemInList(int nItem)
{
    //TD18626 AVteam
    if (SetItemState(nItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED) == TRUE)
    {
        return true;
    }
    else
    {
        return false;
    }

    //TD18626 AVteam
}

// TD 15349

BEGIN_MESSAGE_MAP(DvaListCtrl, ListCtrlSelNoFocus)
    //{{AFX_MSG_MAP(DvaListCtrl)
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)

    ON_WM_DESTROY()
    ON_WM_ERASEBKGND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DvaListCtrl message handlers
void DvaListCtrl::OnDestroy()
{
    CleanupItemData();

    ListCtrlSelNoFocus::OnDestroy();
}

bool DvaListCtrl::refreshContents(Keys& messageKeys)
{
    m_originalKeyData = messageKeys;

    std::vector<TA_IRS_Bus::StationDvaMessageRecord*> records;
    bool noInvalidRecordsFound = true;

    records = m_listModel.fetchLatestRecords(messageKeys);

    LockControlRedraw redrawLock(*this);

    DeleteAllItems();

    for (std::vector<TA_IRS_Bus::StationDvaMessageRecord*>::iterator it = records.begin();
         it != records.end();
         it++)
    {
        if (0 == (*it))
        {
            // Add in a dummy entry to notify user a deleted item exists
            // In this way, users always know when a now deleted item
            // exists in selection (rather than hiding)
            // Messages can be deleted from Maintenance terminal
            int nItem = InsertItem(0,
                                   CachedConfig::getInstance()->getDeletedStationDvaMessageName().c_str());

            noInvalidRecordsFound = false;
        }
        else
        {
            SetItemData(*it);
        }
    }

    return noInvalidRecordsFound;
}

bool DvaListCtrl::doesMessageKeyExist(unsigned long messageKey)
{
    return m_listModel.doesMappingExist(messageKey);
}

bool DvaListCtrl::doAllMessageKeysExist(const DvaMessageUpdateData& messageKeys)
{
    for (DvaMessageUpdateData::const_iterator itr = messageKeys.begin();
         itr != messageKeys.end();
         itr++)
    {
        if (!doesMessageKeyExist(*itr))
        {
            // This message key not present, so they can't all be
            return false;
        }
    }

    // ALl keys must have been present
    return true;
}

bool DvaListCtrl::doesMessageKeyExist(const DvaMessageUpdateData& messageKeys)
{
    for (DvaMessageUpdateData::const_iterator itr = messageKeys.begin();
         itr != messageKeys.end();
         itr++)
    {
        if (doesMessageKeyExist(*itr))
        {
            // There is at least one match
            return true;
        }
    }

    // No matches
    return false;
}

bool DvaListCtrl::collectMatchingKeys(const DvaMessageUpdateData& inputKeys,
                                      DvaMessageUpdateData& modifiedKeys,
                                      DvaMessageUpdateData& deletedKeys,
                                      bool* typeChanges,
                                      bool* labelChanges)
{
    // Clear the output variables straight off
    modifiedKeys.clear();
    deletedKeys.clear();

    bool matchFound = false;

    if (0 != typeChanges)
    {
        *typeChanges = false;
    }

    if (0 != labelChanges)
    {
        *labelChanges = false;
    }

    for (int i = 0; i < GetItemCount(); i++)
    {
        TA_IRS_Bus::StationDvaMessageRecord* oldRecord = getRecordFromIndex(i);

        unsigned long key = oldRecord->m_pKey;

        if (inputKeys.find(key) == inputKeys.end())
        {
            // This particular record doesn't correspond to input key, may ignore
            continue;
        }

        // One of input keys matched a local key
        matchFound = true;

        // Obtain the latest record from cached maps
        TA_IRS_Bus::StationDvaMessageRecord record;

        try
        {
            record = TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(key);
        }
        catch (const TA_Base_Core::CachedMappingNotFoundException&)
        {
            // Must handle special case of deleted messages (physically deleted)
            if (0 != typeChanges)
            {
                *typeChanges |= true;
            }

            if (0 != labelChanges)
            {
                *labelChanges |= true;
            }

            deletedKeys.insert(key);
            continue;
        }

        // Update status of whether we encountered any type or label changes
        if (0 != typeChanges)
        {
            *typeChanges |= (oldRecord->m_type != record.m_type);
        }

        if (0 != labelChanges)
        {
            *labelChanges |= (oldRecord->m_label != record.m_label);
        }

        // Determine which group to place message in (based on deleted state)
        if (DvaListCtrlModel::isDeletedMessage(record.m_type))
        {
            deletedKeys.insert(key);
        }
        else
        {
            // Otherwise this message exists, but not deleted
            modifiedKeys.insert(key);
        }
    }

    return matchFound;
}

void DvaListCtrl::processDvaMessageUpdates(const DvaMessageUpdateData& updates)
{
    if (!doesMessageKeyExist(updates))
    {
        // If false returned, none of the input set of items exist locally
        // So we just ignore it (normal control flow)
        return;
    }

    // We can simply refresh the data for this item, using item straight from cachedMaps
    // because we know cachedMaps always to be up to date
    try
    {
        refreshItemsFromCachedMaps(updates);
    }
    catch (const TA_Base_Core::CachedMappingNotFoundException&)
    {
        // The item in the list control will not be updated
        PaErrorHandler::displayError(PaErrorHandler::DVA_MESSAGE_UPDATE_FAILURE);
        return;
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // The item in the list control will not be updated
        PaErrorHandler::displayError(PaErrorHandler::DVA_MESSAGE_UPDATE_FAILURE);
        return;
    }
}

/*
void DvaListCtrl::addItemFromCachedMaps(unsigned long messageKey)
{
    TA_IRS_Bus::StationDvaMessageRecord* clonedRecord = new TA_IRS_Bus::StationDvaMessageRecord;

    // Allow CachedMappingException to propogate
    *clonedRecord = TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(messageKey);

    SetItemData(clonedRecord);
}
*/

void DvaListCtrl::refreshItemsFromCachedMaps(const DvaMessageUpdateData& updateKeys)
{
    LOG_SCOPE("DvaListCtrl::refreshItemsFromCachedMaps");

    for (DvaMessageUpdateData::const_iterator itr = updateKeys.begin();
         itr != updateKeys.end();
         itr++)
    {
        if (!doesMessageKeyExist(*itr))
        {
            // This particular key not within this control
            continue;
        }

        TA_IRS_Bus::StationDvaMessageRecord* clonedRecord = new StationDvaMessageRecord;

        try
        {
            *clonedRecord = TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(*itr);

            // Record being inserted should have same key as one being replaced (if already exists)
            if ((*itr) != clonedRecord->m_pKey)
            {
                TA_THROW(TA_Base_Core::CachedMappingNotFoundException());
            }
            else
            {
                // This function will take care of destroying the old message (and replacing with new)
                unsigned long key = (*itr);
                SetItemData(clonedRecord, &key);
            }
        }
        catch (const TA_Base_Core::CachedMappingNotFoundException&)
        {
            delete clonedRecord;
            clonedRecord = 0;

            // .. clearItemWithKey
            // Item no longer exists in cached maps
            std::vector<int> items = findItemIndices(*itr);

            for (int i = 0; i < (int)items.size(); ++i)
            {
                int nItem = items.at(i);

                // Clean up the old record pointer memory, and set text to invalid
                if (0 != getRecordFromIndex(nItem))
                {
                    deleteItemData(nItem);
                    SetItemText(nItem, 0, CachedConfig::getInstance()->getDeletedStationDvaMessageName().c_str());
                }
            }
        }
    }

    return;
}

/*
// This function probably doesn't work (iterator not advanced, so will
// probably be invalidated)
void DvaListCtrl::removeAllMatchingDvaMessages(unsigned long messageKey)
{
    // There should be exactly one message corresponding to messageKey
    std::vector<int> itemIndices = findItemIndices(messageKey);
    int lastIndex = std::numeric_limits<int>::max();

    // Delete the items in reverse order (critical to avoid shuffling / invalidation
    // of elements below the indice, for each deletion call)
    for (std::vector<int>::reverse_iterator itr = itemIndices.rbegin();
                itr != itemIndices.rend();
                itr ++)
    {
        // Ensure when array traversed in reverse, item indices are monotonically decreasing
        // (this should hold, findItemIndices is supposed to return in ascending order)
        TA_ASSERT(lastIndex > (*itr), "itemIndices are not in ascending order");
        lastIndex = (*itr);

        DeleteItem(*itr);
    }
}
*/

void DvaListCtrl::removeSingleDvaMessage(unsigned long messageKey)
{
    // There should be exactly one message corresponding to messageKey
    std::vector<int> itemIndices = findItemIndices(messageKey);

    TA_ASSERT(1 == itemIndices.size(), "Incorrect number of items found");

    DeleteItem(itemIndices[0]);
}

BOOL DvaListCtrl::OnEraseBkgnd(CDC* pDC)
{
    // Do not erase background
    //return TRUE;
    return ListCtrlSelNoFocus::OnEraseBkgnd(pDC);
}

LRESULT DvaListCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (m_sortingEnabled)
    {
        TA_Base_Bus::AutoSortListCtrl::WindowProc(message, wParam, lParam, *this, m_colSortingInfo);
    }

    return ListCtrlSelNoFocus::WindowProc(message, wParam, lParam);
}

void DvaListCtrl::PreSubclassWindow()
{
    ListCtrlSelNoFocus::PreSubclassWindow();

    m_fixedHeaderCtrl.subclassHeader(GetHeaderCtrl());
}
