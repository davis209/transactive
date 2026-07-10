/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/AbstractPAAgent.cpp $
 * @author:  Ripple
 * @version: $Revision: #11 $
 *
 * Last modification: $DateTime: 2025/07/10 16:50:01 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "CachedConfig.h"
#include "AbstractPAAgent.h"
#include "StationPasTableManager.h"
#include "MasterPasTableManager.h"
#include "BroadcastManager.h"
#include "PasHelpers.h"
#include "DataPointWriteQueueProcessor.h"
#include "PASConnection.h"
#include "bus/generic_agent/src/IEntity.h"
#include "bus/generic_agent/src/GenericAgent.h"
#include "bus/scada/datapoint_library/src/DataPoint.h"
#include "bus/scada/datanode_library/src/DataNode.h"
#include "bus/scada/datapoint_library/src/DataPointFactory.h"
#include "bus/scada/proxy_library/src/ScadaProxyFactory.h"
#include "bus/scada/datanode_library/src/SummaryProcessor.h"
#include "bus/scada/DataNodeAgentCorbaDef_Impl/src/DataNodeAgentCorbaDef_Impl.h"
#include "bus/scada/DataNodeAgentCorbaDef_Impl/src/DataNodeAccessCorbaDef_Impl.h"
#include "bus/trains/TrainCorbaProxy/src/TrainAgentProxyFactory.h"
#include "bus/signs_4669/tis_agent_access/src/TISAgentAccessFactory.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "core/exceptions/src/ThrowException.h"
#include "core/exceptions/src/AuthenticationSecurityException.h"
#include "core/exceptions/src/AuthenticationAgentException.h"
#include "core/alarm/src/AlarmHelper.h"
#include "core/alarm/src/AlarmHelperManager.h"
#include "core/message/types/DataPointStateUpdate_MessageTypes.h"
#include "core/message/types/PAAgentAlarms_MessageTypes.h"
#include "core/message/types/PAAgentAudit_MessageTypes.h"
#include "core/message/types/PAAgentStateUpdate_MessageTypes.h"
#include "core/message/types/PAAgentStateUpdateRequest_MessageTypes.h"
#include "core/message/src/MessagePublicationManager.h"
#include "core/message/src/AuditMessageSender.h"
#include "core/message/src/StateUpdateMessageSender.h"
#include "core/message/IDL/src/StateUpdateMessageCorbaDef.h"
#include "core/message/src/MessageSubscriptionManager.h"
#include "core/data_access_interface/entity_access/src/AbstractPAAgentEntityData.h"
#include "core/data_access_interface/entity_access/src/TrainAgentEntityData.h"
#include "core/data_access_interface/entity_access/src/DataPointEntityData.h"
#include "core/data_access_interface/entity_access/src/DataNodeAccessFactory.h"
#include "core/data_access_interface/entity_access/src/EntityAccessFactory.h"
#include "core/data_access_interface/src/LocationAccessFactoryEx.h"
#include "core/data_access_interface/pa_4669/src/PaTrainDvaMessageAccessFactory.h"
#include "core/data_access_interface/pa_4669/src/PaZoneAccessFactory.h"
#include "core/data_access_interface/entity_access/src/STISEntityData.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/RunParams.h"
#include "core/threads/src/Thread.h"
#include <boost/thread.hpp>
#include <time.h>

using TA_Base_Bus::IEntity;
using namespace TA_Base_Core;
using namespace TA_IRS_Bus;

namespace TA_IRS_App
{
    //TD17452
    const std::string AbstractPAAgent::MONITORED_PROCESS_NAME_APPEND = "MonitoredProcess";
    const std::string AbstractPAAgent::AGENT_NAME_APPEND = "Agent";
    //TD17452

    //
    // Constructor
    //
    // ExceptionChecked
    AbstractPAAgent::AbstractPAAgent()
        : m_genericAgent(NULL),
        m_operationMode(TA_Base_Core::NotApplicable),
        m_setupHasBeenCalled(false),
        m_cleanupHasBeenCalled(false),
        m_stateUpdateRequestSender(NULL),
        m_stateUpdateSender(NULL),
        m_pasTableManager(NULL),
        m_broadcastManager(NULL),
        m_dataPointWriteQueueProcessor(NULL),
        m_hasReceivedAndProcessedEntireAgentStateUpdate(false),
        //m_entityData(NULL),
        m_dpBulkPolledInterface(NULL),
        m_dpAccessInterface(NULL),
        m_dnBulkPolledInterface(NULL),
        m_dnAccessInterface(NULL),
        m_scadaProxyFactory(NULL),
        m_summaryProcessor(NULL),
        m_fireCountdownAckMonitor(NULL),
        m_dataPointFactory(NULL)
    {
        LOG_SCOPE("AbstractPAAgent::AbstractPAAgent");
        FUNCTION_ENTRY("Constructor");

        m_dataPoints.clear();
        m_dataNodes.clear();

        // get a handle to the ScadaProxyFactory
        m_scadaProxyFactory = &(TA_Base_Bus::ScadaProxyFactory::getInstance());

        m_dataPointFactory = new TA_Base_Bus::DataPointFactory(this);

        // set the proxies to monitor mode
        m_scadaProxyFactory->setProxiesToMonitorMode();

        FUNCTION_EXIT;
    }

    //
    // Destructor
    //
    // ExceptionChecked
    AbstractPAAgent::~AbstractPAAgent()
    {
        LOG_SCOPE("AbstractPAAgent::~AbstractPAAgent");
        FUNCTION_ENTRY("Destructor");

        TA_ASSERT(m_cleanupHasBeenCalled, "PAAgent::cleanup() must be called from the derived classes");

        // if bulk interface is created
        if (NULL != m_dpBulkPolledInterface)
        {
            delete m_dpBulkPolledInterface;
            m_dpBulkPolledInterface = NULL;
        }

        if (NULL != m_dpAccessInterface)
        {
            delete m_dpAccessInterface;
            m_dpAccessInterface = NULL;
        }

        if (NULL != m_dnBulkPolledInterface)
        {
            delete m_dnBulkPolledInterface;
            m_dnBulkPolledInterface = NULL;
        }

        if (NULL != m_dnAccessInterface)
        {
            delete m_dnAccessInterface;
            m_dnAccessInterface = NULL;
        }

        // set the proxies to monitor mode
        m_scadaProxyFactory->setProxiesToMonitorMode();

        // clean up handle to SummaryProcessor
        if (NULL != m_summaryProcessor)
        {
            TA_Base_Bus::SummaryProcessor::removeInstance(m_summaryProcessor);
            m_summaryProcessor = NULL;
        }

        // remove handle to ScadaProxyFactory
        TA_Base_Bus::ScadaProxyFactory::removeInstance();
        m_scadaProxyFactory = NULL;

        FUNCTION_EXIT;
    }

    void AbstractPAAgent::getAllDataPoints(std::map < unsigned long, TA_Base_Bus::DataPoint*>& dataPointList)
    {
        // stop multi threads accessing the list of DataPoint objects at the same time
        TA_Base_Core::ThreadGuard guard(m_dataPointListLock);

        dataPointList.clear();

        std::vector< TA_Base_Bus::DataPoint* >::iterator itr;

        for (itr = m_dataPoints.begin(); itr != m_dataPoints.end(); itr++)
        {
            dataPointList[(*itr)->getEntityKey()] = *itr;
        }
    }

