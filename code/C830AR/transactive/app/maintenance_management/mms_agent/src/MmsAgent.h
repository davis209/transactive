/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  * 
  * Source:    $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/MmsAgent.h $
  * @author:   Chris DeWolfe
  * @version:  $Revision: #1 $
  * @see:      9999-Q01-02-0016-GenericAgent_Usage_Guidelines.doc
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last mofified by:  $Author: builder $
  *
  * The MMS Agent provides a mechanism to submit messages to the MMS (Maintenance Managment System)
  *
  */
#ifndef MMS_AGENT_H
#define MMS_AGENT_H

#if defined(_MSC_VER)
#pragma warning(disable:4786)
#endif // defined (_MSC_VER)

#include <map>

#include "bus/generic_agent/src/IGenericAgentUser.h"
#include "bus/scada/DataPointAgentCorbaDef_Impl/src/DataPointAgentCorbaDef_Impl.h"
#include "bus/scada/DataPointAgentCorbaDef_Impl/src/DataPointAccessCorbaDef_Impl.h"
#include "bus/scada/DataNodeAgentCorbaDef_Impl/src/IDataNodeAgent.h"
#include "bus/scada/common_library/src/ScadaPersistence.h"

#include "core/data_access_interface/entity_access/src/MmsAgentEntityData.h"
#include "core/data_access_interface/entity_access/src/IEntityData.h"

namespace TA_Base_Core
{
	class MmsAgentEntityData;
	//class IEntityData;
}

namespace TA_Base_Bus
{
	class GenericAgent;
	class DataPoint;
    class DataPointFactory;	
	class DataNode;
	class ScadaProxyFactory;
	class SummaryProcessor;
	class DataNodeAgentCorbaDef_Impl;
	class DataNodeAccessCorbaDef_Impl;
}

namespace TA_IRS_App
{
	class MmsConnection;
	
