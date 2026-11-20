#pragma once

#include <map>
#include <memory>
#include <string>

#include "TisKafkaProducer.h"
#include "TisMessageSubscriber.h"
#include "TisRestController.h"
#include "TisRestServer.h"

#include "bus/generic_agent/src/IGenericAgentUser.h"
#include "bus/scada/DataNodeAgentCorbaDef_Impl/src/IDataNodeAgent.h"
#include "bus/scada/DataPointAgentCorbaDef_Impl/src/DataPointAgentCorbaDef_Impl.h"

namespace TA_Base_Bus
{
    class GenericAgent;
    class IEntity;
    class DataNode;
    class DataPoint;
    class DataPointFactory;
}

namespace TA_Base_Core
{
    class ConfigUpdateDetails;
}

namespace TA_IRS_App
{
    class TISBridgeAgent : public virtual TA_Base_Bus::IGenericAgentUser,
                           public virtual TA_Base_Bus::IDataPointAgent,
                           public virtual TA_Base_Bus::IDataNodeAgent
    {
    public:
        TISBridgeAgent(int argc, char* argv[]);
        virtual ~TISBridgeAgent();

        void startTISBridgeAgent();

        virtual void notifyGroupOffline(const std::string& group);
        virtual void notifyGroupOnline(const std::string& group);
        virtual void agentTerminate();
        virtual TA_Base_Bus::IEntity* createEntity(TA_Base_Core::IEntityDataPtr entityData);
        virtual void agentSetMonitor();
        virtual void agentSetControl();
        virtual void registerForStateUpdates();
        virtual void receiveSpecialisedMessage(const TA_Base_Core::StateUpdateMessageCorbaDef& message);
        virtual void processOnlineUpdate(const TA_Base_Core::ConfigUpdateDetails& updateEvent);

        virtual TA_Base_Bus::DataPoint* getDataPoint(unsigned long entityKey);
        virtual void getAllDataPoints(std::map<unsigned long, TA_Base_Bus::DataPoint*>& dataPointList);
        virtual TA_Base_Bus::DataPointFactory* getDataPointFactory();
        virtual TA_Base_Bus::DataNode* getDataNode(unsigned long entityKey);
        virtual void getAllDataNodes(std::map<unsigned long, TA_Base_Bus::DataNode*>& dataNodeList);

    private:
        virtual void checkOperationMode() {};

        void loadConfiguration();
        void startBridgeServices();
        void stopBridgeServices();
        TisKafkaProducerPtr createKafkaProducer() const;
        unsigned long getUnsignedRunParam(const std::string& name, unsigned long defaultValue) const;
        std::string getStringRunParam(const std::string& name, const std::string& defaultValue = "") const;

        TA_Base_Bus::GenericAgent* m_genericAgent;
        TA_Base_Core::EOperationMode m_operationMode;
        std::string m_agentName;
        unsigned long m_locationKey;
        unsigned short m_restPort;
        std::string m_kafkaTopicPrefix;

        TisKafkaProducerPtr m_kafkaProducer;
        std::unique_ptr<TisMessageSubscriber> m_messageSubscriber;
        std::unique_ptr<TisRestController> m_restController;
        std::unique_ptr<TisRestServer> m_restServer;
    };
}
