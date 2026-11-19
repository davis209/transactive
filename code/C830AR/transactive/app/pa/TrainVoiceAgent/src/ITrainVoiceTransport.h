#pragma once
#include "TrainRequestManager.h"

namespace TA_IRS_App
{
    class ITrainVoiceTransport
    {
    public:
        virtual ~ITrainVoiceTransport() {}
        virtual bool ready() const = 0;
        virtual TrainRequestManager::WaitResult request(const TrainFrame& frame, const std::string& ip,
            unsigned short port, TrainFrame& reply, std::string& error) = 0;
        virtual bool send(const TrainFrame& frame, const std::string& ip, unsigned short port, std::string& error) = 0;
    };
}
