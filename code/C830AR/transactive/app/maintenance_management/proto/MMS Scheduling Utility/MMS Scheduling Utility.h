/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/proto/MMS Scheduling Utility/MMS Scheduling Utility.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// MMS Scheduling Utility.h : main header file for the MMS SCHEDULING UTILITY application
//

#if !defined(AFX_MMSSCHEDULINGUTILITY_H__D2F49743_0940_4D25_9C83_BEB82E730B50__INCLUDED_)
#define AFX_MMSSCHEDULINGUTILITY_H__D2F49743_0940_4D25_9C83_BEB82E730B50__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// MMSSchedulingUtilityApp:
// See MMS Scheduling Utility.cpp for the implementation of this class
//

class MMSSchedulingUtilityApp : public CWinApp
{
public:
	MMSSchedulingUtilityApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MMSSchedulingUtilityApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(MMSSchedulingUtilityApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MMSSCHEDULINGUTILITY_H__D2F49743_0940_4D25_9C83_BEB82E730B50__INCLUDED_)
