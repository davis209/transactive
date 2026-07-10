/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution in any form.
 *
 * Source:    $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/AbstractPAAgent.h $
 * @author:   Andy Parker
 * @version:  $Revision: #3 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include <string>
#include "core/synchronisation/src/NonReEntrantThreadLockable.h"
#include "core/process_management/IDL/src/ProcessManagementDataDefinitionsCorbaDef.h"
#include "core/data_access_interface/entity_access/src/IEntityData.h"
#include "core/message/src/SpecialisedMessageSubscriber.h"
//#include "core/message/IDL/src/CommsMessageCorbaDef.h" //limin, failover issue
#include "core/exceptions/src/DataException.h"
#include "core/exceptions/src/DatabaseException.h"
#include "core/exceptions/src/GenericAgentException.h"
#include "core/exceptions/src/PasExceptions.h"
#include "core/synchronisation/src/ReEntrantThreadLockable.h"

#include "bus/generic_agent/src/IGenericAgentUser.h"
#include "bus/scada/datapoint_library/src/DataPointFactory.h"
#include "bus/scada/DataPointAgentCorbaDef_Impl/src/DataPointAgentCorbaDef_Impl.h"
#include "bus/scada/DataPointAgentCorbaDef_Impl/src/DataPointAccessCorbaDef_Impl.h"
#include "bus/scada/DataNodeAgentCorbaDef_Impl/src/IDataNodeAgent.h"
#include "bus/scada/common_library/src/ScadaPersistence.h"

// song toan++
// bug 459
#include "app/pa/PAAgent/src/FireCountdownAckMonitor.h"
// ++song toan
// bug 459

namespace TA_Base_Bus
{
    class DataPoint;
    class DataPointFactory;
    class DataNode;
    class ScadaProxyFactory;
    class SummaryProcessor;
    class DataNodeAgentCorbaDef_Impl;
    class DataNodeAccessCorbaDef_Impl;
    class GenericAgent;
    class IEntity;
};

namespace TA_Base_Core
{
    class AuditMessageSender;
    class StateUpdateMessageSender;
    class IEntityData;
    class ConfigUpdateDetails;
    struct StateUpdateMessageCorbaDef;
}

namespace TA_IRS_App
{
    class StationPasTableManager;
    class PasTableManager;
    class MasterPasTableManager;
    class BroadcastManager;
    class DataPointWriteQueueProcessor;

    typedef std::vector<TA_Base_Bus::DataPoint*> DataPoints;
    typedef DataPoints::iterator    DataPointsIt;

