/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/DvaListCtrlTestCases.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma warning (disable : 4786 4290)

#include "StdAfx.h"
#include "DvaListCtrlTestCases.h"
#include "common.h"
#include "app/pa/PAManager/src/DvaListCtrl.h"

#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "bus/pa/cachedmaps/src/PaStationDvaMessageCachedMap.h"

#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/TaAssert.h"
#include "bus/mfc_extensions/src/selection_helpers/SelectionHelper.h"


#include "WindowStub.h"

using namespace TA_IRS_Bus;

const unsigned long INVALID_KEY = (unsigned long)(-2);
std::set<void*>  DvaListCtrlTestCases::s_expectedDeletions;

/////////////////////////////////////////////////
//
// refreshContents Tests
//
/////////////////////////////////////////////////
void DvaListCtrlTestCases::testRefreshContents()
{        
    int i;     
    enum { KEYCOUNT = 6 };
          
    Keys keys = getKeysInRange(0, KEYCOUNT);
    
    Assert(m_dvaListCtrl->refreshContents(keys));
    m_listCtrlWindowStub->processMsgQueue();

    // Confirm elements in list correspond to what we passed in (note: may not be ordered)
    Keys keysInList = getKeysInListCtrl();
    
    // Confirms lists contain exactly same set of keys (order may be different)
    Assert(destroyAllMatches(keys, keysInList));

    keys.push_back(INVALID_KEY);     // Now send through invalid key
    // Confirm function returns false when a dud value sent through
    Assert(!m_dvaListCtrl->refreshContents(keys));
    m_listCtrlWindowStub->processMsgQueue();

    bool nullCount = 0;;
    // Confirm that this new element is stored as a null itemdata somewhere in the list
    for (i = 0; i < m_dvaListCtrl->GetItemCount(); i ++)
    {
        if (0 == m_dvaListCtrl->getRecordFromIndex(i))
        {
            nullCount ++;
        }
    }

    Assert(1 == nullCount);

    for (int iteration = 0; iteration < 2; iteration ++)
    {
        // Now refresh with an overlapping set of keys, and confirm all goes through OK
        // (old keys get deleted etc)
        int start = KEYCOUNT / 2;
        keys = getKeysInRange(start, start + KEYCOUNT);
        
        Assert(m_dvaListCtrl->refreshContents(keys));
        m_listCtrlWindowStub->processMsgQueue();
    
        // Gather the new key data
        keysInList = getKeysInListCtrl();

        // Confirms lists contain exactly same set of keys (order may be different)
        Assert(destroyAllMatches(keys, keysInList));

        // We do two iterations so we can throw in this DeleteAllItems call
        m_dvaListCtrl->DeleteAllItems();
        m_listCtrlWindowStub->processMsgQueue();
    }         
}

/////////////////////////////////////////////////
//
// processDvaMessageUpdates Tests
//
/////////////////////////////////////////////////
void DvaListCtrlTestCases::testProcessDvaMessageUpdates_uniqueItems()
{
    UINT i;
    enum { SET_A = 5, SET_B = 10 };

    Keys setA = getKeysInRange(0, SET_A);
    Keys setB = getKeysInRange(SET_A + 1, SET_B);

    // Fill out list within initial data
    m_dvaListCtrl->refreshContents(setA);
    m_listCtrlWindowStub->processMsgQueue();
    m_listCtrlWindowStub->reset();

    PaStationDvaMessageCachedMap& map = CachedMaps::getInstance()->getPaStationDvaMessageCachedMap();
        
    // Confirm DVA Messages stored in DVA List control match the cached map
    confirmListCtrlContentsMatchCachedMap();

    // Change all the labels to something different
    for (i = 0; i < SET_A; i ++)
    {
        LockedPacket<StationDvaMessageRecord> lock = map.getConfigurableCachedDbRecordFromKey(getMessages()[i].m_pKey);
        std::ostringstream label;
        label << "Changed Label:" << i;
        lock.getMutableData().m_label = label.str();
    }
    
    // Confirm all the records are now mismatched (because labels changed locally, but list control out of sync)
    confirmListCtrlContentsMatchCachedMap(false);

    // If we send through a batch of updates not relevant to us
    // Then should be ignored
    DvaMessageUpdateData updateDataB;    // std::set <long>
    for (KeysIt itr = setB.begin(); itr != setB.end(); itr ++)
    {
        updateDataB.insert(*itr);
    }
    
    std::vector<DWORD> itemDataArray;
    for (int nItem = 0; nItem < m_dvaListCtrl->GetItemCount(); nItem ++)
    {
        itemDataArray.push_back(m_dvaListCtrl->GetItemData(nItem));
    }
    m_dvaListCtrl->processDvaMessageUpdates(updateDataB);
    m_listCtrlWindowStub->processMsgQueue();
    // Confirm that no changes in list control
    for (nItem = 0; nItem < m_dvaListCtrl->GetItemCount(); nItem ++)
    {   
        Assert(itemDataArray[nItem] == m_dvaListCtrl->GetItemData(nItem));
    }

    // All records should still be mismatched
    confirmListCtrlContentsMatchCachedMap(false);

    // Now update the first batch of keys
    DvaMessageUpdateData updateDataA;    // std::set <long>
    // Now send through half the keys of update
    for (itr = setA.begin(); itr != setA.end(); itr ++)
    {
        updateDataA.insert(*itr);
    }
    m_dvaListCtrl->processDvaMessageUpdates(updateDataA);
    m_listCtrlWindowStub->processMsgQueue();

    // Confirm that all item data items in list control have changed
    for (nItem = 0; nItem < m_dvaListCtrl->GetItemCount(); nItem ++)
    {   
        Assert(itemDataArray[nItem] != m_dvaListCtrl->GetItemData(nItem));
    }

    // Updated all data in set A, dva list control should be synchronised again
    confirmListCtrlContentsMatchCachedMap();
}


