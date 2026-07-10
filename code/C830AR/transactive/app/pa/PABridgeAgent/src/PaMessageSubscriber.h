#pragma once

#include <string>

#include "KafkaProducer.h"

#include "core/message/src/SpecialisedMessageSubscriber.h"
#include "core/message/IDL/src/CommsMessageCorbaDef.h"

namespace TA_IRS_App
{
    class PaMessageSubscriber :
        public TA_Base_Core::SpecialisedMessageSubscriber<TA_Base_Core::CommsMessageCorbaDef>
    {
    public:
        PaMessageSubscriber(unsigned long locationKey, KafkaProducerPtr producer, const std::string& topicPrefix);
        virtual ~PaMessageSubscriber();

        void subscribe();
        void unsubscribe();
        virtual void receiveSpecialisedMessage(const TA_Base_Core::CommsMessageCorbaDef& message);

    private:
        void publishMessage(const std::string& messageName, const std::string& key, const std::string& payload);

        unsigned long m_locationKey;
        KafkaProducerPtr m_producer;
        std::string m_topicPrefix;
    };
}
