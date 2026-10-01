#include "TisMessageSubscriber.h"

#include "TisJsonConverter.h"
#include "TisJsonUtil.h"

#include "core/message/src/MessageSubscriptionManager.h"
#include "core/message/types/TISComms_MessageTypes.h"
#include "core/utilities/src/DebugUtil.h"

#include <sstream>

namespace
{
    template <typename T>
    std::string number(T value)
    {
        std::ostringstream out;
        out << value;
        return out.str();
    }
}

namespace TA_IRS_App
{
    TisMessageSubscriber::TisMessageSubscriber(unsigned long locationKey, TisKafkaProducerPtr producer, const std::string& topicPrefix)
        : m_locationKey(locationKey),
          m_producer(producer),
          m_topicPrefix(topicPrefix)
    {
    }

    TisMessageSubscriber::~TisMessageSubscriber()
    {
        unsubscribe();
    }

    void TisMessageSubscriber::subscribe()
    {
        TA_Base_Core::MessageSubscriptionManager::getInstance().unsubscribeToMessages(this);
        TA_Base_Core::MessageSubscriptionManager::getInstance().subscribeToCommsMessage(TA_Base_Core::TISComms::TrainDisplayResult, this, 0, 0, m_locationKey);
        TA_Base_Core::MessageSubscriptionManager::getInstance().subscribeToCommsMessage(TA_Base_Core::TISComms::TisTrainDownloadUpdate, this, 0, 0, m_locationKey);
        TA_Base_Core::MessageSubscriptionManager::getInstance().subscribeToCommsMessage(TA_Base_Core::TISComms::TisTrainDataVersionUpdate, this, 0, 0, m_locationKey);
        TA_Base_Core::MessageSubscriptionManager::getInstance().subscribeToCommsMessage(TA_Base_Core::TISComms::TisTrainTimeScheduleChange, this, 0, 0, m_locationKey);
        TA_Base_Core::MessageSubscriptionManager::getInstance().subscribeToCommsMessage(TA_Base_Core::TISComms::IncomingRATISMessage, this, 0, 0, m_locationKey);
        TA_Base_Core::MessageSubscriptionManager::getInstance().subscribeToCommsMessage(TA_Base_Core::TISComms::RATISStatusUpdate, this, 0, 0, m_locationKey);
        TA_Base_Core::MessageSubscriptionManager::getInstance().subscribeToCommsMessage(TA_Base_Core::TISComms::RATISVetting, this, 0, 0, m_locationKey);
    }

    void TisMessageSubscriber::unsubscribe()
    {
        try
        {
            TA_Base_Core::MessageSubscriptionManager::getInstance().unsubscribeToMessages(this);
        }
        catch (...)
        {
        }
    }

    void TisMessageSubscriber::receiveSpecialisedMessage(const TA_Base_Core::CommsMessageCorbaDef& message)
    {
        try
        {
            const std::string messageTypeKey(message.messageTypeKey);
            if (messageTypeKey == TA_Base_Core::TISComms::TrainDisplayResult.getTypeKey())
            {
                const TA_Base_Core::TTISDisplayResult* data = 0;
                if ((message.messageState >>= data) != 0)
                {
                    publishMessage("TrainDisplayResult", number(static_cast<unsigned long>(data->trainId)), TisJsonConverter::toJson(*data));
                }
            }
            else if (messageTypeKey == TA_Base_Core::TISComms::TisTrainDownloadUpdate.getTypeKey())
            {
                const TA_Base_Core::TrainDownloadStatus* data = 0;
                if ((message.messageState >>= data) != 0)
                {
                    publishMessage("TisTrainDownloadUpdate", number(static_cast<unsigned long>(data->trainNumber)), TisJsonConverter::toJson(*data));
                }
            }
            else if (messageTypeKey == TA_Base_Core::TISComms::TisTrainDataVersionUpdate.getTypeKey())
            {
                const TA_Base_Core::TrainDataVersion* data = 0;
                if ((message.messageState >>= data) != 0)
                {
                    publishMessage("TisTrainDataVersionUpdate", number(static_cast<unsigned long>(data->trainNumber)), TisJsonConverter::toJson(*data));
                }
            }
            else if (messageTypeKey == TA_Base_Core::TISComms::TisTrainTimeScheduleChange.getTypeKey())
            {
                const TA_Base_Core::TimeScheduleChange* data = 0;
                if ((message.messageState >>= data) != 0)
                {
                    publishMessage("TisTrainTimeScheduleChange", number(data->timeSchedulePkey), TisJsonConverter::toJson(*data));
                }
            }
            else if (messageTypeKey == TA_Base_Core::TISComms::IncomingRATISMessage.getTypeKey())
            {
                const TA_Base_Bus::ISTISManagerCorbaDef::IncomingRATISEvent* data = 0;
                if ((message.messageState >>= data) != 0)
                {
                    publishMessage("IncomingRATISMessage", number(data->messageID), TisJsonConverter::toJson(*data));
                }
            }
            else if (messageTypeKey == TA_Base_Core::TISComms::RATISStatusUpdate.getTypeKey())
            {
                const TA_Base_Bus::ISTISManagerCorbaDef::RATISMessageApprovalDetails* data = 0;
                if ((message.messageState >>= data) != 0)
                {
                    publishMessage("RATISStatusUpdate", number(data->messageID), TisJsonConverter::toJson(*data));
                }
            }
            else if (messageTypeKey == TA_Base_Core::TISComms::RATISVetting.getTypeKey())
            {
                CORBA::Boolean enabled = false;
                if ((message.messageState >>= CORBA::Any::to_boolean(enabled)) != 0)
                {
                    publishMessage("RATISVetting", "", TisJsonUtil::object({
                        TisJsonUtil::property("enabled", enabled ? "true" : "false", true)
                    }));
                }
            }
        }
        catch (...)
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
                "Exception while converting TISComms message for Kafka");
        }
    }

    void TisMessageSubscriber::publishMessage(const std::string& messageName, const std::string& key, const std::string& payload)
    {
        const std::string topic = m_topicPrefix.empty() ? messageName : (m_topicPrefix + "." + messageName);
        if (m_producer.get() != 0) m_producer->publish(topic, key, payload);
    }
}
