#include "TrainUdpSocket.h"
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#endif

namespace TA_IRS_App
{
#ifdef _WIN32
    typedef SOCKET NativeSocket;
    static void closeSocket(NativeSocket socket) { closesocket(socket); }
#else
    typedef int NativeSocket;
    static void closeSocket(NativeSocket socket) { ::close(socket); }
#endif
    TrainUdpSocket::TrainUdpSocket() : m_socket(-1), m_networkInitialised(false) {}
    TrainUdpSocket::~TrainUdpSocket() { close(); }
    bool TrainUdpSocket::open(const std::string& localAddress, uint16_t port, std::string& error)
    {
        close();
        error.clear();
#ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { error = "WSAStartup failed"; return false; }
        m_networkInitialised = true;
#endif
        NativeSocket fd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (fd == static_cast<NativeSocket>(-1)) { error = "UDP socket creation failed"; close(); return false; }
        m_socket = static_cast<intptr_t>(fd);
        sockaddr_in local = {};
        local.sin_family = AF_INET;
        local.sin_port = htons(port);
        if (inet_pton(AF_INET, localAddress.c_str(), &local.sin_addr) != 1 ||
            ::bind(fd, reinterpret_cast<const sockaddr*>(&local), sizeof(local)) != 0)
        { error = "Invalid local address or UDP bind failed"; close(); return false; }
        return true;
    }
    bool TrainUdpSocket::send(const std::string& address, uint16_t port, const std::vector<uint8_t>& bytes, std::string& error)
    {
        error.clear();
        if (m_socket == -1 || port == 0 || bytes.empty() || bytes.size() > 65507) { error = "Invalid UDP send request"; return false; }
        sockaddr_in dest = {};
        dest.sin_family = AF_INET;
        dest.sin_port = htons(port);
        if (inet_pton(AF_INET, address.c_str(), &dest.sin_addr) != 1) { error = "Invalid UDP destination"; return false; }
        const int written = static_cast<int>(sendto(static_cast<NativeSocket>(m_socket), reinterpret_cast<const char*>(bytes.data()),
            static_cast<int>(bytes.size()), 0, reinterpret_cast<const sockaddr*>(&dest), sizeof(dest)));
        if (written != static_cast<int>(bytes.size())) { error = "UDP send failed"; return false; }
        return true;
    }
    bool TrainUdpSocket::receive(std::vector<uint8_t>& bytes, std::string& address, uint16_t& port, unsigned int timeoutMs, std::string& error)
    {
        bytes.clear(); address.clear(); port = 0; error.clear();
        if (m_socket == -1) { error = "UDP socket is closed"; return false; }
        const NativeSocket fd = static_cast<NativeSocket>(m_socket);
#ifndef _WIN32
        if (fd >= FD_SETSIZE) { error = "UDP socket exceeds select limit"; return false; }
#endif
        fd_set readSet; FD_ZERO(&readSet); FD_SET(fd, &readSet);
        timeval timeout = {}; timeout.tv_sec = static_cast<long>(timeoutMs / 1000); timeout.tv_usec = static_cast<long>((timeoutMs % 1000) * 1000);
#ifdef _WIN32
        const int ready = select(0, &readSet, 0, 0, &timeout);
#else
        const int ready = select(fd + 1, &readSet, 0, 0, &timeout);
#endif
        if (ready == 0) return false;
        if (ready < 0) { error = "UDP select failed"; return false; }
        std::vector<uint8_t> buffer(65535);
        sockaddr_in source = {};
#ifdef _WIN32
        int sourceLength = sizeof(source);
#else
        socklen_t sourceLength = sizeof(source);
#endif
        int count = static_cast<int>(recvfrom(fd, reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()), 0,
            reinterpret_cast<sockaddr*>(&source), &sourceLength));
        if (count < 0) { error = "UDP receive failed"; return false; }
        char text[INET_ADDRSTRLEN] = {};
        if (inet_ntop(AF_INET, &source.sin_addr, text, sizeof(text)) == 0) { error = "Invalid UDP source address"; return false; }
        bytes.assign(buffer.begin(), buffer.begin() + count); address = text; port = ntohs(source.sin_port);
        return true;
    }
    void TrainUdpSocket::close()
    {
        if (m_socket != -1) { closeSocket(static_cast<NativeSocket>(m_socket)); m_socket = -1; }
#ifdef _WIN32
        if (m_networkInitialised) { WSACleanup(); m_networkInitialised = false; }
#endif
    }
}
