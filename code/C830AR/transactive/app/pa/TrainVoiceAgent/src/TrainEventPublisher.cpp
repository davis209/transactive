#include "TrainEventPublisher.h"
#include "json.hpp"
#include "core/utilities/src/DebugUtil.h"
#if defined(USE_LIBRDKAFKA)
#include <rdkafka.h>
#endif

namespace TA_IRS_App
{
    TrainEventPublisher::TrainEventPublisher(const std::string& servers, const std::string& prefix)
        : m_topic((prefix.empty() ? "trainvoice" : prefix) + ".events"), m_producer(0)
    {
#if defined(USE_LIBRDKAFKA)
        if (servers.empty()) return;
        char error[512] = {0};
        rd_kafka_conf_t* conf = rd_kafka_conf_new();
        if (rd_kafka_conf_set(conf, "bootstrap.servers", servers.c_str(), error, sizeof(error)) != RD_KAFKA_CONF_OK)
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Kafka configuration failed: %s", error);
            rd_kafka_conf_destroy(conf);
            return;
        }
        m_producer = rd_kafka_new(RD_KAFKA_PRODUCER, conf, error, sizeof(error));
        if (m_producer == 0)
        {
            rd_kafka_conf_destroy(conf);
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Kafka producer creation failed: %s", error);
        }
#else
        (void)servers;
#endif
    }
    TrainEventPublisher::~TrainEventPublisher()
    {
#if defined(USE_LIBRDKAFKA)
        if (m_producer != 0)
        {
            rd_kafka_flush(static_cast<rd_kafka_t*>(m_producer), 5000);
            rd_kafka_destroy(static_cast<rd_kafka_t*>(m_producer));
        }
#endif
    }
    bool TrainEventPublisher::publish(const std::string& eventType, const std::string& payloadJson)
    {
        if (eventType.empty() || m_producer == 0) return false;
        try
        {
            nlohmann::json envelope;
            envelope["eventType"] = eventType;
            envelope["payload"] = nlohmann::json::parse(payloadJson);
            const std::string value = envelope.dump();
#if defined(USE_LIBRDKAFKA)
            const char emptyKey = '\0';
            rd_kafka_resp_err_t result = rd_kafka_producev(
                static_cast<rd_kafka_t*>(m_producer), RD_KAFKA_V_TOPIC(m_topic.c_str()),
                RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY), RD_KAFKA_V_KEY(&emptyKey, 0),
                RD_KAFKA_V_VALUE(const_cast<char*>(value.data()), value.size()), RD_KAFKA_V_END);
            rd_kafka_poll(static_cast<rd_kafka_t*>(m_producer), 0);
            return result == RD_KAFKA_RESP_ERR_NO_ERROR;
#else
            return false;
#endif
        }
        catch (const nlohmann::json::exception&) { return false; }
    }
}
