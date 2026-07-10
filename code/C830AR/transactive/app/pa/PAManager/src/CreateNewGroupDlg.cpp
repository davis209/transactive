/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/CreateNewGroupDlg.cpp $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
// CreateNewGroupDlg.cpp : implementation file
//

#include "app/pa/PAManager/src/stdafx.h"
#include "app/pa/PAManager/src/pamanager.h"
#include "app/pa/PAManager/src/CreateNewGroupDlg.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

CreateNewGroupDlg::CreateNewGroupDlg(CString windowTitle,
                                     CString textPrompt,
                                     CString defaultName,
                                     CWnd* pParent /*=NULL*/)
    :
    CDialog(CreateNewGroupDlg::IDD, pParent),
    m_windowTitle(windowTitle)
{
    //{{AFX_DATA_INIT(CreateNewGroupDlg)
    m_name = _T(defaultName);
    m_textPrompt = _T(textPrompt);
    //}}AFX_DATA_INIT
}

void CreateNewGroupDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CreateNewGroupDlg)
    DDX_Control(pDX, IDC_EDITNAME, m_nameControl);
    DDX_Text(pDX, IDC_EDITNAME, m_name);
    DDV_MaxChars(pDX, m_name, 40);
    DDX_Text(pDX, IDC_STATICTEXT, m_textPrompt);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CreateNewGroupDlg, CDialog)
    //{{AFX_MSG_MAP(CreateNewGroupDlg)
    ON_WM_SHOWWINDOW()
    ON_BN_CLICKED(ID_CLOSE, OnClose)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CreateNewGroupDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Apply the title
    SetWindowText(m_windowTitle);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CreateNewGroupDlg::OnOK()
{
    CDialog::OnOK();
}

void CreateNewGroupDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialog::OnShowWindow(bShow, nStatus);

    // Select all of default name text
    m_nameControl.SetFocus();
    m_nameControl.SetSel(0, -1);
}

void CreateNewGroupDlg::OnClose()
{
    // Close down the dialog
    CDialog::OnCancel();
}
