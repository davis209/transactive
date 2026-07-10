/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table100.cpp $
 * @author:  Ripple
 * @version: $Revision: #8 $
 *
 * Last modification: $DateTime: 2025/06/24 11:25:13 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table100.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "CachedConfig.h"
#include "PASConnection.h"
#include "core/data_access_interface/src/LocationAccessFactoryEx.h"
#include "core/message/src/MessagePublicationManager.h"
#include "core/message/types/PAAgentComms_MessageTypes.h"
#include "core/message/src/CommsMessageSender.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    Table100::Table100()
        : AbstractPASTable(100)
    {
        FUNCTION_ENTRY("Constructor");

        // Initialise StationPriorities sequence length
        m_priorityScheme.length(MAXSTNID);

        for (int station = 0; station < MAXSTNID; ++station)
        {
            // Initialise ZonePriorities sequence length
            m_priorityScheme[station].length(MAXZONEID);

            for (int zone = 0; zone < MAXZONEID; ++zone)
            {
                // Initialise ModePriorities sequence length
                m_priorityScheme[station][zone].length(MAXMODEID);
            }
        }

        FUNCTION_EXIT;
    }

    void Table100::processRead()
    {
        LOG_SCOPE("Table100::processRead");
        ThreadGuard guard(m_lock);

        unsigned long offset = 0;

        for (int station = 0; station < MAXSTNID; ++station)
        {
            for (int zone = 0; zone < MAXZONEID; ++zone)
            {
                for (int mode = 0; mode < MAXMODEID; ++mode)
                {
                    m_priorityScheme[station][zone][mode] = PasHelpers::instance().get8bit(m_buffer, offset++);
                }
            }
        }
    }

    void Table100::notifyIscs()
    {
        LOG_SCOPE("Table100::notifyIscs");

        // Alert operators to the new scheme, only if updated via table 560
        // TD 15128
        /*PasHelpers::instance().raisePASPrioritySchemeUpdateNonUniqueAlarm();*/
        PasHelpers::instance().submitPaPrioritySchemeUpdateAuditMessage();
        // TD 15128

        try
        {
            // Update any connected PA Managers
            LOG_INFO("Sending PrioritySchemeUpdate message to subscribers");

            CORBA::Any data;

            data <<= getPrioritySchemeCopy();

            m_paAgentCommsSender->sendCommsMessage(
                PAAgentComms::PrioritySchemeUpdate,                     // Context::Type
                CachedConfig::getInstance()->getAgentKey(),             // EntityKey
                data,                                                   // Data
                CachedConfig::getInstance()->getAgentSubsystemKey(),    // Subsystem
                CachedConfig::getInstance()->getAgentLocationKey(),     // LocationKey
                NULL);
        }
        catch (...)
        {
            // Not expecting any exceptions

            // Note that TA_Base_Core::PasConnectionException will never occur as at this point
            // we have already read the table
        }
    }

    TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme* Table100::getPrioritySchemeCopy()
        //throw ( TA_Base_Core::PasConnectionException )
    {
        TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme* newPriorityScheme = new TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme;

        ThreadGuard guard(m_lock);

        *newPriorityScheme = m_priorityScheme;

        return newPriorityScheme;
    }

    ReadTable100::ReadTable100(Table100& table,
                               Table560& table560,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASReadEvent(socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_table560(table560)
    {
    }

    void ReadTable100::readTable()
    {
        LOG_SCOPE("ReadTable100::readTable");
        ThreadGuard guard(m_table.m_lock);

        if (is_depot())
        {
            PASConnection::getInstance()->readTable100Depot(m_table.m_buffer,
                                                            m_table.BUFFER_SIZE);
        }
        else
        {
            PASConnection::getInstance()->readTable(m_table.TABLE_NUMBER,
                                                    m_table.m_buffer,
                                                    m_table.BUFFER_SIZE);
        }

        m_processScheduler.post(new ProcessTable100(m_table));
        m_table560.resetFlags(this,
                              Table560::TABLE_560_TABLE_100_FLAG_A_BIT_OFFSET,
                              Table560::TABLE_560_TABLE_100_FLAG_B_BIT_OFFSET);
    }

    ProcessTable100::ProcessTable100(Table100& table)
        : IEvent()
        , m_table(table)
    {
    }

    void ProcessTable100::consume()
    {
        LOG_SCOPE("ProcessTable100::consume");
        m_table.processRead();
        PasTableReadCounter::instance().increase(m_table.TABLE_NUMBER);
        m_table.notifyIscs();
        delete this;
    }

    void ProcessTable100::cancel()
    {
        LOG_SCOPE("ProcessTable100::cancel");
        delete this;
    }
}
