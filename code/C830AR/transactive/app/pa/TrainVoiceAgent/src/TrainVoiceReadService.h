#pragma once
#include "TrainRestController.h"
#include "ITrainVoiceTransport.h"

namespace TA_IRS_App
{
    // Read APIs are available while train control and event processing remain disabled.
    class TrainVoiceReadService : public ITrainVoiceService
    {
    public:
        explicit TrainVoiceReadService(ITrainVoiceTransport* transport = 0) : m_transport(transport) {}
        virtual std::string handle(const std::string& method, const std::string& path,
                                   const std::string& body, int& status);
    private:
        ITrainVoiceTransport* m_transport;
    };
}
