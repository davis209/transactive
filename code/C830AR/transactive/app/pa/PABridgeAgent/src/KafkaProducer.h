#pragma once

#include <memory>
#include <string>

namespace TA_IRS_App
{
    class IKafkaProducer
    {
    public:
        virtual ~IKafkaProducer() {}
        virtual void publish(const std::string& topic, const std::string& key, const std::string& payload) = 0;
    };

    typedef std::shared_ptr<IKafkaProducer> KafkaProducerPtr;

    class FileKafkaProducer : public IKafkaProducer
    {
    public:
        explicit FileKafkaProducer(const std::string& spoolFile);
        virtual void publish(const std::string& topic, const std::string& key, const std::string& payload);

    private:
        std::string m_spoolFile;
    };

    class NullKafkaProducer : public IKafkaProducer
    {
    public:
        virtual void publish(const std::string& topic, const std::string& key, const std::string& payload);
    };

#if defined(USE_LIBRDKAFKA)
    class LibrdkafkaProducer : public IKafkaProducer
    {
    public:
        LibrdkafkaProducer(const std::string& bootstrapServers);
        virtual ~LibrdkafkaProducer();
        virtual void publish(const std::string& topic, const std::string& key, const std::string& payload);

    private:
        void* m_producer;
    };
#endif
}
