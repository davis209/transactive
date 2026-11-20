#include "TISBridgeAgent.h"

#include "bus/generic_agent/src/GenericAgent.h"

#include "core/data_access_interface/entity_access/src/IEntityData.h"
#include "core/exceptions/src/TransactiveException.h"
#include "core/message/src/MessageSubscriptionManager.h"
#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/RunParams.h"

#include <cstdlib>

namespace TA_IRS_App
{
    TISBridgeAgent::TISBridgeAgent(int argc, char* argv[])
        : m_genericAgent(0),
          m_operationMode(TA_Base_Core::NotApplicable),
          m_locationKey(0),
          m_restPort(8089)
    {
        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo, "Creating GenericAgent");
        m_genericAgent = new TA_Base_Bus::GenericAgent(argc, argv, this);

        m_agentName = TA_Base_Core::RunParams::getInstance().get(RPARAM_ENTITYNAME);

        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo, "Loading TISBridgeAgent configuration");
        loadConfiguration();
    }

    TISBridgeAgent::~TISBridgeAgent()
    {
        stopBridgeServices();
        if (m_genericAgent != 0)
        {
            try { m_genericAgent->deactivateAndDeleteServant(); }
            catch (...) {}
            m_genericAgent = 0;
        }
    }

    void TISBridgeAgent::startTISBridgeAgent()
    {
        try
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
                "Starting TISBridgeAgent bridge services");
            startBridgeServices();
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
                "TISBridgeAgent bridge services started");
        }
        catch (...)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "Unknown Exception",
                "Caught exception while starting TISBridgeAgent bridge services");
        }

        if (m_genericAgent != 0) m_genericAgent->run();
    }

    void TISBridgeAgent::loadConfiguration()
    {
        if (m_genericAgent != 0)
        {
            TA_Base_Core::IEntityDataPtr data = m_genericAgent->getAgentEntityConfiguration();
            if (data.get() != 0) m_locationKey = data->getLocation();
        }
        if (m_locationKey == 0) TA_THROW(TA_Base_Core::TransactiveException("Failed to get TISBridgeAgent location Id"));

        m_restPort = static_cast<unsigned short>(getUnsignedRunParam("--rest-port", 8089));
        m_kafkaTopicPrefix = getStringRunParam("--kafka-topic-prefix", "tis");
    }

    unsigned long TISBridgeAgent::getUnsignedRunParam(const std::string& name, unsigned long defaultValue) const
    {
        const std::string value = TA_Base_Core::RunParams::getInstance().get(name.c_str());
        return value.empty() ? defaultValue : static_cast<unsigned long>(std::strtoul(value.c_str(), 0, 10));
    }

    std::string TISBridgeAgent::getStringRunParam(const std::string& name, const std::string& defaultValue) const
    {
        const std::string value = TA_Base_Core::RunParams::getInstance().get(name.c_str());
        return value.empty() ? defaultValue : value;
    }

    TisKafkaProducerPtr TISBridgeAgent::createKafkaProducer() const
    {
        const std::string bootstrapServers = getStringRunParam("--kafka-servers");
#if defined(USE_LIBRDKAFKA)
        if (!bootstrapServers.empty()) return TisKafkaProducerPtr(new TisLibrdkafkaProducer(bootstrapServers));
#else
        if (!bootstrapServers.empty())
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
                "Kafka bootstrap servers configured but TISBridgeAgent was not built with USE_LIBRDKAFKA");
        }
#endif
        const std::string spoolFile = getStringRunParam("--kafka-spool-file");
        if (!spoolFile.empty()) return TisKafkaProducerPtr(new TisFileKafkaProducer(spoolFile));
        return TisKafkaProducerPtr(new TisNullKafkaProducer());
    }

    void TISBridgeAgent::startBridgeServices()
    {
        if (m_messageSubscriber.get() != 0) return;
        m_kafkaProducer = createKafkaProducer();
        m_messageSubscriber.reset(new TisMessageSubscriber(m_locationKey, m_kafkaProducer, m_kafkaTopicPrefix));
        m_messageSubscriber->subscribe();
        m_restController.reset(new TisRestController());
        m_restServer.reset(new TisRestServer(m_restPort, *m_restController));
        m_restServer->start();

        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
            "TISBridgeAgent started. location=%lu restPort=%u kafkaTopicPrefix=%s",
            m_locationKey, m_restPort, m_kafkaTopicPrefix.c_str());
    }

    void TISBridgeAgent::stopBridgeServices()
    {
        if (m_messageSubscriber.get() != 0)
        {
            TA_Base_Core::MessageSubscriptionManager::getInstance().unsubscribeToMessages(m_messageSubscriber.get());
        }
        if (m_restServer.get() != 0)
        {
            m_restServer->terminateAndWait();
            m_restServer.reset();
        }
        m_restController.reset();
        m_messageSubscriber.reset();
        m_kafkaProducer.reset();
    }

    void TISBridgeAgent::agentSetMonitor()
    {
        if (m_operationMode == TA_Base_Core::Monitor) return;
        m_operationMode = TA_Base_Core::Monitor;
    }

    void TISBridgeAgent::agentSetControl()
    {
        if (m_operationMode == TA_Base_Core::Control) return;
        m_operationMode = TA_Base_Core::Control;
    }

    void TISBridgeAgent::agentTerminate() { stopBridgeServices(); }

    TA_Base_Bus::IEntity* TISBridgeAgent::createEntity(TA_Base_Core::IEntityDataPtr entityData)
    {
        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
            "TISBridgeAgent ignores child entity %s", entityData->getName().c_str());
        return 0;
    }

    void TISBridgeAgent::notifyGroupOffline(const std::string& group)
    {
        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
            "TISBridgeAgent notified group offline: %s", group.c_str());
    }

    void TISBridgeAgent::notifyGroupOnline(const std::string& group)
    {
        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
            "TISBridgeAgent notified group online: %s", group.c_str());
    }

    void TISBridgeAgent::registerForStateUpdates() {}
    void TISBridgeAgent::receiveSpecialisedMessage(const TA_Base_Core::StateUpdateMessageCorbaDef&) {}
    void TISBridgeAgent::processOnlineUpdate(const TA_Base_Core::ConfigUpdateDetails&) {}
    TA_Base_Bus::DataPoint* TISBridgeAgent::getDataPoint(unsigned long) { return 0; }
    void TISBridgeAgent::getAllDataPoints(std::map<unsigned long, TA_Base_Bus::DataPoint*>& values) { values.clear(); }
    TA_Base_Bus::DataPointFactory* TISBridgeAgent::getDataPointFactory() { return 0; }
    TA_Base_Bus::DataNode* TISBridgeAgent::getDataNode(unsigned long) { return 0; }
    void TISBridgeAgent::getAllDataNodes(std::map<unsigned long, TA_Base_Bus::DataNode*>& values) { values.clear(); }
}