void DvaListCtrlTestCases::testProcessDvaMessageUpdates_duplicateItems()
{
    // Confirm the list control can handle insertion and refresh of duplicate items
    // (items with a matching key)
    
    Keys duplicateList;
    const unsigned long key_a = getMessages()[0].m_pKey;
    const unsigned long key_b = getMessages()[1].m_pKey;

    // Add in two items, duplicated 4 times
    duplicateList.push_back(key_a);
    duplicateList.push_back(key_b);
    duplicateList.push_back(key_a);
    duplicateList.push_back(key_b);
    duplicateList.push_back(key_a);
    duplicateList.push_back(key_b);
    duplicateList.push_back(key_a);
    duplicateList.push_back(key_b);

    m_dvaListCtrl->refreshContents(duplicateList);
    m_listCtrlWindowStub->processMsgQueue();

    // Confirm the list control expanded to the required size (ie didn't attempt to compact duplicates)
    Assert(m_dvaListCtrl->GetItemCount() == (int)duplicateList.size());
    confirmListCtrlContentsMatchCachedMap();

    // Now refresh both items in cached map
    PaStationDvaMessageCachedMap& map = CachedMaps::getInstance()->getPaStationDvaMessageCachedMap();
    {
        LockedPacket<StationDvaMessageRecord> lock = map.getConfigurableCachedDbRecordFromKey(key_a);
        lock.getMutableData().m_label = "Changed label for key_a";
    }
    {
        LockedPacket<StationDvaMessageRecord> lock = map.getConfigurableCachedDbRecordFromKey(key_b);
        lock.getMutableData().m_label = "Changed label for key_b";
    }

    DvaMessageUpdateData refreshData;
    refreshData.insert(key_a);

    m_dvaListCtrl->processDvaMessageUpdates(refreshData);
    m_listCtrlWindowStub->processMsgQueue();

    // Now all items matching key a should be updated
    for (int i = 0; i < m_dvaListCtrl->GetItemCount(); i ++)
    {
        StationDvaMessageRecord* pMessage = m_dvaListCtrl->getRecordFromIndex(i);
    
        Assert(0 != pMessage);
    
        StationDvaMessageRecord compare = map.getCachedDbRecordFromKey(pMessage->m_pKey);
        
        if (pMessage->m_pKey == key_a)
        {
            // key_a should match
            Assert(*pMessage == compare);
        }
        else
        {
            // key_b should not match (not been refreshed yet)
            Assert(!(*pMessage == compare));
        }
    }

    // Now send through second update, should make fully in sync again
    refreshData.insert(key_b);
    m_dvaListCtrl->processDvaMessageUpdates(refreshData);
    m_listCtrlWindowStub->processMsgQueue();
    // Confirm synchronised again
    confirmListCtrlContentsMatchCachedMap();
}


/////////////////////////////////////////////////
//
// doesMessageKeyExist Tests
//
/////////////////////////////////////////////////
void DvaListCtrlTestCases::testDoesMessageKeyExist()
{
    srand(10);

    for (int iteration = 0; iteration < 10; iteration ++)
    {
        Keys keyList;
        Keys notInList;
    
        // Create a random set of keys
        for (UINT i = 0; i < getMessages().size(); i ++)
        {
            if (rand() % 2 == 0)
            {
                keyList.push_back(getMessages()[i].m_pKey);
            }
            else
            {
                notInList.push_back(getMessages()[i].m_pKey);
            }
        }
        
        m_dvaListCtrl->refreshContents(keyList);
        m_listCtrlWindowStub->processMsgQueue();

        do
        {
            DvaMessageUpdateData existingSet;
            // Now confirm all the keys exist
            for (KeysIt itr = keyList.begin(); itr != keyList.end(); itr ++)
            {
                Assert(m_dvaListCtrl->doesMessageKeyExist(*itr));
                existingSet.insert(*itr);
            }
            Assert(m_dvaListCtrl->doesMessageKeyExist(existingSet));
            Assert(m_dvaListCtrl->doAllMessageKeysExist(existingSet));
            
            if (notInList.size() > 0)
            {
                existingSet.insert(*notInList.begin());
                Assert(!m_dvaListCtrl->doAllMessageKeysExist(existingSet));
            }
                        
            // Confirm all keys not added are not present
            DvaMessageUpdateData notPresentSet;
            for (itr = notInList.begin(); itr != notInList.end(); itr ++)
            {        
                Assert(!m_dvaListCtrl->doesMessageKeyExist(*itr));
                notPresentSet.insert(*itr);
            }
            Assert(!m_dvaListCtrl->doesMessageKeyExist(notPresentSet));

            // Create a temporary list that only has one valid key in it
            if (keyList.size() > 0)
            {
                DvaMessageUpdateData tempList = notPresentSet;
                Assert(!m_dvaListCtrl->doesMessageKeyExist(tempList));
                tempList.insert(*keyList.begin());      // Add in a single valid key

                Assert(m_dvaListCtrl->doesMessageKeyExist(tempList));
                Assert(!m_dvaListCtrl->doAllMessageKeysExist(tempList));
            }
                        
            // Progressively remove keys, and confirm function continues returning proper result
            unsigned long keyToRemove = keyList.back();
            keyList.pop_back();
            notInList.push_back(keyToRemove);
            m_dvaListCtrl->removeSingleDvaMessage(keyToRemove);
            m_listCtrlWindowStub->processMsgQueue();
        }
        while (keyList.size() > 0);
    }
}



/////////////////////////////////////////////////
//
// removeSingleDvaMessage Tests
//
/////////////////////////////////////////////////
void DvaListCtrlTestCases::testRemoveSingleDvaMessage()
{
    Keys allKeys = getKeysInRange(0, getMessages().size());

    // Fill out list with initial data
    m_dvaListCtrl->refreshContents(allKeys);

    Keys currentKeys = allKeys;

    while (currentKeys.size() > 0)
    {
        // Progressively remove dva messages, confirming at each stage it remains in sync
        m_dvaListCtrl->removeSingleDvaMessage(currentKeys.back());
        m_listCtrlWindowStub->processMsgQueue();
    
        currentKeys.pop_back();

        // Confirm both list control array, and local array, contains same set of keys
        Keys keysInList = getKeysInListCtrl();
        Keys compareTo = currentKeys;
        Assert(destroyAllMatches(keysInList, compareTo));
    }
    
    try
    {
        m_dvaListCtrl->removeSingleDvaMessage(12);
        Assert(false);  // This message doesn't exist
    }
    catch (...)
    {
        Assert(true);
    }

    unsigned long duplicateKey = getMessages()[0].m_pKey;
    Keys duplicates;
    duplicates.push_back(duplicateKey);
    duplicates.push_back(duplicateKey);

    // Now add in duplicate items - function cannot handle duplicates properly
    m_dvaListCtrl->refreshContents(duplicates);
    m_listCtrlWindowStub->processMsgQueue();
        
    Assert(2 == m_dvaListCtrl->GetItemCount());

    try
    {
        m_dvaListCtrl->removeSingleDvaMessage(duplicateKey);
        // This function should have failed: as there are duplicates
        Assert(false);
    }
    catch (...)
    {
        Assert(true);
    }
}