    // ExceptionChecked
    void AbstractPAAgent::setup(int argc, char* argv[])
        //throw(TA_Base_Core::GenericAgentException, TA_Base_Core::DatabaseException, TA_Base_Core::DataException)
    {
        LOG_SCOPE("AbstractPAAgent::setup");
        FUNCTION_ENTRY("setup");

        // This flag forces clients to call this method after base class construction and before
        // anything else.
        m_setupHasBeenCalled = true;

        try
        {
            if (!m_genericAgent)
            {
                m_genericAgent = new TA_Base_Bus::GenericAgent(argc, argv, this);
            }

            if (!m_stateUpdateRequestSender)
            {
                m_stateUpdateRequestSender = MessagePublicationManager::getInstance().getStateUpdateMessageSender(PAAgentStateUpdateRequest::Context);
            }

            if (!m_stateUpdateSender)
            {
                m_stateUpdateSender = MessagePublicationManager::getInstance().getStateUpdateMessageSender(PAAgentStateUpdate::Context);
            }

            TA_IRS_Bus::TrainAgentProxyFactory::getInstance().agentInitialise(m_genericAgent->getAgentEntityConfiguration());

            // song toan++
            // TD 3624
            try
            {
                processConfiguration();
            }
            // TD17559
            catch (TA_Base_Core::DataException& dae)
            {
                LOG_EXCEPTION_CATCH(SourceInfo, "DataException", dae.what());
            }
            catch (TA_Base_Core::DatabaseException& dbe)
            {
                LOG_EXCEPTION_CATCH(SourceInfo, "DatabaseException", dbe.what());
            }
            catch (InvalidPasConfigurationException& e)
            {
                LOG_EXCEPTION_CATCH(SourceInfo, "InvalidPasConfigurationException", e.what());
            }
            catch (...)
            {
                LOG_EXCEPTION_CATCH(SourceInfo, "Unknown", " processConfiguration()-- Exception throw while process the configuration.");
            }

            // TD17559
            // ++song toan
            // TD 3624

            initialiseAndValidateCachedMaps();

            // The instance of the Pas Table Manager is Master/Station specific.
            IEntityDataPtr entityData = m_genericAgent->getAgentEntityConfiguration();

            if (LocationX{entityData->getLocation()}.is_occ_or_bocc())
            {
                m_pasTableManager = new MasterPasTableManager(entityData->getLocation());
            }
            else
            {
                // Station or Depot
                m_pasTableManager = new StationPasTableManager(entityData->getLocation());
            }

            m_broadcastManager = new BroadcastManager(*(m_pasTableManager));

            if (!m_dataPointWriteQueueProcessor)
            {
                m_dataPointWriteQueueProcessor = new DataPointWriteQueueProcessor(m_pasTableManager->getTable570());
            }

            createSpecificComponents();

            // register the local entities into ScadaProxyFactory
            m_scadaProxyFactory->registerLocalEntities(m_entityData->getAgentName(), m_genericAgent->getEntityMap());
            // create SummaryProcessor
            m_summaryProcessor = TA_Base_Bus::SummaryProcessor::getInstance();
            m_summaryProcessor->setEntityMap(m_genericAgent->getEntityMap());
        }
        catch (const InvalidPasConfigurationException& e)
        {
            // The InvalidPasConfigurationException should be raised as a unique alarm.  i.e. only
            // one of these alarms are displayed for each PAAgent entity.  If we raise
            // a second InvalidPasConfigurationException alarm without closing the first one, only
            // the first one will be displayed.
            //
            // On startup, the AbstractPAAgent should clear this alarm.
            //
            // Consider the following scenario:
            // - InvalidPasConfigurationException raised for problem A.
            // - Agent exits.
            // - Operators make no changes.
            // - Agent executed.
            // - Previous alarm closed.
            // - InvalidPasConfigurationException raised for problem A.
            // - Agent exits.
            // - Operators corrects problem A.
            // - Agent executed.
            // - Previous alarm closed.
            // - InvalidPasConfigurationException raised for problem B.
            // - Agent exits.
            // - Operators corrects problem B.
            // - Agent executed.
            // - Previous alarm closed.
            // - Agent runs successfully.

            PasHelpers::instance().raiseInvalidPAConfigurationAlarm(e.what());

            cleanup();
            // prevents run() being called by main
            TA_THROW((GenericAgentException(GenericAgentException::CONFIGURATION_ERROR, e.what())));
        }
        catch (...)
        {
            cleanup();
            throw;
        }

        FUNCTION_EXIT;
    }

    // ExceptionChecked
    void AbstractPAAgent::cleanup()
    {
        LOG_SCOPE("AbstractPAAgent::cleanup");
        FUNCTION_ENTRY("cleanup");

        // This flag forces clients to call this method before base class destruction.
        m_cleanupHasBeenCalled = true;

        // Destruction order is important here...

        destroySpecificComponents();

        // Unsubscribe to all messages (if not already).  Important to do this early before we start
        // deleting all the components (in particular the BroadcastManager).
        MessageSubscriptionManager::getInstance().unsubscribeToMessages(
            dynamic_cast<TA_Base_Core::SpecialisedMessageSubscriber<StateUpdateMessageCorbaDef>*>(this));

        if (m_dataPointWriteQueueProcessor)
        {
            delete m_dataPointWriteQueueProcessor;
            m_dataPointWriteQueueProcessor = NULL;
        }

        if (m_broadcastManager)
        {
            delete m_broadcastManager;
            m_broadcastManager = NULL;
        }

        if (m_pasTableManager)
        {
            delete m_pasTableManager;
            m_pasTableManager = NULL;
        }

        // Deactivate the DataPoints' servant.  The GenericAgent
        // will delete the objects from memory space
        m_dataPoints.clear();

        if (m_stateUpdateSender)
        {
            delete m_stateUpdateSender;
            m_stateUpdateSender = NULL;
        }

        if (m_stateUpdateRequestSender)
        {
            delete m_stateUpdateRequestSender;
            m_stateUpdateRequestSender = NULL;
        }

        // Refer to Wierd Stuff 2 in main.cpp.
        // Remove the singletons here coz some of them can cause access violations if the generic agent
        // instance is removed first.  This is caused by dangling pointers after the event consumers
        // and event suppliers are deleted by the generic agent and subsubsequently referenced by the
        // singleton classes.
        //
        // Classes exhibiting this behavior include CachedMaps (due to the collection of DataPointProxy*)
        // and AlarmHelperManager.

        PASConnection::removeInstance();
        AbstractBroadcastCircularList::removeInstance();
        CachedConfig::removeInstance();
        CachedMaps::removeInstance();
        AlarmHelperManager::getInstance().cleanUp();

        // The generic agent must not be deleted before the audit sender and DataPointProxy* as the
        // generic agent destructor cleans up all the StructuredEventSuppliers created for all message
        // senders.

        if (m_genericAgent)
        {
            m_genericAgent->deactivateAndDeleteServant();
            m_genericAgent = NULL;
        }

        FUNCTION_EXIT;
    }

