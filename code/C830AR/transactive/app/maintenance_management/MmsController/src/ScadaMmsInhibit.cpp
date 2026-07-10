/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/ScadaMmsInhibit.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// ScadaMmsInhibit.cpp : implementation file
//

#include "app/maintenance_management/MmsController/src/stdafx.h"
#include "app/maintenance_management/MmsController/src/ScadaMmsInhibit.h"
#include "app/maintenance_management/MmsController/src/MmsControllerModel.h"
#include "app/maintenance_management/MmsController/src/CachedConfig.h"

#include "bus/generic_gui/src/TransactiveMessage.h" 
#include "core/data_access_interface/entity_access/src/DataNodeEntityData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScadaMmsInhibit dialog
using namespace TA_Base_Core;
using namespace TA_Base_Bus;

namespace TA_IRS_App
{
	CScadaMmsInhibit::CScadaMmsInhibit(CWnd* pParent /*=NULL*/)
        : CPropertyPage(CScadaMmsInhibit::IDD), m_ScadaTree(true)
	{
		//{{AFX_DATA_INIT(CScadaMmsInhibit)
	//}}AFX_DATA_INIT
	}
	
	void CScadaMmsInhibit::DoDataExchange(CDataExchange* pDX)
	{
		CDialog::DoDataExchange(pDX);
		//{{AFX_DATA_MAP(CScadaMmsInhibit)
	DDX_Control(pDX, IDC_TREE, m_ScadaTree);
	DDX_Control(pDX, IDC_CHECK_INHIBIT_SUBSYSTEM, m_subsystemInhibitCheckbox);
	DDX_Control(pDX, IDC_BUTTON_SUBSYSTEM_APPLY, m_subsystemInhibitApplyButton);
	//}}AFX_DATA_MAP
	}
	
	
    BEGIN_MESSAGE_MAP(CScadaMmsInhibit, CPropertyPage)
		//{{AFX_MSG_MAP(CScadaMmsInhibit)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_CHECK_INHIBIT_SUBSYSTEM, OnSubsystemCheckboxClicked)
	ON_BN_CLICKED(IDC_BUTTON_SUBSYSTEM_APPLY, OnSubsystemApplyClicked)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, OnSelchangedTree)
	ON_MESSAGE(WM_UPDATE_SUBSYSTEM_CHECKBOX, OnSubsystemCheckboxUpdate)
	//}}AFX_MSG_MAP
	END_MESSAGE_MAP()
		
	/////////////////////////////////////////////////////////////////////////////
	// CScadaMmsInhibit message handlers
		
	void CScadaMmsInhibit::OnShowWindow(BOOL bShow, UINT nStatus) 
	{	
		// TODO: Add your message handler code here
		m_ScadaTree.Expand(m_ScadaTree.GetRootItem(), TVE_EXPAND);	
		updateSubsystemButtons(); // Initialize buttons
	}

 	void CScadaMmsInhibit::updateSubsystemButtons(const EButtonUpdateType updateType)
	{
		CWaitCursor wait;
		
        if (APPLY_BUTTON_ONLY != updateType)
        {
            std::vector<TA_Base_Core::DataNodeEntityData*> selection = m_ScadaTree.getSelectedSubsystems();
                        
            UINT rightToModifyCount = 0;    // Tracks number of subsystems have required priveleges to modify
            UINT inhibitedCount = 0;
            UINT failureCount = 0;
    
            if (selection.size() > 0)
            {
                // Check the subsystem state for every selected subsystem
                for (std::vector<TA_Base_Core::DataNodeEntityData*>::iterator itr = selection.begin();
                        itr != selection.end();
                        ++itr )
                {
                    std::string subsystemEntityName = (*itr)->getName();

                    // Check the rights independently of the inhibit state (quite valid to be unable to retrieve state,
                    // but still have permissiont to attempt to change it)
                    // Get the permissions to modify this subsystem using the appropriate subsystem key
                    unsigned long subsystemEntityKey = (*itr)->getKey();
                    if (CachedConfig::getInstance().isFunctionPermitted(TA_Base_Bus::aca_MMS_SUBSYSTEM_INHIBIT, subsystemEntityKey))
                    {                        
                        ++ rightToModifyCount;
                    }
    
                    try
                    {
						if (m_model != NULL)
						{
							if (m_model->getSubsystemMmsInhibit(subsystemEntityName))
							{
								++ inhibitedCount;
							}
						}
						else
						{
							LOG(SourceInfo, TA_Base_Core::DebugUtil::GenericLog,  
								TA_Base_Core::DebugUtil::DebugError, "MmsControllerModel is null!");
						}                    
                    }
                    catch (...)
                    {
                        std::ostringstream ostr;
                        ostr << "getSubsystemMmsInhibit on subsystem " << subsystemEntityName;
                        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", ostr.str().c_str());
    
                        ++ failureCount;
                    }
                }
            }
            else
            {
                // No subsystems selected - rightToModifyCount will remain at zero (disabling buttons)
            }

            // Enable the inhibit checkbox provided we have the right to modify *at least* one
            // of the currently selected subsystems - in the case there is a mixture of privileges, 
            // an error dialog will be displayed telling the operator which subsystems couldn't be changed
            m_subsystemInhibitCheckbox.EnableWindow(rightToModifyCount > 0);
    
            // Apply the appropriate state to the checkbox
            if (failureCount > 0 || rightToModifyCount == 0)
            { 
                // Upon any failures encountered, just disable the checkbox
                m_subsystemInhibitCheckbox.EnableWindow( false);
            }
            else
            {
                //  No failures so checkbox is available; now set the ticked-state
                m_subsystemInhibitCheckbox.EnableWindow( true);
                if (0 == inhibitedCount)
                {
                    // None of the subsystems are inhibited
                    m_subsystemInhibitCheckbox.SetCheck(BST_UNCHECKED);
                }
                else if (selection.size() == inhibitedCount)
                {
                    // All inhibited
                    m_subsystemInhibitCheckbox.SetCheck(BST_CHECKED);
                }
                else
                {
                    // Otherwise a mixture of inhibited / not inhibited
                    m_subsystemInhibitCheckbox.SetCheck(BST_INDETERMINATE);
                }
            }
        }

		m_subsystemInhibitCheckbox.Invalidate();
        // Update the apply button based on inhibit checkbox state
        if (BST_INDETERMINATE == m_subsystemInhibitCheckbox.GetCheck())
        {
            m_subsystemInhibitApplyButton.EnableWindow(FALSE);
        }
        else
        {
            // Enable apply button when checkbox enabled
            m_subsystemInhibitApplyButton.EnableWindow(m_subsystemInhibitCheckbox.IsWindowEnabled());
        }
	}

	void CScadaMmsInhibit::OnSubsystemCheckboxClicked() 
	{
	    if (BST_INDETERMINATE == m_subsystemInhibitCheckbox.GetCheck())
        {
            m_subsystemInhibitCheckbox.SetCheck(BST_UNCHECKED);
        }
        updateSubsystemButtons(APPLY_BUTTON_ONLY);
	}

	void CScadaMmsInhibit::OnSubsystemApplyClicked() 
	{ 
		if (BST_INDETERMINATE == m_subsystemInhibitCheckbox.GetCheck())
        {
            // Cannot apply indeterminate state (button shouldn't have been enabled in the first place)
            return;
        }
		
        bool applyInhibit = (BST_CHECKED == m_subsystemInhibitCheckbox.GetCheck());
		
        std::vector<TA_Base_Core::DataNodeEntityData*> selection = m_ScadaTree.getSelectedSubsystems();
		
		// TD14164 ++
        std::ostringstream reportStr;
		TA_Base_Bus::TransActiveMessage userMsg;
		CString action;
		// ++ TD14164 
        
        if (applyInhibit)
        {
 			action = "apply";
			userMsg << action;
		}
        else
        {
 			action = "reset";
			userMsg << action;
		}
		
		for (std::vector<TA_Base_Core::DataNodeEntityData*>::iterator itr = selection.begin();
				itr != selection.end();
				++itr )
        {			
			reportStr << "\t" << (*itr)->getName() << std::endl;            
        }
				
		userMsg << reportStr.str() ; // TD14164 
		
        // Define array of subsystems that couldn't be changed due to insufficient operator privileges
        std::vector<std::string> insufficientRights;
		
 		if (IDYES == userMsg.showMsgBox(IDS_UW_190001))
		{        
			if (m_model == NULL)
			{
				LOG(SourceInfo, TA_Base_Core::DebugUtil::GenericLog,  
					TA_Base_Core::DebugUtil::DebugError, "MmsControllerModel is null!");
			}
			else
				m_model->setSubsystemMmsInhibit(selection, applyInhibit);
				
			// After applying the relevant changes - perform a full refresh
			// of the associated subsystem buttons (the checkbox and the associated button)
			// allowing these buttons to reflect the actual system state
			updateSubsystemButtons();
		}
	}

	void CScadaMmsInhibit::OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult) 
	{
        // Must update the subsystem apply button / checkbox in case there is
        // no selection (in which case they should both be disabled)
        MSG msg;
        if (!PeekMessage(&msg, m_hWnd, WM_UPDATE_SUBSYSTEM_CHECKBOX, WM_UPDATE_SUBSYSTEM_CHECKBOX, PM_NOREMOVE)) 
        {
            // Only post if not already posted - this stops the checkbox from updating
            // 'in-between' tree control selections (a problem when changing selections, as 
            // the is an intermediate state where no items are selected)
            PostMessage(WM_UPDATE_SUBSYSTEM_CHECKBOX, 1, 0);	
		}
        
		*pResult = 0;
	}

    LRESULT CScadaMmsInhibit::OnSubsystemCheckboxUpdate(UINT wParam, LONG lParam)
    {
        updateSubsystemButtons();
        return 0;
    } 

	void CScadaMmsInhibit::setMmsController(MmsControllerModel* model)
	{
		m_model = model;
	}
}


 







