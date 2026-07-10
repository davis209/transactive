/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source: $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/MmsControllerDlg.h $
  * @author Darren Sampson
  * @version $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by: $Author: builder $
  * 
  * Main Dialog class for the MMS Controller application
  *
  */

#ifdef __WIN32__
#pragma warning(disable:4786)
#pragma warning(disable:4503)  // warning C4503: '  ' : decorated name length exceeded, name was truncated
#endif

#if !defined(AFX_MMSCONTROLLERDLG_H__1EA52942_C4E8_41F3_8375_72599BB4F27E__INCLUDED_)
#define AFX_MMSCONTROLLERDLG_H__1EA52942_C4E8_41F3_8375_72599BB4F27E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "app/maintenance_management/MmsController/src/resource.h"
#include "app/maintenance_management/MmsController/src/MainTabControl.h"
#include "app/maintenance_management/MmsController/src/MmsControllerModel.h"
#include "app/maintenance_management/MmsController/src/ButtonUpdateType.h"

#include "bus/generic_gui/src/TransActiveDialog.h"


namespace TA_IRS_App
{
    class RightsChangedWindowNotificationHandler;
    
	class CMmsControllerDlg : public TA_Base_Bus::TransActiveDialog
							
	{	
	public:		
		CMmsControllerDlg(TA_Base_Bus::IGUIAccess& genericGUICallback);	
        virtual ~CMmsControllerDlg();

		void init();
        //{{AFX_DATA(CMmsControllerDlg)
		enum { IDD = IDD_MMSCONTROLLER_DIALOG };	
		//}}AFX_DATA
		
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CMmsControllerDlg)
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
		//}}AFX_VIRTUAL

	protected:

		// Generated message map functions
		//{{AFX_MSG(CMmsControllerDlg)
		virtual BOOL OnInitDialog();
        afx_msg void OnHelpHelpMmsController();
		afx_msg void OnHelpAboutMmsController();
		afx_msg void OnCloseClicked();
        afx_msg void OnGlobalApplyClicked();
        afx_msg void OnGlobalCheckboxClicked();
		afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
 		afx_msg void OnDestroy();
    	afx_msg LRESULT OnRightsChanged(UINT wParam, LONG lParam);
		afx_msg LRESULT OnUpdateControls(UINT wParam, LONG lParam);
	afx_msg void OnUpdateAppExit(CCmdUI* pCmdUI);
	afx_msg void OnClose();
	//}}AFX_MSG
		DECLARE_MESSAGE_MAP()

	private:	

         /**
         * Called each time rights change or associated checkbox state changes (to update apply button)
         */
        void updateGlobalButtons(EButtonUpdateType updateType = ALL);

        /// Dialog controls
         CButton     m_globalInhibitCheckbox;
         CButton     m_globalInhibitApplyButton;
 
        // Cached rights value (refreshed automatically on rights change)
        bool        m_hasRightToInhibitGlobal;
              
        RightsChangedWindowNotificationHandler* m_rightsChangeHandler;
        MmsControllerModel*  m_model;        
 		CMainTabControl m_mainTab;	
	private:
		static DWORD WINAPI ThreadFunc(LPVOID lpThreadParameter);
	};

} // TA_IRS_App

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MMSCONTROLLERDLG_H__1EA52942_C4E8_41F3_8375_72599BB4F27E__INCLUDED_)