    // ExceptionChecked
    void AbstractPAAgent::processConfiguration()
        //throw(TA_Base_Core::InvalidPasConfigurationException, TA_Base_Core::DatabaseException, TA_Base_Core::DataException)
    {
        LOG_SCOPE("AbstractPAAgent::processConfiguration");
        FUNCTION_ENTRY("processConfiguration");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");
        TA_ASSERT(m_genericAgent, "m_genericAgent is NULL");

        m_entityData = m_genericAgent->getAgentEntityConfiguration();

        if (NULL == m_entityData.get())
        {
            TA_THROW(InvalidPasConfigurationException("Unrecognised EntityData"));
        }

        //TD17452
        std::string agentName = m_entityData->getName();

        int pos = agentName.find_last_of(AGENT_NAME_APPEND);

        if (pos != -1)
        {
            int stringLength = pos + 1 - AGENT_NAME_APPEND.length();
            agentName = agentName.substr(0, stringLength);
        }

        std::string monitoredProcessName = agentName + MONITORED_PROCESS_NAME_APPEND;

        try
        {
            TA_Base_Core::IEntityData* entityData = EntityAccessFactory::getInstance().getEntity(monitoredProcessName);

            if (entityData != NULL)
            {
                CachedConfig::getInstance()->setMonitorProcessKey(entityData->getKey());
            }

            delete entityData;
            entityData = NULL;
        }
        catch (...)
        {
            LOG_INFO("No data found for monitor process %s", monitoredProcessName);
        }

        //TD17452

        CachedConfig::getInstance()->setAgentKey(m_entityData->getKey());

        // liuyu++ to closeInvalidPAConfigurationAlarm need to set agent location key first

        CachedConfig::getInstance()->setAgentTypeKey(m_entityData->getTypeKey());
        CachedConfig::getInstance()->setAgentLocationKey(m_entityData->getLocation());
        CachedConfig::getInstance()->setAgentSubsystemKey(m_entityData->getSubsystem());

        // We want to close the InvalidPasConfigurationException as soon as we can send another.  Please refer
        // to comments in the Constructor.
        PasHelpers::instance().closeInvalidPAConfigurationAlarm();
        // ++liuyu

        if (!RunParams::getInstance().isSet(RPARAM_LOCATIONKEY))
        {
            // Assumed to be set by Process Management.  Just in case, we set this for rights libraries
            // to prevent asserts.
            char locationKey[40] = {0};
            sprintf(locationKey, "%ld", m_entityData->getLocation());
            RunParams::getInstance().set(RPARAM_LOCATIONKEY, locationKey);
        }

        AbstractPAAgentEntityDataPtr commonPAAgentEntityData = boost::dynamic_pointer_cast<AbstractPAAgentEntityData>(m_genericAgent->getAgentEntityConfiguration());

        if (NULL == commonPAAgentEntityData.get())
        {
            TA_THROW(TA_Base_Core::InvalidPasConfigurationException("Entity is not configured as a PAAgent entity type"));
        }

        CachedConfig::getInstance()->setAgentAssetName(commonPAAgentEntityData->getAssetName());
        CachedConfig::getInstance()->setFastPollRateInMSecs(commonPAAgentEntityData->getFastPollRateInMSecs());
        CachedConfig::getInstance()->setWorkerThreadsSleepTimeInMSecs(commonPAAgentEntityData->getWorkerThreadsSleepTimeInMSecs());
        CachedConfig::getInstance()->setSocketTimeoutInSecs(commonPAAgentEntityData->getSocketTimeoutInSecs());
        CachedConfig::getInstance()->setTisDisplayDurationInSecs(commonPAAgentEntityData->getTisDisplayDurationInSecs());
        CachedConfig::getInstance()->setKeepTerminatedBroadcastsInSecs(commonPAAgentEntityData->getKeepTerminatedBroadcastsInSecs());
        CachedConfig::getInstance()->setMaxBroadcasts(commonPAAgentEntityData->getMaxBroadcasts());
        CachedConfig::getInstance()->setRetries(commonPAAgentEntityData->getRetries());
        CachedConfig::getInstance()->setPrimaryFEPCAddress(commonPAAgentEntityData->getPrimaryFEPCAddress());
        CachedConfig::getInstance()->setSecondaryFEPCAddress(commonPAAgentEntityData->getSecondaryFEPCAddress());
        CachedConfig::getInstance()->setPrimaryFEPCPort(commonPAAgentEntityData->getPrimaryFEPCPort());
        CachedConfig::getInstance()->setSecondaryFEPCPort(commonPAAgentEntityData->getSecondaryFEPCPort());
        // ++ Bug456
        CachedConfig::getInstance()->setDvaBroadcastTimeoutInMsecs(commonPAAgentEntityData->getDvaBroadcastTimeoutInMsecs());
        // ++ Bug456
        CachedConfig::getInstance()->setAgentAssetName(commonPAAgentEntityData->getAssetName());

        // Determine special location flags:

        if (LocationX{m_entityData->getLocation()}.is_occ_or_bocc())
        {
            CachedConfig::getInstance()->setIsMaster(true);
            CachedConfig::getInstance()->setIsDepot(false);
            CachedMaps::getInstance()->setOccDepotFlags(true, false);
        }
        else
        {
            // Station or Depot
            CachedConfig::getInstance()->setIsMaster(false);

            if (LocationX{m_entityData->getLocation()}.is_depot())
            {
                // Depot
                CachedConfig::getInstance()->setIsDepot(true);
                CachedMaps::getInstance()->setOccDepotFlags(false, true);
            }
            else
            {
                // Standard station
                CachedConfig::getInstance()->setIsDepot(false);
                CachedMaps::getInstance()->setOccDepotFlags(false, false);
            }
        }

        // Note the following can raise the TA_Base_Core::DatabaseException or TA_Base_Core::DataException.

        IEntityDataList allEntities;
        IEntityDataList::iterator it;

        // Locate the train agent key for subscribing to messages

        allEntities = EntityAccessFactory::getInstance().getEntitiesOfTypeAtLocation(TrainAgentEntityData::getStaticType(), m_entityData->getLocation());

        if (allEntities.size() != 1)
        {
            PasHelpers::instance().submitPaAgentDependencyFailureAuditMessage("Train agent", "Train broadcasts not possible");
        }
        else
        {
            CachedConfig::getInstance()->setLocalTrainAgentKey(allEntities[0]->getKey());
            CachedConfig::getInstance()->setLocalTrainAgentEntityName(allEntities[0]->getName());
        }

        for (it = allEntities.begin(); it != allEntities.end(); ++it)
        {
            delete* it;
        }

        allEntities.clear();

        // Locate the TIS agent key for the CORBA name

        try
        {
            // force the access factory to build an agent reference
            TA_Base_Bus::TISAgentAccessFactory::getInstance().getSTISAgent();
        }
        catch (DatabaseException&)
        {
            PasHelpers::instance().submitPaAgentDependencyFailureAuditMessage("TIS agent", "TIS requests not possible.");
        }
        catch (DataException&)
        {
            PasHelpers::instance().submitPaAgentDependencyFailureAuditMessage("TIS agent", "TIS requests not possible.");
        }

        // Setup cached datapoints

        // Verify expected datapoint names

        DataPoints datapoints = getDataPointsByToken(commonPAAgentEntityData->getMyMajorSummaryAlarmToken());

        if (datapoints.size() != 1)
        {
            std::ostringstream msg;
            msg << "Failed to locate unique major summary alarm child entity datapoint ("
                << commonPAAgentEntityData->getMyMajorSummaryAlarmToken() << ")";
            TA_THROW(InvalidPasConfigurationException(msg.str().c_str()));
        }
        else
        {
            CachedConfig::getInstance()->setMajorSummaryAlarmDP(datapoints.front());
        }

        datapoints = getDataPointsByToken(commonPAAgentEntityData->getMyMinorSummaryAlarmToken());

        if (datapoints.size() != 1)
        {
            std::ostringstream msg;
            msg << "Failed to locate unique minor summary alarm child entity datapoint ("
                << commonPAAgentEntityData->getMyMinorSummaryAlarmToken() << ")";
            TA_THROW(InvalidPasConfigurationException(msg.str().c_str()));
        }
        else
        {
            CachedConfig::getInstance()->setMinorSummaryAlarmDP(datapoints.front());
        }

        // TD15174
        // OCC and DEPOT have not FireCountdown datapoints
        if (LocationX{m_entityData->getLocation()}.is_station_or_depot())
        {
            datapoints = getDataPointsByToken(commonPAAgentEntityData->getMyFireCountdownActiveToken());

            if (datapoints.size() != 1)
            {
                std::ostringstream msg;
                msg << "Failed to locate unique fire countdown active child entity datapoint ("
                    << commonPAAgentEntityData->getMyFireCountdownActiveToken() << ")";
                TA_THROW(InvalidPasConfigurationException(msg.str().c_str()));
            }
            else
            {
                CachedConfig::getInstance()->setFireCountdownActiveDP(datapoints.front());
            }

            datapoints = getDataPointsByToken(commonPAAgentEntityData->getMyFireCountdownActiveOutputToken());

            if (datapoints.size() != 1)
            {
                std::ostringstream msg;
                msg << "Failed to locate unique fire countdown active output child entity datapoint ("
                    << commonPAAgentEntityData->getMyFireCountdownActiveOutputToken() << ")";
                TA_THROW(InvalidPasConfigurationException(msg.str().c_str()));
            }
            else
            {
                CachedConfig::getInstance()->setFireCountdownActiveOutputDP(datapoints.front());
            }
        }

        // TD15174

        // load the paging console Id's
        try
        {
            std::list<unsigned long> pagingConsoleZones = TA_Base_Core::PaZoneAccessFactory::getInstance().getPagingConsolePaZoneKeys();

            CachedConfig::getInstance()->setPagingConsoleKeys(pagingConsoleZones);
        }
        catch (TA_Base_Core::DatabaseException& dbe)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "DatabaseException", dbe.what());

