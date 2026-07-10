/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/MmsInhibitListCtrl.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// MmsInhibitListCtrl.cpp : implementation file
//

#include "app/maintenance_management/MmsController/src/stdafx.h"
#include "app/maintenance_management/MmsController/src/mmscontroller.h"
#include "app/maintenance_management/MmsController/src/MmsInhibitListCtrl.h"
#include "core/data_access_interface/entity_access/src/IEntityData.h"
#include "core/data_access_interface/entity_access/src/EntityStatusData.h"
#include "core/data_access_interface/src/SessionAccessFactory.h"
#include "core/data_access_interface/src/ISession.h"
#include "core/exceptions/src/DataException.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMmsInhibitListCtrl
namespace TA_IRS_App
{
	
	CMmsInhibitListCtrl::CMmsInhibitListCtrl()
	{
		
	}
	
	CMmsInhibitListCtrl::~CMmsInhibitListCtrl()
	{
	}
	
	
	BEGIN_MESSAGE_MAP(CMmsInhibitListCtrl, CListCtrl)
		//{{AFX_MSG_MAP(CMmsInhibitListCtrl)
		ON_WM_SHOWWINDOW()
		//}}AFX_MSG_MAP
		END_MESSAGE_MAP()
		
		/////////////////////////////////////////////////////////////////////////////
		// CMmsInhibitListCtrl message handlers
		
	void CMmsInhibitListCtrl::OnShowWindow(BOOL bShow, UINT nStatus) 
	{
		
		CListCtrl::OnShowWindow(bShow, nStatus);	
		
		// TODO: Add your message handler code here
		// m_sortInfo.defineSortingSemantic(i,sortBy);
		// m_sortInfo.setCallback(this);
		// m_sortInfo.setCurrentSort(DateTimeColumn, AutoSortListCtrl::DESCENDING);
		// m_sortInfo.activateSort(DateTimeColumn);
	
		
	}
	
