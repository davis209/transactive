#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "bus/generic_agent/src/IGenericAgentUser.h"
#include "bus/scada/DataNodeAgentCorbaDef_Impl/src/IDataNodeAgent.h"
#include "bus/scada/DataPointAgentCorbaDef_Impl/src/DataPointAgentCorbaDef_Impl.h"

namespace TA_Base_Bus { class GenericAgent; class IEntity; class DataPoint; class DataNode; class DataPointFactory; }
namespace TA_Base_Core { class ConfigUpdateDetails; }

namespace TA_IRS_App
{
    class TrainRestController;
    class TrainRestServer;
    class TrainEventPublisher;
    class TrainVoiceReadService;
    class TrainVoiceTransport;
    class TrainVoiceAgent : public virtual TA_Base_Bus::IGenericAgentUser,
                            public virtual TA_Base_Bus::IDataPointAgent,
                            public virtual TA_Base_Bus::IDataNodeAgent
    {
    public:
        TrainVoiceAgent(int argc, char* argv[]);
        virtual ~TrainVoiceAgent();
        void run();
        virtual void notifyGroupOffline(const std::string& group);
        virtual void notifyGroupOnline(const std::string& group);
        virtual void agentTerminate();
        virtual TA_Base_Bus::IEntity* createEntity(TA_Base_Core::IEntityDataPtr data);
        virtual void agentSetMonitor();
        virtual void agentSetControl();
        virtual void registerForStateUpdates();
        virtual void receiveSpecialisedMessage(const TA_Base_Core::StateUpdateMessageCorbaDef& message);
        virtual void processOnlineUpdate(const TA_Base_Core::ConfigUpdateDetails& update);
        virtual TA_Base_Bus::DataPoint* getDataPoint(unsigned long key);
        virtual void getAllDataPoints(std::map<unsigned long, TA_Base_Bus::DataPoint*>& points);
        virtual TA_Base_Bus::DataPointFactory* getDataPointFactory();
        virtual TA_Base_Bus::DataNode* getDataNode(unsigned long key);
        virtual void getAllDataNodes(std::map<unsigned long, TA_Base_Bus::DataNode*>& nodes);
    private:
        virtual void checkOperationMode() {}
        TA_Base_Bus::GenericAgent* m_genericAgent;
        TA_Base_Core::EOperationMode m_operationMode;
        std::unique_ptr<TrainRestController> m_restController;
        std::unique_ptr<TrainVoiceReadService> m_readService;
        std::unique_ptr<TrainRestServer> m_restServer;
        std::unique_ptr<TrainEventPublisher> m_eventPublisher;
        std::unique_ptr<TrainVoiceTransport> m_transport;
        std::mutex m_lifecycleLock;
        void stopServices();
    };
}
