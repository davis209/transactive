#include "TrainRequestManager.h"

namespace TA_IRS_App
{
    TrainRequestManager::TrainRequestManager(std::chrono::milliseconds protection)
        : m_protection(protection.count() < 0 ? std::chrono::milliseconds(0) : protection) {}

    TrainRequestManager::TicketPtr TrainRequestManager::reserve(uint8_t trainId, uint8_t function, const uint8_t sourceDevice[2])
    {
        if (trainId == 0 || trainId == 255 || sourceDevice == 0) return TicketPtr();
        std::lock_guard<std::mutex> guard(m_lock);
        const Clock::time_point now = Clock::now();
        for (unsigned int i = 0; i < 256; ++i)
        {
            const uint8_t sequence = static_cast<uint8_t>((m_next[trainId] + i) & 0xff);
            const Key key(trainId, sequence);
            if (m_pending.find(key) != m_pending.end()) continue;
            std::map<Key, Clock::time_point>::iterator blocked = m_protectedUntil.find(key);
            if (blocked != m_protectedUntil.end() && blocked->second > now) continue;
            if (blocked != m_protectedUntil.end()) m_protectedUntil.erase(blocked);
            TicketPtr ticket(new Ticket());
            ticket->trainId = trainId;
            ticket->sequence = sequence;
            ticket->function = function;
            ticket->expectedSource[0] = sourceDevice[0];
            ticket->expectedSource[1] = sourceDevice[1];
            ticket->done = false;
            ticket->cancelled = false;
            m_pending[key] = ticket;
            m_next[trainId] = (static_cast<unsigned int>(sequence) + 1) & 0xff;
            return ticket;
        }
        return TicketPtr();
    }

    TrainRequestManager::ReplyResult TrainRequestManager::receive(const TrainFrame& frame)
    {
        uint8_t function, result, deviceId, deviceType;
        std::string error;
        if (!TrainProtocol::parseAudioReply(frame, function, result, deviceId, deviceType, error)) return Invalid;
        // Reset is always an asynchronous event, even when its sequence matches a request.
        if (result == 2) return ResetEvent;
        std::lock_guard<std::mutex> guard(m_lock);
        std::map<Key, TicketPtr>::iterator it = m_pending.find(Key(frame.sourceTrainId, frame.sequence));
        if (it == m_pending.end()) return Unmatched;
        TicketPtr ticket = it->second;
        if (ticket->done || ticket->cancelled || frame.destinationTrainId != 0 || function != ticket->function ||
            frame.sourceDevice[0] != ticket->expectedSource[0] || frame.sourceDevice[1] != ticket->expectedSource[1]) return Unmatched;
        ticket->reply = frame;
        ticket->done = true;
        ticket->condition.notify_all();
        return Matched;
    }

    void TrainRequestManager::release(const TicketPtr& ticket)
    {
        const Key key(ticket->trainId, ticket->sequence);
        std::map<Key, TicketPtr>::iterator it = m_pending.find(key);
        if (it != m_pending.end() && it->second == ticket)
        {
            m_pending.erase(it);
            m_protectedUntil[key] = Clock::now() + m_protection;
        }
    }

    TrainRequestManager::WaitResult TrainRequestManager::wait(const TicketPtr& ticket, std::chrono::milliseconds timeout, TrainFrame& reply)
    {
        if (!ticket) return Cancelled;
        std::unique_lock<std::mutex> lock(m_lock);
        ticket->condition.wait_for(lock, timeout, [&] { return ticket->done || ticket->cancelled; });
        WaitResult result = ticket->done ? Completed : (ticket->cancelled ? Cancelled : TimedOut);
        if (result == Completed) reply = ticket->reply;
        release(ticket);
        return result;
    }

    void TrainRequestManager::cancel(const TicketPtr& ticket)
    {
        if (!ticket) return;
        std::lock_guard<std::mutex> guard(m_lock);
        if (!ticket->done) ticket->cancelled = true;
        release(ticket);
        ticket->condition.notify_all();
    }
    void TrainRequestManager::cancelAll()
    {
        std::lock_guard<std::mutex> guard(m_lock);
        while (!m_pending.empty())
        {
            TicketPtr ticket = m_pending.begin()->second;
            if (!ticket->done) ticket->cancelled = true;
            release(ticket);
            ticket->condition.notify_all();
        }
    }
}
