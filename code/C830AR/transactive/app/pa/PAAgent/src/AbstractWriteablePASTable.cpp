/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/AbstractWriteablePASTable.cpp $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "AbstractWriteablePASTable.h"
#include "PASWriteEvent.h"
#include "CachedConfig.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include <thread>
#include <chrono>
#include <string>

using namespace std::literals;

namespace TA_IRS_App
{
    AbstractWriteablePASTable::AbstractWriteablePASTable(unsigned long tableNumber)
        : AbstractPASTable(tableNumber)
    {
    }

    void AbstractWriteablePASTable::destroyCurrentWriteEvents(TA_IRS_Bus::Scheduler& socketScheduler)
    {
        LOG_SCOPE("AbstractWriteablePASTable::destroyCurrentWriteEvents");

        TA_Base_Core::ThreadGuard guard(m_writeEventsLock);

        if (m_currentWriteEvents.empty())
        {
            return;
        }

        for (auto event : m_currentWriteEvents)
        {
            socketScheduler.remove(event);
        }

        std::this_thread::sleep_for(100ms);

        // Delete all the write events and clear the vector

        for (auto event : m_currentWriteEvents)
        {
            if (socketScheduler.currentEvent() == event)
            {
                LOG_DEBUG("destroyCurrentWriteEvents(): %p is on schedule, will be delete later", event);
                continue;
            }

            LOG_DEBUG("destroyCurrentWriteEvents(): delete event %p", event);
            delete event;
        }

        m_currentWriteEvents.clear();
    }

    void AbstractWriteablePASTable::repostCurrentWriteEvents(TA_IRS_Bus::Scheduler& socketScheduler)
    {
        LOG_SCOPE("AbstractWriteablePASTable::repostCurrentWriteEvents");

        TA_Base_Core::ThreadGuard guard(m_writeEventsLock);

        if (m_currentWriteEvents.empty())
        {
            return;
        }

        for (auto event : m_currentWriteEvents)
        {
            socketScheduler.remove(event);
        }

        std::this_thread::sleep_for(100ms);

        // Repost all the current write events with an new expiry time into the scheduler.
        // This means re-creating all the events as expiry time can only be set at
        // point of creation.
        std::vector<PASWriteEvent*> newEvents;

        ACE_Time_Value tv(0, CachedConfig::getInstance()->getWorkerThreadsSleepTimeInMSecs() * 1000);
        tv += ACE_OS::gettimeofday();

        for (auto event : m_currentWriteEvents)
        {
            PASWriteEvent* newEvent = event->recreateEvent(tv);
            newEvents.push_back(newEvent);
            socketScheduler.post(newEvent);

            if (socketScheduler.currentEvent() == event)
            {
                LOG_DEBUG("repostCurrentWriteEvents(): %p is on schedule, will be delete later", event);
                continue;
            }

            LOG_DEBUG("repostCurrentWriteEvents(): delete event %p", event);
            delete event;
        }

        m_currentWriteEvents = newEvents;
    }

    bool AbstractWriteablePASTable::addWriteEvent(PASWriteEvent& event)
    {
        TA_Base_Core::ThreadGuard guard(m_writeEventsLock);

        // add a write event to the list of current write events unless
        // it already exists in the list.
        auto itr = std::find(m_currentWriteEvents.begin(), m_currentWriteEvents.end(), &event);

        if (itr == m_currentWriteEvents.end())
        {
            m_currentWriteEvents.push_back(&event);
            return true;
        }

        return false;
    }

    bool AbstractWriteablePASTable::removeWriteEvent(PASWriteEvent& event)
    {
        LOG_SCOPE("AbstractWriteablePASTable::removeWriteEvent");

        TA_Base_Core::ThreadGuard guard(m_writeEventsLock);

        // Remove the event from the current write event list unless it does
        // not exist in the list.
        auto itr = std::find(m_currentWriteEvents.begin(), m_currentWriteEvents.end(), &event);

        if (itr != m_currentWriteEvents.end())
        {
            PASWriteEvent* event = *itr;
            m_currentWriteEvents.erase(itr);
            delete event;
            return true;
        }

        return false;
    }
}
