/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/job_request_gui/src/JobRequestMgr.h $
  * @author:  
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  *
  */

#if !defined(AFX_JOBREQUESTMGR_H__13EF9430_909E_4319_9238_94999F357E69__INCLUDED_)
#define AFX_JOBREQUESTMGR_H__13EF9430_909E_4319_9238_94999F357E69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "bus/generic_gui/src/TransActiveWinApp.h"

/////////////////////////////////////////////////////////////////////////////
// CJobRequestMgrApp:
// See JobRequestMgr.cpp for the implementation of this class
//
//--entity-name=JobRequestManager --launch-condition=alarm:345:alarmId
namespace TA_IRS_App
{
	class CJobRequestMgrApp : public TA_Base_Bus::TransActiveWinApp
	{
	public:
		CJobRequestMgrApp();

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CJobRequestMgrApp)
		public:
		//}}AFX_VIRTUAL

	// Implementation

		//{{AFX_MSG(CJobRequestMgrApp)
			// NOTE - the ClassWizard will add and remove member functions here.
			//    DO NOT EDIT what you see in these blocks of generated code !
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
	};

} //end namespace TA_IRS_App

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JOBREQUESTMGR_H__13EF9430_909E_4319_9238_94999F357E69__INCLUDED_)
