/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/NonPhysicalEntityFactory.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// NonPhysicalEntityFactory.cpp: implementation of the NonPhysicalEntityFactory class.
//
//////////////////////////////////////////////////////////////////////

#include "app/maintenance_management/MmsController/src/stdafx.h"
#include "app/maintenance_management/MmsController/src/mmscontroller.h"
//#include "app/maintenance_management/MmsController/src/MmsControllerModel.h"
#include "app/maintenance_management/MmsController/src/NonPhysicalEntityFactory.h"


#include "core/data_access_interface/entity_access/src/EntityAccessFactory.h"
#include "core/data_access_interface/src/LocationAccessFactory.h"
#include "core/data_access_interface/src/SubsystemAccessFactory.h"
#include "core/data_access_interface/src/ILocation.h"
#include "core/data_access_interface/entity_access/src/IEntityData.h"
#include "core/data_access_interface/entity_access/src/ConfigEntity.h"
#include "core/utilities/src/DebugUtil.h"
#include "core/exceptions/src/DatabaseException.h"
#include "core/exceptions/src/DataException.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace TA_IRS_App
{	
	NonPhysicalEntityFactory::NonPhysicalEntityFactory()		
	{
		FUNCTION_ENTRY("NonPhysicalEntityFactory constructor");
		//refreshEntityData();
		FUNCTION_EXIT;				
	}
	
	NonPhysicalEntityFactory::~NonPhysicalEntityFactory()
	{
		FUNCTION_ENTRY("~NonPhysicalEntityFactory");
 		m_locationEntityTypeMap.clear();
		m_allEntityTypes.clear();
		m_allLocations.clear();
		m_nonPhysicalSubsystemEntities.clear();

		FUNCTION_EXIT;
	}  

    bool NonPhysicalEntityFactory::hasChildren(unsigned long entityKey, bool hasChildren)
    {
		FUNCTION_ENTRY("hasChildren");
		if (!m_nonPhysicalSubsystemEntities.empty())	 
		{	
			ConfigIterator iter = m_nonPhysicalSubsystemEntities.find(entityKey);
			if (iter != m_nonPhysicalSubsystemEntities.end())
				hasChildren = false;			
		}
		else
			hasChildren = false;
		
		FUNCTION_EXIT;
		return hasChildren;        
    }    


	bool NonPhysicalEntityFactory::isNonPhysicalSubsystemEntity(unsigned long entityKey)
	{
		FUNCTION_ENTRY("isNonPhysicalSubsystemEntity");
		bool bNonPhysicalEntity = false;

		if (!m_nonPhysicalSubsystemEntities.empty())
		{
			ConfigIterator iter = m_nonPhysicalSubsystemEntities.find(entityKey);
			if (iter != m_nonPhysicalSubsystemEntities.end())
				bNonPhysicalEntity = true;			
		}
		FUNCTION_EXIT;
		return bNonPhysicalEntity;
	}   


    TA_Base_Core::IEntityData* NonPhysicalEntityFactory::getEntity(unsigned long entityKey)
    {        
		FUNCTION_ENTRY("getEntity");
		TA_Base_Core::IEntityData* result = NULL;
		if (m_nonPhysicalSubsystemEntities.empty())
		{
			try
			{				
				result = TA_Base_Core::EntityAccessFactory::getInstance().getEntity(entityKey);
			}
			catch(...)
			{
				LOG_EXCEPTION_CATCH(SourceInfo, "Unknown Exception", "Entity data cannot be acccessed.");
			}
		}
		else
		{
			ConfigIterator iter = m_nonPhysicalSubsystemEntities.find(entityKey);
			if (iter != m_nonPhysicalSubsystemEntities.end())
			{
				result = dynamic_cast<TA_Base_Core::IEntityData*>(iter->second);
			}
			else
			{
				try
				{	// the entity data to be retrieved is not a non physical subsystem entity				
					result = TA_Base_Core::EntityAccessFactory::getInstance().getEntity(entityKey);
				}
				catch(...)
				{
					LOG_EXCEPTION_CATCH(SourceInfo, "Unknown Exception", "Entity data cannot be acccessed.");
				}

			}
		}
		FUNCTION_EXIT;
		return result;
    }


    TA_Base_Core::IEntityData* NonPhysicalEntityFactory::getEntity(const std::string& entityName)
    {
		FUNCTION_ENTRY("getEntity");
        // Simple pass-through
		TA_Base_Core::IEntityData* result = NULL;
		try
		{
			result = TA_Base_Core::EntityAccessFactory::getInstance().getEntity(entityName);
		}
		catch (...)
		{			
			LOG_EXCEPTION_CATCH(SourceInfo, "Unknown Exception", "Could not retrieve entity data");
		}

		FUNCTION_EXIT;
        return result;
    }

	bool NonPhysicalEntityFactory::isLocationEntity(unsigned long entityKey)
	{
		FUNCTION_ENTRY("isLocationEntity");
		bool isLocation = false;
		try
		{
			TA_Base_Core::ILocation* location = TA_Base_Core::LocationAccessFactory::getInstance().getLocationByKey(entityKey);
			if (NULL != location)
			{
				isLocation = true;
			}
			delete location;
			location = NULL;
		}
		catch(...)
		{
			// entitykey passed is not a valid location type
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo, "Entity is not a valid location");
			
		}
		FUNCTION_EXIT;
		return isLocation;
	}

	void NonPhysicalEntityFactory::refreshEntityData(std::map<unsigned long, TA_Base_Core::ConfigEntity*>& entities, bool bForceUpdate)
	{
		//TA_Base_Core::ThreadGuard guard( m_nonPhyisicalEntities);

		if (bForceUpdate)
		{
			m_nonPhysicalSubsystemEntities.clear();
		}

		if (m_nonPhysicalSubsystemEntities.empty())
		{		

			m_nonPhysicalSubsystemEntities = entities; // MmsControllerModel::getNonPhysicalSubsystems();	
			
			// reload the entity types for the new set of non physical subsystem entities
 			unsigned long previousLocation = -1;
			TEntityDataMap entityTypeListMap;
			for (ConfigIterator iter=m_nonPhysicalSubsystemEntities.begin();iter!=m_nonPhysicalSubsystemEntities.end(); ++iter)
			{
				TA_Base_Core::ConfigEntity* entityData = m_nonPhysicalSubsystemEntities[iter->first];
				if (0 != entityData)
				{
					unsigned long currentLocation = entityData->getLocation();
					unsigned long typeKey = entityData->getTypeKey();
					std::string typeName = entityData->getType();
					
					m_allLocations[currentLocation] = entityData->getLocationName();
					m_allEntityTypes[typeKey] = typeName;
					
					if (previousLocation != currentLocation)
					{
						if (!entityTypeListMap.empty())
						{							
							m_locationEntityTypeMap[previousLocation] = entityTypeListMap;
							entityTypeListMap.clear();
						}
						previousLocation = currentLocation;
					}
					entityTypeListMap[typeKey] = typeName;					
				}
			}

			if (!entityTypeListMap.empty())
			{
				m_locationEntityTypeMap[previousLocation] = entityTypeListMap;
			}
		}
	}

	std::map<unsigned long, std::string> NonPhysicalEntityFactory::getEntityTypesAtLocation(unsigned long locationkey)
	{
		return m_locationEntityTypeMap[locationkey];
	}

	std::vector<TA_Base_Core::IEntityData*> NonPhysicalEntityFactory::getEntitiesOfTypeAtLocation( unsigned long entityTypeKey,
			unsigned long locationkey)
    {
		FUNCTION_ENTRY("getEntitiesOfTypeAtLocation");
		std::vector<TA_Base_Core::IEntityData*> result;
		
		if (isEntityTypeExisting(entityTypeKey))
		{		 	
			if (m_nonPhysicalSubsystemEntities.empty())
			{
				FUNCTION_EXIT;
				return std::vector<TA_Base_Core::IEntityData*>();
			}
			else
			{	 
 				for (ConfigIterator iter=m_nonPhysicalSubsystemEntities.begin(); iter!=m_nonPhysicalSubsystemEntities.end(); ++iter)
				{    
					TA_Base_Core::IEntityData* entityData = dynamic_cast<TA_Base_Core::IEntityData*>(iter->second);
					if ((0 != entityData) && (entityData->getLocation() == locationkey) && 
						(entityData->getTypeKey() == entityTypeKey))
					{
						result.push_back(iter->second);
					}
				}
			}
		}
		else
		{
			result =  std::vector<TA_Base_Core::IEntityData*>();
		}
		FUNCTION_EXIT;
		return result;       
    } 

	bool NonPhysicalEntityFactory::isEntityTypeExisting(unsigned long entityTypeKey)
	{
		FUNCTION_ENTRY("isEntityType");
		bool bEntityTypeKeyFound;

		std::map<unsigned long, std::string>::iterator it;
		it = m_allEntityTypes.find(entityTypeKey);
		if (it != m_allEntityTypes.end())
			bEntityTypeKeyFound = true;

		FUNCTION_EXIT;
		return bEntityTypeKeyFound;
	}

}
