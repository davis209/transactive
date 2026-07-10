/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DVAVersionsPage.h $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * Controls the DVA version display page
 *
 */

#if !defined(AFX_DvaVersionsPage_H__DF68E4D7_114F_4F13_91FD_8B0068F1D5D7__INCLUDED_)
#define AFX_DvaVersionsPage_H__DF68E4D7_114F_4F13_91FD_8B0068F1D5D7__INCLUDED_

#include "app/pa/PAManager/src/DvaVersionListCtrl.h"
#include "app/pa/PAManager/src/DvaVersionChangeSubscriber.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "bus/mfc_extensions/src/list_ctrl_with_hidden_scrollbar/ListCtrlHiddenSB.h"

#include "app/pa/PAManager/src/DvaPublicVersionChangeSubscriber.h"
#include "app/pa/PAManager/src/DvaPrivateVersionChangeSubscriber.h"
class PAManagerDlg;
/////////////////////////////////////////////////////////////////////////////
// DvaVersionsPage dialog

class DvaVersionsPage : public CPropertyPage
{
    DECLARE_DYNCREATE(DvaVersionsPage)

    // Construction

public:

    DvaVersionsPage(PAManagerDlg* paManagerDlg = NULL);
    ~DvaVersionsPage();

    /**
     * postDialogCreate
     *
     * This function should be called once this property page has been initialized
     * and added to the tab control.  Some of the GetClientRect functions *do not* work
     * properly when put in OnInitDialog, driving the need for this function.  It should
     * be called before the page is to become visible (as view related setup is performed)
     *
     */
    void postDialogCreate();

    // Overrides
    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(DvaVersionsPage)

protected:

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation

protected:

    // Generated message map functions
    //{{AFX_MSG(DvaVersionsPage)
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg LRESULT OnDvaVersionUpdate(UINT wParam, LONG lParam);
    afx_msg void OnItemchangingDvaHeaderLst(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemchangingDvaHeaderLst2(NMHDR* pNMHDR, LRESULT* pResult);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:

    /**
     * setupMainVersionList
     *
     * Helper function to set up the main DVA version list
     *
     */
    void    setupMainVersionList();

    /**
     * setupFirstHeaderList
     *
     * Helper function to set up the topmost header list control
     *
     */
    void    setupFirstHeaderList();

    /**
     * setupSecondHeaderList
     *
     * Helper function to set up the second (down) list control
     *
     */
    void    setupSecondHeaderList();

    /**
     * setupListView (helper function)
     *
     * Sets up basic elements of input list (adds columns)
     *
     * @param list the list to apply changes to
     *
     * @param columnCount the number of columns to add to the list
     *
     * @param accountForScroll whether or not to account for width of scroll
     *          bar when setting the default column width
     *
     * @param format the formatting (left / centre / right aligned) to apply
     *
     */
    static void    setupListView(CListCtrl& list,
                                 DWORD columnCount,
                                 bool accountForScroll,
                                 DWORD format);

    // Dialog Data
    //{{AFX_DATA(DvaVersionsPage)
    enum { IDD = IDD_DVA_VERSIONS };
    TA_Base_Bus::CListCtrlHiddenSB	m_secondVersionHeaderList;
    TA_Base_Bus::CListCtrlHiddenSB	m_firstVersionHeaderList;
    DvaVersionListCtrl	m_versionList;
    CListCtrl	m_versionHeaderList;
    //}}AFX_DATA

    DvaVersionChangeSubscriber  m_dvaVersionChangeSubscriber;
    DvaPublicVersionChangeSubscriber m_dvaPublicVersionChangeSubscriber;
    DvaPrivateVersionChangeSubscriber m_dvaPrivateVersionChangeSubscriber;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DvaVersionsPage_H__DF68E4D7_114F_4F13_91FD_8B0068F1D5D7__INCLUDED_)
