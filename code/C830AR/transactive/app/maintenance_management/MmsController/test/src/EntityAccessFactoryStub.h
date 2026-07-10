/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/EntityAccessFactoryStub.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma once

#include <string>

namespace TA_Base_Core
{
    class IEntityData;

	typedef std::vector< IEntityData* > IEntityDataList;

    struct CorbaName;

    class EntityAccessFactory
    {
    public:

        static EntityAccessFactory& getInstance(int = 0);

        TA_Base_Core::CorbaName getCorbaNameOfEntity(const std::string equipmentName);

        IEntityDataList getEntitiesOfTypeAtLocation( const std::string& type, unsigned long locationKey, int = 0 );
        IEntityDataList getChildEntitiesOf(const unsigned long key, const bool readWrite = false, int = 0);
        IEntityDataList getEntitiesOfType(const std::string& type, const bool readWrite = false, int = 0);
        IEntityData* getEntity(const unsigned long key, const bool readWrite = false, int = 0);
        IEntityData* getEntity(const std::string& name, const bool readWrite = false, int = 0);
    };
}