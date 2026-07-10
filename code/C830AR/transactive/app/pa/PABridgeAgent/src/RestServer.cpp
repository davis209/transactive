#include "RestServer.h"

#include "core/utilities/src/DebugUtil.h"

#include "httplib.h"

namespace TA_IRS_App
{
    RestServer::RestServer(unsigned short port, PaRestController& controller)
        : TA_Base_Core::Thread("PABridgeRestServer"),
          m_port(port),
          m_controller(controller),
          m_running(false),
          m_server(new httplib::Server())
    {
        registerRoutes();
    }

    RestServer::~RestServer()
    {
        terminate();
        delete m_server;
        m_server = 0;
    }

    void RestServer::registerRoutes()
    {
        m_server->Get(".*", [this](const httplib::Request& req, httplib::Response& res) {
            int statusCode = 200;
            const std::string body = m_controller.handle("GET", req.target, "", statusCode);
            res.status = statusCode;
            res.set_content(body, "application/json");
        });

        m_server->Post(".*", [this](const httplib::Request& req, httplib::Response& res) {
            int statusCode = 200;
            const std::string body = m_controller.handle("POST", req.target, req.body, statusCode);
            res.status = statusCode;
            res.set_content(body, "application/json");
        });

        m_server->Options(".*", [](const httplib::Request& req, httplib::Response& res) {
            res.status = 204;
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        });
    }

    void RestServer::run()
    {
        m_running = true;
        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
            "PABridgeAgent REST API listening on port %u", m_port);

        if (!m_server->listen("0.0.0.0", m_port))
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
                "PABridgeAgent REST API failed to listen on port %u", m_port);
        }
        m_running = false;
    }

    void RestServer::terminate()
    {
        if (m_running && m_server != 0)
        {
            m_server->stop();
        }
        m_running = false;
    }
}
