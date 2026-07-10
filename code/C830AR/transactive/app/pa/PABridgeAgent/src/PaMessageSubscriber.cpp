#include "PaMessageSubscriber.h"

#include "PaJsonConverter.h"

#include "core/message/src/MessageSubscriptionManager.h"
#include "core/message/types/PAAgentComms_MessageTypes.h"
#include "core/utilities/src/DebugUtil.h"

namespace TA_IRS_App
{
    PaMessageSubscriber::PaMessageSubscriber(unsigned long locationKey, KafkaProducerPtr producer, const std::string& topicPrefix)
        : m_locationKey(locationKey),
          m_producer(producer),
          m_topicPrefix(topicPrefix)
    {
    }

    PaMessageSubscriber::~PaMessageSubscriber()
    {
        unsubscribe();
    }

    void PaMessageSubscriber::subscribe()
    {
        TA_Base_Core::MessageSubscriptionManager& manager = TA_Base_Core::MessageSubscriptionManager::getInstance();
        manager.unsubscribeToMessages(this);

        manager.subscribeToCommsMessage(TA_Base_Core::PAAgentComms::BroadcastProgressUpdate, this, 0, 0, m_locationKey);
        manager.subscribeToCommsMessage(TA_Base_Core::PAAgentComms::CurrentBroadcastsUpdate, this, 0, 0, m_locationKey);
        manager.subscribeToCommsMessage(TA_Base_Core::PAAgentComms::DvaMessagesUpdate, this, 0, 0, m_locationKey);
        manager.subscribeToCommsMessage(TA_Base_Core::PAAgentComms::DvaVersionsUpdate, this, 0, 0, m_locationKey);
        manager.subscribeToCommsMessage(TA_Base_Core::PAAgentComms::PrioritySchemeUpdate, this, 0, 0, m_locationKey);
    }

    void PaMessageSubscriber::unsubscribe()
    {
        try
        {
            TA_Base_Core::MessageSubscriptionManager::getInstance().unsubscribeToMessages(this);
        }
        catch (...)
        {
        }
    }

    void PaMessageSubscriber::receiveSpecialisedMessage(const TA_Base_Core::CommsMessageCorbaDef& message)
    {
        try
        {
            std::string messageTypeKey(message.messageTypeKey);

            if (messageTypeKey == TA_Base_Core::PAAgentComms::BroadcastProgressUpdate.getTypeKey())
            {
                const TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData* data = 0;
                if ((message.messageState >>= data) != 0)
                {
                    publishMessage("BroadcastProgressUpdate", data->broadcastId.in(), PaJsonConverter::toJson(*data));
                }
            }
            else if (messageTypeKey == TA_Base_Core::PAAgentComms::CurrentBroadcastsUpdate.getTypeKey())
            {
                const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData* data = 0;
                if ((message.messageState >>= data) != 0)
                {
                    publishMessage("CurrentBroadcastsUpdate", data->toBroadcastId.in(), PaJsonConverter::toJson(*data));
                }
            }
            else if (messageTypeKey == TA_Base_Core::PAAgentComms::DvaMessagesUpdate.getTypeKey())
            {
                const TA_Base_Bus::IPAAgentCorbaDef::DvaMessagesUpdateData* data = 0;
                if ((message.messageState >>= data) != 0)
                {
                    publishMessage("DvaMessagesUpdate", "", PaJsonConverter::toJson(*data));
                }
            }
            else if (messageTypeKey == TA_Base_Core::PAAgentComms::DvaVersionsUpdate.getTypeKey())
            {
                const TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords* allData = 0;
                const TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecords* publicData = 0;
                const TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecords* privateData = 0;
                if ((message.messageState >>= allData) != 0)
                {
                    publishMessage("DvaVersionsUpdate", "", PaJsonConverter::toJson(*allData));
                }
                else if ((message.messageState >>= publicData) != 0)
                {
                    publishMessage("DvaPublicVersionsUpdate", "", PaJsonConverter::toJson(*publicData));
                }
                else if ((message.messageState >>= privateData) != 0)
                {
                    publishMessage("DvaPrivateVersionsUpdate", "", PaJsonConverter::toJson(*privateData));
                }
            }
            else if (messageTypeKey == TA_Base_Core::PAAgentComms::PrioritySchemeUpdate.getTypeKey())
            {
                const TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme* data = 0;
                if ((message.messageState >>= data) != 0)
                {
                    publishMessage("PrioritySchemeUpdate", "", PaJsonConverter::toJson(*data));
                }
            }
        }
        catch (...)
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
                "Exception while converting PAAgentComms message for Kafka");
        }
    }

    void PaMessageSubscriber::publishMessage(const std::string& messageName, const std::string& key, const std::string& payload)
    {
        const std::string topic = m_topicPrefix.empty() ? messageName : (m_topicPrefix + "." + messageName);
        if (m_producer.get() != 0)
        {
            m_producer->publish(topic, key, payload);
        }
    }
}
