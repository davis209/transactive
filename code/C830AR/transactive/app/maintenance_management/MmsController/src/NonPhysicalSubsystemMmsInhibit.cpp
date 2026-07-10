/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/NonPhysicalSubsystemMmsInhibit.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// NonPhysicalSubsystemMmsInhibit.cpp : implementation file
//

#include "app/maintenance_management/MmsController/src/stdafx.h"
#include "app/maintenance_management/MmsController/src/NonPhysicalSubsystemMmsInhibit.h"
#include "app/maintenance_management/MmsController/src/MmsControllerModel.h"

#include "bus/generic_gui/src/TransactiveMessage.h" 

#include "core/data_access_interface/entity_access/src/IConfigEntity.h"
#include "core/data_access_interface/entity_access/src/ConfigEntity.h"
#include "core/data_access_interface/entity_access/src/IEntityData.h"
#include "core/data_access_interface/entity_access/src/EntityStatusData.h"
#include "core/data_access_interface/src/SessionAccessFactory.h"
#include "core/data_access_interface/src/ISession.h"
#include "core/utilities/src/RunParams.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNonPhysicalSubsystemMmsInhibit dialog

namespace TA_IRS_App
{	
	
	const std::string CNonPhysicalSubsystemMmsInhibit::MMS_INHIBIT_ON  = "1";
	const std::string CNonPhysicalSubsystemMmsInhibit::MMS_INHIBIT_OFF = "0";

