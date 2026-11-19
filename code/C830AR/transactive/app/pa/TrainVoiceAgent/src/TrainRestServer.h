#pragma once
#include "core/threads/src/Thread.h"
#include "TrainRestController.h"
namespace httplib { class Server; }
namespace TA_IRS_App
{
    class TrainRestServer : public TA_Base_Core::Thread
    {
    public:
        TrainRestServer(const std::string& bindAddress, unsigned short port, TrainRestController& controller);
        virtual ~TrainRestServer();
        virtual void run();
        virtual void terminate();
    private:
        unsigned short m_port;
        std::string m_bindAddress;
        TrainRestController& m_controller;
        httplib::Server* m_server;
    };
}
