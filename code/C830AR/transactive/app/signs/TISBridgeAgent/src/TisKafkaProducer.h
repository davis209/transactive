#pragma once

#include <memory>
#include <string>

namespace TA_IRS_App
{
    class ITisKafkaProducer
    {
    public:
        virtual ~ITisKafkaProducer() {}
        virtual void publish(const std::string& topic, const std::string& key, const std::string& payload) = 0;
    };

    typedef std::shared_ptr<ITisKafkaProducer> TisKafkaProducerPtr;

    class TisFileKafkaProducer : public ITisKafkaProducer
    {
    public:
        explicit TisFileKafkaProducer(const std::string& spoolFile);
        virtual void publish(const std::string& topic, const std::string& key, const std::string& payload);

    private:
        std::string m_spoolFile;
    };

    class TisNullKafkaProducer : public ITisKafkaProducer
    {
    public:
        virtual void publish(const std::string& topic, const std::string& key, const std::string& payload);
    };

#if defined(USE_LIBRDKAFKA)
    class TisLibrdkafkaProducer : public ITisKafkaProducer
    {
    public:
        explicit TisLibrdkafkaProducer(const std::string& bootstrapServers);
        virtual ~TisLibrdkafkaProducer();
        virtual void publish(const std::string& topic, const std::string& key, const std::string& payload);

    private:
        void* m_producer;
    };
#endif
}
