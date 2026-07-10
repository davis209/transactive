#include "PaBridgeAgent.h"

#include "bus/generic_agent/src/GenericAgent.h"

#include "core/data_access_interface/entity_access/src/IEntityData.h"
#include "core/exceptions/src/TransactiveException.h"
#include "core/message/src/MessageSubscriptionManager.h"
#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/RunParams.h"

#include <cstdlib>

namespace TA_IRS_App
{
    PaBridgeAgent::PaBridgeAgent(int argc, char* argv[])
        : m_genericAgent(0),
          m_operationMode(TA_Base_Core::NotApplicable),
          m_paAgentLocationKey(0),
          m_restPort(8088)
    {
        m_genericAgent = new TA_Base_Bus::GenericAgent(argc, argv, this);
        m_agentName = TA_Base_Core::RunParams::getInstance().get(RPARAM_ENTITYNAME);
        loadConfiguration();
    }

    PaBridgeAgent::~PaBridgeAgent()
    {
        stopBridgeServices();

        if (m_genericAgent != 0)
        {
            try
            {
                m_genericAgent->deactivateAndDeleteServant();
            }
            catch (...)
            {
            }
            m_genericAgent = 0;
        }
    }

    void PaBridgeAgent::startPaBridgeAgent()
    {
        const std::string operationMode = TA_Base_Core::RunParams::getInstance().get(RPARAM_OPERATIONMODE);
        if (RPARAM_CONTROL == operationMode)
        {
            agentSetControl();
        }
        else
        {
            agentSetMonitor();
        }

        if (m_genericAgent != 0)
        {
            m_genericAgent->run();
        }
    }

    void PaBridgeAgent::loadConfiguration()
    {
        m_paAgentEntityName = TA_Base_Core::RunParams::getInstance().get("PaAgentEntityName");
        if (m_paAgentEntityName.empty())
        {
            m_paAgentEntityName = TA_Base_Core::RunParams::getInstance().get("PAAgentEntityName");
        }

        m_paAgentLocationKey = getUnsignedRunParam("PaAgentLocationKey", 0);
        if (m_paAgentLocationKey == 0)
        {
            m_paAgentLocationKey = getUnsignedRunParam("LocationKey", 0);
        }

        if (m_paAgentLocationKey == 0 && m_genericAgent != 0)
        {
            TA_Base_Core::IEntityDataPtr agentEntityData = m_genericAgent->getAgentEntityConfiguration();
            if (agentEntityData.get() != 0)
            {
                m_paAgentLocationKey = agentEntityData->getLocation();
            }
        }

        m_restPort = static_cast<unsigned short>(getUnsignedRunParam("RestPort", 8088));

        m_kafkaTopicPrefix = TA_Base_Core::RunParams::getInstance().get("KafkaTopicPrefix");
        if (m_kafkaTopicPrefix.empty())
        {
            m_kafkaTopicPrefix = "paagent";
        }

        if (m_paAgentEntityName.empty())
        {
            TA_THROW(TA_Base_Core::TransactiveException("PABridgeAgent requires --PaAgentEntityName"));
        }
        if (m_paAgentLocationKey == 0)
        {
            TA_THROW(TA_Base_Core::TransactiveException("PABridgeAgent requires --PaAgentLocationKey or a configured agent location"));
        }
    }

    unsigned long PaBridgeAgent::getUnsignedRunParam(const std::string& name, unsigned long defaultValue) const
    {
        const std::string value = TA_Base_Core::RunParams::getInstance().get(name.c_str());
        if (value.empty())
        {
            return defaultValue;
        }
        return static_cast<unsigned long>(std::strtoul(value.c_str(), 0, 10));
    }

