/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/NonPhysicalSubsystemTree.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// NonPhysicalSubsystemTree.h: interface for the NonPhysicalSubsystemTree class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NONPHYSICALSUBSYSTEMTREE_H__4AD6E9CD_BDC4_4BA0_9EC1_06B4792B2E57__INCLUDED_)
#define AFX_NONPHYSICALSUBSYSTEMTREE_H__4AD6E9CD_BDC4_4BA0_9EC1_06B4792B2E57__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bus/mfc_extensions/src/mfc_extensions.h"
#include "app/maintenance_management/MmsController/src/NonPhysicalEntityFactory.h"

namespace TA_Base_Core
{
	class IEntityData;
	class ConfigEntity;
}
 
namespace TA_IRS_App
{		
    class NonPhysicalEntityTreeItem;

	class NonPhysicalSubsystemTree  :  public CObjectTreeCtrl
	{
	public:
		NonPhysicalSubsystemTree(bool bAllowSubsystemSelection = true);
		virtual ~NonPhysicalSubsystemTree();

		// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(NonPhysicalSubsystemTree)
	protected:
		virtual void PreSubclassWindow();
		virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
		
		//}}AFX_VIRTUAL		
		//operations
	public:
		void populateTree(std::map<unsigned long, TA_Base_Core::ConfigEntity*>& entities, bool bForceUpdate = false);
		int  countNodes(HTREEITEM item);
		bool isEntitySelected();
		std::vector<TA_Base_Core::IEntityData*> getSelectedEntities();
		int countSelectedItems();
 
		// Generated message map functions
    protected:
		//{{AFX_MSG(NonPhysicalSubsystemTree)
		afx_msg void OnSelchangingTree(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg BOOL OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		//}}AFX_MSG
		
		DECLARE_MESSAGE_MAP()
			 
	private:
		
		NonPhysicalEntityTreeItem* getEntityFromTreeItem(const TV_ITEM& tvItem);

		// This is the entity type that will be at the root of our tree
        static const std::string ROOT_ENTITY_TYPE;

		NonPhysicalEntityFactory m_entityFactory;

	    bool m_onlyAllowSubsystemSelection;
	
	};
}
#endif // !defined(AFX_NONPHYSICALSUBSYSTEMTREE_H__4AD6E9CD_BDC4_4BA0_9EC1_06B4792B2E57__INCLUDED_)
