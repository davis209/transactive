/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/NonPhysicalSubsystemTree.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// NonPhysicalSubsystemTree.cpp: implementation of the NonPhysicalSubsystemTree class.
//
//////////////////////////////////////////////////////////////////////

#include "app/maintenance_management/MmsController/src/stdafx.h"
#include "app/maintenance_management/MmsController/src/mmscontroller.h"
#include "app/maintenance_management/MmsController/src/NonPhysicalSubsystemTree.h"
#include "app/maintenance_management/MmsController/src/NonPhysicalEntityFactory.h"
#include "app/maintenance_management/MmsController/src/NonPhysicalEntityItem.h"

#include "core/data_access_interface/entity_access/src/EntityData.h"
#include "core/data_access_interface/entity_access/src/ConfigEntity.h"
 
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace TA_IRS_App
{	
	const std::string NonPhysicalSubsystemTree::ROOT_ENTITY_TYPE = "Entities";

	NonPhysicalSubsystemTree::NonPhysicalSubsystemTree(bool bAllowSubsystemSelection)
		:m_onlyAllowSubsystemSelection(bAllowSubsystemSelection)
	{		
        m_bMulti = TRUE;
        m_bAllowDragSelection = FALSE;
	}
	
	NonPhysicalSubsystemTree::~NonPhysicalSubsystemTree()
	{
	 
	}
	
	BEGIN_MESSAGE_MAP(NonPhysicalSubsystemTree, CObjectTreeCtrl)
	//{{AFX_MSG_MAP(NonPhysicalSubsystemTree)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		ON_WM_CREATE()
		ON_NOTIFY_REFLECT_EX(TVN_SELCHANGED, OnSelchangedTree)
	    ON_NOTIFY_REFLECT(TVN_SELCHANGING, OnSelchangingTree)
	//}}AFX_MSG_MAP
	END_MESSAGE_MAP()

	/////////////////////////////////////////////////////////////////////////////
	// NonPhysicalSubsystemTree message handlers


	BOOL NonPhysicalSubsystemTree::PreCreateWindow(CREATESTRUCT& cs) 
	{				
		return CObjectTreeCtrl::PreCreateWindow(cs);
	}


	void NonPhysicalSubsystemTree::PreSubclassWindow() 
	{
 		CObjectTreeCtrl::PreSubclassWindow();
	}

	int NonPhysicalSubsystemTree::OnCreate(LPCREATESTRUCT lpCreateStruct) 
	{ 
		if (CObjectTreeCtrl::OnCreate(lpCreateStruct) == -1)
			return -1;		
		return 0;
	}


	void NonPhysicalSubsystemTree::populateTree(std::map<unsigned long, TA_Base_Core::ConfigEntity*>& entities, bool bForceUpdate)
    {			 
		// the items in the tree control must be populated with new set
		// of entities, delete the previous ones		
		m_entityFactory.refreshEntityData(entities, bForceUpdate);
		// add root item
		NonPhysicalEntityTreeItem* item = new NonPhysicalEntityTreeItem(&m_entityFactory, 
			0,ROOT_ENTITY_TYPE);
		AddRootItem(*item, true);	
	}
 
 	int NonPhysicalSubsystemTree::countNodes(HTREEITEM item)
	{
		// First expand the node to ensure that all children are loaded and then set it back. This ensures
		// the view does not change but the items are loaded
		Expand(item, TVE_TOGGLE);
		Expand(item, TVE_TOGGLE);
		
		HTREEITEM child = GetChildItem(item);
		if ( child == NULL )
		{
			// This child has no children so return 0
			return 0;
		}
		
		// else we must determine how many children we have
		int numberOfNodes = 0;
		while(child != NULL)
		{
			// Add this node to the number of nodes plus the count of all the child nodes
			numberOfNodes = numberOfNodes + 1 + countNodes(child);
			
			child = GetNextItem(child, TVGN_NEXT);
		}
		
		return numberOfNodes;
	}

	bool NonPhysicalSubsystemTree::isEntitySelected()
    {
        // Since we don't allow the operator to select anything but subsystems, 
        // all we need to do is check if an item is selected (any item)
        return GetSelectedCount() > 0;
    }

	BOOL NonPhysicalSubsystemTree::OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult) 
	{		
        NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

        // Allow parent to process sel changed notification
        return FALSE;
    }

	int NonPhysicalSubsystemTree::countSelectedItems()
	{
		std::vector<CObjectTreeItem*> selectedItems = getSelectedItems();
		return selectedItems.size();
	}

	std::vector<TA_Base_Core::IEntityData*> NonPhysicalSubsystemTree::getSelectedEntities()
    {
        std::vector<CObjectTreeItem*> selectedItems = getSelectedItems();
        std::vector<TA_Base_Core::IEntityData*> result;
 

        for (std::vector<CObjectTreeItem*>::iterator itr = selectedItems.begin();
                itr != selectedItems.end(); 
                ++itr)
        {
            NonPhysicalEntityTreeItem* entityTreeItem = dynamic_cast<NonPhysicalEntityTreeItem*>(*itr);
            
            if (0 != entityTreeItem) 
            {
				if (entityTreeItem->isEntityType())
				{
					unsigned long locationKey = entityTreeItem->getParentKey();
					unsigned long entityKey = entityTreeItem->getEntityKey();
					std::vector<TA_Base_Core::IEntityData*> childEntities = 
						 m_entityFactory.getEntitiesOfTypeAtLocation(entityKey, locationKey);
					if (!childEntities.empty())
					{
						std::vector<TA_Base_Core::IEntityData*>::iterator childIter;
						for(childIter=childEntities.begin(); childIter!=childEntities.end();++childIter)
						{
							result.push_back(*childIter);
						}
					}
				}
                TA_Base_Core::IEntityData* entityData = entityTreeItem->getEntityData();
                
                // Only add the subsystem nodes to result
                if (0 != entityData && (m_entityFactory.isNonPhysicalSubsystemEntity(entityData->getKey())))
				{
                    result.push_back(entityData);
                }
            }
        }
        return result;
    }

	NonPhysicalEntityTreeItem* NonPhysicalSubsystemTree::getEntityFromTreeItem(const TV_ITEM& tvItem)
    {
        if (0 != tvItem.lParam)
        {
            NonPhysicalEntityTreeItem* selectedItem = reinterpret_cast<NonPhysicalEntityTreeItem*>(tvItem.lParam);
            return selectedItem;
        }
        return 0;
    }


	 void NonPhysicalSubsystemTree::OnSelchangingTree(NMHDR* pNMHDR, LRESULT* pResult) 
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
				NonPhysicalEntityTreeItem* entityTreeItem = getEntityFromTreeItem(pNMTreeView->itemNew);
				
				if (0 != entityTreeItem) 
				{
					if (entityTreeItem->hasChildren() && !entityTreeItem->isEntityType())
					{
						// Attempted to select location and entity types, block action							
						*pResult = TRUE;
					}
				}
			}
		}        
	 }
}
