/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source: $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/MmsEntityFactory.cpp $
  * @author Jade Lee
  * @version $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by: $Author: builder $
  * 
  * Provides customisation of entity factory to suppress nodes
  * greater than three levels (subsystem level) deep
  *
  */

#pragma warning ( disable : 4250 4786 4284)

#include "app/maintenance_management/MmsController/src/stdafx.h"
#include "app/maintenance_management/MmsController/src/MmsEntityFactory.h"
#include "core/data_access_interface/entity_access/src/DataPointEntityData.h"
#include "core/data_access_interface/entity_access/src/DataNodeEntityData.h"
#include "core/data_access_interface/entity_access/src/EntityAccessFactory.h"
#include <map>

namespace TA_IRS_App
{

    int MmsEntityFactory::getDatanodeEntityLevel(TA_Base_Core::IEntityData& entity)
    {
        using namespace TA_Base_Core;

        if (entity.getType() == DataNodeEntityData::getStaticType())				
        {
            try
            {
			    DataNodeEntityData& dnEntity = dynamic_cast<DataNodeEntityData&>(entity);
                        
                // Determine if this entity is at subsystem level - if so, its
                // children shouldn't be enumerated.  We check the depth of the node
                // by inspecting its structured name, DBG.PA.Public is an example of a subsystem
                // - just look for two (or more) full stop delimiters                
                std::string::size_type i = -1;
                int dotCount = 0;
                
                std::string entityName = dnEntity.getName();

                while ((i = entityName.find('.', i + 1)) != std::string::npos)
                {
                    ++ dotCount;
                }
                             
                // DBG returns 0
                // DBG.PA returns 1
                return dotCount;
            }
            catch (...)
            {
                // Upon error, assume not a valid datanode
            }
		}

        // Not a datanode
        return -1;
    }


    bool MmsEntityFactory::isDatanodeEntitySubsytemOrLower(TA_Base_Core::IEntityData& entity)
    {
        int level = getDatanodeEntityLevel(entity);

        return (level >= 2);
    }


    bool MmsEntityFactory::isDatanodeEntitySubsytem(TA_Base_Core::IEntityData& entity)
    {
        int level = getDatanodeEntityLevel(entity);

        return (2 == level);
    }


    bool MmsEntityFactory::shouldExpandChildren(unsigned long entityKey)
    {
        ChildExpansionMap::iterator it = m_childExpansionMap.find(entityKey);
        
        if (it == m_childExpansionMap.end())
        {
            // No cached result (in current implementation this should never happen)
            std::auto_ptr<TA_Base_Core::IEntityData> entity(getEntity(entityKey));
            return cacheExpandChildrenState(entity.get());
        }
        else
        {
            return it->second;
        }       
    }


    bool MmsEntityFactory::shouldExpandChildren(TA_Base_Core::IEntityData& entity)
    {
        if (entity.getType() == TA_Base_Core::DataPointEntityData::getStaticType())
        {
            // This is a datapoint so it definitely does not have any children
            return false;
        }

        if (isDatanodeEntitySubsytemOrLower(entity))
        {
            // Subsystem or greater depth, do not expand children
            return false;
        }

        // As far as we can tell, should expand children (if we have any children)
        return true;
    }


    bool MmsEntityFactory::cacheExpandChildrenState(TA_Base_Core::IEntityData* entity)
    {
        if (0 != entity)
        {
            bool expandChildren = shouldExpandChildren(*entity);           
            m_childExpansionMap[entity->getKey()] = expandChildren;
            return expandChildren;
        }
        else
        {
            return false;
        }
    }


    void MmsEntityFactory::cacheExpandChildrenState(std::vector<TA_Base_Core::IEntityData*>& entities)
    {
        for (std::vector<TA_Base_Core::IEntityData*>::const_iterator it = entities.begin(); it != entities.end(); ++it )
        {
            cacheExpandChildrenState(*it);
        }
    }


    std::vector<TA_Base_Core::IEntityData*> MmsEntityFactory::getChildEntitiesOf(unsigned long entityKey)
    {
        if (!shouldExpandChildren(entityKey))
        {
            // Cut off tree at the subsytem level, return empty vector
            return std::vector<TA_Base_Core::IEntityData*>();
        }
        else
        {
            std::vector<TA_Base_Core::IEntityData*> result = 
                TA_Base_Core::EntityAccessFactory::getInstance().getChildEntitiesOf(entityKey);

            cacheExpandChildrenState(result);

            return result;
        }
    }


    bool MmsEntityFactory::hasChildren(unsigned long entityKey, bool hasChildren)
    {
        if (!shouldExpandChildren(entityKey))
        {
            // Cut off tree at the subsytem level
            return false;
        }        
        
        return hasChildren;        
    }    


    std::vector<TA_Base_Core::IEntityData*> MmsEntityFactory::getEntitiesOfType(const std::string& entityTypeName)
    {
        // Simple pass-through (caching has children state)
        std::vector<TA_Base_Core::IEntityData*> result =  
                TA_Base_Core::EntityAccessFactory::getInstance().getEntitiesOfType(entityTypeName);

        cacheExpandChildrenState(result);

        return result;
    }


    TA_Base_Core::IEntityData* MmsEntityFactory::getEntity(unsigned long entityKey)
    {
        // Simple pass-through
        TA_Base_Core::IEntityData* result = TA_Base_Core::EntityAccessFactory::getInstance().getEntity(entityKey);
        cacheExpandChildrenState(result);
        return result;
    }


    TA_Base_Core::IEntityData* MmsEntityFactory::getEntity(const std::string& entityName)
    {
        // Simple pass-through
        TA_Base_Core::IEntityData* result = TA_Base_Core::EntityAccessFactory::getInstance().getEntity(entityName);
        cacheExpandChildrenState(result);
        return result;
    }
}
