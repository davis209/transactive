/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DvaVersionListCtrl.h $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * Helper class for displaying DVA Message data
 *
 */

#if !defined(AFX_DvaVersionListCtrl_H__AA87B8BD_6BE9_484A_B86B_BD8B058B88F8__INCLUDED_)
#define AFX_DvaVersionListCtrl_H__AA87B8BD_6BE9_484A_B86B_BD8B058B88F8__INCLUDED_

#include <string>
#include <set>

#include "bus/mfc_extensions/src/coloured_controls/HighlightItemListCtrl.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "bus/pa/cachedmaps/src/CachedMapDefinitions.h"

class DvaVersionListCtrl : public TA_Base_Bus::HighlightItemListCtrl
{
    // Construction

public:

    DvaVersionListCtrl();

    virtual ~DvaVersionListCtrl();

    /**
     * addRowData
     *
     * Adds a single row to this version list control using the specified data
     *
     * @param rowData the data to use when adding the row
     *
     * @exception InvalidPasPacketException if the key contained in rowData matches
     *              one from an item already in the list
     *
     */
    void    addRowData(const TA_IRS_Bus::DvaVersionRecord& rowData);
    void    addRowData(const TA_IRS_Bus::DvaPublicVersionRecord& rowData);
    void    addRowData(const TA_IRS_Bus::DvaPrivateVersionRecord& rowData);

    /**
     * updateRowData
     *
     * Updates a single row of this version list control with the specified data
     *
     * @param rowData the data to use in updating the row
     *
     * @exception InvalidPasPacketException if the key contained in rowData doesn't
     *          correspond to any item currently in this list
     *
     */
    void    updateRowData(const TA_IRS_Bus::DvaVersionRecord& rowData);
    void    updateRowData(const TA_IRS_Bus::DvaPublicVersionRecord& rowData);
    void    updateRowData(const TA_IRS_Bus::DvaPrivateVersionRecord& rowData);

    /**
     * updateRowData
     *
     * Passthrough function, converts the input data into format required
     *  for this class, and passes on to main updateRowData function
     *
     * @param rowData the data to use when adding the row
     *
     */
    void updateRowData(const TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecord& rowData);
    void updateRowData(const TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecord& rowData);
    void updateRowData(const TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecord& rowData);

    /**
     * Allows this list control to scroll other list controls
     *  (intercepting it's own HSCROLL calls)
     * Each time horizontal scroll event is received, it scrolls
     *  all the registered child objects by the same amount
     *
     * @param child the child list control to scroll
     *
     */
    void addHorizontalScrollingChild(CListCtrl& child);

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(DvaVersionListCtrl)
    //}}AFX_VIRTUAL

    // Generated message map functions

protected:

    //{{AFX_MSG(DvaVersionListCtrl)
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

private:

    /**
     * getStationName
     *
     * @return string representation of station suitable for display in this list
     *
     * @param locationKey the location key the stations located at
     *
     */
    std::string getStationName(unsigned long locationKey) const;

    /**
     * convertVersionToString
     *
     * Converts a version object to a string representation
     *
     * @param version the input version (as received from update packet)
     *
     * @return string representation suitable for display in cells in this list
     *
     */
    std::string convertVersionToString(const char* version);

    /**
     * findVersionIndex
     *
     * Finds the index of the specified version in this listbox
     *
     * @param locationKey the location key search parameter
     *
     * @return the index of the corresponding version row, or -1 if not found
     *
     */
    int findVersionIndex(unsigned long locationKey) const;

    /**
     * setRowText
     *
     * Updates the row text with information contained in passed object
     *
     * @param rowData the data source for the update
     *
     * @param nItem the item index of the row to be updated in this list
     *
     */
    void    setRowText(int nItem, const TA_IRS_Bus::DvaVersionRecord& rowData);
    void    setRowText(int nItem, const TA_IRS_Bus::DvaPublicVersionRecord& rowData);
    void    setRowText(int nItem, const TA_IRS_Bus::DvaPrivateVersionRecord& rowData);

    struct ListContainer
    {
        CListCtrl* listControl;
        int wOffset;
    };

    std::vector<ListContainer> m_hScrollChildren;

    DWORD   m_excessWidth;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DvaVersionListCtrl_H__AA87B8BD_6BE9_484A_B86B_BD8B058B88F8__INCLUDED_)
