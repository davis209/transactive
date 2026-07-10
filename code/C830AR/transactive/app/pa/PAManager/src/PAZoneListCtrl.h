/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAZoneListCtrl.h $
 * @author Sean Liew
 * @version $Revision: #4 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * Maintains a list control for PA Zones
 *  Because there is only one PA Zone list in the PA Manager application
 *  this control is mostly self contained (listens for update data updates
 *  independently, updating view as required)
 *
 */

#if !defined(AFX_PAZONELISTCTRL_H__0F637CDF_8898_4678_938E_4506A31BBE48__INCLUDED_)
#define AFX_PAZONELISTCTRL_H__0F637CDF_8898_4678_938E_4506A31BBE48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>

#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "bus/pa/cachedmaps/src/CachedMapDefinitions.h"
#include "bus/mfc_extensions/src/list_ctrl_selection_without_focus/ListCtrlSelNoFocus.h"
#include "bus/mfc_extensions/src/auto_sort_list_ctrl/AutoSortListCtrl.h"
#include "bus/mfc_extensions/src/fixed_column_header_ctrl/FixedHeaderCtrl.h"
// liuyu++ TD8849
#include "bus/mfc_extensions/src/auto_sort_list_ctrl/ISortingCallback.h"
// ++liuyu TD8849

class PrioritySchemeUpdateSubscriber;