/////////////////////////////////////////////////
//
// getSelectedMessageKey Tests
//
/////////////////////////////////////////////////
void DvaListCtrlTestCases::testGetSelectedMessageKey()
{
    Keys allKeys = getKeysInRange(0, getMessages().size());
    
    // Add in some duplicates
    allKeys.push_back(getMessages()[0].m_pKey);
    allKeys.push_back(getMessages()[0].m_pKey);
    allKeys.push_back(getMessages()[0].m_pKey);
    // Finally add in some records with invalid keys
    allKeys.push_back(INVALID_KEY);
    allKeys.push_back(INVALID_KEY);

    // Fill out list with initial data
    m_dvaListCtrl->refreshContents(allKeys);
    m_listCtrlWindowStub->processMsgQueue();

    for (int nItem = 0; nItem < m_dvaListCtrl->GetItemCount(); nItem ++)
    {        
        StationDvaMessageRecord* pRecord = m_dvaListCtrl->getRecordFromIndex(nItem);
        
        // Make current message the selection
        TA_Base_Bus::SelectionHelper::deselectAll(*m_dvaListCtrl);
        m_dvaListCtrl->SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
        
        if (0 != pRecord)
        {        
            Assert(pRecord->m_pKey == m_dvaListCtrl->getSelectedMessageKey());
        }
        else
        {
            // For the invalid records, the key should be -1
            Assert(TA_Base_Core::DatabaseKey::getInvalidKey() == m_dvaListCtrl->getSelectedMessageKey());
        }
    }

    // Confirm that when there is no selection, an invalid key is returned
    TA_Base_Bus::SelectionHelper::deselectAll(*m_dvaListCtrl);
    Assert(TA_Base_Core::DatabaseKey::getInvalidKey() == m_dvaListCtrl->getSelectedMessageKey());
}



/////////////////////////////////////////////////
//
// getRecordFromIndex Tests
//
/////////////////////////////////////////////////
void DvaListCtrlTestCases::testGetRecordFromIndex()
{
    Keys allKeys = getKeysInRange(0, getMessages().size());
    PaStationDvaMessageCachedMap& map = CachedMaps::getInstance()->getPaStationDvaMessageCachedMap();
    m_dvaListCtrl->refreshContents(allKeys);
    m_listCtrlWindowStub->processMsgQueue();

    for (int i = 0; i < m_dvaListCtrl->GetItemCount(); i ++)
    {
        StationDvaMessageRecord message = *m_dvaListCtrl->getRecordFromIndex(i);
        StationDvaMessageRecord cache = map.getCachedDbRecordFromKey(message.m_pKey);
        
        Assert(message == cache);
    }
}



/////////////////////////////////////////////////
//
// collectMatchingKeys Tests
//
/////////////////////////////////////////////////
void DvaListCtrlTestCases::testCollectMatchingKeys_returnvalue()
{
    int FINAL_ITEM = getMessages().size() / 2;
    Keys allKeys = getKeysInRange(0, FINAL_ITEM);
    m_dvaListCtrl->refreshContents(allKeys);
    m_listCtrlWindowStub->processMsgQueue();

    /**
     * @return true if any of the input keys belong to an item in this list
     *              (that is, one or more item(s) added to either of output sets)
     */
    DvaMessageUpdateData input;
    DvaMessageUpdateData a, b;

    input = getUpdatesInRange(0, FINAL_ITEM);
    Assert(m_dvaListCtrl->collectMatchingKeys(input, a, b));

    input = getUpdatesInRange(FINAL_ITEM - 1, getMessages().size());
    // Should be a single matching key
    Assert(m_dvaListCtrl->collectMatchingKeys(input, a, b));

    // None of the keys after FINAL_ITEM should be present, so should return false
    input = getUpdatesInRange(FINAL_ITEM, getMessages().size());
    Assert(!m_dvaListCtrl->collectMatchingKeys(input, a, b));

    input.insert(getMessages()[0].m_pKey);
    // Should be a single matching key
    Assert(m_dvaListCtrl->collectMatchingKeys(input, a, b));
}


void DvaListCtrlTestCases::testCollectMatchingKeys_listOutputs()
{
    Keys allKeys = getKeysInRange(0, getMessages().size());
    m_dvaListCtrl->refreshContents(allKeys);
    m_listCtrlWindowStub->processMsgQueue();

    DvaMessageUpdateData input = getUpdatesInRange(0, getMessages().size());
    DvaMessageUpdateData modifiedKeys, deletedKeys;
    // 'modifiedkeys' is a little deceiving - more like 'not-deleted keys that are in the input array'
    Assert(m_dvaListCtrl->collectMatchingKeys(input, modifiedKeys, deletedKeys));
    Assert(modifiedKeys == input);
    Assert(deletedKeys.size() == 0);

    input = getUpdatesInRange(0, 5);
    Assert(m_dvaListCtrl->collectMatchingKeys(input, modifiedKeys, deletedKeys));
    Assert(modifiedKeys == input);
    Assert(deletedKeys.size() == 0);

    PaStationDvaMessageCachedMap& map = CachedMaps::getInstance()->getPaStationDvaMessageCachedMap();
    // Now delete some messages (the first 5)
    DvaMessageUpdateData verifyDeleted;
    for (UINT i = 0; i < 5; i ++)
    {
        LockedPacket<StationDvaMessageRecord> lock = 
                map.getConfigurableCachedDbRecordFromKey(getMessages()[i].m_pKey);
        lock.getMutableData().m_type = DvaListCtrlModel::getDeletedType();
        verifyDeleted.insert(lock.getLockedData().m_pKey);
    }

    input = getUpdatesInRange(0, getMessages().size());
    Assert(m_dvaListCtrl->collectMatchingKeys(input, modifiedKeys, deletedKeys));
    // Confirm that all the keys we deleted are present in deletedKeys output
    Assert(deletedKeys == verifyDeleted);
    // Subtract all deleted keys from the original input
    for (DvaMessageUpdateData::iterator itr = deletedKeys.begin(); itr != deletedKeys.end(); itr ++)
    {
        input.erase(*itr);
    }
    // Confirm modifiedKeys only contains the remaining items
    Assert(input == modifiedKeys);
}


