#pragma once

#include "TrainProtocol.h"
#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>

namespace TA_IRS_App
{
    class TrainRequestManager
    {
    public:
        enum ReplyResult { Matched, ResetEvent, Unmatched, Invalid };
        enum WaitResult { Completed, TimedOut, Cancelled };
        struct Ticket
        {
            uint8_t trainId;
            uint8_t sequence;
        private:
            friend class TrainRequestManager;
            uint8_t function;
            uint8_t expectedSource[2];
            bool done;
            bool cancelled;
            TrainFrame reply;
            std::condition_variable condition;
        };
        typedef std::shared_ptr<Ticket> TicketPtr;

        // A sequence remains unavailable during the late-reply protection period.
        explicit TrainRequestManager(std::chrono::milliseconds protection);
        TicketPtr reserve(uint8_t trainId, uint8_t function, const uint8_t sourceDevice[2]);
        ReplyResult receive(const TrainFrame& frame);
        WaitResult wait(const TicketPtr& ticket, std::chrono::milliseconds timeout, TrainFrame& reply);
        void cancel(const TicketPtr& ticket);
        void cancelAll();
    private:
        typedef std::pair<uint8_t, uint8_t> Key;
        typedef std::chrono::steady_clock Clock;
        void release(const TicketPtr& ticket);
        std::mutex m_lock;
        std::map<Key, TicketPtr> m_pending;
        std::map<Key, Clock::time_point> m_protectedUntil;
        std::map<uint8_t, unsigned int> m_next;
        std::chrono::milliseconds m_protection;
    };
}
