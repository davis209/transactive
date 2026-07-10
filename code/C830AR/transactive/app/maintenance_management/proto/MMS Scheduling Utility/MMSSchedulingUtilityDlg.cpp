/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/proto/MMS Scheduling Utility/MMSSchedulingUtilityDlg.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// MMSSchedulingUtilityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MMS Scheduling Utility.h"
#include "MMSSchedulingUtilityDlg.h"
#include <sstream>



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MMSSchedulingUtilityDlg dialog

MMSSchedulingUtilityDlg::MMSSchedulingUtilityDlg(CWnd* pParent /*=NULL*/)
	: CDialog(MMSSchedulingUtilityDlg::IDD, pParent)
{
	time_t now;
	time(&now);

	CString periodStr = toString(getPeriods(now)).c_str();

	unsigned int period = 1;
	m_lastPeriodInStr = "1";

	time_t nextDate = getNextDate(period);

	//{{AFX_DATA_INIT(MMSSchedulingUtilityDlg)
	m_dateIn = now;
	m_periodsOut = periodStr;
	m_periodIn = period;
	m_dateOut = nextDate;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}



void MMSSchedulingUtilityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MMSSchedulingUtilityDlg)
	DDX_Control(pDX, IDC_PERIODS_OUT, m_periodsOutCtrl);
	DDX_Control(pDX, IDC_DATE_IN, m_dateInCtrl);
	DDX_Control(pDX, IDC_DATE_OUT, m_dateOutCtrl);
	DDX_Control(pDX, IDC_PERIOD_IN, m_periodInCtrl);
	DDX_DateTimeCtrl(pDX, IDC_DATE_IN, m_dateIn);
	DDX_Text(pDX, IDC_PERIODS_OUT, m_periodsOut);
	DDX_Text(pDX, IDC_PERIOD_IN, m_periodIn);
	DDX_DateTimeCtrl(pDX, IDC_DATE_OUT, m_dateOut);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(MMSSchedulingUtilityDlg, CDialog)
	//{{AFX_MSG_MAP(MMSSchedulingUtilityDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(DTN_CLOSEUP, IDC_DATE_IN, OnCloseupDateIn)
	ON_EN_UPDATE(IDC_PERIOD_IN, OnUpdatePeriodIn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MMSSchedulingUtilityDlg message handlers

BOOL MMSSchedulingUtilityDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void MMSSchedulingUtilityDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void MMSSchedulingUtilityDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR MMSSchedulingUtilityDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void MMSSchedulingUtilityDlg::OnCloseupDateIn(NMHDR* pNMHDR, LRESULT* pResult) 
{

	
	CTime entry;
	m_dateInCtrl.GetTime(entry);
	time_t t = entry.GetTime();
	std::vector<unsigned int> periods = getPeriods(t);
	CString periodStr = toString(periods).c_str();	
	m_periodsOutCtrl.SetWindowText(periodStr);
	*pResult = 0;
}




void MMSSchedulingUtilityDlg::OnUpdatePeriodIn() 
{
	CString currentText;
	this->m_periodInCtrl.GetWindowText(currentText);
	unsigned int currentVal = atoi( currentText );

	//if not valid abort edit
	if(currentVal < 1 || currentVal > 365)
	{
		m_periodInCtrl.SetWindowText(m_lastPeriodInStr);
		return;
	}
	else
	{
		m_lastPeriodInStr = currentText;
		m_periodIn = currentVal;
		time_t t= getNextDate(m_periodIn);
		m_dateOutCtrl.SetTime(t);

	}
	
}

time_t MMSSchedulingUtilityDlg::getNextDate(unsigned int period)
{
	//assert period > 0 <= 365

	//start by checking today's date
	time_t nextDate;
	time(&nextDate);
	int cnt = 0;

	while(true)
	{
		std::vector<unsigned int> periods = getPeriods(nextDate);

		for(int i = 0; i < periods.size(); i++)
		{

			if(period == periods[i]) 
			{
				return nextDate;
			}
		}

		//add one day to nextDate;
		nextDate = nextDate + (60 * 60 * 24);

		cnt++;
		//assert(cnt < 366);
	}

	return nextDate;
}

std::vector<unsigned int> MMSSchedulingUtilityDlg::getPeriods(time_t date)
{
	//todo assert date >= 0
	
	//convert seconds to days;
	//I have added OFFSET so that mondays always have a 7 period
	//todo adjust this if they want something else
	unsigned int OFFSET = 3;
	const unsigned int days = (date / 60 / 60 / 24) + OFFSET;

	std::vector<unsigned int> periods;


	//now see what ints divide evenly into the number of days
	for(unsigned int factor = 1; factor <= days && factor <= 365; factor++)
	{
		if(0 == days % factor)
		{
			periods.push_back(factor);
		}
	}

	return periods;
}

std::string  MMSSchedulingUtilityDlg::toString(const std::vector<unsigned int>& periods)
{
	std::ostringstream str;

	for(int i = 0; i < periods.size() - 1; i++)
	{
		str << periods[i] << ", ";
	}
	str << periods[i];

	std::string s;
	s = str.str();

	return s;
}
