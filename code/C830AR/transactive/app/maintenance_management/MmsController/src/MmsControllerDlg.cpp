/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:  $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/MmsControllerDlg.cpp $
  * @author  Darren Sampson
  * @version $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by: $Author: builder $
  * 
  * Main Dialog class for the MMS Controller application
  *
  */

#pragma warning ( disable : 4250 4786 4284)

#include "app/maintenance_management/MmsController/src/stdafx.h"
#include "app/maintenance_management/MmsController/src/MmsControllerDlg.h"
#include "app/maintenance_management/MmsController/src/MmsController.h"
#include "app/maintenance_management/MmsController/src/RightsChangedWindowNotificationHandler.h"
#include "app/maintenance_management/MmsController/src/CachedConfig.h"

// displays the Transactive About Box
#include "bus/generic_gui/src/HelpLauncher.h"

#include "bus/generic_gui/src/TransactiveMessage.h" // TD14164 
#include "core/data_access_interface/entity_access/src/DataNodeEntityData.h"
#include "core/data_access_interface/entity_access/src/EntityAccessFactory.h"
#include "bus/security/access_control/actions/src/AccessControlledActions.h"
#include "core/utilities/src/DebugUtil.h"

using namespace TA_Base_Core;
using namespace TA_Base_Bus;

namespace TA_IRS_App
{
	CMmsControllerDlg::CMmsControllerDlg(TA_Base_Bus::IGUIAccess& genericGUICallback)
		: 
        TransActiveDialog(genericGUICallback, CMmsControllerDlg::IDD),
        m_rightsChangeHandler(0),
		m_model(NULL),
        m_hasRightToInhibitGlobal(false)
	{
		FUNCTION_ENTRY("CMmsControllerDlg");
		// If necessary this code can go in OnInitDialog before the call to TransActiveDialog::OnInitDialog()
		TA_Base_Bus::ResizingProperties properties;
		properties.canMaximise	= false;
		properties.maxHeight	= -1;
		properties.maxWidth		= -1;
		properties.minWidth		= -1;
		properties.minHeight	= -1;
		setResizingProperties(properties);
		FUNCTION_EXIT;
	}

    CMmsControllerDlg::~CMmsControllerDlg()
    {
		FUNCTION_ENTRY("~CMmsControllerDlg");
		if (m_model != NULL)
		{
			delete m_model;
			m_model = NULL;
		}

        if (NULL != m_rightsChangeHandler)
        {
			LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Rights change handler should be deleted in OnDestroy" );
        }

		FUNCTION_EXIT;
    }


	void CMmsControllerDlg::DoDataExchange(CDataExchange* pDX)
	{
		TransActiveDialog::DoDataExchange(pDX);
		//{{AFX_DATA_MAP(CMmsControllerDlg)
		DDX_Control(pDX, IDC_TAB, m_mainTab);
        DDX_Control(pDX, IDC_CHECK_GLOBAL_INHIBIT, m_globalInhibitCheckbox);
	    DDX_Control(pDX, IDC_BUTTON_GLOBAL_APPLY, m_globalInhibitApplyButton);
	    //}}AFX_DATA_MAP
	}

	BEGIN_MESSAGE_MAP(CMmsControllerDlg, TransActiveDialog)
		//{{AFX_MSG_MAP(CMmsControllerDlg)
        ON_COMMAND(ID_HELP_HELP, OnHelpHelpMmsController)
		ON_COMMAND(ID_HELP_ABOUTMMSCONTROLLER, OnHelpAboutMmsController)
	    ON_BN_CLICKED(IDC_CLOSE, OnCloseClicked)
        ON_BN_CLICKED(IDC_BUTTON_GLOBAL_APPLY, OnGlobalApplyClicked)
        ON_BN_CLICKED(IDC_CHECK_GLOBAL_INHIBIT, OnGlobalCheckboxClicked)
	    ON_WM_SHOWWINDOW()
        ON_WM_DESTROY()
        ON_MESSAGE(WM_RM_RIGHTS_CHANGE, OnRightsChanged)		
		ON_MESSAGE(WM_UPDATE_DIALOG_CONTROLS, OnUpdateControls)
	    ON_WM_CREATE()
	    ON_UPDATE_COMMAND_UI(ID_APP_EXIT, OnUpdateAppExit)
	    ON_WM_CLOSE()
	
	//}}AFX_MSG_MAP
	END_MESSAGE_MAP()

	void CMmsControllerDlg::init()
	{
		m_model = new MmsControllerModel;
		m_model->setTargetWnd(this->GetSafeHwnd());
		m_mainTab.setMmsController(m_model);
        // Begin listening out for rights changed prior to refreshing rights initially
        m_rightsChangeHandler =  new RightsChangedWindowNotificationHandler(m_hWnd);        
        
        // Refresh dialog state based on current rights (should do right at end of initialisation)
        // - this will update the subsystem / global button states

		//OnRightsChanged(0, 0);
		//MMS majorization =>author:jiangshengguang 2008-09-01
		HANDLE hThread;
		DWORD threadID;
		hThread = CreateThread( NULL, NULL, ( LPTHREAD_START_ROUTINE )ThreadFunc, this, 0, &threadID );
		CloseHandle( hThread );
		//MMS majorization =>author:jiangshengguang 2008-09-01
	}

	DWORD WINAPI CMmsControllerDlg::ThreadFunc( LPVOID lpThreadParameter )
	{
		CMmsControllerDlg* CMC = (CMmsControllerDlg*) lpThreadParameter;
		CMC->OnRightsChanged(0, 0);
		return TRUE;
	}

