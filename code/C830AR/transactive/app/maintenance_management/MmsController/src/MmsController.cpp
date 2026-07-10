/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source: $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/MmsController.cpp $
  * @author Darren Sampson
  * @version $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by: $Author: builder $
  * 
  * Main application entry point for the MMS Controller application
  *
  */

#pragma warning ( disable : 4250 4786 4284)

#include "app/maintenance_management/MmsController/src/stdafx.h"
#include "app/maintenance_management/MmsController/src/MmsController.h"
#include "app/maintenance_management/MmsController/src/MmsControllerDlg.h"
#include "app/maintenance_management/MmsController/src/MmsControllerGUI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using TA_Base_Bus::TransActiveWinApp;

namespace TA_IRS_App
{
	BEGIN_MESSAGE_MAP(CMmsControllerApp, TransActiveWinApp)
		//{{AFX_MSG_MAP(CMmsControllerApp)
			// NOTE - the ClassWizard will add and remove mapping macros here.
			//    DO NOT EDIT what you see in these blocks of generated code!
		//}}AFX_MSG		
	END_MESSAGE_MAP()
	
	CMmsControllerApp::CMmsControllerApp() : 
		TA_Base_Bus::TransActiveWinApp(new CMmsControllerGUI(), "MMS Controller")
	{
	}
	
	CMmsControllerApp theApp;
}