void DvaListCtrlTestCases::testCollectMatchingKeys_typeLabelOutputs()
{
    Keys allKeys = getKeysInRange(0, getMessages().size());
    m_dvaListCtrl->refreshContents(allKeys);
    m_listCtrlWindowStub->processMsgQueue();
    
    PaStationDvaMessageCachedMap& map = CachedMaps::getInstance()->getPaStationDvaMessageCachedMap();
   
    // Counter to ensure we set unique labels when forcing a label change
    DWORD uniqueLabel = 1;

    srand(20);
    for (int iteration = 0; iteration < 50; iteration ++)
    {
        // Generate a random interval in which to udpate messages
        UINT start = rand() % getMessages().size();
        UINT end = rand() % getMessages().size();        
        if (end < start)
        {
            std::swap(start, end);
        }   

        bool modifyLabels = (rand() % 2 == 0) ? true : false;
        bool modifyTypes = (rand() % 2 == 0) ? true : false;

        DvaMessageUpdateData input = getUpdatesInRange(start, end);
        
        bool labelChanges = false;
        bool typeChanges = false;
            
        DvaMessageUpdateData expectedDeletedKeys;

        // Use some counters in order to vary the number of modifications to types / labels
        UINT labelModifiedCount = 0;
        UINT typeModifiedCount = 0;

        // Make the relevant updates
        for (UINT i = start; i < end; i ++)
        {
            LockedPacket<StationDvaMessageRecord> lock = 
                    map.getConfigurableCachedDbRecordFromKey(getMessages()[i].m_pKey);

            if (modifyTypes && (typeModifiedCount == 0 || rand() % 2 == 0))
            {
                // Cycle through changes in type
                if (DvaListCtrlModel::getEmergencyType() == lock.getMutableData().m_type)
                {
                    lock.getMutableData().m_type = DvaListCtrlModel::getNormalType();
                }
                else if (DvaListCtrlModel::getNormalType() == lock.getMutableData().m_type)
                {
                    lock.getMutableData().m_type = DvaListCtrlModel::getDeletedType();
                }
                else
                {
                    lock.getMutableData().m_type = DvaListCtrlModel::getEmergencyType();
                }
                typeModifiedCount ++;
            }
            if (modifyLabels && (labelModifiedCount == 0 || rand() % 2 == 0))
            {
                std::ostringstream modifiedLabel;
                modifiedLabel << "Unique label " << (uniqueLabel++);
                lock.getMutableData().m_label = modifiedLabel.str();
                labelModifiedCount ++;
            }

            if (DvaListCtrlModel::getDeletedType() == lock.getLockedData().m_type)
            {
                expectedDeletedKeys.insert(lock.getLockedData().m_pKey);
            }
        }

        DvaMessageUpdateData modifiedKeys, deletedKeys;
        m_dvaListCtrl->collectMatchingKeys(getUpdatesInRange(start, end), 
                                            modifiedKeys, 
                                            deletedKeys, 
                                            &typeChanges, 
                                            &labelChanges);
    
        // Confirm sum of outputs matches inputs
        Assert(getUpdatesInRange(start, end).size() == modifiedKeys.size() + deletedKeys.size());
        Assert(expectedDeletedKeys == deletedKeys);

        // Confirm the type and label modification flags correlate with expected
        // (provided there were any changes propogated)
        if (start != end)
        {
            Assert(modifyTypes == typeChanges);
            Assert(modifyLabels == labelChanges);      
        
            // Synchronise the list control after each iteration
            m_dvaListCtrl->processDvaMessageUpdates(getUpdatesInRange(start, end));  
            m_listCtrlWindowStub->processMsgQueue();
        }
    }
}



/////////////////////////////////////////////////
//
// sortAlphabeticallyAscending Tests
//
/////////////////////////////////////////////////
void DvaListCtrlTestCases::testSortAlphabeticallyAscending()
{
    // Each time we create an ordered list, must insert into list in alphabetical order
    std::vector <std::pair<int, std::string> > ordered;    
    ordered.push_back( std::pair<int, std::string>(0, "Alpha") );
    ordered.push_back( std::pair<int, std::string>(2, "Beta") );
    ordered.push_back( std::pair<int, std::string>(1, "Charlie") );

    confirmSortsAlphabetically(ordered);

    ordered.clear();
    ordered.push_back( std::pair<int, std::string>(6, "Aaardvark") );
    ordered.push_back( std::pair<int, std::string>(2, "Apple") );
    ordered.push_back( std::pair<int, std::string>(7, "Avacado") );
    confirmSortsAlphabetically(ordered);

    ordered.clear();
    ordered.push_back( std::pair<int, std::string>(0, "Animal") );
    ordered.push_back( std::pair<int, std::string>(2, "Cattle") );
    ordered.push_back( std::pair<int, std::string>(7, "Zebra") );
    ordered.push_back( std::pair<int, std::string>(3, "Zoo") );
    confirmSortsAlphabetically(ordered);

    ordered.clear();
    ordered.push_back( std::pair<int, std::string>(7, "Animal") );
    ordered.push_back( std::pair<int, std::string>(6, "Bob") );
    ordered.push_back( std::pair<int, std::string>(5, "Cattle") );
    ordered.push_back( std::pair<int, std::string>(4, "Zac") );
    ordered.push_back( std::pair<int, std::string>(3, "zzzzz") );
    confirmSortsAlphabetically(ordered);
}


