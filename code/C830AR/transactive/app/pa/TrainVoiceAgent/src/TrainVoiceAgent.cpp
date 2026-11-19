#include "TrainVoiceAgent.h"
#include "bus/generic_agent/src/GenericAgent.h"
#include "TrainRestController.h"
#include "TrainRestServer.h"
#include "TrainEventPublisher.h"
#include "TrainVoiceReadService.h"
#include "TrainVoiceTransport.h"
#include "core/utilities/src/RunParams.h"
#include <cstdlib>
#include <stdexcept>

namespace TA_IRS_App
{
    TrainVoiceAgent::TrainVoiceAgent(int argc, char* argv[])
        : m_genericAgent(0), m_operationMode(TA_Base_Core::NotApplicable)
    {
        m_genericAgent = new TA_Base_Bus::GenericAgent(argc, argv, this);
    }
    TrainVoiceAgent::~TrainVoiceAgent()
    {
        stopServices();
        if (m_genericAgent != 0)
        {
            try { m_genericAgent->deactivateAndDeleteServant(); } catch (...) {}
            m_genericAgent = 0;
        }
    }
    void TrainVoiceAgent::run()
    {
        const std::string portValue = TA_Base_Core::RunParams::getInstance().get("--rest-port");
        char* end = 0;
        const long port = portValue.empty() ? 8089 : std::strtol(portValue.c_str(), &end, 10);
        if (port < 1 || port > 65535 || (!portValue.empty() && (end == portValue.c_str() || *end != '\0')))
            throw std::invalid_argument("Invalid REST port");
        {
            std::lock_guard<std::mutex> guard(m_lifecycleLock);
            m_eventPublisher.reset(new TrainEventPublisher(
                TA_Base_Core::RunParams::getInstance().get("--kafka-servers"),
                TA_Base_Core::RunParams::getInstance().get("--kafka-topic-prefix")));
            std::string bindAddress = TA_Base_Core::RunParams::getInstance().get("--train-bind-address");
            if (bindAddress.empty()) bindAddress = "0.0.0.0";
            m_transport.reset(new TrainVoiceTransport(*m_eventPublisher, bindAddress, 61002));
            m_transport->setControl(m_operationMode == TA_Base_Core::Control);
            m_transport->start();
            m_readService.reset(new TrainVoiceReadService(m_transport.get()));
            m_restController.reset(new TrainRestController(m_readService.get()));
            std::string restBind = TA_Base_Core::RunParams::getInstance().get("--rest-bind-address");
            if (restBind.empty()) restBind = "127.0.0.1";
            m_restServer.reset(new TrainRestServer(restBind, static_cast<unsigned short>(port), *m_restController));
            m_restServer->start();
        }
        m_genericAgent->run();
        stopServices();
    }
    void TrainVoiceAgent::stopServices()
    {
        std::lock_guard<std::mutex> guard(m_lifecycleLock);
        if (m_transport.get() != 0) m_transport->terminateAndWait();
        if (m_restServer.get() != 0) { m_restServer->terminateAndWait(); m_restServer.reset(); }
        m_restController.reset();
        m_readService.reset();
        m_transport.reset();
        m_eventPublisher.reset();
    }
    void TrainVoiceAgent::notifyGroupOffline(const std::string&) {}
    void TrainVoiceAgent::notifyGroupOnline(const std::string&) {}
    void TrainVoiceAgent::agentTerminate() { stopServices(); }
    TA_Base_Bus::IEntity* TrainVoiceAgent::createEntity(TA_Base_Core::IEntityDataPtr) { return 0; }
    void TrainVoiceAgent::agentSetMonitor() { std::lock_guard<std::mutex> guard(m_lifecycleLock); m_operationMode = TA_Base_Core::Monitor; if (m_transport) m_transport->setControl(false); }
    void TrainVoiceAgent::agentSetControl() { std::lock_guard<std::mutex> guard(m_lifecycleLock); m_operationMode = TA_Base_Core::Control; if (m_transport) m_transport->setControl(true); }
    void TrainVoiceAgent::registerForStateUpdates() {}
    void TrainVoiceAgent::receiveSpecialisedMessage(const TA_Base_Core::StateUpdateMessageCorbaDef&) {}
    void TrainVoiceAgent::processOnlineUpdate(const TA_Base_Core::ConfigUpdateDetails&) {}
    TA_Base_Bus::DataPoint* TrainVoiceAgent::getDataPoint(unsigned long) { return 0; }
    void TrainVoiceAgent::getAllDataPoints(std::map<unsigned long, TA_Base_Bus::DataPoint*>& points) { points.clear(); }
    TA_Base_Bus::DataPointFactory* TrainVoiceAgent::getDataPointFactory() { return 0; }
    TA_Base_Bus::DataNode* TrainVoiceAgent::getDataNode(unsigned long) { return 0; }
    void TrainVoiceAgent::getAllDataNodes(std::map<unsigned long, TA_Base_Bus::DataNode*>& nodes) { nodes.clear(); }
}
