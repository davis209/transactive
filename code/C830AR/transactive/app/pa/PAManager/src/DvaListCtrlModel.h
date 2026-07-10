/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DvaListCtrlModel.h $
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

#if !defined(AFX_DVALISTCONTROL_MODEL_H__AA87B8BD_6BE9_484A_B86B_BD8B058B88F8__INCLUDED_)
#define AFX_DVALISTCONTROL_MODEL_H__AA87B8BD_6BE9_484A_B86B_BD8B058B88F8__INCLUDED_

#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include <map>

class DvaListCtrlModel
{
    friend class DvaListCtrlModelTestCases;

public:

    DvaListCtrlModel();

    virtual ~DvaListCtrlModel() {}

    /**
     * fetchLatestRecords
     *
     * *NOTE*: It is the responsibility of the client function to delete the pointers
     *  contained in the array - there may be NULL entries returned if the key
     *  references a non existent record
     *
     * @param messageKeys a set of keys defining the messages to be
     *              displayed in this control
     *
     *
     * @return array of most up to date record data of provided keys
     *
     */
    static std::vector<TA_IRS_Bus::StationDvaMessageRecord*> fetchLatestRecords(Keys& messageKeys);

    /**
     * updateMapping
     *
     * Updates internal key<=>data map
     *
     * @param messageKey the key to map object
     *
     * @param newData the new data associated with key
     *
     */
    void    updateMapping(unsigned long messageKey, DWORD messageData);

    /**
     * getMessageData
     *
     * @pre messageKey must exist in the map (be set with updateMapping)
     *
     * @param messageKey key to the data
     *
     * @return the data set associated with this key
     *
     */
    std::vector<DWORD>   getMessageData(unsigned long messageKey);

    /**
     * clearMapping
     *
     * Clears the mapping of the specified element (defined by key)
     *
     * @pre messageKey must exist in the map (be set with updateMapping)
     *
     * @param messageKey key to the data
     *
     * @param data the associated data (we're clearing the specific pair(key,data))
     *
     */
    void    clearMapping(unsigned long messageKey, DWORD data);

    /**
     * doesMappingExist
     *
     * @return true if the specified messageKey exists in mapping
     *
     */
    bool    doesMappingExist(unsigned long messageKey);

    /**
     * isEmergencyMessage
     *
     * @param type the message type identifier
     *
     * @return true if the supplied message type is an emergency message
     *
     */
    static bool    isEmergencyMessage(char type);

    /**
     * isDeletedMessage
     *
     * @param type the message type identifier
     *
     * @return true if the supplied message type is a 'deleted' message
     *
     */
    static bool    isDeletedMessage(char type);

    /**
     * getEmergencyType
     *
     * @return the character identifier for the emergency type
     *
     */
    static char getEmergencyType();

    /**
     * getNormalType
     *
     * @return the character identifier for the normal type
     *
     */
    static char getNormalType();

    /**
     * getDeletedType
     *
     * @return the character identifier for the deleted type
     *
     */
    static char getDeletedType();

    /**
     * isTisEnabled
     *
     * @return true if this message has TIS synchronization available
     *
     * @param record the record used to determine STIS availability
     *
     */
    static bool isTisEnabled(const TA_IRS_Bus::StationDvaMessageRecord& record);

private:

    // Map to maintain relationship between dva message and it's location in the list control
    std::multimap<unsigned long, DWORD>     m_controlMap;
};

#endif // !defined(AFX_DVALISTCONTROL_MODEL_H__AA87B8BD_6BE9_484A_B86B_BD8B058B88F8__INCLUDED_)
