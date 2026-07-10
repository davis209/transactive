/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/NonPhysicalEntityItem.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// NonPhysicalEntityItem.cpp: implementation of the NonPhysicalEntityItem class.
//
//////////////////////////////////////////////////////////////////////

#include "app/maintenance_management/MmsController/src/stdafx.h"
#include "app/maintenance_management/MmsController/src/mmscontroller.h"
#include "app/maintenance_management/MmsController/src/NonPhysicalEntityItem.h"
#include "app/maintenance_management/MmsController/src/NonPhysicalEntityFactory.h"
#include "core/data_access_interface/entity_access/src/IEntityData.h"
 
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace TA_IRS_App
{	

	NonPhysicalEntityTreeItem::NonPhysicalEntityTreeItem(NonPhysicalEntityFactory* entityRetrievalFactory, 
		unsigned long entityKey, 		
		std::string  entityLabel,
		bool bExpandAllEntityTreeItems /*= true*/, 
		ETreeItemLevel treeItemLevel /*= TREEITEM_UNKNOWNLEVEL */,
		unsigned long	parentKey	   /*= 0*/)
		 : CObjectTreeItem(), 
		   m_entityRetrievalFactory(entityRetrievalFactory), 
		   m_entityKey(entityKey), 		   
		   m_entityLabel(entityLabel),
		   m_parentKey(parentKey),
		   m_bExpandAllEntityTreeItems(bExpandAllEntityTreeItems),
		   m_treeLevel(treeItemLevel)		   
	{
		if (treeItemLevel == TREEITEM_UNKNOWNLEVEL)	
		{			
			m_treeLevel = TREEITEM_ROOT;
		}		
	}

	NonPhysicalEntityTreeItem::~NonPhysicalEntityTreeItem()
	{
	}

	bool NonPhysicalEntityTreeItem::hasChildren()
	{
		return (m_entityRetrievalFactory->isNonPhysicalSubsystemEntity(m_entityKey) ? false : true);
		//return ((m_treeLevel != TREEITEM_NONPHYSICALENTITY) ? true : false);
	}

	bool NonPhysicalEntityTreeItem::isEntityType()
	{
		return ((m_treeLevel == TREEITEM_ENTITYTYPE) ? true : false);
	}

	bool NonPhysicalEntityTreeItem::createChildItems()
	{	
		// Get systems associated with this station    
		ETreeItemLevel childItemLevel;
		bool bHasChildren = hasChildren();
		
		if (bHasChildren)
		{
			if (m_treeLevel == TREEITEM_ROOT) // root item
			{	// get all locations
				std::map<unsigned long, std::string> locations;

				locations = m_entityRetrievalFactory->getAllValidLocations();
				childItemLevel = TREEITEM_LOCATION;

				std::map<unsigned long, std::string>::iterator iter;
				for (iter = locations.begin(); iter != locations.end(); ++iter)
				{
					NonPhysicalEntityTreeItem* newItem = new NonPhysicalEntityTreeItem(m_entityRetrievalFactory, 
						(*iter).first,(*iter).second,m_bExpandAllEntityTreeItems, childItemLevel, m_entityKey);
					AddChildItem(*newItem, true, true);
				}
			}
			else if (m_treeLevel == TREEITEM_LOCATION)
			{	// child items of a location are not of type IEntityData, 
				// therefore must have a special case in creating its treeItem object				
				
				childItemLevel = TREEITEM_ENTITYTYPE;
				// pass in the entityKey of the location to retrieve the entity types
				std::map<unsigned long, std::string> entityTypes = m_entityRetrievalFactory->getEntityTypesAtLocation(m_entityKey);
				if(!entityTypes.empty())
				{	
					std::map<unsigned long, std::string>::iterator typeIter;
					for (typeIter=entityTypes.begin(); typeIter!=entityTypes.end(); ++typeIter)
					{
						NonPhysicalEntityTreeItem* newItem = new NonPhysicalEntityTreeItem(m_entityRetrievalFactory, 
							(*typeIter).first,(*typeIter).second, m_bExpandAllEntityTreeItems, childItemLevel, m_entityKey);
						AddChildItem(*newItem, true, true);
					}
				}
				
			}
			else if (m_treeLevel == TREEITEM_ENTITYTYPE)
			{
				std::vector<TA_Base_Core::IEntityData*> childItems;

 				childItems = m_entityRetrievalFactory->getEntitiesOfTypeAtLocation(m_entityKey, m_parentKey);
				childItemLevel = TREEITEM_NONPHYSICALENTITY;
			
				std::vector<TA_Base_Core::IEntityData*>::iterator iter;
				for (iter = childItems.begin(); iter != childItems.end(); ++iter)
				{
					NonPhysicalEntityTreeItem* newItem = new NonPhysicalEntityTreeItem(m_entityRetrievalFactory, 
						(*iter)->getKey(),(*iter)->getName(),m_bExpandAllEntityTreeItems, childItemLevel, m_entityKey);
					AddChildItem(*newItem, true, true);
				}
			}
			else
				bHasChildren = false; 			
		}
		SortChildren();
		return bHasChildren;
	}

	TA_Base_Core::IEntityData* NonPhysicalEntityTreeItem::getEntityData()
	{
		TA_Base_Core::IEntityData* entityData = m_entityRetrievalFactory->getEntity(m_entityKey);
		return entityData;
	}

	const char* NonPhysicalEntityTreeItem::getItemLabelText()
	{
		return m_entityLabel.c_str();
	}
	
	const std::string NonPhysicalEntityTreeItem::getKey()
	{
		// return m_entityData->getName();
		return m_entityLabel;
	}

	unsigned long NonPhysicalEntityTreeItem::getParentKey() const
	{
		return m_parentKey;	
	}

	unsigned long NonPhysicalEntityTreeItem::getEntityKey() const
	{
		return m_entityKey;
	}
	HICON NonPhysicalEntityTreeItem::getItemImage(bool itemExpanded)
	{
		// No icons by default
		static HICON stationIcon=0;
		
		return stationIcon;
	}
}





