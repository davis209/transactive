/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source: $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/MmsEntityFactory.h $
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

#pragma once

#include "bus/scada/scada_tree/src/IEntityFactory.h"
#include <map>

namespace TA_IRS_App
{
    class MmsEntityFactory : public TA_Base_Bus::IEntityFactory
    {
    public:
        virtual ~MmsEntityFactory() {};

        /**
         * Interface implementations (see IEntityFactory for further details)
         */
        virtual std::vector<TA_Base_Core::IEntityData*> getChildEntitiesOf(unsigned long entityKey);
        virtual std::vector<TA_Base_Core::IEntityData*> getEntitiesOfType(const std::string& entityTypeName);
        virtual TA_Base_Core::IEntityData* getEntity(unsigned long entityKey);
        virtual TA_Base_Core::IEntityData* getEntity(const std::string& entityName);
        virtual bool hasChildren(unsigned long entityKey, bool hasChildren);

        /**
         * @return the level of the specified datanode (or -1 if not a datanode)
         *          Top level datanodes (level 0) are stations (DBG, CVC etc);
         *          next level (1) are systems (DBG.PA);
         *          followed by subsystems (level 2) - DBG.PA.PUBLIC etc
         */
        static int getDatanodeEntityLevel(TA_Base_Core::IEntityData& entity);

        /**
         * @return true if the specified entity is a datanode entity and has a level
         *          at or below subsystem level in the datanode tree
         */
        static bool isDatanodeEntitySubsytemOrLower(TA_Base_Core::IEntityData& entity);

        /**
         * @return true if the specified entity is a datanode entity at the
         *          subsystem level
         */
        static bool isDatanodeEntitySubsytem(TA_Base_Core::IEntityData& entity);

    private:

        /**
         * @return false if the entity defined by the input key is a subsystem level datapoint
         *          (or lower), and should not have it's child items enumerated
         *          This function uses cached expansion states, so is only valid provided none of
         *          the datanodes change during the lifetime of the class
         *          (have SCADA, station, systems, then subsystems)
         */
        bool shouldExpandChildren(unsigned long entityKey);

        /**
         * Actual function that determines whether an entity should have its children expanded
         * @see shouldExpandChildren(unsigned long) for details
         */
        static bool shouldExpandChildren(TA_Base_Core::IEntityData& entity);

        /**
         * Caches the 'expand children' state of the specified node, so that cached states
         *      can be returned in function shouldExpandChildren(unsigned long)
         * @return the value of the cached expand children state, @see shouldExpandChildren(unsigned long)
         */
        bool cacheExpandChildrenState(TA_Base_Core::IEntityData* entity);

        /**
         * Helper function to perform caching on an array of entities,
         *   see single entity implementation for details
         */
        void cacheExpandChildrenState(std::vector<TA_Base_Core::IEntityData*>& entities);

        typedef std::map<unsigned long, bool> ChildExpansionMap;
        /// Caches child expansion states, otherwise tree control requests too many entities
        /// (causing significant slowdown)
        ChildExpansionMap m_childExpansionMap;
    };
}
