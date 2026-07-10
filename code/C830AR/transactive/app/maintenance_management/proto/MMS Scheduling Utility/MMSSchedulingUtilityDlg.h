/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/proto/MMS Scheduling Utility/MMSSchedulingUtilityDlg.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// MMSSchedulingUtilityDlg.h : header file
//

#if !defined(AFX_MMSSCHEDULINGUTILITYDLG_H__EA6EA646_C7B9_4545_80B7_A41B6222A37F__INCLUDED_)
#define AFX_MMSSCHEDULINGUTILITYDLG_H__EA6EA646_C7B9_4545_80B7_A41B6222A37F__INCLUDED_



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <string>
#include <ctime>

/////////////////////////////////////////////////////////////////////////////
// MMSSchedulingUtilityDlg dialog

class MMSSchedulingUtilityDlg : public CDialog
{
// Construction
public:
	MMSSchedulingUtilityDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(MMSSchedulingUtilityDlg)
	enum { IDD = IDD_MMSSCHEDULINGUTILITY_DIALOG };
	CEdit	m_periodsOutCtrl;
	CDateTimeCtrl	m_dateInCtrl;
	CDateTimeCtrl	m_dateOutCtrl;
	CEdit	m_periodInCtrl;
	CTime	m_dateIn;
	CString	m_periodsOut;
	UINT	m_periodIn;
	CTime	m_dateOut;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MMSSchedulingUtilityDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(MMSSchedulingUtilityDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnCloseupDateIn(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdatePeriodIn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
std::vector<unsigned int> getPeriods(time_t date);

std::string toString(const std::vector<unsigned int>& periods);

time_t getNextDate(unsigned int);

CString m_lastPeriodInStr;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
};
#endif // !defined(AFX_MMSSCHEDULINGUTILITYDLG_H__EA6EA646_C7B9_4545_80B7_A41B6222A37F__INCLUDED_)