	BOOL CMmsControllerDlg::OnInitDialog()
	{
		FUNCTION_ENTRY("OnInitDialog");
		TransActiveDialog::OnInitDialog();

		HICON appIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_DNC));
		SetIcon(appIcon, TRUE);		
		

		FUNCTION_EXIT;
		return TRUE;  
	}


	void CMmsControllerDlg::OnHelpHelpMmsController() 
	{
        TA_Base_Bus::HelpLauncher::getInstance().displayHelp();
	}


	void CMmsControllerDlg::OnHelpAboutMmsController() 
	{
		TA_Base_Bus::HelpLauncher::getInstance().displayAboutBox();	
	}

	void CMmsControllerDlg::OnGlobalApplyClicked()
    {
        if (BST_INDETERMINATE == m_globalInhibitCheckbox.GetCheck())
        {
            // Cannot apply indeterminate state (button shouldn't have been enabled in the first place)
            return;
        }

        bool applyInhibit = (BST_CHECKED == m_globalInhibitCheckbox.GetCheck());

        int result = IDNO;

		// TD14164 ++
		TA_Base_Bus::TransActiveMessage userMsg;
		CString action;
		// ++ TD14164 

        if (applyInhibit)
        {
            // UW-??
			// TD14164 ++
            /*result = AfxMessageBox("Please confirm you want to apply the global MMS message inhibit.", MB_YESNO);*/
			action = "apply";
			userMsg << action;
			// ++ TD14164 
        }
        else
        {
            // UW-??
			// TD14164 ++
            /*result = AfxMessageBox("Please confirm you want to reset the global MMS message inhibit.", MB_YESNO);*/
			action = "reset";
			userMsg << action;
			// ++ TD14164 
        }

		result = userMsg.showMsgBox(IDS_UW_190002); // TD14164 
        if (IDYES == result)
        {
            m_model->setGlobalMmsInhibit(applyInhibit);
            
            // Following application of global inhibit, refresh the checkbox
            // state (which indicates the actual state as indicated by agent)
            updateGlobalButtons();
        }
    }


	void CMmsControllerDlg::OnCloseClicked() 
	{
		////TD18095, jianghp, to fix the performance of showing manager application
		TransActiveDialog::DestroyWindow();
	}


	void CMmsControllerDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
	{		
		TransActiveDialog::OnShowWindow(bShow, nStatus);	
		m_mainTab.ShowWindow(SW_SHOW);
//		m_ScadaTree.Expand(m_ScadaTree.GetRootItem(), TVE_EXPAND);
	}


    LRESULT CMmsControllerDlg::OnRightsChanged(UINT wParam, LONG lParam)
    {
        // Refresh the cached right states (there are only two that need checking)
        m_hasRightToInhibitGlobal = CachedConfig::getInstance().isMmsFunctionPermitted(TA_Base_Bus::aca_MMS_GLOBAL_INHIBIT);
        
        ////////////////////
        // Refresh checkboxes / buttons associated with these rights

        updateGlobalButtons();

        return 0;
    }


	LRESULT CMmsControllerDlg::OnUpdateControls(UINT wParam, LONG lParam)
    {
        m_mainTab.updateTabControls();
        return 0;
    }


	void CMmsControllerDlg::OnGlobalCheckboxClicked()
    {
        // Cycle is Off-On-Intederminate - but don't want to be able to cycle to indeterminate
        if (BST_INDETERMINATE == m_globalInhibitCheckbox.GetCheck())
        {
            m_globalInhibitCheckbox.SetCheck(BST_UNCHECKED);
        }
        updateGlobalButtons(APPLY_BUTTON_ONLY);
    }

  
    void CMmsControllerDlg::updateGlobalButtons(EButtonUpdateType updateType)
    {
        // Refresh the state of the global inhibit checkbox if requested
        if (APPLY_BUTTON_ONLY != updateType)
        {
            try
            {
                if (m_model->getGlobalMmsInhibit())
                {
                    m_globalInhibitCheckbox.SetCheck(BST_CHECKED);
                }
                else
                {
                    m_globalInhibitCheckbox.SetCheck(BST_UNCHECKED);
                }
            }
            catch (...)
            {
                LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "getGlobalMmsInhibit");
    
                // Error querying state, leave in indeterminate state
                m_globalInhibitCheckbox.SetCheck(BST_INDETERMINATE);
            }

            bool globalEnabled = m_hasRightToInhibitGlobal;
            
            m_globalInhibitCheckbox.EnableWindow(globalEnabled);
        }

        // Update the associated enabled state of the apply button
        if (BST_INDETERMINATE == m_globalInhibitCheckbox.GetCheck())
        {
            // Suppress apply button while the checkbox is in an indeterminate state
            m_globalInhibitApplyButton.EnableWindow(FALSE);
        }
        else
        {
            m_globalInhibitApplyButton.EnableWindow(m_globalInhibitCheckbox.IsWindowEnabled());
        }
    }

    void CMmsControllerDlg::OnDestroy()
    {
        // Delete the rights change notification object before window handle becomes invalid    
        delete m_rightsChangeHandler;
        m_rightsChangeHandler = 0;
    }

	void CMmsControllerDlg::OnUpdateAppExit(CCmdUI* pCmdUI) 
	{
		// TODO: Add your command update UI handler code here
	}
	
	void CMmsControllerDlg::OnClose() 
	{
		// TODO: Add your message handler code here and/or call default
		TransActiveDialog::DestroyWindow();
	}

} // TA_IRS_App

