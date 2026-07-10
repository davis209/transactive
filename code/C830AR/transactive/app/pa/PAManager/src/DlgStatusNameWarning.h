/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DlgStatusNameWarning.h $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
#if !defined(AFX_DLGSTATUSNAMEWARNING_H__782A623D_5E93_48E5_9CD4_8B98B723D98A__INCLUDED_)
#define AFX_DLGSTATUSNAMEWARNING_H__782A623D_5E93_48E5_9CD4_8B98B723D98A__INCLUDED_

#if _MSC_VER > 1000
    #pragma once
#endif // _MSC_VER > 1000
// DlgStatusNameWarning.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgStatusNameWarning dialog

class CDlgStatusNameWarning : public CDialog
{
    // Construction

public:

    CDlgStatusNameWarning(CWnd* pParent = NULL);   // standard constructor

    // Dialog Data
    //{{AFX_DATA(CDlgStatusNameWarning)
    enum { IDD = IDD_DIALOGNAMEWARNING };
    // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDlgStatusNameWarning)

protected:

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation

protected:

    /**
     * OnCancel
     *
     * This method has been implemented to hide accidental calls when
     * the ESC key is pressed. It can be overriden with new behaviour if required.
     */
    virtual afx_msg void OnCancel() {}

    // Generated message map functions
    //{{AFX_MSG(CDlgStatusNameWarning)
    afx_msg void OnCancelChanges();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSTATUSNAMEWARNING_H__782A623D_5E93_48E5_9CD4_8B98B723D98A__INCLUDED_)
