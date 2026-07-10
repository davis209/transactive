#pragma once

#include <string>

#include "PaRestController.h"
#include "core/threads/src/Thread.h"

namespace httplib
{
    class Server;
}

namespace TA_IRS_App
{
    class RestServer : public TA_Base_Core::Thread
    {
    public:
        RestServer(unsigned short port, PaRestController& controller);
        virtual ~RestServer();

        virtual void run();
        virtual void terminate();

    private:
        void registerRoutes();

        unsigned short m_port;
        PaRestController& m_controller;
        bool m_running;
        httplib::Server* m_server;
    };
}
