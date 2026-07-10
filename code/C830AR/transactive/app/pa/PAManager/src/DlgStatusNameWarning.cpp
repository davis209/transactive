/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DlgStatusNameWarning.cpp $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
// DlgStatusNameWarning.cpp : implementation file
//

#include "app/pa/PAManager/src/stdafx.h"
#include "app/pa/PAManager/src/pamanager.h"
#include "app/pa/PAManager/src/DlgStatusNameWarning.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgStatusNameWarning dialog

CDlgStatusNameWarning::CDlgStatusNameWarning(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgStatusNameWarning::IDD, pParent)
{
    //{{AFX_DATA_INIT(CDlgStatusNameWarning)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

void CDlgStatusNameWarning::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgStatusNameWarning)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgStatusNameWarning, CDialog)
    //{{AFX_MSG_MAP(CDlgStatusNameWarning)
    ON_BN_CLICKED(ID_CANCEL_CHANGES, OnCancelChanges)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgStatusNameWarning message handlers
void CDlgStatusNameWarning::OnCancelChanges()
{
    // Call dialog cancel directly (the local implementation does nothing -
    // to stop the user hitting the escape key to cancel the dialog)
    CDialog::OnCancel();
}
