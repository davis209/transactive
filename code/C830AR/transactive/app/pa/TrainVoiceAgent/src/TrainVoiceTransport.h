#pragma once
#include "core/threads/src/Thread.h"
#include "TrainProtocol.h"
#include "TrainRequestManager.h"
#include "TrainUdpSocket.h"
#include "ITrainVoiceTransport.h"
#include <atomic>
#include <mutex>
#include <string>

namespace TA_IRS_App
{
    class TrainEventPublisher;
    class TrainVoiceTransport : public TA_Base_Core::Thread, public ITrainVoiceTransport
    {
    public:
        TrainVoiceTransport(TrainEventPublisher& publisher, const std::string& bindAddress, unsigned short port);
        virtual ~TrainVoiceTransport();
        virtual void run();
        virtual void terminate();
        void setKafkaPublishingEnabled(bool enabled);
        bool ready() const;
        TrainRequestManager::WaitResult request(const TrainFrame& frame, const std::string& ip,
            unsigned short port, TrainFrame& reply, std::string& error);
        bool send(const TrainFrame& frame, const std::string& ip, unsigned short port, std::string& error);
    private:
        void handleFrame(const std::vector<uint8_t>& bytes, const std::string& sourceIp);
        TrainEventPublisher& m_publisher;
        const std::string m_bindAddress;
        const unsigned short m_port;
        std::atomic<bool> m_running, m_ready;
        std::atomic<unsigned int> m_sendWaiters;
        std::mutex m_socketLock;
        std::mutex m_publisherLock;
        bool m_kafkaPublishingEnabled;
        TrainUdpSocket m_socket;
        TrainRequestManager m_requests;
    };
}