            // set an empty list - no side effect except audit messages will be logged
            // for the local playback of messages to paging consoles
            std::list<unsigned long> pagingConsoleZones;
            CachedConfig::getInstance()->setPagingConsoleKeys(pagingConsoleZones);
        }

        processSpecificConfiguration();

        FUNCTION_EXIT;
    }

    // ExceptionChecked
    DataPoints AbstractPAAgent::getDataPointsByToken(const std::string& token)
    {
        FUNCTION_ENTRY("getPaAgentDataPointsByToken");

        DataPoints matches;
        // Search for string matches
        DataPointsIt it = m_dataPoints.begin();

        for (; it != m_dataPoints.end(); ++it)
        {
            std::string name = (*it)->getDataPointName();

            if (name.find(token) != std::string::npos)
            {
                matches.push_back(*it);
            }
        }

        FUNCTION_EXIT;
        return matches;
    }

    bool AbstractPAAgent::createAllEntities(TA_Base_Core::IEntityDataPtr agentEntityData,
                                            const TA_Base_Core::SharedIEntityDataList& entityDataList,
                                            TA_Base_Bus::IEntityList& createdEntities)
    {
        LOG_SCOPE("AbstractPAAgent::createAllEntities");
        bool createdDnSpecificUtils = false;

        for (TA_Base_Core::SharedIEntityDataList::const_iterator iter = entityDataList.begin();
             iter != entityDataList.end(); ++iter)
        {
            // only datanodes need these things, so if there are no datanodes, why create them?
            if ((false == createdDnSpecificUtils) &&
                ((*iter)->getType() == TA_Base_Core::DataNodeEntityData::getStaticType()))
            {
                // create an instance of DataNodeAccessFactory if not done already
                LOG_INFO("Start of create DataNode Hierarchy Map");

                TA_Base_Core::DataNodeAccessFactory::getInstance()->createDataNodeHierarchyMap(agentEntityData->getKey(), entityDataList);

                // initialise ScadaPersistence
                m_persistence = TA_Base_Bus::ScadaPersistencePtr(TA_Base_Bus::ScadaPersistence::createScadaPersistenceForAgent(TA_Base_Bus::ScadaPersistence::DATANODE, agentEntityData->getKey()));

                createdDnSpecificUtils = true;
            }

            createdEntities.push_back(TA_Base_Bus::IEntityList::value_type((*iter)->getKey(), createEntity(*iter)));
        }

        // return true to indicate not to call createEntity
        return true;
    }

    void AbstractPAAgent::entityCreationComplete()
    {
        // loading complete, free some memory
        TA_Base_Core::DataNodeAccessFactory::getInstance()->destroyDataNodeHierarchyMap();
    }

    //
    // createEntity
    //
    // ExceptionChecked
    IEntity* AbstractPAAgent::createEntity(TA_Base_Core::IEntityDataPtr EntityData)
        //throw(TA_Base_Core::InvalidPasConfigurationException)
    {
        LOG_SCOPE(boost::format("AbstractPAAgent::createEntity[%s]") % EntityData->getName());
        FUNCTION_ENTRY("createEntity");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");

        //
        // Note: the objects of interface IEntityData and IEntity are not owned
        // by this specific agent, but by the GenericAgent.  As such no object
        // clean up shall be done with regards to these objects.
        //

        IEntity* theEntity = 0;
        std::string entityDataType = EntityData->getType();

        // if the specified entity is of the type DataPoint
        if (TA_Base_Core::DataPointEntityData::getStaticType() == entityDataType)
        {
            // create and return the DataPoint entity
            FUNCTION_EXIT;
            return createAndInitialiseDataPointEntity(EntityData);
        }
        else if (entityDataType == TA_Base_Core::DataNodeEntityData::getStaticType())
        {
            TA_Base_Bus::DataNode* dn = NULL;

            try
            {
                dn = new TA_Base_Bus::DataNode(boost::dynamic_pointer_cast<TA_Base_Core::DataNodeEntityData>(EntityData), m_persistence);

                m_dataNodes.push_back(dn);
                theEntity = dn;
            }
            catch (...)
            {
                LOG_ERROR("Error creating DataNode %s (%ul)", EntityData->getName(), EntityData->getKey());

                // To ensure the generic agent is constructed even if one DataNode creation fails
            }
        }
        // otherwise don't care
        else
        {
            // do nothing
        }

        FUNCTION_EXIT;
        //cl-20955 zhangjunsheng
        //return NULL;
        return theEntity;
    }

    // ExceptionChecked
    IEntity* AbstractPAAgent::createAndInitialiseDataPointEntity(TA_Base_Core::IEntityDataPtr EntityData)
        //throw(TA_Base_Core::InvalidPasConfigurationException)
    {
        FUNCTION_ENTRY("createDataPointEntity");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");

        // cast the EntityData to DataPointEntityData
        TA_Base_Core::DataPointEntityDataPtr dataPointEntityData = boost::dynamic_pointer_cast<TA_Base_Core::DataPointEntityData> (EntityData);

        TA_Base_Bus::DataPoint* theEntity = m_dataPointFactory->createDataPoint(dataPointEntityData);

        if (!theEntity)
        {
            std::ostringstream msg;
            msg << "Child entity datapoint " << dataPointEntityData->getName()
                << " has not configured correctly and cannot be created";
            TA_THROW(TA_Base_Core::InvalidPasConfigurationException(msg.str().c_str()));
        }

        m_dataPoints.push_back(theEntity);

        // Initialise the datapoint to an initial invalid state:
        TA_Base_Bus::DpValue dpValue = theEntity->getCurrentValue();

        TA_Base_Bus::DataPointState newState(dpValue, time(NULL), TA_Base_Bus::QUALITY_BAD_NOT_CONNECTED);
        theEntity->updateFieldState(newState);

        FUNCTION_EXIT;
        return theEntity;
    }

    //
    // agentTerminate
    //
    // ExceptionChecked
    void AbstractPAAgent::agentTerminate()
    {
        LOG_SCOPE("AbstractPAAgent::agentTerminate");
        FUNCTION_ENTRY("agentTerminate");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");
        TA_ASSERT(m_dataPointWriteQueueProcessor, "m_dataPointWriteQueueProcessor is NULL");

        stopSpecificComponents();

        // Wake up 'Blocking' reads
        PASConnection::getInstance()->setTerminate(true);

        if (m_dataPointWriteQueueProcessor->getCurrentState() == Thread::THREAD_STATE_RUNNING)
        {
            m_dataPointWriteQueueProcessor->terminateAndWait();
        }

        m_broadcastManager->disable();

        // if bulk interface is created
        if (NULL != m_dpBulkPolledInterface)
        {
            delete m_dpBulkPolledInterface;
            m_dpBulkPolledInterface = NULL;
        }

        if (NULL != m_dpAccessInterface)
        {
            delete m_dpAccessInterface;
            m_dpAccessInterface = NULL;
        }

        if (NULL != m_dnBulkPolledInterface)
        {
            delete m_dnBulkPolledInterface;
            m_dnBulkPolledInterface = NULL;
        }

        if (NULL != m_dnAccessInterface)
        {
            delete m_dnAccessInterface;
            m_dnAccessInterface = NULL;
        }

        FUNCTION_EXIT;
    }

    //
    // agentSetMonitor
    //
    // ExceptionChecked
    void AbstractPAAgent::agentSetMonitor()
    {
        LOG_SCOPE("AbstractPAAgent::agentSetMonitor");
        FUNCTION_ENTRY("agentSetMonitor");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");
        TA_ASSERT(m_dataPointWriteQueueProcessor, "m_dataPointWriteQueueProcessor is NULL");

        ThreadGuard guard(m_lockForOperationModes);

        if (m_operationMode == TA_Base_Core::Monitor)
        {
            return;
        }

        m_operationMode = TA_Base_Core::Monitor;

        agentSetMonitorSpecifics();

        if (m_dataPointWriteQueueProcessor->getCurrentState() == Thread::THREAD_STATE_RUNNING)
        {
            m_dataPointWriteQueueProcessor->terminateAndWait();
        }

        {
            LOG_SCOPE("DataPoint");

            // For each of the data points created for this agent, set the data point to monitor mode
            DataPointsIt it = m_dataPoints.begin();

            for (; it != m_dataPoints.end(); ++it)
            {
                LOG_SCOPE((*it)->getDataPointName());
                TA_ASSERT(*it, "DataPoint* in m_dataPoints is NULL");
                (*it)->setToMonitorMode();
            }
        }

        boost::async([this]
        {
            registerForMonitorMessages();
        });

        m_pasTableManager->setMonitor();
        m_broadcastManager->disable();

        // if bulk interface is set up
        if (NULL != m_dpBulkPolledInterface)
        {
            // stop receiving remote requests
            LOG_SCOPE("DataPointAgentCorbaDef_Impl");
            m_dpBulkPolledInterface->setToMonitor();
        }

        if (NULL != m_dpAccessInterface)
        {
            // stop receiving remote requests
            LOG_SCOPE("DataPointAccessCorbaDef_Impl");
            m_dpAccessInterface->setToMonitor();
        }

        if (NULL != m_dnBulkPolledInterface)
        {
            // stop receiving remote requests
            LOG_SCOPE("DataNodeAgentCorbaDef_Impl");
            m_dnBulkPolledInterface->setToMonitor();
        }

        if (NULL != m_dnAccessInterface)
        {
            // stop receiving remote requests
            LOG_SCOPE("DataNodeAccessCorbaDef_Impl");
            m_dnAccessInterface->setToMonitor();
        }

        {
            LOG_SCOPE("DataNode");

            // set all datanodes to monitor mode
            for (DataNodeList::iterator itr = m_dataNodes.begin(); itr != m_dataNodes.end(); ++itr)
            {
                LOG_SCOPE((*itr)->getName());
                (*itr)->setOperationMode(false);
            }
        }

        // set the proxies to monitor mode
        m_scadaProxyFactory->setProxiesToMonitorMode();

        FUNCTION_EXIT;
    }

    //
    // agentSetControl
    //
    // ExceptionChecked
    void AbstractPAAgent::agentSetControl()
    {
        LOG_SCOPE("AbstractPAAgent::agentSetControl");
        FUNCTION_ENTRY("agentSetControl");
        TA_ASSERT(m_dataPointWriteQueueProcessor, "m_dataPointWriteQueueProcessor is NULL");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");

        ThreadGuard guard(m_lockForOperationModes);

        if (m_operationMode == TA_Base_Core::Control)
        {
            return;
        }

        m_operationMode = TA_Base_Core::Control;

        PasTableReadCounter::instance().reset();

        m_broadcastManager->setModeChangingFlag(true);

        m_pasTableManager->setControl();
        m_broadcastManager->enable();

        boost::async([this]
        {
            registerForControlMessages();
        });

        {
            LOG_SCOPE("DataNode");

            // set all datanodes to control mode
            for (DataNodeList::reverse_iterator itr = m_dataNodes.rbegin(); itr != m_dataNodes.rend(); ++itr)
            {
                LOG_SCOPE((*itr)->getName());
                (*itr)->setOperationMode(true);
            }
        }

        // set the proxies to control mode
        m_scadaProxyFactory->setProxiesToControlMode();

        {
            LOG_SCOPE("DataPoint");

            // For each of the data points created for this agent, set the data point to control mode
            DataPointsIt it = m_dataPoints.begin();

            for (; it != m_dataPoints.end(); ++it)
            {
                LOG_SCOPE((*it)->getDataPointName());
                TA_ASSERT(*it, "DataPoint* in m_dataPoints is NULL");
                (*it)->setToControlMode();
            }
        }

        if (m_dataPointWriteQueueProcessor->getCurrentState() != Thread::THREAD_STATE_RUNNING)
        {
            m_dataPointWriteQueueProcessor->start();
        }

        agentSetControlSpecifics();

        // if bulk interface is set up
        if (NULL != m_dpBulkPolledInterface)
        {
            // start receiving remote requests
            LOG_SCOPE("DataPointAgentCorbaDef_Impl");
            m_dpBulkPolledInterface->setToControl();
        }

        if (NULL != m_dpAccessInterface)
        {
            // start receiving remote requests
            LOG_SCOPE("DataPointAccessCorbaDef_Impl");
            m_dpAccessInterface->setToControl();
        }

        // if bulk interface is set up
        if (NULL != m_dnBulkPolledInterface)
        {
            // start receiving remote requests
            LOG_SCOPE("DataNodeAgentCorbaDef_Impl");
            m_dnBulkPolledInterface->setToControl();
        }

        if (NULL != m_dnAccessInterface)
        {
            // start receiving remote requests
            LOG_SCOPE("DataNodeAccessCorbaDef_Impl");
            m_dnAccessInterface->setToControl();
        }

#if 0
        m_broadcastManager->setModeChangingFlag(false);
#else
        boost::async([this]
        {
            if (AbstractBroadcastCircularList::getInstance()->getSize())
            {
                PasTableReadCounter::instance().wait_for_connection_complete();
            }

            m_broadcastManager->setModeChangingFlag(false);
        });
#endif

        FUNCTION_EXIT;
    }

    //
    // notifyGroupOffline
    //
    void AbstractPAAgent::notifyGroupOffline(const std::string& group)
    {
        FUNCTION_ENTRY("notifyGroupOffline");

        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");
        // TODO: Degraded mode operations

        FUNCTION_EXIT;
    }

    //
    // notifyGroupOnline
    //
    void AbstractPAAgent::notifyGroupOnline(const std::string& group)
    {
        FUNCTION_ENTRY("notifyGroupOnline");

        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");
        // TODO: Degraded mode operations

        FUNCTION_EXIT;
    }

    //
    // registerForControlMessages
    //
    void AbstractPAAgent::registerForControlMessages()
    {
        LOG_SCOPE("AbstractPAAgent::registerForControlMessages");
        FUNCTION_ENTRY("registerForControlMessages");

        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");

        // Unsubscribe if already subscribed...
        //limin--, failover issue
        //MessageSubscriptionManager::getInstance().unsubscribeToMessages(
        //        dynamic_cast<TA_Base_Core::SpecialisedMessageSubscriber<CommsMessageCorbaDef>*>(this));
        //--limin

        MessageSubscriptionManager::getInstance().unsubscribeToMessages(
            dynamic_cast<TA_Base_Core::SpecialisedMessageSubscriber<StateUpdateMessageCorbaDef>*>(this));

        // Subscribe to receive the PAAgentStateUpdateRequest messages
        // This is received when the monitor instance starts up

        MessageSubscriptionManager::getInstance().subscribeToStateUpdateMessage(
            PAAgentStateUpdateRequest::PAAgentStateUpdateRequest,
            dynamic_cast<SpecialisedMessageSubscriber<StateUpdateMessageCorbaDef>*>(this),
            CachedConfig::getInstance()->getAgentName());

        FUNCTION_EXIT;
    }

    //
    // registerForMonitorMessages
    //
    void AbstractPAAgent::registerForMonitorMessages()
    {
        LOG_SCOPE("AbstractPAAgent::registerForMonitorMessages");
        FUNCTION_ENTRY("registerForMonitorMessages");

        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");

        // Unsubscribe if already subscribed...
        //limin--, failover issue
        //MessageSubscriptionManager::getInstance().unsubscribeToMessages(
        //        dynamic_cast<TA_Base_Core::SpecialisedMessageSubscriber<CommsMessageCorbaDef>*>(this));
        //--limin

        MessageSubscriptionManager::getInstance().unsubscribeToMessages(
            dynamic_cast<TA_Base_Core::SpecialisedMessageSubscriber<StateUpdateMessageCorbaDef>*>(this));

        // Subscribe to receive the PAAgentStateUpdate messages
        // This is received in response to a sending of an PAAgentStateUpdateRequest message

        MessageSubscriptionManager::getInstance().subscribeToStateUpdateMessage(
            PAAgentStateUpdate::Context,
            dynamic_cast<SpecialisedMessageSubscriber<StateUpdateMessageCorbaDef>*>(this),
            CachedConfig::getInstance()->getAgentName());

        FUNCTION_EXIT;
    }

    //
    // registerForStateUpdates
    //
    void AbstractPAAgent::registerForStateUpdates()
    {
        FUNCTION_ENTRY("registerForStateUpdates");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");

        if (0 == RunParams::getInstance().get(RPARAM_OPERATIONMODE).compare(RPARAM_CONTROL))
        {
            boost::async([this]
            {
                registerForControlMessages();
            });
        }
        else
        {
            boost::async([this]
            {
                registerForMonitorMessages();
            });
        }

        FUNCTION_EXIT;
    }

    //
    // processOnlineUpdate
    //
    // ExceptionChecked
    void AbstractPAAgent::processOnlineUpdate(const ConfigUpdateDetails& updateEvent)
    {
        FUNCTION_ENTRY("processOnlineUpdate");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");

        unsigned long updateEventPKey = updateEvent.getKey();

        if (CachedConfig::getInstance()->getAgentKey() == updateEventPKey)
        {
            // update PA Agent configuration
            // Can only be an Update
            updatePAAgentConfiguration(updateEvent);

            return;
        }
        else
        {
            // Must be for the child entities (datapoints)
            // Can be an Update / Create / Delete modification.
            updateDataPointConfiguration(updateEvent);
        }

        FUNCTION_EXIT;
    }

    // ExceptionChecked
    void AbstractPAAgent::updateDataPointConfiguration(const TA_Base_Core::ConfigUpdateDetails& updateEvent)
    {
        // No audit messages are raised from here - If a datapoint is being updated then it will
        // raise its own audit message.

        FUNCTION_ENTRY("updateDataPointConfiguration");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");

        try
        {
            TA_Base_Bus::DataPoint* theDataPoint = 0;
            unsigned long updateEventPKey = updateEvent.getKey();

            DataPointsIt it = m_dataPoints.begin();

            for (; it != m_dataPoints.end(); ++it)
            {
                TA_ASSERT(*it, "DataPoint* in m_dataPoints is NULL");

                if ((*it)->getEntityKey() == updateEventPKey)
                {
                    theDataPoint = (*it);
                    break;
                }
            }

            if (theDataPoint)
            {
                // get the type of updating event
                switch (updateEvent.getModifications())
                {
                    case TA_Base_Core::Update:
                    {
                        // pass the config update to the data point for updating
                        theDataPoint->update(updateEvent);

                        // exit switch statement
                        break;
                    }

                    case TA_Base_Core::Delete:
                    case TA_Base_Core::Create:
                    {
                        LOG_ERROR("Received online update to create/delete a datapoint %s. The online creation/deletion of datapoints is not supported",
                                  theDataPoint->getDataPointName());
                    }
                }
            }
            else
            {
                LOG_ERROR("Online update for unrecognised child entity pKey=%d ignored", updateEvent.getKey());
            }
        } // try block
        catch (TA_Base_Core::DataException)
        {
            // This means that the new configuration cannot be interpreted properly from
            // entity data.  These values will be ignored for the life of the application.

            LOG_ERROR("Online update of datapoint pKey=%d failed - invalid configuration", updateEvent.getKey());
        }
        catch (TA_Base_Core::DatabaseException&)
        {
            LOG_ERROR("Online update of datapoint pKey=%d failed - loss of database", updateEvent.getKey());
        }
        catch (...)
        {
            LOG_ERROR("Online update of datapoint pKey=%d failed", updateEvent.getKey());
        }

        FUNCTION_EXIT;
    }

    // ExceptionChecked
    void AbstractPAAgent::updatePAAgentConfiguration(const TA_Base_Core::ConfigUpdateDetails& updateEvent)
    {
        FUNCTION_ENTRY("updatePAAgentConfiguration");

        if (updateEvent.getModifications() != TA_Base_Core::Update)
        {
            LOG_WARN("Unexpected Delete/Create modification ignored");
            return;
        }

        // get the list of changed parameter from the config update
        const std::vector< std::string > changes = updateEvent.getChangedParams();
        TA_ASSERT(changes.size() > 0, "Empty changes should have been prevented by GenericAgent");

        // get configuration data for this PA Agent
        AbstractPAAgentEntityDataPtr entityData = boost::dynamic_pointer_cast<AbstractPAAgentEntityData>
            (m_genericAgent->getAgentEntityConfiguration());

        TA_ASSERT(entityData.get(), "Unexpected change in entityData class since initialisation");

        // force a reread of the configuration
        entityData->invalidate();

        // Note that all parameters are either accessed directly out of CachedConfig
        // or automatically notify ICachedConfigObservers to take up the new values.
        // Therefore there is no need to explicitly call classes throughout the PA
        // Agent to reinitialise states.

        for (unsigned int i = 0; i < changes.size(); i++)
        {
            try
            {
                if (changes[i] == AbstractPAAgentEntityData::FAST_POLL_RATE)
                {
                    CachedConfig::getInstance()->setFastPollRateInMSecs(entityData->getFastPollRateInMSecs());
                    PasHelpers::instance().submitPAAgentOnlineUpdateAuditMessage("FastPollRateInMSecs");
                }
                else if (changes[i] == AbstractPAAgentEntityData::WORKER_THREADS_SLEEP_TIME)
                {
                    CachedConfig::getInstance()->setWorkerThreadsSleepTimeInMSecs(entityData->getWorkerThreadsSleepTimeInMSecs());
                    PasHelpers::instance().submitPAAgentOnlineUpdateAuditMessage("WorkerThreadsSleepTimeInMSecs");
                }
                else if (changes[i] == AbstractPAAgentEntityData::SOCKET_TIMEOUT)
                {
                    CachedConfig::getInstance()->setSocketTimeoutInSecs(entityData->getSocketTimeoutInSecs());
                    PasHelpers::instance().submitPAAgentOnlineUpdateAuditMessage("SocketTimeoutInSecs");
                }
                else if (changes[i] == AbstractPAAgentEntityData::TIS_DISPLAY_DURATION)
                {
                    CachedConfig::getInstance()->setTisDisplayDurationInSecs(entityData->getTisDisplayDurationInSecs());
                    PasHelpers::instance().submitPAAgentOnlineUpdateAuditMessage("TisDisplayDurationInSecs");
                }
                else if (changes[i] == AbstractPAAgentEntityData::KEEP_TERMINATED_BROADCASTS)
                {
                    CachedConfig::getInstance()->setKeepTerminatedBroadcastsInSecs(entityData->getKeepTerminatedBroadcastsInSecs());
                    PasHelpers::instance().submitPAAgentOnlineUpdateAuditMessage("KeepTerminatedBroadcastsInSecs");
                }
                else if (changes[i] == AbstractPAAgentEntityData::RETRIES)
                {
                    CachedConfig::getInstance()->setRetries(entityData->getRetries());
                    PasHelpers::instance().submitPAAgentOnlineUpdateAuditMessage("Retries");
                }
                else if (changes[i] == AbstractPAAgentEntityData::PRIMARY_FEPC_ADDRESS)
                {
                    CachedConfig::getInstance()->setPrimaryFEPCAddress(entityData->getPrimaryFEPCAddress());
                    PasHelpers::instance().submitPAAgentOnlineUpdateAuditMessage("PrimaryFEPCAddress");
                }
                else if (changes[i] == AbstractPAAgentEntityData::SECONDARY_FEPC_ADDRESS)
                {
                    CachedConfig::getInstance()->setSecondaryFEPCAddress(entityData->getSecondaryFEPCAddress());
                    PasHelpers::instance().submitPAAgentOnlineUpdateAuditMessage("SecondaryFEPCAddress");
                }
                else if (changes[i] == AbstractPAAgentEntityData::PRIMARY_FEPC_PORT)
                {
                    CachedConfig::getInstance()->setPrimaryFEPCPort(entityData->getPrimaryFEPCPort());
                    PasHelpers::instance().submitPAAgentOnlineUpdateAuditMessage("PrimaryFEPCPort");
                }
                else if (changes[i] == AbstractPAAgentEntityData::SECONDARY_FEPC_PORT)
                {
                    CachedConfig::getInstance()->setSecondaryFEPCPort(entityData->getSecondaryFEPCPort());
                    PasHelpers::instance().submitPAAgentOnlineUpdateAuditMessage("SecondaryFEPCPort");
                }
                else if (changes[i] == AbstractPAAgentEntityData::MAX_BROADCASTS)
                {
                    CachedConfig::getInstance()->setMaxBroadcasts(entityData->getMaxBroadcasts());
                    PasHelpers::instance().submitPAAgentOnlineUpdateAuditMessage("MaxBroadcasts");
                }
                else
                {
                    LOG_ERROR("Online update of parameter %s not supported",
                              changes[i]); //TD14337 yezh++
                }
            } // try block
            catch (TA_Base_Core::InvalidPasConfigurationException)
            {
                // This means that the CachedConfig has detected invalid values and has not
                // overwritten them yet.  These values will be ignored for the life of the
                // application.
                LOG_ERROR("Online update of parameter %s failed - invalid configuration",
                          changes[i]); //TD14337 yezh++
            }
            catch (TA_Base_Core::DataException)
            {
                // This means that the new configuration cannot be interpreted properly from
                // entity data.  These values will be ignored for the life of the application.
                LOG_ERROR("Online update of parameter %s failed - invalid configuration",
                          changes[i]); //TD14337 yezh++
            }
            catch (TA_Base_Core::DatabaseException&)
            {
                LOG_ERROR("Online update of parameter %s failed - loss of database",
                          changes[i]); //TD14337 yezh++
            }
            catch (...)
            {
                // Not expecting anything else
                LOG_ERROR("Online update of parameter %s failed",
                          changes[i]); //TD14337 yezh++
            }
        } // for loop

        FUNCTION_EXIT;
    }

    //
    // run
    //
    // ExceptionChecked
    void AbstractPAAgent::run()
    {
        LOG_SCOPE("AbstractPAAgent::run");
        FUNCTION_ENTRY("run");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");
        TA_ASSERT(m_genericAgent, "m_genericAgent is NULL");

        // check the runtime parameter OperationMode
        std::string operationMode("");
        operationMode = RunParams::getInstance().get("OperationMode");

        // if defined as "Control"
        if (RPARAM_CONTROL == operationMode)
        {
            agentSetControl();
            startingInControlMode();
        }
        else if (RPARAM_MONITOR == operationMode)
        {
            agentSetMonitor();
            startingInMonitorMode();
        }

        CachedConfig::getInstance()->setAgentInRuntime(true);

        // create an instance of the class DataPointAgentCorbaDef_Impl
        m_dpBulkPolledInterface = new TA_Base_Bus::DataPointAgentCorbaDef_Impl(m_entityData->getName(), this);

        TA_ASSERT(m_dpBulkPolledInterface != NULL, "can't create dp bulkpoll interface");

        if (operationMode == RPARAM_CONTROL)
        {
            // start receiving remote requests
            m_dpBulkPolledInterface->setToControl();
        }
        else
        {
            // stop receiving remote requests
            m_dpBulkPolledInterface->setToMonitor();
        }

        m_dpAccessInterface = new TA_Base_Bus::DataPointAccessCorbaDef_Impl(m_entityData->getName(), this);

        TA_ASSERT(m_dpAccessInterface != NULL, "can't create bulkpoll interface");

        if (operationMode == RPARAM_CONTROL)
        {
            // start receiving remote requests
            m_dpAccessInterface->setToControl();
        }
        else
        {
            // stop receiving remote requests
            m_dpAccessInterface->setToMonitor();
        }

        m_dnBulkPolledInterface = new TA_Base_Bus::DataNodeAgentCorbaDef_Impl(m_entityData->getName(), this);

        TA_ASSERT(m_dpBulkPolledInterface != NULL, "can't create dn bulkpoll interface");

        if (operationMode == RPARAM_CONTROL)
        {
            // start receiving remote requests
            m_dnBulkPolledInterface->setToControl();
        }
        else
        {
            // stop receiving remote requests
            m_dnBulkPolledInterface->setToMonitor();
        }

        m_dnAccessInterface = new TA_Base_Bus::DataNodeAccessCorbaDef_Impl(m_entityData->getName(), this);

        TA_ASSERT(m_dnAccessInterface != NULL, "can't create dn access interface");

        if (operationMode == RPARAM_CONTROL)
        {
            // start receiving remote requests
            m_dnAccessInterface->setToControl();
        }
        else
        {
            // stop receiving remote requests
            m_dnAccessInterface->setToMonitor();
        }

        m_genericAgent->run();

        FUNCTION_EXIT;
    }

    // ExceptionChecked
    void AbstractPAAgent::killMeAndRaiseAlarm()
    {
        LOG_SCOPE("AbstractPAAgent::killMeAndRaiseAlarm");
        FUNCTION_ENTRY("killMeAndRaiseAlarm");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");
        TA_ASSERT(m_genericAgent, "m_genericAgent is NULL");

        PasHelpers::instance().raiseInvalidPAConfigurationAlarm("PAAgent in runtime operation will be terminated - inconsistent PA configuration");

        m_genericAgent->onTerminate();

        FUNCTION_EXIT;
    }

    void AbstractPAAgent::startingInMonitorMode()
    {
        LOG_SCOPE("AbstractPAAgent::startingInMonitorMode");

        // Trigger the synchronsation of all datapoint states with those in the Control Agent instance
        // if it is already running

        CORBA::Any data;

        m_stateUpdateRequestSender->sendStateUpdateMessage(PAAgentStateUpdateRequest::PAAgentStateUpdateRequest,
                                                           CachedConfig::getInstance()->getAgentKey(),  // Not used
                                                           CachedConfig::getInstance()->getAgentName(),
                                                           data);                                      // Not used

        // Must wait until we have received the response from the Control Agent
        // otherwise we will end up unsynchronised.
        while (!m_hasReceivedAndProcessedEntireAgentStateUpdate)
        {
            // Wait forever.  On the off chance that the Control Agent is unavailable
            // the System Controller will restart this agent.
            Thread::sleep(CachedConfig::getInstance()->getWorkerThreadsSleepTimeInMSecs());

            if (m_operationMode == TA_Base_Core::Control)
            {
                // This is for the scenario where we have started in Monitor Mode and are stuck
                // waiting forever for the entire state update to be processed.  If in the
                // meantime, we have been forced into Control mode, we ensure that we break
                // out of that loop so that we can enter runtime and therefore later terminate.
                // We can assume that theres no point trying to get the state update as the
                // Control agent is most likely to be down.

                break;
            }

            LOG_DEBUG("startingInMonitorMode(): wait for StateUpdateMessageCorbaDef");
        }
    }

    void AbstractPAAgent::startingInControlMode()
    {
        // Reset any alarms
    }

    //
    // receiveSpecialisedMessage
    //
    // ExceptionChecked
    /*limin--, failover issue
    void AbstractPAAgent::receiveSpecialisedMessage(const CommsMessageCorbaDef& message)
    {
         FUNCTION_ENTRY("receiveSpecialisedMessage");

        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");

        try
        {
            // Nothing processed at this point
        }
        catch (...)
        {
            LOG_FATAL("Exception thrown while processing event");
        }

        FUNCTION_EXIT;
    }
    --limin*/

    // ExceptionChecked
    void AbstractPAAgent::receiveSpecialisedMessage(const TA_Base_Core::StateUpdateMessageCorbaDef& message)
    {
        LOG_SCOPE("AbstractPAAgent::receiveSpecialisedMessage");
        FUNCTION_ENTRY("receiveSpecialisedMessage");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");

        if (m_broadcastManager == NULL)
        {
            // BroadcastManager not created yet.  This can happen since the GenericAgent doAgentSetup()
            // subscribes to the messages in the constructor of Generic GUI yet this is done before
            // BroadcastManager, or anything PA Agent related has constructed.
            return;
        }

        std::string messageTypeKey(message.messageTypeKey);

        if (messageTypeKey == PAAgentStateUpdate::PAAgentStateUpdate.getTypeKey())
        {
            processPAAgentStateUpdate(message);
        }
        else if (messageTypeKey == PAAgentStateUpdate::PAAgentEntireAgentStateUpdate.getTypeKey())
        {
            processPAAgentEntireAgentStateUpdate(message);
        }
        else if (messageTypeKey == PAAgentStateUpdate::PAAgentNewBroadcastUpdate.getTypeKey())
        {
            processPAAgentNewBroadcastUpdate(message);
        }
        else if (messageTypeKey == PAAgentStateUpdate::PAAgentRemovedBroadcastUpdate.getTypeKey())
        {
            processPAAgentRemovedBroadcastUpdate(message);
        }
        else if (messageTypeKey == PAAgentStateUpdate::PAAgentRenamedBroadcastUpdate.getTypeKey())
        {
            processPAAgentRenamedBroadcastUpdate(message);
        }
        else if (messageTypeKey == PAAgentStateUpdate::PAAgentBroadcastParameterUpdate.getTypeKey())
        {
            processPAAgentBroadcastParameterUpdate(message);
        }
        else if (messageTypeKey == PAAgentStateUpdate::PAAgentBroadcastProgressUpdate.getTypeKey())
        {
            processPAAgentBroadcastProgressUpdate(message);
        }
        else if (messageTypeKey == PAAgentStateUpdateRequest::PAAgentStateUpdateRequest.getTypeKey())
        {
            processPAAgentStateUpdateRequest();
        }
        else
        {
            LOG_WARN("Unexpected message received");
        }

        FUNCTION_EXIT;
    }

    // Called by another thread - need to ensure threadsafety of the datapoint container
    // Note that we deliberately avoid sending the DataPointStateUpdateMessage because this
    // would affect other clients such as the OPC Bridge.  Instead we send the PA Agent specific
    // message.

    // ExceptionChecked
    void AbstractPAAgent::processPAAgentStateUpdateRequest()
    {
        LOG_SCOPE("AbstractPAAgent::processPAAgentStateUpdateRequest");
        FUNCTION_ENTRY("processPAAgentStateUpdateRequest");

        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");
        TA_ASSERT(m_stateUpdateSender, "m_stateUpdateSender is NULL");
        TA_ASSERT(m_broadcastManager, "m_broadcastManager is NULL");

        try
        {
            //        // Broadcast PAAgentDataPointUpdate messages for each datapoint
            //        DataPointsIt dpIter = m_dataPoints.begin();
            //        for (; dpIter!=m_dataPoints.end(); ++dpIter)
            //        {
            //            TA_ASSERT(*dpIter, "DataPoint* in m_dataPoints is NULL");
            //
            //            TA_Base_Bus::DataPoint* theDataPoint = *dpIter;
            //
            //            TA_Base_Bus::DataPointCorbaDef::ClientDataPointState_var dpStateV = theDataPoint->GetDataPointState();
            //
            //            CORBA::Any data;
            //            data <<= dpStateV;
            //
            //            m_stateUpdateSender->sendStateUpdateMessage(
            //                PAAgentStateUpdate::PAAgentDataPointUpdate,
            //                theDataPoint->GetEntityKey(),
            //                CachedConfig::getInstance()->getAgentName(),
            //                data );
            //
            //        }

            // Broadcast PAAgentEntireAgentStateUpdate messages for all broadcasts
            TA_Base_Bus::IPAAgentCorbaDef::CompleteBroadcastStateSeq* disposableUpdateData =
                m_broadcastManager->getCompleteBroadcastStateSeqCopy();

            CORBA::Any data;
            data <<= disposableUpdateData;

            m_stateUpdateSender->sendStateUpdateMessage(
                PAAgentStateUpdate::PAAgentEntireAgentStateUpdate,
                0,
                CachedConfig::getInstance()->getAgentName(),
                data);
        }
        catch (...)
        {
            LOG_FATAL("Exception thrown while processing event");
        }

        FUNCTION_EXIT;
    }

    //////////////////////////////////////////////////////////
    // The following is taken from the VirtualDatapointAgent
    //////////////////////////////////////////////////////////

    // Called by another thread - need to ensure threadsafety of the datapoint container
    //
    // ExceptionChecked
    void AbstractPAAgent::processPAAgentStateUpdate(const TA_Base_Core::StateUpdateMessageCorbaDef& message)
    {
        FUNCTION_ENTRY("processPAAgentStateUpdateRequest");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");
        TA_ASSERT(m_broadcastManager, "m_broadcastManager is NULL");

        // Depreciated

        FUNCTION_EXIT;
    }

    void AbstractPAAgent::processPAAgentEntireAgentStateUpdate(const TA_Base_Core::StateUpdateMessageCorbaDef& message)
    {
        LOG_SCOPE("AbstractPAAgent::processPAAgentEntireAgentStateUpdate");
        FUNCTION_ENTRY("processPAAgentEntireAgentStateUpdate");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");
        TA_ASSERT(m_broadcastManager, "m_broadcastManager is NULL");

        try
        {
            TA_Base_Bus::IPAAgentCorbaDef::CompleteBroadcastStateSeq* updateData = NULL;

            // if successfull to extract the stateUpdateInfo
            if (0 != (message.stateUpdateInfo >>= updateData))
            {
                m_broadcastManager->processPAAgentEntireAgentStateUpdate(updateData);
            }

            updateData = NULL;
        }
        catch (...)
        {
            LOG_FATAL("Exception thrown while processing state update");
        }

        m_hasReceivedAndProcessedEntireAgentStateUpdate = true;

        FUNCTION_EXIT;
    }

    void AbstractPAAgent::processPAAgentNewBroadcastUpdate(const TA_Base_Core::StateUpdateMessageCorbaDef& message)
    {
        FUNCTION_ENTRY("processPAAgentNewBroadcastUpdate");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");
        TA_ASSERT(m_broadcastManager, "m_broadcastManager is NULL");

        try
        {
            TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* updateData = NULL;

            // if successfull to extract the stateUpdateInfo
            if (0 != (message.stateUpdateInfo >>= updateData))
            {
                m_broadcastManager->processPAAgentNewBroadcastUpdate(updateData);
            }

            updateData = NULL;
        }
        catch (...)
        {
            LOG_FATAL("Exception thrown while processing state update");
        }

        FUNCTION_EXIT;
    }

    void AbstractPAAgent::processPAAgentRemovedBroadcastUpdate(const TA_Base_Core::StateUpdateMessageCorbaDef& message)
    {
        FUNCTION_ENTRY("processPAAgentRemovedBroadcastUpdate");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");
        TA_ASSERT(m_broadcastManager, "m_broadcastManager is NULL");

        try
        {
            const char* updateData = NULL;

            // if successfull to extract the stateUpdateInfo
            if (0 != (message.stateUpdateInfo >>= CORBA::Any::to_string(updateData, 0)))
            {
                m_broadcastManager->processPAAgentRemovedBroadcastUpdate(updateData);
            }

            updateData = NULL;
        }
        catch (...)
        {
            LOG_FATAL("Exception thrown while processing state update");
        }

        FUNCTION_EXIT;
    }

    void AbstractPAAgent::processPAAgentRenamedBroadcastUpdate(const TA_Base_Core::StateUpdateMessageCorbaDef& message)
    {
        FUNCTION_ENTRY("processPAAgentRenamedBroadcastUpdate");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");
        TA_ASSERT(m_broadcastManager, "m_broadcastManager is NULL");

        try
        {
            TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData* updateData = NULL;

            // if successfull to extract the stateUpdateInfo
            if (0 != (message.stateUpdateInfo >>= updateData))
            {
                m_broadcastManager->processPAAgentRenamedBroadcastUpdate(updateData);
            }

            updateData = NULL;
        }
        catch (...)
        {
            LOG_FATAL("Exception thrown while processing state update");
        }

        FUNCTION_EXIT;
    }

    void AbstractPAAgent::processPAAgentBroadcastParameterUpdate(const TA_Base_Core::StateUpdateMessageCorbaDef& message)
    {
        FUNCTION_ENTRY("processPAAgentBroadcastParameterUpdate");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");
        TA_ASSERT(m_broadcastManager, "m_broadcastManager is NULL");

        try
        {
            TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameter* updateData = NULL;

            // if successfull to extract the stateUpdateInfo
            if (0 != (message.stateUpdateInfo >>= updateData))
            {
                m_broadcastManager->processPAAgentBroadcastParameterUpdate(updateData);
            }

            updateData = NULL;
        }
        catch (...)
        {
            LOG_FATAL("Exception thrown while processing state update");
        }

        FUNCTION_EXIT;
    }

    void AbstractPAAgent::processPAAgentBroadcastProgressUpdate(const TA_Base_Core::StateUpdateMessageCorbaDef& message)
    {
        FUNCTION_ENTRY("processPAAgentBroadcastProgressUpdate");
        TA_ASSERT(m_setupHasBeenCalled, "PAAgent::setup() must be called from the derived classes");
        TA_ASSERT(m_broadcastManager, "m_broadcastManager is NULL");

        try
        {
            TA_Base_Bus::IPAAgentCorbaDef::ProgressReport* updateData = NULL;

            // if successfull to extract the stateUpdateInfo
            if (0 != (message.stateUpdateInfo >>= updateData))
            {
                m_broadcastManager->processPAAgentBroadcastProgressUpdate(updateData);
            }

            updateData = NULL;
        }
        catch (...)
        {
            LOG_FATAL("Exception thrown while processing state update");
        }

        FUNCTION_EXIT;
    }

    TA_Base_Bus::DataPoint* AbstractPAAgent::getDataPoint(unsigned long entityKey)
    {
        TA_Base_Bus::DataPoint* theDataPoint = NULL;

        try
        {
            // Find datapoint
            const std::map<unsigned long, TA_Base_Bus::IEntity*>* entityMap = m_genericAgent->getEntityMap();
            std::map<unsigned long, TA_Base_Bus::IEntity*>::const_iterator it = entityMap->find(entityKey);

            if (it != entityMap->end())
            {
                theDataPoint = dynamic_cast<TA_Base_Bus::DataPoint*>((*it).second);
            }
        }
        catch (...)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "Unknown Exception", "getDataPoint() - Exception thrown while retrieving datapoint");
        }

        return theDataPoint;
    }

    TA_Base_Bus::DataPointFactory* AbstractPAAgent::getDataPointFactory()
    {
        TA_ASSERT(m_dataPointFactory != NULL, "m_dataPointFactory is not created successfully.");

        return m_dataPointFactory;
    }

    TA_Base_Bus::DataNode* AbstractPAAgent::getDataNode(unsigned long entityKey)
    {
        // stop multi threads accessing the list of DataNode objects at the same time
        TA_Base_Core::ThreadGuard guard(m_dataNodeListLock);

        TA_Base_Bus::DataNode* dn = NULL;

        const std::map<unsigned long, TA_Base_Bus::IEntity*>* dataNodes = m_genericAgent->getEntityMap();
        std::map<unsigned long, TA_Base_Bus::IEntity*>::const_iterator itr = dataNodes->find(entityKey);

        if (itr != dataNodes->end())
        {
            dn = dynamic_cast<TA_Base_Bus::DataNode*>(itr->second);

            if (NULL == dn)
            {
                LOG_WARN("Could not convert entity %ul to a DataNode", entityKey);
            }
        }

        return dn;
    }

    void AbstractPAAgent::getAllDataNodes(std::map < unsigned long, TA_Base_Bus::DataNode*>& dataNodeList)
    {
        TA_Base_Core::ThreadGuard guard(m_dataNodeListLock);

        dataNodeList.clear();

        for (DataNodeList::iterator itr = m_dataNodes.begin(); itr != m_dataNodes.end(); ++itr)
        {
            dataNodeList[(*itr)->getEntityKey()] = *itr;
        }
    }
}