    class AbstractPAAgent : public virtual TA_Base_Bus::IGenericAgentUser,
    //public virtual TA_Base_Core::SpecialisedMessageSubscriber <TA_Base_Core::CommsMessageCorbaDef>, //limin, failover issue
    //public virtual TA_Base_Core::SpecialisedMessageSubscriber <TA_Base_Core::StateUpdateMessageCorbaDef>, //limin, failover issue
        public virtual TA_Base_Bus::IDataPointAgent,
        public virtual TA_Base_Bus::IDataNodeAgent
    {
        typedef std::vector< TA_Base_Bus::DataNode* > DataNodeList;

    public:

        AbstractPAAgent();

        virtual ~AbstractPAAgent();

        /**
         * agentTerminate
         *
         * This method is called by the GenericAgent when it is time to
         * terminate.
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
        virtual bool createAllEntities(TA_Base_Core::IEntityDataPtr agentEntityData,
                                       const TA_Base_Core::SharedIEntityDataList& entityDataList,
                                       TA_Base_Bus::IEntityList& createdEntities);

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
         * This method is called by the GenericAgent to allow the child entities to be
         * created.  These entities are then managed by the generic agent.
         *
         * @param Entity data retrieved from the database.
         *
         * @return The constructed entity
         */
        virtual TA_Base_Bus::IEntity* createEntity(TA_Base_Core::IEntityDataPtr entityData);
        //throw(TA_Base_Core::InvalidPasConfigurationException);

        TA_Base_Bus::IEntity* createAndInitialiseDataPointEntity(TA_Base_Core::IEntityDataPtr EntityData);
        //throw(TA_Base_Core::InvalidPasConfigurationException);

        DataPoints getDataPointsByToken(const std::string& token);

        /**
         * agentSetMonitor
         *
         * This method is called by GenericAgent when the Alarm Agent operating mode is
         * set to Monitor.
         */
        virtual void agentSetMonitor();

        virtual void agentSetMonitorSpecifics() = 0;

        /**
         * startingInMonitorMode
         *
         * This method executes code specifically for an agent starting up in monitor mode.
         * e.g. complete state synchronisation.
         */
        void startingInMonitorMode();

        /**
         * startingInControlMode
         *
         * This method executes code specifically for an agent starting up in control mode.
         * e.g. complete table synchronisation ISCS - PAS .
         */
        void startingInControlMode();

        /**
         * agentSetControl
         *
         * This method is called by GenericAgent when the Alarm Agent operating mode is
         * set to Control.
         */
        virtual void agentSetControl();

        virtual void agentSetControlSpecifics() = 0;

        /**
         * notifyGroupOffline
         *
         * This request is passed on by GenericAgent when it receives a call to
         * notifyGroupOffline from the Process Monitor.
         *
         * @param The group name
         */
        virtual void notifyGroupOffline(const std::string& group);

        /**
         * notifyGroupOnline
         *
         * This request is passed on by GenericAgent when it receives a call to
         * notifyGroupOnline from the Process Monitor.
         *
         * @param The group name
         */
        virtual void notifyGroupOnline(const std::string& group);

        /**
         * registerForStateUpdates
         *
         * This pure virtual method forces agents to subscribe for either
         * state updates or state update requests, depending on whether or not they
         * are operating in monitor or control mode.
         */
        virtual void registerForStateUpdates();

        /**
         * receiveSpecialisedMessage
         *
         * This method is called whenever the Agent receives a state update or
         * state update request (depending on their mode of operation)
         *
         */
        virtual void receiveSpecialisedMessage(const TA_Base_Core::StateUpdateMessageCorbaDef& message);

        /**
         * receiveSpecialisedMessage
         *
         * This method is called whenever the Monitor Agent receives a datapoint state update
         *
         */
        //virtual void receiveSpecialisedMessage(const TA_Base_Core::CommsMessageCorbaDef& message); //limin, failover issue

        /**
         * processOnlineUpdate
         *
         * This is called by Generic Agent whenever it gets an online update that it
         * cant handle itself.
         *
         * @param The update event
         */
        void processOnlineUpdate(const TA_Base_Core::ConfigUpdateDetails& updateEvent);

        /**
         * run
         *
         * This method calls GenericAgent::run() which blocks until the System
         * Controller shuts us down
         */
        void run();

        void killMeAndRaiseAlarm();

        TA_Base_Bus::DataPoint* getDataPoint(unsigned long entityKey);

        virtual void getAllDataPoints(std::map < unsigned long, TA_Base_Bus::DataPoint*>& dataPointList);

        virtual TA_Base_Bus::DataPointFactory* getDataPointFactory();

        virtual TA_Base_Bus::DataNode* getDataNode(unsigned long entityKey);

        virtual void getAllDataNodes(std::map < unsigned long, TA_Base_Bus::DataNode*>& dataNodeList);

    protected:

        //TD18706,jianghp, handle the operation mode
        virtual void checkOperationMode() {};

        /**
         * setup
         *
         * This method must be called by the derived classes before any other operations are
         * called in the base class.  It is best to call this method in the constructor of
         * the derived class.  This design is enforced by assertions.
         *
         * Unfortunately, this messiness was necessary because the setup method calls pure
         * virtual functions at specificly ordered points in the method.  If this was called
         * from the base constructor as originally intended, the application asserts because
         * at the time the base constructor is called, the derived class instance does not
         * exist yet.
         *
         */
        void setup(int argc, char* argv[]);
        //throw(TA_Base_Core::GenericAgentException, TA_Base_Core::DatabaseException, TA_Base_Core::DataException);

        /**
         * cleanup
         *
         * This method must be called by the derived classes before the base class destructor
         * is called.  It is best to call this method in the destructor of the derived class.
         * This design is enforced by an assertion in the base class destructor.
         *
         * Unfortunately, this messiness was necessary due to the same reasons described in
         * setup().  If this was called from the base destructor as originally intended, the
         * application asserts because by the time the base destructor is called, the derived
         * class instance would have been destroyed already.
         *
         */
        void cleanup();

        void processPAAgentStateUpdateRequest();
        void processPAAgentStateUpdate(const TA_Base_Core::StateUpdateMessageCorbaDef& message);
        void processPAAgentEntireAgentStateUpdate(const TA_Base_Core::StateUpdateMessageCorbaDef& message);
        void processPAAgentNewBroadcastUpdate(const TA_Base_Core::StateUpdateMessageCorbaDef& message);
        void processPAAgentRemovedBroadcastUpdate(const TA_Base_Core::StateUpdateMessageCorbaDef& message);
        void processPAAgentRenamedBroadcastUpdate(const TA_Base_Core::StateUpdateMessageCorbaDef& message);
        void processPAAgentBroadcastParameterUpdate(const TA_Base_Core::StateUpdateMessageCorbaDef& message);
        void processPAAgentBroadcastProgressUpdate(const TA_Base_Core::StateUpdateMessageCorbaDef& message);

        void processConfiguration();
        //throw(TA_Base_Core::InvalidPasConfigurationException, TA_Base_Core::DatabaseException, TA_Base_Core::DataException);
        virtual void processSpecificConfiguration() = 0;
        //throw(TA_Base_Core::InvalidPasConfigurationException, TA_Base_Core::DatabaseException, TA_Base_Core::DataException) = 0;

        virtual void initialiseAndValidateCachedMaps() = 0;
        //throw(TA_Base_Core::InvalidPasConfigurationException, TA_Base_Core::CachedMappingNotFoundException, TA_Base_Core::DatabaseException, TA_Base_Core::DataException) = 0;

        virtual void createSpecificComponents() = 0;
        //throw(TA_Base_Core::GenericAgentException, TA_Base_Core::InvalidPasConfigurationException, TA_Base_Core::CachedMappingNotFoundException) = 0;
        virtual void destroySpecificComponents() = 0;
        virtual void startSpecificComponents() = 0;
        virtual void stopSpecificComponents() = 0;

        /**
         * registerForControlMessages
         *
         * This method registers for the messages we expect to receive while in
         * control mode.
         */
        void registerForControlMessages();

        /**
         * registerForMonitorMessages
         *
         * This method registers for the messages we expect to receive while in
         * monitor mode.
         */
        void registerForMonitorMessages();

        /**
         * receiveRequestForStateUpdateMsg
         *
         * This method is called we receive a request for
         * state information (this should only happen if we are in Control mode)
         */
        void receiveRequestForStateUpdateMsg();

        void updatePAAgentConfiguration(const TA_Base_Core::ConfigUpdateDetails& updateEvent);
        void updateDataPointConfiguration(const TA_Base_Core::ConfigUpdateDetails& updateEvent);

        void unsubscribeToMessages();//limin, failover

        // Disable copy constructor and assignment operator
        AbstractPAAgent(const AbstractPAAgent& theAgent);
        AbstractPAAgent& operator=(const AbstractPAAgent&);

    protected:

        // Attributes
        TA_Base_Core::NonReEntrantThreadLockable m_lock;
        TA_Base_Core::NonReEntrantThreadLockable m_lockForOperationModes;
        bool                                m_setupHasBeenCalled;
        bool                                m_cleanupHasBeenCalled;
        TA_Base_Bus::GenericAgent* m_genericAgent;
        TA_Base_Core::EOperationMode             m_operationMode;
        volatile bool                       m_hasReceivedAndProcessedEntireAgentStateUpdate;

        TA_Base_Core::AuditMessageSender* m_auditSender;
        TA_Base_Core::StateUpdateMessageSender* m_stateUpdateRequestSender;
        TA_Base_Core::StateUpdateMessageSender* m_stateUpdateSender;
        DataPoints                          m_dataPoints;
        PasTableManager* m_pasTableManager;
        BroadcastManager* m_broadcastManager;
        DataPointWriteQueueProcessor* m_dataPointWriteQueueProcessor;

        // Deliberately created as a member because it is needed before generic agent is created.
        TA_Base_Bus::DataPointFactory* m_dataPointFactory;
        TA_Base_Core::IEntityDataPtr m_entityData;

        TA_Base_Bus::ScadaProxyFactory* m_scadaProxyFactory;
        TA_Base_Bus::SummaryProcessor* m_summaryProcessor;
        TA_Base_Bus::ScadaPersistencePtr m_persistence;

        TA_Base_Bus::DataPointAgentCorbaDef_Impl* m_dpBulkPolledInterface;
        TA_Base_Bus::DataPointAccessCorbaDef_Impl* m_dpAccessInterface;
        TA_Base_Bus::DataNodeAgentCorbaDef_Impl* m_dnBulkPolledInterface;
        TA_Base_Bus::DataNodeAccessCorbaDef_Impl* m_dnAccessInterface;

        TA_Base_Core::ReEntrantThreadLockable m_dataNodeListLock;
        TA_Base_Core::ReEntrantThreadLockable m_dataPointListLock;

        DataNodeList m_dataNodes;

        // song toan++
        // bug 459
        FireCountdownAckMonitor* m_fireCountdownAckMonitor;
        // ++song toan
        // bug 459
        //TD17452
        static const std::string MONITORED_PROCESS_NAME_APPEND;
        static const std::string AGENT_NAME_APPEND;
        //TD17452
    };

    using AbstractPAAgentPtr = std::shared_ptr<AbstractPAAgent>;
}