	void CMmsInhibitListCtrl::buildMmsInhibitList()
	{
		ModifyStyle( NULL, LVS_REPORT | LVS_SHOWSELALWAYS);
		SetExtendedStyle( LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT| LVS_EX_INFOTIP );
		
		InsertColumn(DateTimeColumn,		"Date/Time",		LVCFMT_LEFT, 100);
		InsertColumn(AssetNameColumn,		"Asset Name",		LVCFMT_LEFT, 100);
		InsertColumn(AssetDescriptionColumn,"Asset Description",LVCFMT_LEFT, 160);
		InsertColumn(OperatorNameColumn,	"Operator Name",	LVCFMT_LEFT, 110);
	}

	
	void CMmsInhibitListCtrl::populateNonPhysicalSubsystemList(std::map<unsigned long, TA_Base_Core::IEntityData*>& nonPhysicalSubsystemList)
	{
		
		DeleteAllItems();
		m_entityKeyItemMap.clear();
		
		if (!nonPhysicalSubsystemList.empty())
		{
			int iItem = 0;
			
			std::map<unsigned long, TA_Base_Core::IEntityData*>::iterator iter;
			for (iter=nonPhysicalSubsystemList.begin(); iter!=nonPhysicalSubsystemList.end(); iter++)
			{	
				TA_Base_Core::IEntityData* entityData = dynamic_cast<TA_Base_Core::IEntityData*>(iter->second);
				
				try
				{					
					TA_Base_Core::EntityStatusData statusData = entityData->getEntityStatusData(TA_Base_Core::MMSInhibitStatus);
					
					if (true == entityData->getBoolEntityStatusValue(TA_Base_Core::MMSInhibitStatus))
					{
						
						CString entityKeyStr;
						entityKeyStr.Format("%ul",entityData->getKey());
						InsertItem(iItem,entityKeyStr);
						SetItemData(iItem, (LPARAM) entityData);
						CString dateFormatStr("%d-%b-%y %H:%M:%S");
						CString strDate = CTime(statusData.lastModifyDate).Format(dateFormatStr);					 
						CString operatorName = getOperatorFromSession(statusData.lastSessionId);			 	
						setEntityItemList(iItem, strDate, entityData->getName().c_str(), entityData->getDescription().c_str(), operatorName);
						m_entityKeyItemMap[entityData->getKey()] = iItem;
					}
				}
				catch (TA_Base_Core::DataException&)
				{
					std::ostringstream reasonMessage;
					reasonMessage << "No MMSInhibitStatus data found for entity = " << entityData->getKey();
					LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "DataException", reasonMessage.str().c_str());
				}
			}
		}
	}
			
 
	int CMmsInhibitListCtrl::insertMmsInhibitedEntity(TA_Base_Core::IEntityData* entityData, 
										const CString& dateTime, const CString& operatorName )
	{
 		
		int iItem = -1;

		if (entityData != NULL)
		{		
			iItem = GetItemCount();
				
			CString entityKeyStr;
			entityKeyStr.Format("%ul",entityData->getKey());
			InsertItem(iItem,entityKeyStr);
			SetItemData(iItem, (LPARAM) entityData);	 

			setEntityItemList(iItem, dateTime, entityData->getName().c_str(), entityData->getDescription().c_str(), operatorName);
			m_entityKeyItemMap[entityData->getKey()] = iItem;			
		}
		return iItem;
	}

	int CMmsInhibitListCtrl::insertMmsInhibitedEntity(TA_Base_Core::IEntityData* entityData)
	{		
		int iItem = -1;
		
		if (NULL != entityData) 
		{
			try
			{				
				if (true == entityData->getBoolEntityStatusValue(TA_Base_Core::MMSInhibitStatus))
				{			
					iItem = GetItemCount();
					
					CString entityKeyStr;
					entityKeyStr.Format("%ul",entityData->getKey());
					InsertItem(iItem,entityKeyStr);
					SetItemData(iItem, (LPARAM) entityData);
					
					TA_Base_Core::EntityStatusData statusData = entityData->getEntityStatusData(TA_Base_Core::MMSInhibitStatus);
					
					CString strDate;
					strDate.Format("",statusData.lastModifyDate);
					CString operatorName = getOperatorFromSession(statusData.lastSessionId);			
					setEntityItemList(iItem, strDate, entityData->getName().c_str(), entityData->getDescription().c_str(), operatorName);
					
					m_entityKeyItemMap[entityData->getKey()] = iItem;
					
				}
			}
			catch (TA_Base_Core::DataException&)
			{
				std::ostringstream reasonMessage;
				reasonMessage << "No MMSInhibitStatus data found for entity = " << entityData->getKey();
				LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "DataException", reasonMessage.str().c_str());
			}
		}
		
		return iItem;
	}

	void CMmsInhibitListCtrl::setEntityItemList(int iItem, const CString& strDate, const CString& strAsset, const CString& strDesc, const CString& strOperator )
	{
		SetItemText(iItem,DateTimeColumn, strDate);
		SetItemText(iItem,AssetNameColumn, strAsset);
		SetItemText(iItem,AssetDescriptionColumn, strDesc);
		SetItemText(iItem,OperatorNameColumn, strOperator);		
		
	}

	int CMmsInhibitListCtrl::FindEntity(const unsigned long entityKey)
	{
 		
		int iItem = -1;
		std::map<unsigned long, int>::iterator itemIter = m_entityKeyItemMap.find(entityKey);
		if (itemIter != m_entityKeyItemMap.end())
			iItem = itemIter->second;
		return iItem;
	}

	BOOL CMmsInhibitListCtrl::DeleteEntityItem( unsigned long entityKey)
	{	
 
		int iItem = FindEntity(entityKey);
		m_entityKeyItemMap.erase(entityKey);
		return CListCtrl::DeleteItem(iItem);
	}
	
	CString CMmsInhibitListCtrl::getOperatorFromSession(std::string& sessionId)
	{
		CString operatorName;
		TA_Base_Core::ISession* session = NULL; 
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

	

} //TA_IRS_App







