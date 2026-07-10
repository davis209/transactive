/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/ScadaMmsInhibit.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#if !defined(AFX_SCADAMMSINHIBIT_H__28274E82_256A_46F1_8512_C1E91844EEA6__INCLUDED_)
#define AFX_SCADAMMSINHIBIT_H__28274E82_256A_46F1_8512_C1E91844EEA6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScadaMmsInhibit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScadaMmsInhibit dialog
//#include "bus/scada/proxies2/src/DataNodeProxySmartPtr.h"

#include "app/maintenance_management/MmsController/src/SubsystemLimitedScadaTree.h"
#include "app/maintenance_management/MmsController/src/ButtonUpdateType.h"
#include "app/maintenance_management/MmsController/src/resource.h"

namespace TA_Base_Core
{
    class DataNodeEntityData;
}

namespace TA_IRS_App
{
	class MmsControllerModel;

 	class CScadaMmsInhibit : public CPropertyPage
	{
		// Construction
	public:		
		CScadaMmsInhibit(CWnd* pParent = NULL);   // standard constructor
 		void setMmsController(MmsControllerModel* model);

		/**
         * Called each time the rights change or the subsystem selection changes, or related
         *  checkbox state changes in order to affect the subsystem buttons
         * @param updateType defines whether we do a full or partial refresh (for efficiency)
         */
		void updateSubsystemButtons(const EButtonUpdateType updateType = ALL);


		// Dialog Data
		//{{AFX_DATA(CScadaMmsInhibit)
		enum { IDD = IDD_SCADAINHIBIT };
		CButton	m_subsystemInhibitCheckbox;
		CButton	m_subsystemInhibitApplyButton;
		//}}AFX_DATA
		
		
		// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CScadaMmsInhibit)
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		//}}AFX_VIRTUAL
		
		// Implementation
	protected:
		
		// Generated message map functions
		//{{AFX_MSG(CScadaMmsInhibit)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSubsystemCheckboxClicked();
	afx_msg void OnSubsystemApplyClicked();
	afx_msg void OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnSubsystemCheckboxUpdate(UINT wParam, LONG lParam);
	//}}AFX_MSG
		DECLARE_MESSAGE_MAP()

	private:	  
        
		SubsystemLimitedScadaTree	m_ScadaTree;
		MmsControllerModel*			m_model;
 	 
	};
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCADAMMSINHIBIT_H__28274E82_256A_46F1_8512_C1E91844EEA6__INCLUDED_)
