#include "TrainVoiceTransport.h"
#include "TrainEventConverter.h"
#include "TrainEventPublisher.h"
#include "TrainVoiceAccessFactory.h"
#include "core/utilities/src/DebugUtil.h"

namespace TA_IRS_App
{
    TrainVoiceTransport::TrainVoiceTransport(TrainEventPublisher& publisher, const std::string& bindAddress, unsigned short port)
        : TA_Base_Core::Thread("TrainVoiceUdp"), m_publisher(publisher), m_bindAddress(bindAddress), m_port(port),
          m_running(true), m_ready(false), m_sendWaiters(0), m_requests(std::chrono::seconds(60)) {}
    TrainVoiceTransport::~TrainVoiceTransport() { terminateAndWait(); }
    bool TrainVoiceTransport::ready() const { return m_ready && m_running; }
    void TrainVoiceTransport::terminate() { m_running = false; m_ready = false; m_requests.cancelAll(); }
    void TrainVoiceTransport::run()
    {
        while (m_running)
        {
            if (!m_ready)
            {
                std::string error;
                { std::lock_guard<std::mutex> guard(m_socketLock); m_ready = m_socket.open(m_bindAddress, m_port, error); }
                if (!m_ready)
                {
                    LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Train UDP bind failed: %s", error.c_str());
                    TA_Base_Core::Thread::sleep(1000);
                    continue;
                }
            }
            std::vector<uint8_t> bytes;
            std::string source, error;
            uint16_t sourcePort = 0;
            bool received = false;
            { std::lock_guard<std::mutex> guard(m_socketLock); received = m_socket.receive(bytes, source, sourcePort, 50, error); }
            if (received && m_running) handleFrame(bytes, source);
            else if (!error.empty())
            {
                LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Train UDP receive failed: %s", error.c_str());
                m_ready = false;
            }
            while (m_sendWaiters > 0 && m_running) TA_Base_Core::Thread::sleep(1);
        }
        std::lock_guard<std::mutex> guard(m_socketLock);
        m_socket.close();
    }
    TrainRequestManager::WaitResult TrainVoiceTransport::request(const TrainFrame& requestFrame, const std::string& ip,
        unsigned short port, TrainFrame& reply, std::string& error)
    {
        error.clear();
        if (requestFrame.type != 0x01 || requestFrame.data.size() != 4)
        { error = "Invalid audio request"; return TrainRequestManager::Cancelled; }
        if (!ready()) { error = "Train transport unavailable"; return TrainRequestManager::Cancelled; }
        TrainRequestManager::TicketPtr ticket = m_requests.reserve(requestFrame.destinationTrainId, requestFrame.data[0], requestFrame.destinationDevice);
        if (!ticket) { error = "No sequence available"; return TrainRequestManager::Cancelled; }
        TrainFrame frame = requestFrame;
        frame.sequence = ticket->sequence;
        std::vector<uint8_t> bytes;
        if (!TrainProtocol::encode(frame, bytes, error)) { m_requests.cancel(ticket); return TrainRequestManager::Cancelled; }
        ++m_sendWaiters;
        {
            std::lock_guard<std::mutex> guard(m_socketLock);
            --m_sendWaiters;
            if (!ready() || !m_socket.send(ip, port, bytes, error))
            { m_requests.cancel(ticket); if (error.empty()) error = "Train transport unavailable"; return TrainRequestManager::Cancelled; }
        }
        return m_requests.wait(ticket, std::chrono::seconds(5), reply);
    }
    bool TrainVoiceTransport::send(const TrainFrame& input, const std::string& ip, unsigned short port, std::string& error)
    {
        if (input.type != 0x04 && input.type != 0x25 && input.type != 0x0a && input.type != 0x0b) { error = "Unsupported unacknowledged message"; return false; }
        if (!ready()) { error = "Train transport unavailable"; return false; }
        TrainRequestManager::TicketPtr ticket = m_requests.reserve(input.destinationTrainId, 0, input.destinationDevice);
        if (!ticket) { error = "No sequence available"; return false; }
        TrainFrame frame = input;
        frame.sequence = ticket->sequence;
        std::vector<uint8_t> bytes;
        if (!TrainProtocol::encode(frame, bytes, error)) { m_requests.cancel(ticket); return false; }
        bool sent = false;
        ++m_sendWaiters;
        {
            std::lock_guard<std::mutex> guard(m_socketLock);
            --m_sendWaiters;
            if (ready()) sent = m_socket.send(ip, port, bytes, error);
            else error = "Train transport unavailable";
        }
        m_requests.cancel(ticket);
        return sent;
    }
    void TrainVoiceTransport::handleFrame(const std::vector<uint8_t>& bytes, const std::string& sourceIp)
    {
        TrainFrame frame;
        std::string error;
        if (!TrainProtocol::decode(bytes, frame, error) || frame.destinationTrainId != 0 ||
            frame.destinationDevice[0] != 1 || frame.destinationDevice[1] != 0x80) return;
        try
        {
            const std::vector<TrainVoiceDevice> devices = TrainVoiceAccessFactory::getInstance().getDevices();
            bool known = false;
            for (std::vector<TrainVoiceDevice>::const_iterator it = devices.begin(); it != devices.end(); ++it)
                if (it->trainId == frame.sourceTrainId && it->deviceType == "TCCU" && it->ipAddress == sourceIp) { known = true; break; }
            if (!known) return;
            if (frame.type == 0x02 && m_requests.receive(frame) != TrainRequestManager::ResetEvent) return;
            std::string eventType, payload;
            if (!TrainEventConverter::convert(frame, eventType, payload, error)) return;
            if (eventType == "DriverCallRequested" || (eventType == "PassengerIntercomStateChanged" && frame.data[0] == 1))
            {
                const uint8_t expectedType = eventType == "DriverCallRequested" ? 0x01 : 0x03;
                if (frame.sourceDevice[1] != expectedType || frame.sourceDevice[0] == 0 || frame.sourceDevice[0] == 255)
                {
                    LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Invalid source device for train call request");
                    return;
                }
                const unsigned long sourceId = (static_cast<unsigned long>(frame.sourceDevice[1]) << 8) | frame.sourceDevice[0];
                try { TrainVoiceAccessFactory::getInstance().addPendingRequest(frame.sourceTrainId,
                    eventType == "DriverCallRequested" ? "DRIVER" : "PASSENGER", sourceId); }
                catch (...) { LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Failed to save pending train call request"); }
            }
            if (!m_publisher.publish(eventType, payload))
                LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Unable to queue train event %s", eventType.c_str());
        }
        catch (...) { LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Train event lookup failed"); }
    }
}
