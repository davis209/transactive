/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/NonPhysicalEntityItem.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// NonPhysicalEntityItem.h: interface for the NonPhysicalEntityItem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NONPHYSICALENTITYITEM_H__3CD74713_DCC2_4A6C_A291_15CA47111E66__INCLUDED_)
#define AFX_NONPHYSICALENTITYITEM_H__3CD74713_DCC2_4A6C_A291_15CA47111E66__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bus\mfc_extensions\src\mfc_extensions.h"

namespace TA_Base_Core
{
	class IEntityData;
}

namespace TA_IRS_App
{	
	class NonPhysicalEntityFactory;
 
	class NonPhysicalEntityTreeItem : public CObjectTreeItem
	{
	private:
		enum ETreeItemLevel
		{	TREEITEM_UNKNOWNLEVEL	= -1,
			TREEITEM_ROOT			= 0, 
			TREEITEM_LOCATION		= 1,
			TREEITEM_ENTITYTYPE		= 2,
			TREEITEM_NONPHYSICALENTITY	= 3
		};

	public:
		NonPhysicalEntityTreeItem(NonPhysicalEntityFactory* entityRetrievalFactory, 
				unsigned long entityKey, std::string  entityLabel,	bool bExpandAllEntityTreeItems = true, 
				ETreeItemLevel treeItemLevel = TREEITEM_UNKNOWNLEVEL, unsigned long	parentKey = 0);

		virtual ~NonPhysicalEntityTreeItem();
		
		TA_Base_Core::IEntityData* getEntityData();// { return m_entityData;};
		
		virtual bool createChildItems();
		virtual const char * getItemLabelText();
		virtual bool hasChildren();
		virtual const std::string getKey();
		virtual HICON getItemImage(bool itemExpanded);
		virtual bool isEntityType();
		virtual unsigned long getEntityKey() const;
		virtual unsigned long getParentKey() const;
 		 
	private:
	
		
		std::string					m_entityLabel;
		unsigned long				m_entityKey;
		unsigned long				m_parentKey;
		ETreeItemLevel				m_treeLevel;
		
		//TA_Base_Core::IEntityData*  m_entityData;		
		//bool						m_hasChildren;

 		bool m_bExpandAllEntityTreeItems;
		// This is used to retrieve the entity information. If this is NULL then we use the
		// EntityAccessFactory instead and must delete our own pointers. If this is not NULL then we are
		// not in charge of pointers and we must not delete them.
		NonPhysicalEntityFactory*        m_entityRetrievalFactory;

	};
}

#endif // !defined(AFX_NONPHYSICALENTITYITEM_H__3CD74713_DCC2_4A6C_A291_15CA47111E66__INCLUDED_)
