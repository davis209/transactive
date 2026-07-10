/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/MainTabControl.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#if !defined(AFX_MAINTABCONTROL_H__8721B8E6_DB98_4445_ACB4_765AF2BADA75__INCLUDED_)
#define AFX_MAINTABCONTROL_H__8721B8E6_DB98_4445_ACB4_765AF2BADA75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MainTabControl.h : header file
//
#include <vector>

#include "bus/mfc_extensions/src/tab_dialog_control/TabDialogCtrl.h"
#include "app/maintenance_management/MmsController/src/ScadaMmsInhibit.h"
#include "app/maintenance_management/MmsController/src/NonPhysicalSubsystemMmsInhibit.h"


/////////////////////////////////////////////////////////////////////////////
// CMainTabControl window

using namespace TA_Base_Bus;

namespace TA_IRS_App
{

	class MmsControllerModel;

	class CMainTabControl : public CTabDialogCtrl
	{
		// Construction
	public:
		CMainTabControl();
		
		// Attributes
	public:
		
		// Operations
	public:
 		
		// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CMainTabControl)
	protected:
	//}}AFX_VIRTUAL
		
		// Implementation
	public:
		void setMmsController(MmsControllerModel* model);
		void updateTabControls();

 		virtual ~CMainTabControl();
		
		// Generated message map functions
	protected:
		//{{AFX_MSG(CMainTabControl)
		// NOTE - the ClassWizard will add and remove member functions here.
		//}}AFX_MSG
		
		DECLARE_MESSAGE_MAP()

	private:
		enum ETabIndex
		{
			EScadaTab = 0,
			ESystemTab = 1
		};

		CScadaMmsInhibit m_scadaMmsInhibit;	 
		CNonPhysicalSubsystemMmsInhibit m_systemMmsInhibit;
		MmsControllerModel* m_model;
 
	};
}
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINTABCONTROL_H__8721B8E6_DB98_4445_ACB4_765AF2BADA75__INCLUDED_)