    KafkaProducerPtr PaBridgeAgent::createKafkaProducer() const
    {
        const std::string bootstrapServers = TA_Base_Core::RunParams::getInstance().get("KafkaBootstrapServers");
#if defined(USE_LIBRDKAFKA)
        if (!bootstrapServers.empty())
        {
            return KafkaProducerPtr(new LibrdkafkaProducer(bootstrapServers));
        }
#else
        if (!bootstrapServers.empty())
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
                "KafkaBootstrapServers is configured but PABridgeAgent was not built with USE_LIBRDKAFKA");
        }
#endif

        const std::string spoolFile = TA_Base_Core::RunParams::getInstance().get("KafkaSpoolFile");
        if (!spoolFile.empty())
        {
            return KafkaProducerPtr(new FileKafkaProducer(spoolFile));
        }

        return KafkaProducerPtr(new NullKafkaProducer());
    }

    void PaBridgeAgent::startBridgeServices()
    {
        if (m_messageSubscriber.get() != 0)
        {
            return;
        }

        m_kafkaProducer = createKafkaProducer();
        m_messageSubscriber.reset(new PaMessageSubscriber(m_paAgentLocationKey, m_kafkaProducer, m_kafkaTopicPrefix));
        m_messageSubscriber->subscribe();

        m_restController.reset(new PaRestController(m_paAgentEntityName));
        m_restServer.reset(new RestServer(m_restPort, *m_restController));
        m_restServer->start();

        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
            "PABridgeAgent started. paAgent=%s location=%lu restPort=%u kafkaTopicPrefix=%s",
            m_paAgentEntityName.c_str(), m_paAgentLocationKey, m_restPort, m_kafkaTopicPrefix.c_str());
    }

    void PaBridgeAgent::stopBridgeServices()
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

    void PaBridgeAgent::agentSetMonitor()
    {
        if (m_operationMode == TA_Base_Core::Monitor)
        {
            return;
        }
        m_operationMode = TA_Base_Core::Monitor;
        startBridgeServices();
    }

    void PaBridgeAgent::agentSetControl()
    {
        if (m_operationMode == TA_Base_Core::Control)
        {
            return;
        }
        m_operationMode = TA_Base_Core::Control;
        startBridgeServices();
    }

    void PaBridgeAgent::agentTerminate()
    {
        stopBridgeServices();
    }

    TA_Base_Bus::IEntity* PaBridgeAgent::createEntity(TA_Base_Core::IEntityDataPtr entityData)
    {
        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
            "PABridgeAgent ignores child entity %s", entityData->getName().c_str());
        return 0;
    }

    void PaBridgeAgent::notifyGroupOffline(const std::string& group)
    {
        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
            "PABridgeAgent notified group offline: %s", group.c_str());
    }

    void PaBridgeAgent::notifyGroupOnline(const std::string& group)
    {
        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
            "PABridgeAgent notified group online: %s", group.c_str());
    }

    void PaBridgeAgent::registerForStateUpdates()
    {
    }

    void PaBridgeAgent::receiveSpecialisedMessage(const TA_Base_Core::StateUpdateMessageCorbaDef& message)
    {
    }

    void PaBridgeAgent::processOnlineUpdate(const TA_Base_Core::ConfigUpdateDetails& updateEvent)
    {
        loadConfiguration();
    }

    TA_Base_Bus::DataPoint* PaBridgeAgent::getDataPoint(unsigned long entityKey)
    {
        return 0;
    }

    void PaBridgeAgent::getAllDataPoints(std::map<unsigned long, TA_Base_Bus::DataPoint*>& dataPointList)
    {
        dataPointList.clear();
    }

    TA_Base_Bus::DataPointFactory* PaBridgeAgent::getDataPointFactory()
    {
        return 0;
    }

    TA_Base_Bus::DataNode* PaBridgeAgent::getDataNode(unsigned long entityKey)
    {
        return 0;
    }

    void PaBridgeAgent::getAllDataNodes(std::map<unsigned long, TA_Base_Bus::DataNode*>& dataNodeList)
    {
        dataNodeList.clear();
    }
}
