#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "httplib.h"
#include "TrainRestServer.h"
#include "core/utilities/src/DebugUtil.h"

namespace TA_IRS_App
{
    TrainRestServer::TrainRestServer(const std::string& bindAddress, unsigned short port, TrainRestController& controller)
        : TA_Base_Core::Thread("TrainVoiceRestServer"), m_port(port), m_bindAddress(bindAddress), m_controller(controller), m_server(new httplib::Server())
    {
        const auto handler = [this](const httplib::Request& request, httplib::Response& response) {
            int status = 200;
            const std::string body = m_controller.handle(request.method, request.target, request.body, status);
            response.status = status;
            response.set_content(body, "application/json");
        };
        m_server->Get(".*", handler);
        m_server->Post(".*", handler);
        m_server->Delete(".*", handler);
        m_server->set_payload_max_length(1024 * 1024);
    }
    TrainRestServer::~TrainRestServer() { terminate(); delete m_server; }
    void TrainRestServer::run()
    {
        if (!m_server->listen(m_bindAddress.c_str(), m_port))
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "TrainVoiceAgent REST listen failed on %u", m_port);
    }
    void TrainRestServer::terminate() { if (m_server != 0) m_server->stop(); }
}