    class MmsAgent : public virtual TA_Base_Bus::IGenericAgentUser,
					 public virtual TA_Base_Bus::IDataPointAgent,
					 public virtual TA_Base_Bus::IDataNodeAgent
    {
		typedef std::vector< TA_Base_Bus::DataNode* > DataNodeList;
    public:

        /**
          * The only avaiable CTOR
          */
        MmsAgent(int argc, char* argv[]);

        /**
          * Destructor
          */
        virtual ~MmsAgent();

       //IGenericAgentUser Methods:

        /**
          * agentTerminate
          *
          * This pure virtual method is called from the onTerminate
          * method in GenericAgent. This allows the agent to do any
          * of its own cleaning up before exiting.
		  *
          */
        virtual void agentTerminate();

        /** 
          * createAllEntities
          *
          * [optional]
          *
          * This allows for bulk entity creation.
          * It is called first (before any calls to createEntity) to allow agents to
          * optimise entity creation. If true is returned, the entities are already created
          * if false is returned, then createEntity is called one by one for each entity.
          * Also entityCreationComplete will be called at the end of the creation
          *
          * Note that even if using this method, createEntity should still be implemented
          * as it is used for entity updates
          *
          * @param agentEntityData the agent's entity data
          * @param entityDataList a list of shared pointers to all entities
          * @param createdEntities the entities created to return to generic agent
          *
          * @return true if entity creation was done, false if createEntity should be used
          *
          */
        virtual bool createAllEntities( TA_Base_Core::IEntityDataPtr agentEntityData,
                                        const TA_Base_Core::SharedIEntityDataList& entityDataList,
                                        TA_Base_Bus::IEntityList& createdEntities );

        /** 
          * entityCreationComplete
          *
          * [optional]
          *
          * this is called back after 
          * createEntity has been called for all entities.
          * The client can do any clean up of temporary data.
          */
        virtual void entityCreationComplete();

        /**
          * createEntity
          *
          * This method takes Entity Configuration data stored in the database and
          * generates an object that implements the IEntity interface. The returned
          * objects are stored and managed within Generic Agent. The Agent can get a 
          * pointer to the objects via the getEntityMap method in GenericAgent.
          *
          * If create entity throws an exception when Generic Agent is creating its
          * vector of entities (in the GenericAgent::doEntityConfiguration method), 
          * the Generic Agent construction will fail.
		  * 
		  * @throws TA_Base_Core::BadParameterException if you pass the agent config it does not
		  * like, or a child can not be created for any reason
          */
	    virtual TA_Base_Bus::IEntity* createEntity(TA_Base_Core::IEntityDataPtr entityData);

        /**
          * agentSetMonitor
          *
          * This pure virtual method is called by GenericAgent whenever it receives
          * a request from the Process Monitor to change its operating state to monitor.
          * The derived agent should respond by subscribing/unsubscribing to/from the 
          * appropriate message channels and performing any other internal house keeping.
		  *
		  * Note that setting the mode of child entities is handled by the gen_agent framework and
		  * should not be done in this method
		  */
        virtual void agentSetMonitor();

        /**
          * agentSetControl
          *
          * This pure virtual method is called by GenericAgent whenever it receives
          * a request from the Process Monitor to change its operating state to control.
          * The derived agent should respond by subscribing/unsubscribing to/from the 
          * appropriate message channels and performing any other internal house keeping.
		  *
		  * Note that setting the mode of child entities is handled by the gen_agent framework and
		  * should not be done in this method
		  *
		  *
          */
        virtual void agentSetControl();

		/** 
		  * notifyGroupOffline
		  *
		  * This request is passed on by GenericAgent when it receives a call to 
		  * notifyGroupOffline from the Process Monitor.
		  *
		  * The MMS Agent does not modify its behaviour based on the online/offline
		  * state of the System Controller. Hence, this method does nothing
		  *
		  * @param The group name
		  */
        virtual void notifyGroupOffline( const std::string& group );

		/** 
		  * notifyGroupOnline
		  *
		  * This request is passed on by GenericAgent when it receives a call to 
		  * notifyGroupOnline from the Process Monitor.
		  *
		  * The System Status Agent does not modify its behaviour based on the online/offline
		  * state of the System Controller. Hence, this method does nothing
		  *
		  * @param The group name
		  */
        virtual void notifyGroupOnline( const std::string& group );

        /**
          * registerForStateUpdates
          *
          * This pure virtual method forces agents to subscribe for either
          * state updates or state update requests, depending on whether or not they
          * are operating in monitor or control mode.
		  * 
          */
        virtual void registerForStateUpdates();

        /**
          * receiveSpecialisedMessage
          *
          * This method is called whenever the Agent receives a state update or
          * state update request (depending on their mode of operation)
		  *
		  * 
          */
        virtual void receiveSpecialisedMessage(const TA_Base_Core::StateUpdateMessageCorbaDef& message);

        /**
          * processOnlineUpdate
          *
          * Generic Agent will handle the receiving of online updates. However, in some
          * cases, it will be necessary to pass the updates on to the registered agent.
          * For example, if a child entity needs to be updated.
		  *
          *
          * @param The update event
          */
        virtual void processOnlineUpdate(const TA_Base_Core::ConfigUpdateDetails& updateEvent);

		//End of IGenericAgentUser Methods

		/**
		  * run
		  *
     	  * Calls GenericAgent::run() which blocks until the System
		  * Controller shuts us down
		  */
		void run();

		static TA_Base_Core::MmsAgentEntityDataPtr getAgentData() { return m_agentEntityData; }


		//
		// operations required by IDataPointAgent interface
		//

		virtual TA_Base_Bus::DataPoint * getDataPoint ( unsigned long entityKey );

        virtual void getAllDataPoints (std::map < unsigned long, TA_Base_Bus::DataPoint *>& dataPointList );
        
        virtual TA_Base_Bus::DataPointFactory* getDataPointFactory();
		
		virtual TA_Base_Bus::DataNode * getDataNode( unsigned long entityKey );
		
        virtual void getAllDataNodes (std::map < unsigned long, TA_Base_Bus::DataNode *>& dataNodeList );

    private: //data members:

		//The agent and related data:
		TA_Base_Bus::GenericAgent* m_genericAgent;
        
        TA_Base_Bus::DataPointFactory* m_dataPointFactory;
		unsigned long m_agentKey;

		static TA_Base_Core::MmsAgentEntityDataPtr m_agentEntityData;

		TA_Base_Core::EOperationMode m_operationMode;

		std::map< unsigned long, TA_Base_Bus::DataPoint * > m_datapoints;

		TA_Base_Core::ReEntrantThreadLockable m_dataPointListtLock;
		TA_Base_Core::ReEntrantThreadLockable m_dataNodeListLock;

		TA_Base_Bus::ScadaProxyFactory* m_scadaProxyFactory;
		TA_Base_Bus::SummaryProcessor* m_summaryProcessor;
		TA_Base_Bus::ScadaPersistencePtr m_persistence;
		
		TA_Base_Bus::DataPointAgentCorbaDef_Impl* m_dpBulkPolledInterface;
        TA_Base_Bus::DataPointAccessCorbaDef_Impl * m_dpAccessInterface;
		TA_Base_Bus::DataNodeAgentCorbaDef_Impl * m_dnBulkPolledInterface;
		TA_Base_Bus::DataNodeAccessCorbaDef_Impl * m_dnAccessInterface;
        
		DataNodeList m_dataNodes;

        std::string m_agentName;

	private: //methods:
		//TD18706,jianghp, handle the operation mode
		virtual void checkOperationMode(){};
        // Disable copy constructor and assignment operator
        MmsAgent( const MmsAgent& agent);
        MmsAgent& operator=(const MmsAgent& agent);

		void setChildrenToControl();

		void setChildrenToMonitor();

    };
} // end namespace TA_IRS_App

#endif // !defined(MMS_AGENT_H)