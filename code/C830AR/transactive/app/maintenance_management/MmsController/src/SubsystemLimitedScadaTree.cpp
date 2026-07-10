/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source: $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/SubsystemLimitedScadaTree.cpp $
  * @author Jade Lee
  * @version $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by: $Author: builder $
  * 
  * Defines a specialization of the scada tree that only shows
  * up to subsystem level (and optionally only allows selected of subsystem datanodes)
  *
  */

#pragma warning ( disable : 4250 4786 4284)

#include "app/maintenance_management/MmsController/src/stdafx.h"
#include "app/maintenance_management/MmsController/src/SubsystemLimitedScadaTree.h"
#include "bus/scada/scada_tree/src/EntityTreeItem.h"
#include "core/data_access_interface/entity_access/src/DataNodeEntityData.h"


namespace TA_IRS_App
{
    typedef TA_Base_Bus::CScadaTreeCtrl CScadaTreeCtrl;

    SubsystemLimitedScadaTree::SubsystemLimitedScadaTree(bool onlyAllowSubsystemSelection)
    :
    CScadaTreeCtrl(&m_mmsEntityFactory, false),
    m_onlyAllowSubsystemSelection(onlyAllowSubsystemSelection)
    {
        m_bMulti = TRUE;
        // Turn of the drag selection - it can be excessively slow (all the selection changed
        // notifications in turn require processing to determine state of subsystem checkbox, 
        // so the drag select operation can be quite chunky)
        m_bAllowDragSelection = FALSE;
    }


    SubsystemLimitedScadaTree::~SubsystemLimitedScadaTree()
    {
    }


	BEGIN_MESSAGE_MAP(SubsystemLimitedScadaTree, CScadaTreeCtrl)
		//{{AFX_MSG_MAP(CMmsControllerDlg)
	    ON_NOTIFY_REFLECT_EX(TVN_SELCHANGED, OnSelchangedTree)
	    ON_NOTIFY_REFLECT(TVN_SELCHANGING, OnSelchangingTree)
        //}}AFX_MSG_MAP
	END_MESSAGE_MAP()

    void SubsystemLimitedScadaTree::OnSelchangingTree(NMHDR* pNMHDR, LRESULT* pResult) 
	{
    	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
		    
        // By default accept the action
        *pResult = 0;
              
        TV_ITEM& tvItem = pNMTreeView->itemNew;

        // Check if attempting to modify selection -
        // Unusual heuristics used here (doesn't consistently set
        // the state flags as you would expect, calling SetItemState with 
        // TVIS_SELECTED won't necessarily fill out the state fields..)
        bool selectionAdjusted = (pNMTreeView->itemOld.mask & TVIF_STATE) ||
                                 (pNMTreeView->itemNew.mask & TVIF_STATE);
                                 
        // Also, when there are no items selected (on startup), 
        // the first notification to come through (to select the root node)
        // doesn't have any state flags, must rely on checking for this notification
        // by looking at the handle of the old item which will be null (quite hacky)
        if (selectionAdjusted || (pNMTreeView->itemOld.hItem == 0x0))
        {
            if (m_onlyAllowSubsystemSelection)
            {
                TA_Base_Core::IEntityData* entityData = getEntityFromTreeItem(pNMTreeView->itemNew);
    
                if (0 != entityData &&
                    !MmsEntityFactory::isDatanodeEntitySubsytem(*entityData))
                {
                    // Attempted to select non-subsystem entity, block action
                    *pResult = TRUE;
                }
            }
        }        
    }

    
    TA_Base_Core::IEntityData* SubsystemLimitedScadaTree::getEntityFromTreeItem(const TV_ITEM& tvItem)
    {
        if (0 != tvItem.lParam)
        {
            TA_Base_Bus::CEntityTreeItem* selectedItem = reinterpret_cast<TA_Base_Bus::CEntityTreeItem*>(tvItem.lParam);

            return selectedItem->getEntityData();
        }

        return 0;
    }


    std::vector<TA_Base_Core::DataNodeEntityData*> SubsystemLimitedScadaTree::getSelectedSubsystems()
    {
        std::vector<CObjectTreeItem*> selectedItems = getSelectedItems();
        std::vector<TA_Base_Core::DataNodeEntityData*> result;

        for (std::vector<CObjectTreeItem*>::iterator itr = selectedItems.begin();
                itr != selectedItems.end(); 
                ++itr)
        {
            TA_Base_Bus::CEntityTreeItem* entityTreeItem = dynamic_cast<TA_Base_Bus::CEntityTreeItem*>(*itr);
            
            if (0 != entityTreeItem)
            {
                TA_Base_Core::IEntityData* entityData = entityTreeItem->getEntityData();
                
                // Only add the subsystem nodes to result
                if (0 != entityData &&
                    entityData->getType() == TA_Base_Core::DataNodeEntityData::getStaticType())				
                {
                    TA_Base_Core::DataNodeEntityData* dnEntity = dynamic_cast<TA_Base_Core::DataNodeEntityData*>(entityData);
                
                    if (0 != dnEntity)
                    {            
                        result.push_back(dnEntity);
                    }
                }
            }
        }

        return result;
    }


    BOOL SubsystemLimitedScadaTree::OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult) 
	{		
        NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

        // Allow parent to process sel changed notification
        return FALSE;
    }


    bool SubsystemLimitedScadaTree::isSubsystemSelected()
    {
        // Since we don't allow the operator to select anything but subsystems, 
        // all we need to do is check if an item is selected (any item)
        return GetSelectedCount() > 0;
    }
}