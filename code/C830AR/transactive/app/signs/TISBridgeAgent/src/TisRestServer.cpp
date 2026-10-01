#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "httplib.h"
#include "TisRestServer.h"

#include "core/utilities/src/DebugUtil.h"

namespace
{
    void setCorsHeaders(httplib::Response& response)
    {
        response.set_header("Access-Control-Allow-Origin", "*");
        response.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        response.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    }
}

namespace TA_IRS_App
{
    TisRestServer::TisRestServer(unsigned short port, TisRestController& controller)
        : m_port(port),
          m_controller(controller),
          m_running(false),
          m_server(new httplib::Server())
    {
        registerRoutes();
    }

    TisRestServer::~TisRestServer()
    {
        terminate();
        delete m_server;
        m_server = 0;
    }

    void TisRestServer::registerRoutes()
    {
        m_server->Get(".*", [this](const httplib::Request& request, httplib::Response& response) {
            int statusCode = 200;
            response.set_content(m_controller.handle("GET", request.target, "", statusCode), "application/json");
            response.status = statusCode;
            setCorsHeaders(response);
        });

        m_server->Post(".*", [this](const httplib::Request& request, httplib::Response& response) {
            int statusCode = 200;
            response.set_content(m_controller.handle("POST", request.target, request.body, statusCode), "application/json");
            response.status = statusCode;
            setCorsHeaders(response);
        });

        m_server->Options(".*", [](const httplib::Request&, httplib::Response& response) {
            response.status = 204;
            setCorsHeaders(response);
        });
    }

    void TisRestServer::run()
    {
        m_running = true;
        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
            "TISBridgeAgent REST API listening on port %u", m_port);
        if (!m_server->listen("0.0.0.0", m_port))
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
                "TISBridgeAgent REST API failed to listen on port %u", m_port);
        }
        m_running = false;
    }

    void TisRestServer::terminate()
    {
        if (m_running && m_server != 0) m_server->stop();
        m_running = false;
    }
}
