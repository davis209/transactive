/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/MmsInhibitListCtrl.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#if !defined(AFX_MMSINHIBITLISTCTRL_H__1B9A01D2_C51D_4E4C_88CC_588BC01C1AAA__INCLUDED_)
#define AFX_MMSINHIBITLISTCTRL_H__1B9A01D2_C51D_4E4C_88CC_588BC01C1AAA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MmsInhibitListCtrl.h : header file
//
//#include "bus/mfc_extensions/src/auto_sort_list_ctrl/AutoSortListCtrl.h"
//#include "bus/mfc_extensions/src/auto_sort_list_ctrl/ISortingCallback.h"
#include <map>
 
/////////////////////////////////////////////////////////////////////////////
// CMmsInhibitListCtrl window
namespace TA_Base_Core
{
	class IEntityData;
	
}

namespace TA_IRS_App
{
	
	class CMmsInhibitListCtrl : public CListCtrl
	{
		// Construction
	public:
		CMmsInhibitListCtrl();
		
		// Attributes
	public:
		virtual ~CMmsInhibitListCtrl();
		// Operations
	public:
		
		// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CMmsInhibitListCtrl)
		//}}AFX_VIRTUAL
		
		// Implementation
	public:
		virtual int FindEntity(const unsigned long entityKey);
 		virtual int insertMmsInhibitedEntity(TA_Base_Core::IEntityData* entityData, const CString& dateTime, 
			const CString& operatorName );
		virtual int insertMmsInhibitedEntity(TA_Base_Core::IEntityData* entityData);
		virtual void buildMmsInhibitList();
		virtual BOOL DeleteEntityItem( unsigned long entityKey);
		virtual	void populateNonPhysicalSubsystemList(std::map<unsigned long, TA_Base_Core::IEntityData*>& nonPhysicalSubsystemList);

		// Generated message map functions
	protected:
		//{{AFX_MSG(CMmsInhibitListCtrl)
		afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
		//}}AFX_MSG
		
		DECLARE_MESSAGE_MAP()
			
	private:

		CString getOperatorFromSession(std::string& sessionId);
		void setEntityItemList(int iItem, const CString& strDate, const CString& strAsset, 
			const CString& strDesc, const CString& strOperator );
		
		enum EColumns
		{
			DateTimeColumn			= 0,
			AssetNameColumn			= 1,
			AssetDescriptionColumn	= 2,
			OperatorNameColumn		= 3
		};
		
		
		std::map<unsigned long, int> m_entityKeyItemMap;
		//TA_Base_Bus::AutoSortListCtrl::SortInfo m_sortInfo;
		

	};
} // TA_IRS_App
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MMSINHIBITLISTCTRL_H__1B9A01D2_C51D_4E4C_88CC_588BC01C1AAA__INCLUDED_)
