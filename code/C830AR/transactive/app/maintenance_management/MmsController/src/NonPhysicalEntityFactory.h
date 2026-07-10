/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/NonPhysicalEntityFactory.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// NonPhysicalEntityFactory.h: interface for the NonPhysicalEntityFactory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NONPHYSICALENTITYFACTORY_H__88B2B86D_113C_4E0C_8A80_35C0DB28368C__INCLUDED_)
#define AFX_NONPHYSICALENTITYFACTORY_H__88B2B86D_113C_4E0C_8A80_35C0DB28368C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <vector>

namespace TA_Base_Core
{
    class IEntityData;
    class ConfigEntity;
}

namespace TA_IRS_App
{
    class NonPhysicalEntityFactory
    {
    public:
        NonPhysicalEntityFactory();
        virtual ~NonPhysicalEntityFactory();

        virtual TA_Base_Core::IEntityData* getEntity(unsigned long entityKey);
        virtual TA_Base_Core::IEntityData* getEntity(const std::string& entityName);
        virtual bool hasChildren(unsigned long entityKey, bool hasChildren = true);

        virtual bool isNonPhysicalSubsystemEntity(unsigned long entityKey);
        virtual bool isLocationEntity(unsigned long entityKey);

        virtual void refreshEntityData(std::map<unsigned long, TA_Base_Core::ConfigEntity*>& entities, bool bForceUpdate = false);
        virtual std::map<unsigned long, std::string> getAllValidLocations() { return m_allLocations; }
        virtual std::map<unsigned long, std::string> getEntityTypesAtLocation(unsigned long locationkey);
        virtual std::map<unsigned long, std::string> getAllEntityTypes() { return m_allEntityTypes; }
        virtual std::vector<TA_Base_Core::IEntityData*> getEntitiesOfTypeAtLocation(unsigned long entityTypeKey, unsigned long locationkey);
        virtual bool isEntityTypeExisting(unsigned long entityTypeKey);

    private:

        TA_Base_Core::ReEntrantThreadLockable m_nonPhyisicalEntities;

        std::map<unsigned long, TA_Base_Core::ConfigEntity*> m_nonPhysicalSubsystemEntities;

        typedef std::map<unsigned long, TA_Base_Core::ConfigEntity*>::iterator ConfigIterator;
        typedef std::map<unsigned long, std::string> TEntityDataMap;
        std::map<unsigned long, TEntityDataMap> m_locationEntityTypeMap;
        TEntityDataMap m_allEntityTypes;
        TEntityDataMap m_allLocations;
    };
}

#endif // !defined(AFX_NONPHYSICALENTITYFACTORY_H__88B2B86D_113C_4E0C_8A80_35C0DB28368C__INCLUDED_)
