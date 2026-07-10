/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/Createnewgroupdlg.h $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
#if !defined(AFX_ADDDYNAMICTRAINGROUP_H__9C1E6B0B_2251_4AAB_A299_10D709805287__INCLUDED_)
#define AFX_ADDDYNAMICTRAINGROUP_H__9C1E6B0B_2251_4AAB_A299_10D709805287__INCLUDED_

#if _MSC_VER > 1000
    #pragma once
#endif // _MSC_VER > 1000
// CreateNewGroupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CreateNewGroupDlg dialog

class CreateNewGroupDlg : public CDialog
{
    // Construction

public:

    /**
     * CreateNewGroupDlg (constructor)
     *
     * @param windowTitle
     *
     * @param textPrompt the text displaying just before text
     *          entry panel (ie Please enter a new...)
     *
     * @param initialName the initial text in text entry panel
     *
     */
    CreateNewGroupDlg(CString windowTitle,
                      CString textPrompt,
                      CString initialName,
                      CWnd* pParent = NULL);   // standard constructor

    /**
     * getName
     *
     * @return the name the user entered in dialog box
     *
     */
    CString getName() {
        return m_name;
    }

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CreateNewGroupDlg)

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
    //{{AFX_MSG(CreateNewGroupDlg)
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnClose();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:

    // Dialog Data
    //{{AFX_DATA(CreateNewGroupDlg)
    enum { IDD = IDD_DIALOGENTERSAVENAME };
    CEdit	m_nameControl;
    CString	m_name;
    CString	m_textPrompt;
    //}}AFX_DATA

    CString m_windowTitle;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDDYNAMICTRAINGROUP_H__9C1E6B0B_2251_4AAB_A299_10D709805287__INCLUDED_)
