#pragma once
#include <string>

namespace TA_IRS_App
{
    class TrainEventPublisher
    {
    public:
        TrainEventPublisher(const std::string& servers, const std::string& prefix);
        ~TrainEventPublisher();
        // Returns false when the event cannot be queued locally.
        bool publish(const std::string& eventType, const std::string& payloadJson);
    private:
        TrainEventPublisher(const TrainEventPublisher&);
        TrainEventPublisher& operator=(const TrainEventPublisher&);
        std::string m_topic;
        void* m_producer;
    };
}
