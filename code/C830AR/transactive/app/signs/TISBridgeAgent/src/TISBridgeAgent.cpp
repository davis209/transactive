#include "TISBridgeAgent.h"

#include "bus/generic_agent/src/GenericAgent.h"
#include "bus/security/authentication_library/src/AuthenticationLibrary.h"

#include "core/data_access_interface/entity_access/src/ConsoleAccessFactory.h"
#include "core/data_access_interface/entity_access/src/IEntityData.h"
#include "core/exceptions/src/TransactiveException.h"
#include "core/message/src/MessageSubscriptionManager.h"
#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/Hostname.h"
#include "core/utilities/src/RunParams.h"

#include <chrono>
#include <cstdlib>

namespace TA_IRS_App
{
    TISBridgeAgent::TISBridgeAgent(int argc, char* argv[])
        : m_genericAgent(0),
          m_operationMode(TA_Base_Core::NotApplicable),
          m_locationKey(0),
          m_restPort(8089),
          m_sessionUserKey(0),
          m_sessionProfileKey(0),
          m_sessionLocationKey(0),
          m_sessionConsoleId(0),
          m_stopSessionRetry(false),
          m_sessionRetryRunning(false)
    {
        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo, "Creating GenericAgent");
        m_genericAgent = new TA_Base_Bus::GenericAgent(argc, argv, this);

        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo, "Creating AuthenticationLibrary");
        m_authenticationLibrary.reset(new TA_Base_Bus::AuthenticationLibrary());
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
        m_authenticationLibrary.reset();
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
        m_stisEntityName = getStringRunParam("--stis-agent-name");
        m_ttisEntityName = getStringRunParam("--ttis-agent-name");
        if (m_stisEntityName.empty() && m_ttisEntityName.empty())
        {
            TA_THROW(TA_Base_Core::TransactiveException(
                "TISBridgeAgent requires --stis-agent-name or --ttis-agent-name"));
        }
        if (m_stisEntityName.empty()) m_stisEntityName = m_ttisEntityName;
        if (m_ttisEntityName.empty()) m_ttisEntityName = m_stisEntityName;

        if (m_genericAgent != 0)
        {
            TA_Base_Core::IEntityDataPtr data = m_genericAgent->getAgentEntityConfiguration();
            if (data.get() != 0) m_locationKey = data->getLocation();
        }
        if (m_locationKey == 0) TA_THROW(TA_Base_Core::TransactiveException("Failed to get TISBridgeAgent location Id"));

        m_restPort = static_cast<unsigned short>(getUnsignedRunParam("--rest-port", 8089));
        m_kafkaTopicPrefix = getStringRunParam("--kafka-topic-prefix", "tis");
        loadAuthenticationConfiguration();
    }

    void TISBridgeAgent::loadAuthenticationConfiguration()
    {
        m_sessionUserKey = getUnsignedRunParam("--user-id", 0);
        m_sessionProfileKey = getUnsignedRunParam("--profile-id", 0);
        m_sessionPassword = getStringRunParam("--user-pwd");
        m_sessionLocationKey = m_locationKey;

        const std::string hostname = TA_Base_Core::Hostname::getHostname();
        std::unique_ptr<TA_Base_Core::IConsole> console(TA_Base_Core::ConsoleAccessFactory::getInstance().getConsole(hostname));
        m_sessionConsoleId = console->getKey();

        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
            "TISBridgeAgent resolved hostname %s to console key %lu", hostname.c_str(), m_sessionConsoleId);

        if (m_sessionUserKey == 0) TA_THROW(TA_Base_Core::TransactiveException("TISBridgeAgent requires --user-id"));
        if (m_sessionProfileKey == 0) TA_THROW(TA_Base_Core::TransactiveException("TISBridgeAgent requires --profile-id"));
        if (m_sessionPassword.empty()) TA_THROW(TA_Base_Core::TransactiveException("TISBridgeAgent requires --user-pwd"));
        if (m_sessionConsoleId == 0) TA_THROW(TA_Base_Core::TransactiveException("Add a console-type entity in DB for this local host"));
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

    bool TISBridgeAgent::requestBridgeSession()
    {
        if (!getBridgeSessionId().empty()) return true;
        if (m_authenticationLibrary.get() == 0)
        {
            TA_THROW(TA_Base_Core::TransactiveException("TISBridgeAgent authentication library is not initialised"));
        }

        const std::string sessionId = m_authenticationLibrary->requestSession(
            m_sessionUserKey, m_sessionProfileKey, m_sessionLocationKey, m_sessionConsoleId, m_sessionPassword);
        if (sessionId.empty()) TA_THROW(TA_Base_Core::TransactiveException("TISBridgeAgent failed to acquire a session id"));

        {
            std::lock_guard<std::mutex> guard(m_sessionLock);
            m_sessionId = sessionId;
        }
        TA_Base_Core::RunParams::getInstance().set(RPARAM_SESSIONID, sessionId.c_str());
        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
            "TISBridgeAgent acquired session id for user=%lu profile=%lu location=%lu console=%lu",
            m_sessionUserKey, m_sessionProfileKey, m_sessionLocationKey, m_sessionConsoleId);
        return true;
    }

    void TISBridgeAgent::startSessionRetry()
    {
        std::lock_guard<std::mutex> guard(m_sessionLock);
        if (m_sessionRetryRunning) return;
        m_stopSessionRetry = false;
        m_sessionRetryRunning = true;
        m_sessionRetryThread = std::thread(&TISBridgeAgent::sessionRetryLoop, this);
    }

    void TISBridgeAgent::stopSessionRetry()
    {
        {
            std::lock_guard<std::mutex> guard(m_sessionLock);
            if (!m_sessionRetryRunning) return;
            m_stopSessionRetry = true;
        }
        m_sessionRetryCondition.notify_all();
        if (m_sessionRetryThread.joinable()) m_sessionRetryThread.join();
        std::lock_guard<std::mutex> guard(m_sessionLock);
        m_sessionRetryRunning = false;
    }

    void TISBridgeAgent::sessionRetryLoop()
    {
        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(m_sessionLock);
                if (m_stopSessionRetry || !m_sessionId.empty()) return;
            }
            try
            {
                if (requestBridgeSession()) return;
            }
            catch (const std::exception& e)
            {
                LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
                    "TISBridgeAgent failed to acquire session id, will retry: %s", e.what());
            }
            catch (...)
            {
                LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
                    "TISBridgeAgent failed to acquire session id, will retry");
            }

            std::unique_lock<std::mutex> lock(m_sessionLock);
            if (m_stopSessionRetry) return;
            m_sessionRetryCondition.wait_for(lock, std::chrono::seconds(5));
        }
    }

    std::string TISBridgeAgent::getBridgeSessionId() const
    {
        std::lock_guard<std::mutex> guard(m_sessionLock);
        return m_sessionId;
    }

    void TISBridgeAgent::endBridgeSession()
    {
        const std::string sessionId = getBridgeSessionId();
        if (sessionId.empty() || m_authenticationLibrary.get() == 0) return;
        try { m_authenticationLibrary->endSession(sessionId); }
        catch (...)
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
                "TISBridgeAgent failed to end session id cleanly");
        }
        {
            std::lock_guard<std::mutex> guard(m_sessionLock);
            m_sessionId.clear();
        }
        TA_Base_Core::RunParams::getInstance().set(RPARAM_SESSIONID, "");
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
        startSessionRetry();
        m_kafkaProducer = createKafkaProducer();
        m_messageSubscriber.reset(new TisMessageSubscriber(m_locationKey, m_kafkaProducer, m_kafkaTopicPrefix));
        m_messageSubscriber->subscribe();
        m_restController.reset(new TisRestController(m_stisEntityName, m_ttisEntityName,
            [this]() { return getBridgeSessionId(); }));
        m_restServer.reset(new TisRestServer(m_restPort, *m_restController));
        m_restServer->start();

        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
            "TISBridgeAgent started. stis=%s ttis=%s location=%lu restPort=%u kafkaTopicPrefix=%s",
            m_stisEntityName.c_str(), m_ttisEntityName.c_str(), m_locationKey, m_restPort, m_kafkaTopicPrefix.c_str());
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
        stopSessionRetry();
        m_restController.reset();
        m_messageSubscriber.reset();
        m_kafkaProducer.reset();
        endBridgeSession();
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
