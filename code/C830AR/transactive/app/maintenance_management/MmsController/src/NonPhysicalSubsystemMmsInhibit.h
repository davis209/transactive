/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/NonPhysicalSubsystemMmsInhibit.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#if !defined(AFX_NONPHYSICALSUBSYSTEMMMSINHIBIT_H__C57F8A24_8DA4_4B28_82F5_F235C7AB5A59__INCLUDED_)
#define AFX_NONPHYSICALSUBSYSTEMMMSINHIBIT_H__C57F8A24_8DA4_4B28_82F5_F235C7AB5A59__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NonPhysicalSubsystemMmsInhibit.h : header file
//
#include "app/maintenance_management/MmsController/src/resource.h"
#include "app/maintenance_management/MmsController/src/NonPhysicalSubsystemTree.h"
#include "app/maintenance_management/MmsController/src/ButtonUpdateType.h"
#include "app/maintenance_management/MmsController/src/MmsInhibitListCtrl.h"
#include "app/maintenance_management/MmsController/src/InhibitResult.h"
 
/////////////////////////////////////////////////////////////////////////////
// CNonPhysicalSubsystemMmsInhibit dialog
namespace TA_Base_Core
{
	class ConfigEntity;	
	class IEntityData;
}
namespace TA_IRS_App
{

	class MmsControllerModel;
	
	class CNonPhysicalSubsystemMmsInhibit : public CPropertyPage
	{
		// Construction
	public:
		CNonPhysicalSubsystemMmsInhibit(CWnd* pParent = NULL);   // standard constructor
		
		void setMmsController(MmsControllerModel* model);

		void updateSystemButtons(const EButtonUpdateType updateType = ALL);
		void updateSystemInhibitList(bool bForceUpdate = false);
		 
		// Dialog Data
		//{{AFX_DATA(CNonPhysicalSubsystemMmsInhibit)
		enum { IDD = IDD_SUBSYSTEMSINHIBIT };		
		CMmsInhibitListCtrl m_mmsInhibitedList;
		CButton	m_removeSystemButton;
		CButton	m_applySystemInhibitButton;
		CButton	m_addSystemButton;
		//}}AFX_DATA	
		
		// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CNonPhysicalSubsystemMmsInhibit)
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support	
		//}}AFX_VIRTUAL
		
		// Implementation
	protected:
		
		// Generated message map functions
		//{{AFX_MSG(CNonPhysicalSubsystemMmsInhibit)
		virtual BOOL OnInitDialog();
		afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);		
		afx_msg void OnSelchangedTreeSystem(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg LRESULT OnSystemButtonsUpdate(UINT wParam, LONG lParam);
		afx_msg void OnNonPhysicalSubsystemAddClick();
		afx_msg void OnNonPhysicalSubsystemRemoveClick();
		afx_msg void OnNonPhysicalSubsystemApplyClick();
	    afx_msg void OnClickMmsInhibitedList(NMHDR* pNMHDR, LRESULT* pResult);
	    afx_msg void OnSetfocusTreeSystem(NMHDR* pNMHDR, LRESULT* pResult);
	    afx_msg LRESULT OnUpdateMMSInhibitList(UINT wParam, LONG lParam);
	//}}AFX_MSG
		DECLARE_MESSAGE_MAP()

	private:
		void applyMmsInhibit(std::vector<TA_Base_Core::ConfigEntity*>& entityMmsInhibitList, bool bApplyAction);
	 
		CString getOperatorName();
	
		bool checkAndSelectNonPhysicalSubsystemEntities();

        //Mintao++ TD16762
        bool isNonPhysicalSubSystemTreePopulated;
	
		NonPhysicalSubsystemTree m_nonPhysicalSubSystemTree;
		MmsControllerModel*		 m_model;
		
		typedef std::map<unsigned long, TA_Base_Core::ConfigEntity*> ConfigList;		
	 
		ConfigList m_cachedEntitiesforMMSInhibitOn;
		ConfigList m_cachedEntitiesforMMSInhibitOff;

		static const std::string MMS_INHIBIT_ON;
		static const std::string MMS_INHIBIT_OFF;

	};
	
}
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NONPHYSICALSUBSYSTEMMMSINHIBIT_H__C57F8A24_8DA4_4B28_82F5_F235C7AB5A59__INCLUDED_)
