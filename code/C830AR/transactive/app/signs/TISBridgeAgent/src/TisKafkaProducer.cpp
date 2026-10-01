#include "TisKafkaProducer.h"

#if defined(USE_LIBRDKAFKA)
#include <rdkafka.h>
#endif

#include "TisJsonUtil.h"

#include "core/utilities/src/DebugUtil.h"

#include <fstream>

namespace TA_IRS_App
{
    TisFileKafkaProducer::TisFileKafkaProducer(const std::string& spoolFile)
        : m_spoolFile(spoolFile)
    {
    }

    void TisFileKafkaProducer::publish(const std::string& topic, const std::string& key, const std::string& payload)
    {
        std::ofstream out(m_spoolFile.c_str(), std::ios::out | std::ios::app);
        out << TisJsonUtil::object({
            TisJsonUtil::property("topic", topic),
            TisJsonUtil::property("key", key),
            TisJsonUtil::property("payload", payload, true)
        }) << std::endl;
    }

    void TisNullKafkaProducer::publish(const std::string& topic, const std::string& key, const std::string& payload)
    {
        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
            "Kafka disabled. topic=%s key=%s payload=%s", topic.c_str(), key.c_str(), payload.c_str());
    }

#if defined(USE_LIBRDKAFKA)
    TisLibrdkafkaProducer::TisLibrdkafkaProducer(const std::string& bootstrapServers)
        : m_producer(0)
    {
        char errstr[512] = {0};
        rd_kafka_conf_t* conf = rd_kafka_conf_new();
        if (rd_kafka_conf_set(conf, "bootstrap.servers", bootstrapServers.c_str(), errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Kafka config error: %s", errstr);
        }

        m_producer = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
        if (m_producer == 0)
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Unable to create Kafka producer: %s", errstr);
        }
    }

    TisLibrdkafkaProducer::~TisLibrdkafkaProducer()
    {
        if (m_producer != 0)
        {
            rd_kafka_flush(static_cast<rd_kafka_t*>(m_producer), 5000);
            rd_kafka_destroy(static_cast<rd_kafka_t*>(m_producer));
            m_producer = 0;
        }
    }

    void TisLibrdkafkaProducer::publish(const std::string& topic, const std::string& key, const std::string& payload)
    {
        if (m_producer == 0)
        {
            return;
        }

        rd_kafka_resp_err_t err = rd_kafka_producev(
            static_cast<rd_kafka_t*>(m_producer),
            RD_KAFKA_V_TOPIC(topic.c_str()),
            RD_KAFKA_V_KEY(key.c_str(), key.size()),
            RD_KAFKA_V_VALUE(payload.c_str(), payload.size()),
            RD_KAFKA_V_END);

        if (err != RD_KAFKA_RESP_ERR_NO_ERROR)
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
                "Kafka publish failed: %s", rd_kafka_err2str(err));
        }
        rd_kafka_poll(static_cast<rd_kafka_t*>(m_producer), 0);
    }
#endif
}