void DvaListCtrlTestCases::confirmSortsAlphabetically(std::vector <std::pair<int, std::string> > ordered)
{
    unsigned int i;

    Keys allKeys;
    DvaMessageUpdateData updateKeys;
    for (i = 0; i < ordered.size(); i ++)
    {
        allKeys.push_back(getMessages()[ordered[i].first].m_pKey);
        updateKeys.insert(allKeys.back());
    }
       
    // Fill in the list before cached map has been updated - since 
    // we're sorting dynamically (after cached map change)
    m_dvaListCtrl->refreshContents(allKeys);
    m_listCtrlWindowStub->processMsgQueue();

    PaStationDvaMessageCachedMap& map = CachedMaps::getInstance()->getPaStationDvaMessageCachedMap();

    // Update all the labels
    for (i = 0; i < ordered.size(); i ++)
    {
        std::pair<int, std::string>& element = ordered[i];
        unsigned long key = getMessages()[element.first].m_pKey;

        map.getConfigurableCachedDbRecordFromKey(key).getMutableData().m_label = element.second;
    }
    
    m_dvaListCtrl->processDvaMessageUpdates(updateKeys);

    // Since automatic sorting added, should automatically sort in ascending order
    // once message queue processed
    //m_dvaListCtrl->sortAlphabeticallyAscending();
    m_listCtrlWindowStub->processMsgQueue();

    // Confirm the ordering in the list control matches the ordering in 'ordered' array
    for (int nItem = 0; nItem < m_dvaListCtrl->GetItemCount(); nItem ++)
    {
        unsigned long expectedKey = getMessages()[ordered[nItem].first].m_pKey;
        StationDvaMessageRecord* record = m_dvaListCtrl->getRecordFromIndex(nItem);
        Assert(record->m_pKey == expectedKey);
    }
}



/////////////////////////////////////////////////
//
// findItemIndices Tests
//
/////////////////////////////////////////////////
void DvaListCtrlTestCases::testFindItemIndices()
{
    // Finds the items in list containing the specified key
    Keys allKeys;
    
    srand(20);

    // Iterate through adding in / removing items to key set, and confirm
    // getItemIndices returns correct objects
    for (int iteration = 0; iteration < 120; iteration ++)
    {
        unsigned long key = getMessages()[rand() % getMessages().size()].m_pKey;

        // Have more probability of adding a new item
        if (rand() % 3 > 0)
        {
            allKeys.push_back(key);
        }
        else
        {
            // Otherwise attempt to remove the first instance of this key
            KeysIt itr = std::find(allKeys.begin(), allKeys.end(), key);
            if (itr != allKeys.end())
            {
                allKeys.erase(itr);
            }
        }

        m_dvaListCtrl->refreshContents(allKeys);
        m_listCtrlWindowStub->processMsgQueue();
        
        std::set<unsigned long> uniqueKeys;
        std::set<int> indiceStore;
        {
            for (KeysIt itr = allKeys.begin(); itr != allKeys.end(); itr ++)
            {
                uniqueKeys.insert(*itr);
            }
        }

        // Look at each of the keys (once per unique key)
        for (std::set<unsigned long>::iterator itr = uniqueKeys.begin(); itr != uniqueKeys.end(); itr ++)
        {
            std::vector<int> indices = m_dvaListCtrl->findItemIndices(*itr);

            // Confirm that record for item at each index is correct
            for (unsigned int i = 0; i < indices.size(); i ++)
            {
                indiceStore.insert(indices[i]);

                StationDvaMessageRecord* pRecord = m_dvaListCtrl->getRecordFromIndex(indices[i]);
                Assert(pRecord->m_pKey == *itr);
            }

            // Also need to confirm that indices isn't missing any items
            // we can do this by summing the indices returned at each stage - if by the
            // end we haven't retunred all indices present in control, we know it failed
        }

        // Confirm all indices were returned at one stage
        Assert((int)indiceStore.size() == m_dvaListCtrl->GetItemCount());
    }       
}



/////////////////////////////////////////////////
//
// DeleteAllItems Tests
//
/////////////////////////////////////////////////
void DvaListCtrlTestCases::testDeleteAllItems()
{
    for (int testType = 0; testType < 2; testType ++)
    {
        // Confirm that calling this on the list control causes it to free
        // up its memory as required
        Keys allKeys = getKeysInRange(0, getMessages().size());
        // Add some duplicates
        allKeys.push_back(getMessages()[0].m_pKey);
        allKeys.push_back(getMessages()[0].m_pKey);
        allKeys.push_back(getMessages()[1].m_pKey);
        // Add some invalid ones
        allKeys.push_back(INVALID_KEY);
        allKeys.push_back(INVALID_KEY);
        
        m_dvaListCtrl->refreshContents(allKeys);
        m_listCtrlWindowStub->processMsgQueue();
    
        // Set all the items we expect to be deleted
        for (int nItem = 0; nItem < m_dvaListCtrl->GetItemCount(); nItem ++)
        {
            // NOTE: Do not add in null items (won't need to be deleted of course)
            if (0 != m_dvaListCtrl->getRecordFromIndex(nItem))
            {
                s_expectedDeletions.insert((void*)m_dvaListCtrl->getRecordFromIndex(nItem));
            }
        }
    
        Assert(s_expectedDeletions.size() > 0);
    
        // Place a hook to confirm the delete requests come through on all StationDvaMessageRecords    
        _CRT_ALLOC_HOOK oldHook = _CrtSetAllocHook(deleteCheckHook);
        
        try
        {
            if (0 == testDeleteAllItems)
            {
                m_dvaListCtrl->DeleteAllItems();
                Assert(0 == m_dvaListCtrl->GetItemCount());
            }
            else
            {
                // For second test iteration, test function cleanupItemData (does same thing, but
                // doesn't actually remove the items)
                m_dvaListCtrl->CleanupItemData();
                Assert((int)allKeys.size() == m_dvaListCtrl->GetItemCount());
            }
        }    
        catch (...)
        {
            // Things might get pretty messed up if we don't restore the hook
            _CrtSetAllocHook( oldHook );
            throw;
        }
        _CrtSetAllocHook( oldHook );
    
        // All items should have been deleted leaving the expected deletions array empty
        Assert(0 == s_expectedDeletions.size());
    }
}