	CNonPhysicalSubsystemMmsInhibit::CNonPhysicalSubsystemMmsInhibit(CWnd* pParent /*=NULL*/)
        : CPropertyPage(CNonPhysicalSubsystemMmsInhibit::IDD)
	{
		//{{AFX_DATA_INIT(CNonPhysicalSubsystemMmsInhibit)
		// NOTE: the ClassWizard will add member initialization here
		//}}AFX_DATA_INIT
        //Mintao++ TD16762
        isNonPhysicalSubSystemTreePopulated = false;
	}
	
	
	void CNonPhysicalSubsystemMmsInhibit::DoDataExchange(CDataExchange* pDX)
	{
        CPropertyPage::DoDataExchange(pDX);
		//{{AFX_DATA_MAP(CNonPhysicalSubsystemMmsInhibit)
	    DDX_Control(pDX, IDC_TREE_SYSTEM, m_nonPhysicalSubSystemTree );
	    DDX_Control(pDX, IDC_LIST_SYSTEM_MMS_INHIBIT, m_mmsInhibitedList );
	    DDX_Control(pDX, IDC_BUTTON_REMOVE_SYSTEM, m_removeSystemButton);
	    DDX_Control(pDX, IDC_BUTTON_APPLY_SYSTEMINHIBIT, m_applySystemInhibitButton);
	    DDX_Control(pDX, IDC_BUTTON_ADD_SYSTEM, m_addSystemButton);
        //}}AFX_DATA_MAP
	}
	
	
    BEGIN_MESSAGE_MAP(CNonPhysicalSubsystemMmsInhibit, CPropertyPage)
	//{{AFX_MSG_MAP(CNonPhysicalSubsystemMmsInhibit)
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_SYSTEM, OnSelchangedTreeSystem)
	ON_MESSAGE(WM_UPDATE_SYSTEM_BUTTONS, OnSystemButtonsUpdate)
	ON_BN_CLICKED(IDC_BUTTON_ADD_SYSTEM, OnNonPhysicalSubsystemAddClick)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_SYSTEM, OnNonPhysicalSubsystemRemoveClick)
	ON_BN_CLICKED(IDC_BUTTON_APPLY_SYSTEMINHIBIT, OnNonPhysicalSubsystemApplyClick)
	ON_NOTIFY(NM_CLICK, IDC_LIST_SYSTEM_MMS_INHIBIT, OnClickMmsInhibitedList)
	ON_NOTIFY(NM_SETFOCUS, IDC_TREE_SYSTEM, OnSetfocusTreeSystem)
	ON_MESSAGE(WM_UPDATE_MMS_INHIBITION_LIST, OnUpdateMMSInhibitList)

	//}}AFX_MSG_MAP
	END_MESSAGE_MAP()
		
	/////////////////////////////////////////////////////////////////////////////
	// CNonPhysicalSubsystemMmsInhibit message handlers
		
	BOOL CNonPhysicalSubsystemMmsInhibit::OnInitDialog() 
	{
        CPropertyPage::OnInitDialog();
		
		//m_nonPhysicalSubSystemTree.populateTree();
		m_mmsInhibitedList.buildMmsInhibitList();

		return TRUE;  // return TRUE unless you set the focus to a control
		// EXCEPTION: OCX Property Pages should return FALSE
	}
	void CNonPhysicalSubsystemMmsInhibit::OnShowWindow(BOOL bShow, UINT nStatus) 
	{
		// CDialog::OnShowWindow(bShow, nStatus);		
		// TODO: Add your message handler code here	
        //Mintao++ TD16762
        if (!isNonPhysicalSubSystemTreePopulated)
        {
		    if (m_model != NULL)
		    {
                CWaitCursor waitCursor;
		        m_nonPhysicalSubSystemTree.populateTree(m_model->getNonPhysicalSubsystems(), true);
			    updateSystemInhibitList(true);
                isNonPhysicalSubSystemTreePopulated = true;
            }
		    m_addSystemButton.EnableWindow(FALSE);
		    m_removeSystemButton.EnableWindow(FALSE);
		    m_applySystemInhibitButton.EnableWindow(FALSE);		
	    
		    
		    m_nonPhysicalSubSystemTree.Expand(m_nonPhysicalSubSystemTree.GetRootItem(), TVE_EXPAND);	
        }
        //Mintao++ TD16762
	}
	
	void CNonPhysicalSubsystemMmsInhibit::setMmsController(MmsControllerModel *model)
	{
		m_model = model;
    }


    void CNonPhysicalSubsystemMmsInhibit::updateSystemButtons(const EButtonUpdateType updateType)
	{
 
		if (updateType == ADD_BUTTON_ONLY)
		{		
			// retrieve selected nonphysical entities
			// for each entity selected, check if entity is already in the inhibited list
 			m_addSystemButton.EnableWindow(TRUE);
			m_removeSystemButton.EnableWindow(FALSE); 
		}
		else if (updateType == REMOVE_BUTTON_ONLY)
		{		
			// retrieve selected nonphysical entities
			// for each entity selected, check if entity is already in the inhibited list
 			m_addSystemButton.EnableWindow(FALSE);			
			m_removeSystemButton.EnableWindow(TRUE);
		}	
		else if (updateType == DISABLE_ADD_BUTTON)
		{
			m_addSystemButton.EnableWindow(FALSE);
		}
		else if (updateType == DISABLE_REMOVE_BUTTON)
		{
			m_removeSystemButton.EnableWindow(FALSE);
		}
		else
		{
			m_addSystemButton.EnableWindow(FALSE);		
			m_removeSystemButton.EnableWindow(FALSE);
		}

		if (m_cachedEntitiesforMMSInhibitOn.empty() && m_cachedEntitiesforMMSInhibitOff.empty())
		{
			m_applySystemInhibitButton.EnableWindow(FALSE);
		}
		else
		{
			m_applySystemInhibitButton.EnableWindow(TRUE);			
		}
	}
 
	
	void CNonPhysicalSubsystemMmsInhibit::OnSetfocusTreeSystem(NMHDR* pNMHDR, LRESULT* pResult) 
	{
		// TODO: Add your control notification handler code here

		if (m_nonPhysicalSubSystemTree.isEntitySelected())
			updateSystemButtons(ADD_BUTTON_ONLY);
		else
			updateSystemButtons(DISABLE_ADD_BUTTON);
		*pResult = 0;
	}

	void CNonPhysicalSubsystemMmsInhibit::OnSelchangedTreeSystem(NMHDR* pNMHDR, LRESULT* pResult) 
	{
		// Must update the buttons in case there is
        // no selection (in which case they should both be disabled)
		if (m_nonPhysicalSubSystemTree.isEntitySelected())
			updateSystemButtons(ADD_BUTTON_ONLY);
		else
			updateSystemButtons(DISABLE_ADD_BUTTON);    
		
		*pResult = 0;
	}

	LRESULT CNonPhysicalSubsystemMmsInhibit::OnSystemButtonsUpdate(UINT wParam, LONG lParam)
    {
		updateSystemButtons(EButtonUpdateType(wParam));
        return 0;
    } 


	LRESULT CNonPhysicalSubsystemMmsInhibit::OnUpdateMMSInhibitList(UINT wParam, LONG lParam)
    {
	
		updateSystemInhibitList();
	
        return 0;
    } 

	bool CNonPhysicalSubsystemMmsInhibit::checkAndSelectNonPhysicalSubsystemEntities()
	{		
		// retrieve selected items from the tree			
 		std::vector<TA_Base_Core::IEntityData*> entitiesToInhibit = m_nonPhysicalSubSystemTree.getSelectedEntities();
		std::vector<std::string> failedToAdd;

		if (entitiesToInhibit.size() > 0)			
		{ 	
			time_t currentTimeAdded;
			time(&currentTimeAdded);
			
 		    CString dateFormatStr("%d-%b-%y %H:%M:%S");
		    CString dateAdded = CTime(currentTimeAdded).Format(dateFormatStr);

			CString operatorName = getOperatorName();
					
			for (std::vector<TA_Base_Core::IEntityData*>::iterator it = entitiesToInhibit.begin();
				it != entitiesToInhibit.end(); ++it)
			{
				TA_Base_Core::IEntityData* entityData =(*it);
				
				//check if the entityData is not in the List				
				if (m_mmsInhibitedList.FindEntity(entityData->getKey()) == -1)
				{	
					m_mmsInhibitedList.insertMmsInhibitedEntity(entityData, dateAdded, operatorName); 					
					TA_Base_Core::ConfigEntity* configEntityData = dynamic_cast<TA_Base_Core::ConfigEntity*>(entityData);
					configEntityData->setEntityStatusData(TA_Base_Core::MMSInhibitStatus, TA_Base_Core::BOOLEAN_DATA, MMS_INHIBIT_ON, currentTimeAdded);
					m_cachedEntitiesforMMSInhibitOn[entityData->getKey()] = configEntityData; 
				}
				else
				{
					failedToAdd.push_back(entityData->getName());
				}
			}	

			// if some entities failed to set mms inhibition before applying any changes,
			// these entities might be already in the list
			if (!failedToAdd.empty())
			{

			}
		}		 
	    return (failedToAdd.empty());
	}

	void CNonPhysicalSubsystemMmsInhibit::OnNonPhysicalSubsystemAddClick() 
	{
		// TODO: Add your control notification handler code here
		//check if the selected entities are not already existing in the
		// list of inhibited non physical subsystem entities
		// 
 		if(checkAndSelectNonPhysicalSubsystemEntities() == true)
		{
			  updateSystemButtons();
		}
		
	}

	void CNonPhysicalSubsystemMmsInhibit::OnNonPhysicalSubsystemRemoveClick() 
	{
 		// get selected items in the list 				
		int selectedItems = m_mmsInhibitedList.GetSelectedCount();
		
		if (selectedItems > 0)
		{
			int iItem = -1;			
			int* arraySelected = new int[selectedItems];			
			int count = 0;
 
 			do
			{
				iItem = m_mmsInhibitedList.GetNextItem(iItem, LVNI_SELECTED);
				if (iItem != -1)
				{				 
					arraySelected[count++] = iItem;	
				}
			} while (iItem != -1);
			
			int DATE_COLUMN = 0;
			for (int ctr=count-1; ctr>=0; --ctr)
			{
				iItem = arraySelected[ctr];
				TA_Base_Core::IEntityData* entityData = reinterpret_cast<TA_Base_Core::IEntityData*>(m_mmsInhibitedList.GetItemData(iItem));
				
				// if entityData is mms inhibited, 
				// confirm it with the user
				if (entityData !=  NULL)
				{						
					TA_Base_Core::ConfigEntity* configEntityData = dynamic_cast<TA_Base_Core::ConfigEntity*>(entityData);
					TA_Base_Core::EntityStatusData statusData = configEntityData->getEntityStatusData(TA_Base_Core::MMSInhibitStatus);
					statusData.entityStatusDataValue = MMS_INHIBIT_OFF;
					configEntityData->setEntityStatusData(statusData);
					unsigned long entityKey = configEntityData->getKey(); 
								
					m_mmsInhibitedList.DeleteEntityItem(entityKey);
					// check if the entity is previously added for inhibition,
					// before an apply was done
					ConfigList::iterator itr = m_cachedEntitiesforMMSInhibitOn.find(entityKey);
					if (itr != m_cachedEntitiesforMMSInhibitOn.end())
					{
						// remove from the cache if the mms inhibition is cancelled
						m_cachedEntitiesforMMSInhibitOn.erase(entityKey);
						// no need to add such item to the cached entities for mms inhibit off
						// since operator could just have changed its mind .. 
					}
					else
					{
						// cache this entity since status data has to change
						m_cachedEntitiesforMMSInhibitOff[entityKey] = configEntityData;	
					}
				}		 
			
			}
		}

		updateSystemButtons();
	}

	void CNonPhysicalSubsystemMmsInhibit::OnNonPhysicalSubsystemApplyClick() 
	{
		// TODO: Add your control notification handler code here
		//ConfigList mmsNonPhysicalEntities = MmsControllerModel::getNonPhysicalSubsystems();

		
		TA_Base_Bus::TransActiveMessage userMsg;
		std::string action;

		typedef std::vector<TA_Base_Core::ConfigEntity*> EntityList;
		EntityList entitiesToChangeMmsInhibit;

		if (!m_cachedEntitiesforMMSInhibitOn.empty())
		{	
			std::ostringstream applyMmsInhibitToEntities; 			
 			
			for (ConfigList::iterator itr = m_cachedEntitiesforMMSInhibitOn.begin();
					itr != m_cachedEntitiesforMMSInhibitOn.end();++itr )
			{
				TA_Base_Core::ConfigEntity* entityData = dynamic_cast<TA_Base_Core::ConfigEntity*>(itr->second);
				applyMmsInhibitToEntities << "\t" << entityData->getName().c_str() << std::endl;
				entitiesToChangeMmsInhibit.push_back(entityData);
			}

			action = "apply";
			userMsg << action;
			userMsg << applyMmsInhibitToEntities.str();

			if (IDYES == userMsg.showMsgBox(IDS_UW_190001))
			{				
				applyMmsInhibit(entitiesToChangeMmsInhibit, true);
			}
			else
			{
				// remove all cached entities whose mms inhibition are cancelled
				// from the mms inhibit list
				for (ConfigList::iterator itrToDelete = m_cachedEntitiesforMMSInhibitOn.begin();
					itrToDelete != m_cachedEntitiesforMMSInhibitOn.end();++itrToDelete )
				{
					TA_Base_Core::ConfigEntity* entityData = dynamic_cast<TA_Base_Core::ConfigEntity*>(itrToDelete->second);
				    m_mmsInhibitedList.DeleteEntityItem(entityData->getKey());
				}				
			}
			// delete items from the cache whether mms inhibition are successful or have failed
			m_cachedEntitiesforMMSInhibitOn.clear();
			entitiesToChangeMmsInhibit.clear();
		}

		if (!m_cachedEntitiesforMMSInhibitOff.empty())
		{	
			std::ostringstream removeMmsInhibitToEntities; 			
 			
			for (ConfigList::iterator itr = m_cachedEntitiesforMMSInhibitOff.begin();
					itr != m_cachedEntitiesforMMSInhibitOff.end();++itr )
			{
				TA_Base_Core::ConfigEntity* entityData = dynamic_cast<TA_Base_Core::ConfigEntity*>(itr->second);
				removeMmsInhibitToEntities << "\t" << entityData->getName().c_str() << std::endl;
				entitiesToChangeMmsInhibit.push_back(entityData);
			}

			action = "reset";
			userMsg << action;
			userMsg << removeMmsInhibitToEntities.str();

			if (IDYES == userMsg.showMsgBox(IDS_UW_190001))
			{
				applyMmsInhibit(entitiesToChangeMmsInhibit, false);
			}	
			else
			{
				// insert back items that were inhibited
				for (ConfigList::iterator itrToRevert = m_cachedEntitiesforMMSInhibitOff.begin();
					itrToRevert != m_cachedEntitiesforMMSInhibitOff.end();++itrToRevert )
				{
					TA_Base_Core::ConfigEntity* entityData = dynamic_cast<TA_Base_Core::ConfigEntity*>(itrToRevert->second);
				    m_mmsInhibitedList.insertMmsInhibitedEntity(entityData);
				}		
			}
			m_cachedEntitiesforMMSInhibitOff.clear();
			entitiesToChangeMmsInhibit.clear();
		}		
	 
		updateSystemButtons();
	}

	void CNonPhysicalSubsystemMmsInhibit::applyMmsInhibit(std::vector<TA_Base_Core::ConfigEntity*>& entityMmsInhibitList, bool bApplyAction)
	{
		if (NULL != m_model)
		{
			try
			{
				m_model->setNonPhysicalSubsystemMmsInhibit(entityMmsInhibitList, bApplyAction);
			}
			catch(...)
			{
				 std::ostringstream ostr;
                        ostr << "unknown exception on setNonPhysicalSubsystemMmsInhibit";
                        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", ostr.str().c_str());
			}
		}
		else
		{
			LOG(SourceInfo, TA_Base_Core::DebugUtil::GenericLog,  
								TA_Base_Core::DebugUtil::DebugError, "MmsControllerModel is null!");
		}
		
	}

	void CNonPhysicalSubsystemMmsInhibit::updateSystemInhibitList(bool bForceUpdate)
	{
 		if (m_model != NULL)
		{			
			std::map<unsigned long, TA_Base_Core::IEntityData*> mmsInhibitedEntities =  m_model->getNonPhysicalSubsystemMmsInhibit(bForceUpdate);
			m_mmsInhibitedList.populateNonPhysicalSubsystemList(mmsInhibitedEntities);
		}
	}	 
 
	void CNonPhysicalSubsystemMmsInhibit::OnClickMmsInhibitedList(NMHDR* pNMHDR, LRESULT* pResult) 
	{
		// TODO: Add your control notification handler code here
		int selectedItems = m_mmsInhibitedList.GetSelectedCount();
				
		if (selectedItems > 0)
			updateSystemButtons(REMOVE_BUTTON_ONLY);	
		
		*pResult = 0;
	}
 
	
	CString CNonPhysicalSubsystemMmsInhibit::getOperatorName()
	{
		TA_Base_Core::ISession* session = NULL; 
		CString operatorName;
		std::string sessionId = TA_Base_Core::RunParams::getInstance().get(RPARAM_SESSIONID);

		try
		{
			session  = TA_Base_Core::SessionAccessFactory::getInstance().getSession(sessionId);
			operatorName = session->getOperatorName().c_str();
		}
		catch(...)
		{
			LOG_EXCEPTION_CATCH(SourceInfo, "Unknown Exception", "Failed to retrieve session details.");
		}

		delete session;
        session = NULL;

		return operatorName;
	}
}



