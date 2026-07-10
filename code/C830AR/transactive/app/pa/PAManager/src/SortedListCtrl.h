/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/SortedListCtrl.h $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
/* Static definitions are provided so that customised list control can use
 * the sorting functionality without actually having to derive from SortedListCtrl
 * (because often we want to derive from a different base list control, and multiple
 *  inheritance is not suitable for these cases)
 */

/*
class SortedListCtrl : public CListCtrl
{
public:

    static BOOL SortOnColumnClick(CListCtrl& listControl, NMHDR* pNMHDR, LRESULT* pResult)
    {
        *pResult = 0;

        NMLISTVIEW* view = (NMLISTVIEW*) pNMHDR;
        if (-1 == view->iItem)
        {
            requestColumnSort(listControl, view->iSubItem);
        }

        // Allow processing by parent
        return FALSE;
    }

protected:

    afx_msg BOOL OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
    {
        return SortOnColumnClick(*this, pNMHDR, pResult);
    }

private:

    static void requestColumnSort(CListCtrl& listControl, DWORD columnIndex)
    {
        // Gather up the item data values, so we can restore them after sort
    }

    enum ESortType
    {
        BY_STRING,
        BY_INTEGER
    };

    enum ECurrentSort
    {
        ASCENDING,
        DESCENDING
    };

    // Define what kind of sort to perform on a column by column basis, default to BY_STRING
    std::map<int, ESortType>  m_sortingSemantics;
    // Current sort state, if currently undefined (as in startup), will enter ASCENDING first off
    std::map<int, ECurrentSort>  m_sortState;

    DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(SortedListCtrl, CListCtrl)
    ON_NOTIFY_REFLECT_EX(LVN_COLUMNCLICK, OnColumnClick)
END_MESSAGE_MAP()

*/