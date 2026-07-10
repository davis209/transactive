/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/BroadcastParametersDlg.h $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
#if !defined(AFX_BROADCASTPARAMETERSDLG_H__D65D9351_B3FB_4BEF_90DC_49393FAE16CF__INCLUDED_)
#define AFX_BROADCASTPARAMETERSDLG_H__D65D9351_B3FB_4BEF_90DC_49393FAE16CF__INCLUDED_

#if _MSC_VER > 1000
    #pragma once
#endif // _MSC_VER > 1000
// BroadcastParametersDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// BroadcastParametersDlg dialog

class BroadcastParametersDlg : public CDialog
{
    // Construction

public:

    BroadcastParametersDlg(CWnd* pParent = NULL);   // standard constructor

    void SetBroadcastId(const std::string& bi)
    {
        m_broadcastId = bi;
    }

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(BroadcastParametersDlg)

protected:

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation

protected:

    std::string m_broadcastId;

    // Generated message map functions
    //{{AFX_MSG(BroadcastParametersDlg)
    virtual void OnOK();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:

    // Dialog Data
    //{{AFX_DATA(BroadcastParametersDlg)
    enum { IDD = IDD_BROADCAST_PARAMETERS_DLG };
    CEdit	m_dataEb;
    //}}AFX_DATA
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BROADCASTPARAMETERSDLG_H__D65D9351_B3FB_4BEF_90DC_49393FAE16CF__INCLUDED_)