/////////////////////////////////////////////////
//
// DeleteItem Tests
//
/////////////////////////////////////////////////
void DvaListCtrlTestCases::testDeleteItem()
{
    // Confirm that calling this on the list control causes it to free
    // up its memory as required
    Keys allKeys = getKeysInRange(0, getMessages().size());
    // Add some duplicates
    allKeys.push_back(getMessages()[0].m_pKey);
    allKeys.push_back(getMessages()[1].m_pKey);
    // Add some invalid ones
    allKeys.push_back(INVALID_KEY);
    allKeys.push_back(INVALID_KEY);
    
    // Delete items from the list at random
    srand(20);

    m_dvaListCtrl->refreshContents(allKeys);
    m_listCtrlWindowStub->processMsgQueue();

    // Place a hook to confirm the delete requests come through on all StationDvaMessageRecords    
    _CRT_ALLOC_HOOK oldHook = _CrtSetAllocHook(deleteCheckHook);    

    try
    {
    
        while (m_dvaListCtrl->GetItemCount() > 0)
        {
            int itemIndex = rand() % m_dvaListCtrl->GetItemCount();
            
            if (0 != m_dvaListCtrl->getRecordFromIndex(itemIndex))
            {
                s_expectedDeletions.insert((void*)m_dvaListCtrl->getRecordFromIndex(itemIndex));                
            }
                
            bool removalStillRequired = false;

            if (rand() % 2 == 0)
            {
                m_dvaListCtrl->DeleteItem(itemIndex);
            }
            else
            {
                // Otherwise use deleteItemData first, and delete the item from list (later)
                m_dvaListCtrl->deleteItemData(itemIndex);
                removalStillRequired = true;
            }

            // Confirm it was deleted
            Assert(s_expectedDeletions.size() == 0);

            if (removalStillRequired)
            {
                m_dvaListCtrl->DeleteItem(itemIndex);
            }
        }
    }
    catch (...)
    {
        _CrtSetAllocHook( oldHook );
        throw;
    }

    _CrtSetAllocHook( oldHook );
}

void DvaListCtrlTestCases::testSetItemData_replaceInvalid()
{
    Keys allKeys = getKeysInRange(0, getMessages().size());
    m_dvaListCtrl->refreshContents(allKeys);
    
    StationDvaMessageRecord* record = new StationDvaMessageRecord(createRecord(1, 1, "Dummy Label", 'N'));
                    
    Keys currentKeys = getKeysInListCtrl();
    // Confirm that if we pass in an existing key pointer, that points to a key not in the list
    // - then there is no change
    unsigned long invalidKey = INVALID_KEY;   
    
    try
    {                     
        m_dvaListCtrl->SetItemData(record, &invalidKey);
        Assert(false);
    }
    catch (...)
    {
        // Sending in a dud key should violate the preconditions
        Assert(true);
    }
    
    // Confirm no change
    Assert(getKeysInListCtrl() == currentKeys);
    
    // Perform our own cleanup
    delete record;
    record = 0;
}               

    
void DvaListCtrlTestCases::testSetItemData_duplicateKeys()
{
    // This function tests that the replace functionality works when duplicate keys
    // exist (that is, it updates all records with a matching key)
    // When duplicates exist, control dynamically allocates memory for the clones
    // (still taking ownership of the original one we sent in)
     _CRT_ALLOC_HOOK oldHook = _CrtSetAllocHook(deleteCheckHook);

    try
    {
        // Fill in the list control initially
        Keys originalKeys = getKeysInRange(0, getMessages().size());
        // Add in three duplicates of each key (all interwoven)
        Keys allKeys;
        enum { DUPLICATES = 3 };

        for (int i = 0; i < DUPLICATES; i ++)
        {
            for (KeysIt itr = originalKeys.begin(); itr != originalKeys.end(); itr ++)
            {
                allKeys.push_back(*itr);
            }
        }
    
        // Dispatch to list control
        m_dvaListCtrl->refreshContents(allKeys);

        // Set of all records created from this test (to confirm all cleaned up)
        std::set<void*> createdRecords;
        
        // Now for each key, replace the record
        for (KeysIt keyItr = originalKeys.begin(); keyItr != originalKeys.end(); keyItr ++)
        {
            std::ostringstream label;
            label << "DuplicateReplace:" << (*keyItr);
            StationDvaMessageRecord* record = new StationDvaMessageRecord(createRecord(1, 1, label.str(), 'N'));

            unsigned long key = (*keyItr);
            std::vector<int> updateIndices = m_dvaListCtrl->findItemIndices(key);
            
            // We expect all instances of existing records to be deleted
            for (unsigned int u = 0; u < updateIndices.size(); u ++)
            {
                s_expectedDeletions.insert(m_dvaListCtrl->getRecordFromIndex(updateIndices[u]));
            }
            
            Assert(DUPLICATES == s_expectedDeletions.size());
            m_dvaListCtrl->SetItemData(record, &key);

            // Confirm all old records deleted
            Assert(0 == s_expectedDeletions.size());
            
            int matches = 0;

            // Now confirm labels have been updated for all items
            for (u = 0; u < updateIndices.size(); u ++)
            {   
                CString newString = m_dvaListCtrl->GetItemText(updateIndices[u], 0);
                Assert(label.str().compare(newString.GetBuffer(0)) == 0);
                
                StationDvaMessageRecord* r = m_dvaListCtrl->getRecordFromIndex(updateIndices[u]);
                createdRecords.insert(r);

                // From all the updates records, one of them should correspond to 'record' - the others
                // were created dynamically by the list
                if (r == record)
                {
                    matches ++;
                }
            }

            // Confirm there is one matching record, that matches the record we passed in to SetItemData
            Assert(1 == matches);
        }    

        // Confirm all records created are cleanup up properly
        s_expectedDeletions = createdRecords;
        Assert(s_expectedDeletions.size() > 0);
        m_dvaListCtrl->DeleteAllItems();
        Assert(0 == s_expectedDeletions.size());
    }
    catch (...)
    {
        // Things might get pretty messed up if we don't restore the hook
        _CrtSetAllocHook( oldHook );
        throw;
    }
    _CrtSetAllocHook( oldHook );
}


