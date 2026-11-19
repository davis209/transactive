#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace TA_IRS_App
{
    // A single owner must serialize open/receive/send/close calls.
    class TrainUdpSocket
    {
    public:
        TrainUdpSocket();
        ~TrainUdpSocket();
        bool open(const std::string& localAddress, uint16_t port, std::string& error);
        bool send(const std::string& address, uint16_t port, const std::vector<uint8_t>& bytes, std::string& error);
        // Returns false on timeout or error; timeout leaves error empty.
        bool receive(std::vector<uint8_t>& bytes, std::string& address, uint16_t& port, unsigned int timeoutMs, std::string& error);
        void close();
    private:
        TrainUdpSocket(const TrainUdpSocket&);
        TrainUdpSocket& operator=(const TrainUdpSocket&);
        intptr_t m_socket;
        bool m_networkInitialised;
    };
}
