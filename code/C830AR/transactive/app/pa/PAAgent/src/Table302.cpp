/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table302.cpp $
 * @author:  Ripple
 * @version: $Revision: #5 $
 *
 * Last modification: $DateTime: 2025/06/24 11:25:13 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table302.h"
#include "PasHelpers.h"
#include "PASConnection.h"
#include "AbstractBroadcastCircularList.h"
#include "PasTableManager.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/MakeNameValuePairsString.h"
#include <time.h>

using namespace boost::describe::operators;
using namespace TA_Base_Core;

namespace TA_IRS_App
{
    Table302::Table302()
        : AbstractPASTable(302)
        , m_readWait()
        , m_processWait()
    {
        FUNCTION_ENTRY("Constructor");

        m_messageSequenceStatuses.resize(MAXMSGSEQ);

        FUNCTION_EXIT;
    }

    void Table302::processRead()
    {
        LOG_SCOPE("Table302::processRead");

        ThreadGuard guard(m_lock);

        unsigned long offset = 0;

        for (int sequenceId = 1; sequenceId <= MAXMSGSEQ; ++sequenceId)
        {
            LOG_SCOPE(boost::format("SEQUENCE[%d]") % sequenceId);
#if 0

            m_messageSequenceStatuses[sequenceId - 1].m_announceId = PasHelpers::instance().get8bit(m_buffer, offset++);
            // jeffrey++ TES712
            //m_messageSequenceStatuses[sequenceId-1].m_priority = PasHelpers::instance().get8bit(m_buffer, offset++);
            m_messageSequenceStatuses[sequenceId - 1].m_sourceid = PasHelpers::instance().get8bit(m_buffer, offset++);
            // ++jeffrey TES712
            m_messageSequenceStatuses[sequenceId - 1].m_status = static_cast<MessageSequenceStatus::EState>(PasHelpers::instance().get8bit(m_buffer, offset++));
#else
            MessageSequenceStatus status;
            status.m_announceId = PasHelpers::instance().get8bit(m_buffer, offset++);
            status.m_sourceid = PasHelpers::instance().get8bit(m_buffer, offset++);
            status.m_status = static_cast<MessageSequenceStatus::EState>(PasHelpers::instance().get8bit(m_buffer, offset++));

            auto& m_status = m_messageSequenceStatuses[sequenceId - 1];
            LOG_DEBUG_IF(m_status != status, "processRead(): %s", nvps(m_status, status));
            m_status = status;
#endif
        }

        m_processWait.broadcast();
    }

    unsigned int Table302::getSequenceId(CORBA::Octet announceId)
    {
        ThreadGuard guard(m_lock);

        for (int sequenceId = 1; sequenceId <= MAXMSGSEQ; ++sequenceId)
        {
            if (m_messageSequenceStatuses[sequenceId - 1].m_announceId == announceId)
            {
                return sequenceId;
            }
        }

        return 0;
    }

    MessageSequenceStatus Table302::getMessageSequenceStatus(int messageSequenceId)
        //throw ( TA_Base_Core::PasConnectionException )
    {
        TA_ASSERT(messageSequenceId > 0 && messageSequenceId <= MAXMSGSEQ, "messageSequenceId is out of range");

        m_readWait.timedWait(CachedConfig::getInstance()->getFastPollRateInMSecs());
        m_processWait.timedWait(CachedConfig::getInstance()->getFastPollRateInMSecs());

        ThreadGuard guard(m_lock);

        return m_messageSequenceStatuses[messageSequenceId - 1];
    }

    int Table302::getNextAvailableMessageSequenceId()
        //throw (TA_Base_Core::NoFreeMessageSequenceIdException, TA_Base_Core::PasConnectionException )
    {
        static int lastSeqId = 0;

        ThreadGuard guard(m_lock);

        for (int count = 0; count < MAXMSGSEQ; ++count)
        {
            if (lastSeqId == MAXMSGSEQ)
            {
                // loop around
                lastSeqId = 1;
            }
            else
            {
                ++lastSeqId;
            }

            if (m_messageSequenceStatuses[lastSeqId - 1].m_announceId == 0)
            {
                // Free to use
                return lastSeqId;
            }
            else
            {
                // Check to see whether we can reset this one
                if (!AbstractBroadcastCircularList::getInstance()->isStationAnnounceIdUsed(
                    m_messageSequenceStatuses[lastSeqId - 1].m_announceId))
                {
                    // PAS thinks this messageId is used but we have no knowledge of it.
                    return lastSeqId;
                }
            }
        }

        // All of the occupied messageSequenceIds are taken.

        TA_THROW(NoFreeMessageSequenceIdException());

        return 0;
    }

    bool Table302::isRunning(int messageSequenceId)
    {
        TA_ASSERT(messageSequenceId > 0 && messageSequenceId <= MAXMSGSEQ, "messageSequenceId is out of range");

        ThreadGuard guard(m_lock);

        return (m_messageSequenceStatuses[messageSequenceId - 1].m_status != MessageSequenceStatus::BroadcastFinishedAbnormally &&
                m_messageSequenceStatuses[messageSequenceId - 1].m_status != MessageSequenceStatus::BroadcastFinishedNormally);
    }

    ReadTable302::ReadTable302(Table302& table,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASReadEvent(socketScheduler, processScheduler, eventSource)
        , m_table(table)
    {
    }

    ReadTable302::ReadTable302(ACE_Time_Value expiryTime,
                               Table302& table,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASReadEvent(expiryTime, socketScheduler, processScheduler, eventSource)
        , m_table(table)
    {
    }

    void ReadTable302::readTable()
    {
        LOG_SCOPE("ReadTable302::readTable");

        ThreadGuard guard(m_table.m_lock);

        PASConnection::getInstance()->readTable(m_table.TABLE_NUMBER,
                                                m_table.m_buffer,
                                                m_table.BUFFER_SIZE);

        m_table.m_readWait.broadcast();

        m_socketScheduler.post(new ReadTable302(getNextTimeToExpire(),
                                                m_table,
                                                m_socketScheduler,
                                                m_processScheduler,
                                                m_pasEventSource));

        m_processScheduler.post(new ProcessTable302(m_table));
    }

    ProcessTable302::ProcessTable302(Table302& table)
        : m_table(table)
    {
    }

    void ProcessTable302::consume()
    {
        LOG_SCOPE("ProcessTable302::consume");

        m_table.processRead();
        PasTableReadCounter::instance().increase(m_table.TABLE_NUMBER);

        delete this;
    }

    void ProcessTable302::cancel()
    {
        LOG_SCOPE("ProcessTable302::cancel");
        delete this;
    }
}
