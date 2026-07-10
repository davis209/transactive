/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/EntityAccessFactoryStub.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#include "StdAfx.h"
#include "EntityAccessFactoryStub.h"
#include "core/naming/src/NamedObject.h"

namespace TA_Base_Core
{
    EntityAccessFactory& EntityAccessFactory::getInstance(int)
    {
        static EntityAccessFactory sInstance;
        return sInstance;
    }


    TA_Base_Core::CorbaName EntityAccessFactory::getCorbaNameOfEntity(const std::string equipmentName)
    {
        return TA_Base_Core::CorbaName("AgentName", equipmentName);            
    }
    

    IEntityDataList EntityAccessFactory::getEntitiesOfTypeAtLocation( 
        const std::string& type, unsigned long locationKey, int)     
    { 
        IEntityDataList r; 
        return r;
    }
        
    
    IEntityDataList EntityAccessFactory::getChildEntitiesOf(
        const unsigned long key, const bool readWrite, int)
    { 
        IEntityDataList r; 
        return r;
    }


    IEntityDataList EntityAccessFactory::getEntitiesOfType(const std::string& type, const bool readWrite, int)
    { 
        IEntityDataList r; 
        return r;
    }


    IEntityData* EntityAccessFactory::getEntity(const unsigned long key, const bool readWrite, int)
    {
        return 0;
    }


    IEntityData* EntityAccessFactory::getEntity(const std::string& name, const bool readWrite, int)
    {
        return 0;
    }

}
