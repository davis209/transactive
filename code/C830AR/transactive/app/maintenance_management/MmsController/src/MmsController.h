/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source: $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/MmsController.h $
  * @author Darren Sampson
  * @version $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by: $Author: builder $
  * 
  * Main application entry point for the MMS Controller application
  *
  */

#if !defined(AFX_MmsCONTROLLER_H__1ACF5E6B_3440_4FFC_8DAE_034BF0EC2F77__INCLUDED_)
#define AFX_MmsCONTROLLER_H__1ACF5E6B_3440_4FFC_8DAE_034BF0EC2F77__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "bus/generic_gui/src/TransActiveWinApp.h"


namespace TA_IRS_App
{
	class CMmsControllerApp : public TA_Base_Bus::TransActiveWinApp
	{
	public:
		CMmsControllerApp();


		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CMmsControllerApp)				
		//}}AFX_VIRTUAL
	

		//{{AFX_MSG(CMmsControllerApp)
			// NOTE - the ClassWizard will add and remove member functions here.
			//    DO NOT EDIT what you see in these blocks of generated code !
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
	};

} // TA_IRS_App

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MmsCONTROLLER_H__1ACF5E6B_3440_4FFC_8DAE_034BF0EC2F77__INCLUDED_)