class PaZoneListCtrl : public TA_Base_Bus::ListCtrlSelNoFocus
// liuyu++ TD8849
, public TA_Base_Bus::ISortingCallback
// ++liuyu TD8849
{
    // Construction

public:

    PaZoneListCtrl();

    // Attributes

public:

    // Operations

public:

    enum
    {
        PA_COL_LOCATION_NAME    = 0,
        // song toan++
        // Bug #276: Priority column is not updated as needed.
        //
        // Label column only exists at OCC, not at other stations. Therefore, we can't
        // use constants to index correctly the Zone list control. Instead, we replace
        // these constants by member data. These members will be assigned correct values
        // at constructor according to station type.
        //
        // To minimise the changes, the new members will bear same names.
        //
        //
        // ++song toan
        // Bug #276
    };

    /**
     * getSelectedKeys
     *
     * @return an array of keys defining the currently selected objects
     *      any selected object that is a null entry will
     *      not make it into the result (that is,
     *      those with key==TA_Base_Core::DatabaseKey::getInvalidKey())
     *
     */
    //Keys getSelectedKeys() const;

    /**
     * getSelectedKeys
     *
     * @return an array of keys defining the current contents of list
     *      any object that is a null entry will
     *      not make it into the result (that is,
     *      those with key==TA_Base_Core::DatabaseKey::getInvalidKey())
     *
     */
    Keys getAllKeys() const;

    /**
     * initialise
     *
     * Calls once off on startup to set up the control
     * Also initialises Priority Scheme udpate listener, performs
     * initial collection of priority scheme data
     *
     * @param agent the agent to use to collect initial priority scheme data
     *
     * @pre the m_hWnd member must be valid (ie control window already constructed)
     *
     */
    void initialise(PAAgentNamedObject& agent);

    /**
     * SetItemData
     *
     * Interprets the input record and adds new row
     * to this control
     *
     * @pre a record with a matching pKey has not been added prior
     *      (unless DeleteAllItems was called after previous addition)
     *
     * @param record the data used to fill out text fields in control
     *
     */
    BOOL SetItemData(const TA_IRS_Bus::PaZoneRecord& record);

    /**
     * DeleteAllItems
     *
     * Overriden to clear out local map on deletion
     *
     */
    BOOL    DeleteAllItems();

    /**
     * updateZoneList
     *
     * Performs a complete refresh of this list control using the input data
     * if the input parameter keys matches the data (exactly) that was last passed
     *  to this function, there is no change (database not queried) - should
     *  call invalidateZoneData to force a refresh
     *
     * @param keys the new set of keys used to define what zones should be displayed
     *
     */
    void updateZoneList(const Keys& keys);

    /**
     * invalidateZoneData
     *
     * forces udpateZoneList to always perform a full refresh whenever called
     *
     */
    void    invalidateZoneData();

    /**
     * containsPrimaryKey
     *
     * @param zoneKey the primary key of the zone we're looking for
     *
     * @return true if a matching zone was found (matching item from
     *              the last call to updateZoneList)
     *
     */
    bool    containsPrimaryKey(unsigned long zoneKey) const;

    /**
     * setPriorityEstimationMode
     *
     * Updates the currently used mode for zone priority estimation
     *      this value is used to index directly into the ModePriorities sequence
     *      returned from the PA Agent (so it's up to the user to choose a valid
     *      value, otherwise will get PRIORITY_UNAVAILABLE displayed)
     * This function prompts a refresh of this control (to display new data)
     *
     * @param newMode the new mode to use when calculating priority estimations
     *          (for display in Estimated Priority column).  Set to -1 if
     *          the current mode is undefined (so no priority calculations can be
     *          performed).  If the currently set mode matches the new mode, the
     *          function returns early and nothing is done.
     *
     */
    void    setPriorityEstimationMode(int newMode);

    // liuyu++ TD8849
    /**
     * sortCallback
     *
     * User defined sorting function
     *
     * @return positive value if item lParam2 before lParam1, negative if lParam1 should
     *      come first
     *
     * @param lParam1, lParam2 user defined values passed in from sort operation (app specific)
     * @param columnIndex the index of the column being sorted
     * @param currentSort defines the type of sorting specified (ie ascending or descending)
     */
    virtual int sortCallback(LPARAM lParam1, LPARAM lParam2, int columnIndex, TA_Base_Bus::AutoSortListCtrl::ECurrentSort currentSort);
    // ++liuyu TD8849

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(PaZoneListCtrl)
    //}}AFX_VIRTUAL

    // Implementation

public:

    virtual ~PaZoneListCtrl();

    // Generated message map functions

protected:

    //{{AFX_MSG(PaZoneListCtrl)
    afx_msg LRESULT OnPrioritySchemeUpdate(UINT wParam, LONG lParam);
    afx_msg void OnDestroy();
    //}}AFX_MSG
    void CleanupItemData();

    DECLARE_MESSAGE_MAP()

    /**
     * Intercept all messages to allow sorting
     *
     */
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

private:

    /**
     * getPriorityStr
     *
     * @return the priority (string representation) for the specified zone, if
     *          there is no priority data set, returns PRIORITY_UNAVAILABLE
     *
     * @param record the zone we're getting currently priority for
     *
     */
    const std::string getPriorityStr(const TA_IRS_Bus::PaZoneRecord& record) const;

    /**
     * refreshPriorityData
     *
     * Uses the input priority data to update the Est Priority columns
     *  The data passed in will be subsequently used when removing/adding new zones
     *  (meaning the data is cached locally)
     *
     * @param priorityScheme the current priority data set
     *
     */
    void    refreshPriorityData(const TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme& priorityScheme);

    /**
     * addNullEntry
     *
     * Called to add in an entry showing zone not found
     * (when a PA Zone no longer exists)
     *
     * @param the key of the zone that no longer exists
     *
     */
    void addNullEntry(unsigned long key);

    /**
     * refreshZonePriorityDisplay
     *
     * Refreshes all the zone priority column entries in this control
     *
     */
    void refreshZonePriorityDisplay();

    /// The currently displayed zone data in this list
    std::list<unsigned long> m_currentZoneListKeys;

    // To allow us to keep the Zone List "Priority" col up to date
    PrioritySchemeUpdateSubscriber* m_prioritySchemeUpdateSubscriber;

    TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme m_prioritySchemeData;

    // Current mode for the purposes of Estimated Priority (ie emergency adhoc mode etc)
    // This value is used to estimate priority for each zone based on mode
    int m_currentMode = -1;

    // song toan++
    // Bug #276
    //
    // m_colSortingInfo was changed to pointer because number of columns of the
    // list control is not a constant anymore. we can initialize only after
    // number of columns was calculated at the constructor's body
    TA_Base_Bus::AutoSortListCtrl::SortInfo* m_colSortingInfo;
    // ++song toan
    // Bug #276

    TA_Base_Bus::FixedHeaderCtrl     m_fixedHeaderCtrl;

    // song toan++
    // Bug #276

    bool m_bIsAtOCC;        // true if runs at OCC

    // these members replace enumeration corresponding constants
    // and will be determined at constructor
    int PA_COL_LABEL, PA_COL_AREA, PA_COL_PRIORITY, PA_COL_MAX;

    // ++song toan
    // Bug #276
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAZONELISTCTRL_H__0F637CDF_8898_4678_938E_4506A31BBE48__INCLUDED_)
