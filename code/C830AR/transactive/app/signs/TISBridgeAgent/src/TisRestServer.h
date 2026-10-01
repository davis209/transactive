#pragma once

#include "TisRestController.h"
#include "core/threads/src/Thread.h"

namespace httplib
{
    class Server;
}

namespace TA_IRS_App
{
    class TisRestServer : public TA_Base_Core::Thread
    {
    public:
        TisRestServer(unsigned short port, TisRestController& controller);
        virtual ~TisRestServer();

        virtual void run();
        virtual void terminate();

    private:
        void registerRoutes();

        unsigned short m_port;
        TisRestController& m_controller;
        bool m_running;
        httplib::Server* m_server;
    };
}
