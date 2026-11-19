#pragma once
#include <string>

namespace TA_IRS_App
{
    // Operations are provided by the train transport and database layer.
    class ITrainVoiceService
    {
    public:
        virtual ~ITrainVoiceService() {}
        virtual std::string handle(const std::string& method, const std::string& path,
                                   const std::string& body, int& status) = 0;
    };

    class TrainRestController
    {
    public:
        explicit TrainRestController(ITrainVoiceService* service);
        std::string handle(const std::string& method, const std::string& path,
                           const std::string& body, int& status) const;
    private:
        ITrainVoiceService* m_service;
    };
}