void DvaListCtrlTestCases::testSetItemData_addReplace()
{
     _CRT_ALLOC_HOOK oldHook = _CrtSetAllocHook(deleteCheckHook);

    try
    {
        // Don't bother testing sortNewItemsById for now (functionality redundant since
        // client request to always have alphabetically sorted)
    
        // Fill in the list control initially
        Keys allKeys = getKeysInRange(0, getMessages().size());
        m_dvaListCtrl->refreshContents(allKeys);
            
        enum EIteration 
        {
            UPDATE_EXISTING,
            ADD_NEW,
            ITERATION_MAX
        };
        
        std::set<void*> itemsAddedToList;

        for (int iteration = 0; iteration < ITERATION_MAX; iteration ++)
        {
            // Now pass in a valid key, and confirm the old item deleted, and that new item takes its place
            // (and that new item subsequently deleted)
            for (int i = 0; i < 3; i ++)
            {
                std::ostringstream label;
                label << "NewRecord:" << i << "," << iteration;
                StationDvaMessageRecord* record = new StationDvaMessageRecord(createRecord(1, 1, label.str(), 'N'));
                               
                int nItem = 5;
                // Confirm that updating item updates its text
                CString oldString = m_dvaListCtrl->GetItemText(nItem, 0);                
                
                unsigned long keyToReplace = m_dvaListCtrl->getRecordFromIndex(nItem)->m_pKey;
                unsigned long* existingKey = &keyToReplace;

                if (ADD_NEW == iteration)
                {   
                    itemsAddedToList.insert(record);

                    // In subsequent iterations, we add in new items instead of replacing
                    Assert(TRUE == m_dvaListCtrl->SetItemData(record, NULL));
                    m_dvaListCtrl->findItemIndices(record->m_pKey);      
                    
                    // Find this newly added item
                    for (int item = 0; item < m_dvaListCtrl->GetItemCount(); item ++)
                    {
                        if (m_dvaListCtrl->getRecordFromIndex(item) == record)
                        {
                            break;
                        }
                    }            
                     
                    nItem = item;

                    // This next assert will fail if we didn't find the newly inserted item
                    Assert(nItem < m_dvaListCtrl->GetItemCount());
                }
                else if (UPDATE_EXISTING == iteration)
                {
                    // Expect the old item to be deleted
                    s_expectedDeletions.insert(m_dvaListCtrl->getRecordFromIndex(nItem));
                    // Perform the replace operation
                    Assert(TRUE == m_dvaListCtrl->SetItemData(record, &keyToReplace));
                    Assert(0 == s_expectedDeletions.size());    // Should have been deleted
                
                    // Confirm record pointer has been updated
                    Assert(record == m_dvaListCtrl->getRecordFromIndex(nItem));
                }
                
                // Confirm label has changed for this item
                CString newString = m_dvaListCtrl->GetItemText(nItem, 0);
                
                if (UPDATE_EXISTING == iteration)
                {
                    Assert(oldString != newString);
                }
                
                // Confirm the new label matches the label of the record we sent in
                Assert(label.str().compare(newString.GetBuffer(0)) == 0);
            }    
        }        

        // Ensure all the items added to the list over the duration of this function are cleaned up
        s_expectedDeletions = itemsAddedToList;
        Assert(s_expectedDeletions.size() > 0);
        m_dvaListCtrl->DeleteAllItems();
        Assert(0 == s_expectedDeletions.size());
    }
    catch (...)
    {
        // Things might get pretty messed up if we don't restore the hook
        _CrtSetAllocHook( oldHook );
        throw;
    }
    _CrtSetAllocHook( oldHook );
}


void DvaListCtrlTestCases::testFindSortedIdInsertionPoint()
{
    // findSortedInsertionPoint finds the ordered insertion
    // point in the list as defined by the ID
    Keys allKeys = getKeysInRange(0, getMessages().size());
    m_dvaListCtrl->refreshContents(allKeys);

#define IDSORT(x) ((x + 1) * 5)

    // Ensure the items are sorted initially
    for (int i = 0; i < m_dvaListCtrl->GetItemCount(); i ++)
    {
        StationDvaMessageRecord* pMessage = m_dvaListCtrl->getRecordFromIndex(i);        
        pMessage->m_id = IDSORT(i);
    }

    // In real world operation, findSortedIdInsertionPoint will be operating
    // on an already sorted list (which is what is done above) - so don't
    // have to be concerned with functionality on unsorted list (contextually doesn't
    // make any sense to do this)
    StationDvaMessageRecord record;
    for (i = 0; i < m_dvaListCtrl->GetItemCount(); i ++)
    {
        record.m_id = IDSORT(i) - 1;
        // If id is one below, should be inserting before
        int pos = m_dvaListCtrl->findSortedIdInsertionPoint(record);
        Assert(pos == i);
     
        record.m_id = IDSORT(i) + 1;
        // If id is one above, should be inserting after
        pos = m_dvaListCtrl->findSortedIdInsertionPoint(record);
        Assert(pos == i + 1);
    }

#undef IDSORT
}


int __cdecl DvaListCtrlTestCases::deleteCheckHook(
        int      nAllocType,
        void   * pvData,
        size_t   nSize,
        int      nBlockUse,
        long     lRequest,
        const unsigned char * szFileName,
        int      nLine)
{
    if ( nBlockUse == _CRT_BLOCK )
    {
        // Ignore internal C runtime library allocations
        return( true );
    }

    if (3 == nAllocType)
    {
        // Freeing memory,,
        std::set<void*>::iterator itr = s_expectedDeletions.find(pvData);
        if (itr != s_expectedDeletions.end())
        {
            s_expectedDeletions.erase(itr);
        }
    }

    return (TRUE);
}

void DvaListCtrlTestCases::confirmListCtrlContentsMatchCachedMap(bool confirmMatch)
{
    PaStationDvaMessageCachedMap& map = CachedMaps::getInstance()->getPaStationDvaMessageCachedMap();

    for (int i = 0; i < m_dvaListCtrl->GetItemCount(); i ++)
    {
        StationDvaMessageRecord* pMessage = m_dvaListCtrl->getRecordFromIndex(i);
    
        // Handle cases where record is null (no entry)
        if (0 != pMessage)
        {
            StationDvaMessageRecord compare = map.getCachedDbRecordFromKey(pMessage->m_pKey);
            
            if (confirmMatch)
            {
                Assert(*pMessage == compare);
            }
            else
            {
                Assert(!(*pMessage == compare));
            }
        }
    }

}


DvaMessageUpdateData DvaListCtrlTestCases::getUpdatesInRange(UINT lower, UINT upper)
{
    DvaMessageUpdateData result;
    Keys keys = getKeysInRange(lower, upper);

    for (KeysIt itr = keys.begin(); itr != keys.end(); itr ++)
    {
        result.insert(*itr);
    }

    return result;
}


Keys DvaListCtrlTestCases::getKeysInRange(UINT lower, UINT upper)
{
    Keys keys;
    for (UINT i = lower; i < upper; i ++)
    {
        keys.push_back(getMessages()[i].m_pKey);
    }
    return keys;
}


