#include "TrainRestController.h"

namespace TA_IRS_App
{
    TrainRestController::TrainRestController(ITrainVoiceService* service) : m_service(service) {}

    std::string TrainRestController::handle(const std::string& method, const std::string& path,
                                            const std::string& body, int& status) const
    {
        if (method == "GET" && path == "/health")
        {
            if (m_service == 0) { status = 503; return "{\"status\":\"starting\"}"; }
            return m_service->handle(method, path, body, status);
        }
        if (m_service == 0)
        {
            status = 503;
            return "{\"error\":\"train service unavailable\"}";
        }
        return m_service->handle(method, path, body, status);
    }
}
