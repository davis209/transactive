/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DvaListCtrlModel.cpp $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * Contains business logic control for DVA lists.  Quite a lot more complicated by the fact
 * that a multimap needs to be used to track elements in the model using only their
 * record ID (multiple items with same record ID, but different memory pointers, exist
 * in the list, and need to be updated when ad-hoc message updates come through)
 *
 */

#include "app/pa/PAManager/src/StdAfx.h"
#include "app/pa/PAManager/src/DvaListCtrlModel.h"
#include "core/utilities/src/DebugUtil.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_IRS_Bus;

DvaListCtrlModel::DvaListCtrlModel()
{
}

std::vector<TA_IRS_Bus::StationDvaMessageRecord*> DvaListCtrlModel::fetchLatestRecords(Keys& messageKeys)
{
    std::vector<TA_IRS_Bus::StationDvaMessageRecord*> resultArray;

    for (Keys::iterator it = messageKeys.begin(); it != messageKeys.end(); ++it)
    {
        TA_IRS_Bus::StationDvaMessageRecord* clonedRecord = new TA_IRS_Bus::StationDvaMessageRecord;

        try
        {
            *clonedRecord = TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(*it);
        }
        catch (const TA_Base_Core::CachedMappingNotFoundException&)
        {
            // The record wasn't found - this means the list item
            // no longer exists in the database (just add a null entry)
            delete clonedRecord;
            clonedRecord = 0;
        }

        resultArray.push_back(clonedRecord);
    }

    return resultArray;
}

bool DvaListCtrlModel::doesMappingExist(unsigned long messageKey)
{
    return m_controlMap.find(messageKey) != m_controlMap.end();
}

void DvaListCtrlModel::updateMapping(unsigned long messageKey, DWORD newData)
{
    typedef std::multimap<unsigned long, DWORD>::iterator KeyMapIterator;

    // If the target already exists, no need to do anything
    std::map<unsigned long, DWORD>::value_type insertion(messageKey, newData);

    /*
    // Must add in duplicates - since cleanup code will call clearMapping
    // for as many times as it called updateMapping (so putting this in
    // will cause crash on cleanup)
    if (std::find(m_controlMap.lower_bound(messageKey),
                  m_controlMap.upper_bound(messageKey),
                  insertion) != m_controlMap.upper_bound(messageKey))
    {
        // An item with matching key / newData pair already exists, no need to do
        return;
    }
    */

    // insert into the map if not already present
    m_controlMap.insert(insertion);
}

std::vector<DWORD> DvaListCtrlModel::getMessageData(unsigned long messageKey)
{
    std::multimap<unsigned long, DWORD>::iterator itr = m_controlMap.find(messageKey);

    TA_ASSERT(itr != m_controlMap.end(), "Key data not found");
    //return itr->second;

    std::vector<DWORD> resultingArray;

    do
    {
        resultingArray.push_back(itr->second);

        itr ++;
    }
    while (itr != m_controlMap.end() && itr->first == messageKey);

    return resultingArray;
}

void DvaListCtrlModel::clearMapping(unsigned long messageKey, DWORD data)
{
    // Should be using m_controlMap.lower_bound .. m_controlMap.upper_bound functions for clarity..
    std::multimap<unsigned long, DWORD>::iterator itr = m_controlMap.find(messageKey);

    while (itr != m_controlMap.end())
    {
        // Find the specific mapping (could be multiple with same ->first value)
        if (itr->second == data)
        {
            // Found the specific pair we're after
            m_controlMap.erase(itr);
            return;
        }

        // Otherwise must check next pair in multimap
        itr++;
    }

    TA_ASSERT(FALSE, "Couldn't find mapping pair");
}

bool DvaListCtrlModel::isEmergencyMessage(char type)
{
    return (type == getEmergencyType());
}

bool DvaListCtrlModel::isDeletedMessage(char type)
{
    return (type == getDeletedType());
}

char DvaListCtrlModel::getEmergencyType()
{
    return 'E';
}

char DvaListCtrlModel::getNormalType()
{
    return 'N';
}

char DvaListCtrlModel::getDeletedType()
{
    return 'X';
}

bool DvaListCtrlModel::isTisEnabled(const TA_IRS_Bus::StationDvaMessageRecord& record)
{
    // -1 for key is indicator that TIS unavailable
    // Only if all parameters are set is it TIS enabled
    if (record.m_tisLibrarySection == -1 ||
    record.m_tisLibraryVersion == -1 ||
    record.m_tisMessageTag == -1)
    {
        // One of fields unset (require all to be TIS enabled)
        return false;
    }
    else
    {
        return true;
    }
}