Keys DvaListCtrlTestCases::getKeysInListCtrl()
{
    Keys keysInList;
    for (int i = 0; i < m_dvaListCtrl->GetItemCount(); i ++)
    {
        keysInList.push_back(m_dvaListCtrl->getRecordFromIndex(i)->m_pKey);
    }
    return keysInList;
}
    

bool DvaListCtrlTestCases::destroyAllMatches(Keys& setA, Keys& setB)
{
    Keys clonedKeys = setA;

    for (KeysIt itr = clonedKeys.begin(); itr != clonedKeys.end(); itr ++)
    {   
        for (;;)
        {
            KeysIt aIterator = std::find(setA.begin(), setA.end(), *itr);
            KeysIt bIterator = std::find(setB.begin(), setB.end(), *itr);
    
            if (aIterator != setA.end() && bIterator != setB.end())
            {
                // This element is common between the two
                setA.erase(aIterator);
                setB.erase(bIterator);
                continue;   // Check if there are any more..
            }
            else
            {
                break;
            }
        }
    }

    // Return true if both sets have been emptied
    return setA.size() == 0 && setB.size() == 0;
}


std::vector<StationDvaMessageRecord> DvaListCtrlTestCases::getMessages()
{
    std::vector<StationDvaMessageRecord> result;
    StationDvaMessageRecords temp = CachedMaps::getInstance()->getPaStationDvaMessageCachedMap().getAllRecords();

    for (StationDvaMessageRecordsIt itr = temp.begin(); itr != temp.end(); itr ++)
    {
        result.push_back(*itr);
    }
    return result;
}


StationDvaMessageRecord DvaListCtrlTestCases::createRecord(unsigned long locationKey, 
                                                                unsigned long id,
                                                                std::string label,
                                                                char type)
{
    static unsigned long primaryKey = 1;

    StationDvaMessageRecord result;
    result.m_pKey = (primaryKey ++ );
    result.m_locationKey = locationKey;
    result.m_id = id;
    result.m_label = label;
    result.m_type = type;

    return result;
}


void DvaListCtrlTestCases::tearDownSuite() 
{
    TA_App::PaTest::restoreLocation();
}


void DvaListCtrlTestCases::setUpSuite( const std::string & inFile ) 
{
    TA_App::PaTest::setToValidLocation();
}


void DvaListCtrlTestCases::setUp( const std::string & INFile , const std::string & OUTFile )
{
    // Refresh the set of messages for each iteration (as might be changed within a test function)
    PaStationDvaMessageCachedMap& cachedMap = CachedMaps::getInstance()->getPaStationDvaMessageCachedMap();
    
    // This in the station DVA Message cached map with some suitable data
    cachedMap.clearKeyToRecordMap();

    StationDvaMessageRecord record;
    record = createRecord(1, 1, "MessageA", 'N');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageB", 'N');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageC", 'N');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageD", 'N');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageE", 'N');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageF", 'N');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageG", 'N');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageH", 'N');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);

    record = createRecord(1, 1, "MessageA", 'E');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageB", 'E');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageC", 'E');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageD", 'E');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageE", 'E');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageF", 'E');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageG", 'E');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageH", 'E');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);

    cachedMap.flagAsInitialised();

    // For each test, create a new DVA List control
    m_listCtrlWindowStub = new WindowStub(WC_LISTVIEW);


    m_dvaListCtrl = new DvaListCtrl();

    m_dvaListCtrl->Attach(m_listCtrlWindowStub->getHWnd());
}


void DvaListCtrlTestCases::tearDown( const std::string & INFile ) 
{
    if (0 != m_dvaListCtrl)
    {
        m_dvaListCtrl->Detach();
        delete m_dvaListCtrl;
        m_dvaListCtrl = 0;
    }

    delete m_listCtrlWindowStub;
    m_listCtrlWindowStub = 0;
}


TestSuite * DvaListCtrlTestCases::suite()
{
    TestSuite * testSuite = new TestSuite( "DvaListCtrlTestCases" );  
    
// Use temporary hashdef to add tests
#define ADD_UNIT_TEST(function) \
    testSuite->addTest( new TestCaller< DvaListCtrlTestCases >(\
                        #function,\
                        &DvaListCtrlTestCases::##function,this ) );

    //
    // refreshContents tests
    //
    ADD_UNIT_TEST(testRefreshContents);


    //
    // processDvaMessageUpdates tests
    //
    ADD_UNIT_TEST(testProcessDvaMessageUpdates_uniqueItems);   
    ADD_UNIT_TEST(testProcessDvaMessageUpdates_duplicateItems);   

    //
    // doesMessageKeyExist tests
    //
    ADD_UNIT_TEST(testDoesMessageKeyExist);    

    //
    // removeSingleDvaMessage tests
    //
    ADD_UNIT_TEST(testRemoveSingleDvaMessage);

    //
    // getSelectedMessageKey tests
    //
    ADD_UNIT_TEST(testGetSelectedMessageKey);

    //
    // getRecordFromIndex tests
    //
    ADD_UNIT_TEST(testGetRecordFromIndex);

    //
    // collectMatchingKeys tests
    //
    ADD_UNIT_TEST(testCollectMatchingKeys_returnvalue);
    ADD_UNIT_TEST(testCollectMatchingKeys_listOutputs);
    ADD_UNIT_TEST(testCollectMatchingKeys_typeLabelOutputs);

    //
    // sortAlphabeticallyAscending tests
    //
    ADD_UNIT_TEST(testSortAlphabeticallyAscending);

    //
    // findItemIndices tests
    //
    ADD_UNIT_TEST(testFindItemIndices);

    //
    // DeleteAllItems tests
    //
    ADD_UNIT_TEST(testDeleteAllItems);

    //
    // DeleteItem tests
    //
    ADD_UNIT_TEST(testDeleteItem);

    //
    // SetItemData tests
    //
    ADD_UNIT_TEST(testSetItemData_replaceInvalid);
    ADD_UNIT_TEST(testSetItemData_addReplace);
    ADD_UNIT_TEST(testSetItemData_duplicateKeys);
    
    //
    // FindSortedIdInsertionPoint tests
    //
    ADD_UNIT_TEST(testFindSortedIdInsertionPoint);

#undef ADD_UNIT_TEST

	return testSuite;
}