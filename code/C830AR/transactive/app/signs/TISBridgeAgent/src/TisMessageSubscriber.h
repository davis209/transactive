#pragma once

#include <string>

#include "TisKafkaProducer.h"

#include "core/message/IDL/src/CommsMessageCorbaDef.h"
#include "core/message/src/SpecialisedMessageSubscriber.h"

namespace TA_IRS_App
{
    class TisMessageSubscriber :
        public TA_Base_Core::SpecialisedMessageSubscriber<TA_Base_Core::CommsMessageCorbaDef>
    {
    public:
        TisMessageSubscriber(unsigned long locationKey, TisKafkaProducerPtr producer, const std::string& topicPrefix);
        virtual ~TisMessageSubscriber();

        void subscribe();
        void unsubscribe();
        virtual void receiveSpecialisedMessage(const TA_Base_Core::CommsMessageCorbaDef& message);

    private:
        void publishMessage(const std::string& messageName, const std::string& key, const std::string& payload);

        unsigned long m_locationKey;
        TisKafkaProducerPtr m_producer;
        std::string m_topicPrefix;
    };
}
