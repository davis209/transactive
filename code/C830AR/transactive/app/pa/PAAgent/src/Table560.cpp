/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table560.cpp $
 * @author:  Ripple
 * @version: $Revision: #7 $
 *
 * Last modification: $DateTime: 2025/06/24 11:25:13 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table560.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "PASConnection.h"
#include "Table100.h"
#include "Table202.h"
#include "Table204.h"
#include "Table1204.h"
#include "Table303.h"
#include "Table1303.h"
#include "Table550.h"
#include "Table570.h"
#include "Table590.h"
#include "Table603.h"
#include "Table604.h"
#include "Table610.h"
#include "Table1610.h"
#include "CachedConfig.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/containers/Vector.h"

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    const size_t Table560::TABLE_560_TABLE_100_FLAG_A_BIT_OFFSET    = 0;
    const size_t Table560::TABLE_560_TABLE_100_FLAG_B_BIT_OFFSET    = 1;
    const size_t Table560::TABLE_560_TABLE_202_FLAG_A_BIT_OFFSET    = 2;
    const size_t Table560::TABLE_560_TABLE_202_FLAG_B_BIT_OFFSET    = 3;
    const size_t Table560::TABLE_560_TABLE_204_FLAG_A_BIT_OFFSET    = 4;
    const size_t Table560::TABLE_560_TABLE_204_FLAG_B_BIT_OFFSET    = 5;
    const size_t Table560::TABLE_560_TABLE_1204_FLAG_A_BIT_OFFSET   = 6;
    const size_t Table560::TABLE_560_TABLE_1204_FLAG_B_BIT_OFFSET   = 7;
    const size_t Table560::TABLE_560_TABLE_303_FLAG_A_BIT_OFFSET    = 8;
    const size_t Table560::TABLE_560_TABLE_303_FLAG_B_BIT_OFFSET    = 9;
    const size_t Table560::TABLE_560_TABLE_1303_FLAG_A_BIT_OFFSET   = 10;
    const size_t Table560::TABLE_560_TABLE_1303_FLAG_B_BIT_OFFSET   = 11;
    const size_t Table560::TABLE_560_TABLE_550_FLAG_A_BIT_OFFSET    = 12;
    const size_t Table560::TABLE_560_TABLE_550_FLAG_B_BIT_OFFSET    = 13;
    const size_t Table560::TABLE_560_TABLE_570_FLAG_A_BIT_OFFSET    = 14;
    const size_t Table560::TABLE_560_TABLE_570_FLAG_B_BIT_OFFSET    = 15;
    const size_t Table560::TABLE_560_TABLE_590_FLAG_A_BIT_OFFSET    = 16;
    const size_t Table560::TABLE_560_TABLE_590_FLAG_B_BIT_OFFSET    = 17;
    const size_t Table560::TABLE_560_TABLE_603_FLAG_A_BIT_OFFSET    = 18;
    const size_t Table560::TABLE_560_TABLE_603_FLAG_B_BIT_OFFSET    = 19;
    const size_t Table560::TABLE_560_TABLE_604_FLAG_A_BIT_OFFSET    = 20;
    const size_t Table560::TABLE_560_TABLE_604_FLAG_B_BIT_OFFSET    = 21;
    const size_t Table560::TABLE_560_TABLE_610_FLAG_A_BIT_OFFSET    = 22;
    const size_t Table560::TABLE_560_TABLE_610_FLAG_B_BIT_OFFSET    = 23;
    const size_t Table560::TABLE_560_TABLE_1610_FLAG_A_BIT_OFFSET   = 24;
    const size_t Table560::TABLE_560_TABLE_1610_FLAG_B_BIT_OFFSET   = 25;

    // ExceptionChecked
    Table560::Table560(TA_IRS_Bus::Scheduler& socketScheduler,
                       TA_IRS_Bus::Scheduler& processScheduler,
                       AbstractPASTable* table100,
                       AbstractPASTable* table202,
                       AbstractPASTable* table204,
                       AbstractPASTable* table1204,
                       AbstractPASTable* table303,
                       AbstractPASTable* table1303,
                       AbstractPASTable* table550,
                       AbstractPASTable* table570,
                       AbstractPASTable* table590,
                       AbstractPASTable* table603,
                       AbstractPASTable* table604,
                       AbstractPASTable* table610,
                       AbstractPASTable* table1610,
                       IPasEventSource& eventSource)
        : AbstractWriteablePASTable(560)
        , m_socketScheduler(socketScheduler)
        , m_processScheduler(processScheduler)
        , m_table100(table100)
        , m_table202(table202)
        , m_table204(table204)
        , m_table1204(table1204)
        , m_table303(table303)
        , m_table1303(table1303)
        , m_table550(table550)
        , m_table570(table570)
        , m_table590(table590)
        , m_table603(table603)
        , m_table604(table604)
        , m_table610(table610)
        , m_table1610(table1610)
        , m_writeTable560(false)
        , m_pasEventSource(eventSource)
        , m_events(std::make_shared<st::vector<TA_IRS_Bus::IEvent*>>())
    {
        FUNCTION_ENTRY("Constructor");
        FUNCTION_EXIT;
    }

    // ExceptionChecked
    Table560::~Table560()
    {
        FUNCTION_ENTRY("Destructor");

        FUNCTION_EXIT;
    }

    bool Table560::isAnyFlagAset()
    {
        ThreadGuard guard(m_lock);

        bool ret = false;

        if (m_table100) ret |= PasHelpers::instance().getBit(m_buffer, TABLE_560_TABLE_100_FLAG_A_BIT_OFFSET);

        if (m_table202) ret |= PasHelpers::instance().getBit(m_buffer, TABLE_560_TABLE_202_FLAG_A_BIT_OFFSET);

        if (m_table204) ret |= PasHelpers::instance().getBit(m_buffer, TABLE_560_TABLE_204_FLAG_A_BIT_OFFSET);

        if (m_table1204) ret |= PasHelpers::instance().getBit(m_buffer, TABLE_560_TABLE_1204_FLAG_A_BIT_OFFSET);

        if (m_table303) ret |= PasHelpers::instance().getBit(m_buffer, TABLE_560_TABLE_303_FLAG_A_BIT_OFFSET);

        if (m_table1303) ret |= PasHelpers::instance().getBit(m_buffer, TABLE_560_TABLE_1303_FLAG_A_BIT_OFFSET);

        if (m_table550) ret |= PasHelpers::instance().getBit(m_buffer, TABLE_560_TABLE_550_FLAG_A_BIT_OFFSET);

        if (m_table570) ret |= PasHelpers::instance().getBit(m_buffer, TABLE_560_TABLE_570_FLAG_A_BIT_OFFSET);

        if (m_table590) ret |= PasHelpers::instance().getBit(m_buffer, TABLE_560_TABLE_590_FLAG_A_BIT_OFFSET);

        if (m_table603) ret |= PasHelpers::instance().getBit(m_buffer, TABLE_560_TABLE_603_FLAG_A_BIT_OFFSET);

        if (m_table604) ret |= PasHelpers::instance().getBit(m_buffer, TABLE_560_TABLE_604_FLAG_A_BIT_OFFSET);

        if (m_table610) ret |= PasHelpers::instance().getBit(m_buffer, TABLE_560_TABLE_610_FLAG_A_BIT_OFFSET);

        if (m_table1610) ret |= PasHelpers::instance().getBit(m_buffer, TABLE_560_TABLE_1610_FLAG_A_BIT_OFFSET);

        return ret;
    }

    bool Table560::isFlagSet(unsigned int flag)
    {
        ThreadGuard guard(m_lock);
        return PasHelpers::instance().getBit(m_buffer, flag);
    }

    bool Table560::isBothFlagSet(unsigned int flagA, unsigned int flagB)
    {
        return isFlagSet(flagA) && isFlagSet(flagB);
    }

    void Table560::processRead()
    {
        LOG_SCOPE("Table560::processRead");

        ThreadGuard guard(m_lock);

        processTableFlag(TABLE_560_TABLE_100_FLAG_A_BIT_OFFSET, TABLE_560_TABLE_100_FLAG_B_BIT_OFFSET);
        processTableFlag(TABLE_560_TABLE_202_FLAG_A_BIT_OFFSET, TABLE_560_TABLE_202_FLAG_B_BIT_OFFSET);
        processTableFlag(TABLE_560_TABLE_204_FLAG_A_BIT_OFFSET, TABLE_560_TABLE_204_FLAG_B_BIT_OFFSET);
        processTableFlag(TABLE_560_TABLE_1204_FLAG_A_BIT_OFFSET, TABLE_560_TABLE_1204_FLAG_B_BIT_OFFSET);
        processTableFlag(TABLE_560_TABLE_303_FLAG_A_BIT_OFFSET, TABLE_560_TABLE_303_FLAG_B_BIT_OFFSET);
        processTableFlag(TABLE_560_TABLE_1303_FLAG_A_BIT_OFFSET, TABLE_560_TABLE_1303_FLAG_B_BIT_OFFSET);
        processTableFlag(TABLE_560_TABLE_550_FLAG_A_BIT_OFFSET, TABLE_560_TABLE_550_FLAG_B_BIT_OFFSET);
        processTableFlag(TABLE_560_TABLE_570_FLAG_A_BIT_OFFSET, TABLE_560_TABLE_570_FLAG_B_BIT_OFFSET);
        processTableFlag(TABLE_560_TABLE_590_FLAG_A_BIT_OFFSET, TABLE_560_TABLE_590_FLAG_B_BIT_OFFSET);
        processTableFlag(TABLE_560_TABLE_603_FLAG_A_BIT_OFFSET, TABLE_560_TABLE_603_FLAG_B_BIT_OFFSET);
        processTableFlag(TABLE_560_TABLE_604_FLAG_A_BIT_OFFSET, TABLE_560_TABLE_604_FLAG_B_BIT_OFFSET);
        processTableFlag(TABLE_560_TABLE_610_FLAG_A_BIT_OFFSET, TABLE_560_TABLE_610_FLAG_B_BIT_OFFSET);
        processTableFlag(TABLE_560_TABLE_1610_FLAG_A_BIT_OFFSET, TABLE_560_TABLE_1610_FLAG_B_BIT_OFFSET);
    }

    void Table560::processTableFlag(unsigned int tableFlagA, unsigned int tableFlagB)
    {
        LOG_SCOPE(boost::format("Table560::processTableFlag[A=%d,B=%d]") % tableFlagA % tableFlagB);

        // Guarded in processRead()

        unsigned int byteOffset(0);

        bool isFlagASet = PasHelpers::instance().getBit(m_buffer, tableFlagA);
        bool isFlagBSet = PasHelpers::instance().getBit(m_buffer, tableFlagB);

        if (!isFlagASet && !isFlagBSet)
        {
            // Ignore
            return;
        }
        else if (!isFlagASet && isFlagBSet)
        {
            // PAS is in the process of responding.  We expect both flags to be
            // reset in the next update
            return;
        }
        else if (isFlagASet && !isFlagBSet)
        {
            // Ingore - this should be impossible.  We'll try to reset the flags anyhow
            LOG_ERROR("Unexpected flag table 560 state - Flag A is true and Flag B is false");

            resetFlags(nullptr, tableFlagA, tableFlagB);
            return;
        }

        // (isFlagASet && isFlagBSet)

        TA_IRS_Bus::IEvent* event = NULL;

        switch (tableFlagA)
        {
            case TABLE_560_TABLE_100_FLAG_A_BIT_OFFSET:
                if (NULL == m_table100)
                    break;

                LOG_DEBUG("processTableFlag(): ReadTable100");
                event = new ReadTable100(dynamic_cast<Table100&>(*(m_table100)),
                                         *this,
                                         m_socketScheduler,
                                         m_processScheduler,
                                         m_pasEventSource);
                break;

            case TABLE_560_TABLE_202_FLAG_A_BIT_OFFSET:
                if (NULL == m_table202)
                {
                    break;
                }

                LOG_DEBUG("processTableFlag(): ReadTable202");
                event = new ReadTable202(dynamic_cast<Table202&>(*(m_table202)),
                                         *this,
                                         m_socketScheduler,
                                         m_processScheduler,
                                         m_pasEventSource);
                break;

            case TABLE_560_TABLE_204_FLAG_A_BIT_OFFSET:
                if (NULL == m_table204)
                {
                    break;
                }

                LOG_DEBUG("processTableFlag(): ReadTable204");
                event = new ReadTable204(dynamic_cast<Table204&>(*(m_table204)),
                                         *this,
                                         m_socketScheduler,
                                         m_processScheduler,
                                         m_pasEventSource);
                break;

            case TABLE_560_TABLE_1204_FLAG_A_BIT_OFFSET:
                if (NULL == m_table1204)
                {
                    break;
                }

                LOG_DEBUG("processTableFlag(): ReadTable1204");
                event = new ReadTable1204(dynamic_cast<Table1204&>(*(m_table1204)),
                                          *this,
                                          m_socketScheduler,
                                          m_processScheduler,
                                          m_pasEventSource);
                break;

            case TABLE_560_TABLE_303_FLAG_A_BIT_OFFSET:
                if (NULL == m_table303)
                {
                    break;
                }

                LOG_DEBUG("processTableFlag(): ReadTable303");
                event = new ReadTable303(dynamic_cast<Table303&>(*(m_table303)),
                                         *this,
                                         m_socketScheduler,
                                         m_processScheduler,
                                         m_pasEventSource);
                break;

            case TABLE_560_TABLE_1303_FLAG_A_BIT_OFFSET:
                if (NULL == m_table1303)
                {
                    break;
                }

                LOG_DEBUG("processTableFlag(): ReadTable1303");
                event = new ReadTable1303(dynamic_cast<Table1303&>(*(m_table1303)),
                                          *this,
                                          m_socketScheduler,
                                          m_processScheduler,
                                          m_pasEventSource);
                break;

            case TABLE_560_TABLE_550_FLAG_A_BIT_OFFSET:
                if (NULL == m_table550)
                {
                    break;
                }

                LOG_DEBUG("processTableFlag(): ReadTable550");
                event = new ReadTable550(dynamic_cast<Table550&>(*(m_table550)),
                                         *this,
                                         m_socketScheduler,
                                         m_processScheduler,
                                         m_pasEventSource);
                break;

            case TABLE_560_TABLE_570_FLAG_A_BIT_OFFSET:
                if (NULL == m_table570)
                {
                    break;
                }

                LOG_DEBUG("processTableFlag(): ReadTable570");
                event = new ReadTable570(dynamic_cast<Table570&>(*(m_table570)),
                                         *this,
                                         m_socketScheduler,
                                         m_processScheduler,
                                         m_pasEventSource);
                break;

            case TABLE_560_TABLE_590_FLAG_A_BIT_OFFSET:
                if (NULL == m_table590)
                {
                    break;
                }

                LOG_DEBUG("processTableFlag(): ReadTable590");
                event = new ReadTable590(dynamic_cast<Table590&>(*(m_table590)),
                                         *this,
                                         m_socketScheduler,
                                         m_processScheduler,
                                         m_pasEventSource);
                break;

            case TABLE_560_TABLE_603_FLAG_A_BIT_OFFSET:
                if (NULL == m_table603)
                {
                    break;
                }

                LOG_DEBUG("processTableFlag(): ReadTable603");
                event = new ReadTable603(dynamic_cast<Table603&>(*(m_table603)),
                                         *this,
                                         m_socketScheduler,
                                         m_processScheduler,
                                         m_pasEventSource);
                break;

            case TABLE_560_TABLE_604_FLAG_A_BIT_OFFSET:
                if (NULL == m_table604)
                {
                    break;
                }

                LOG_DEBUG("processTableFlag(): ReadTable604");
                event = new ReadTable604(dynamic_cast<Table604&>(*(m_table604)),
                                         *this,
                                         m_socketScheduler,
                                         m_processScheduler,
                                         m_pasEventSource);
                break;

            case TABLE_560_TABLE_610_FLAG_A_BIT_OFFSET:
                if (NULL == m_table610)
                {
                    break;
                }

                LOG_DEBUG("processTableFlag(): ReadTable610");
                event = new ReadTable610(dynamic_cast<Table610&>(*(m_table610)),
                                         *this,
                                         m_socketScheduler,
                                         m_processScheduler,
                                         m_pasEventSource);
                break;

            case TABLE_560_TABLE_1610_FLAG_A_BIT_OFFSET:
                if (NULL == m_table1610)
                {
                    break;
                }

                LOG_DEBUG("processTableFlag(): ReadTable1610");
                event = new ReadTable1610(dynamic_cast<Table1610&>(*(m_table1610)),
                                          *this,
                                          m_socketScheduler,
                                          m_processScheduler,
                                          m_pasEventSource);
                break;

            default:
                break;
        }

        if (event != NULL)
        {
            m_socketScheduler.post(event);

            m_events->emplace_back(event);

            m_timer.submit_once_or_replace_work("events", std::chrono::seconds(CachedConfig::getInstance()->getSocketTimeoutInSecs() * 3), [=]
            {
                if (m_events->empty())
                {
                    return;
                }

                LOG_SCOPE("Table560::processTableFlag::SimpleTimer::submit_once_or_replace_work");

                LOG_ERROR("processTableFlag(): timedout, there are %d events on schedule", m_events->size());

                m_events->clear();

                m_socketScheduler.post(new ReadTable560(ACE_OS::gettimeofday(),
                                                        *this,
                                                        m_socketScheduler,
                                                        m_processScheduler,
                                                        m_pasEventSource));
            });
        }
    }

    void Table560::resetFlags(TA_IRS_Bus::IEvent* event, unsigned int tableFlagA, unsigned int tableFlagB)
    {
        // not scheduled by Table560, ignore it
        if (m_events->count(event) == 0)
        {
            return;
        }

        // Scopes to prevent clash with lock in isAnyFlagAset()
        {
            ThreadGuard guardWrite(m_lock);
            PasHelpers::instance().setBit(m_buffer, tableFlagA, false);
        }

        m_events->remove(event);

        if (m_events->empty())
        {
            LOG_ERROR_IF(isAnyFlagAset(), "resetFlags(): all events processed, but still has flags");

            m_timer.remove("events");

            // we have processed all flagged tables

            WriteTable560* event = new WriteTable560(*this,
                                                     m_socketScheduler,
                                                     m_processScheduler,
                                                     m_pasEventSource);
            addWriteEvent(*event);
            m_socketScheduler.post(event);
        }
    }

    ReadTable560::ReadTable560(Table560& table,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASReadEvent(socketScheduler, processScheduler, eventSource)
        , m_table(table)
    {
    }

    ReadTable560::ReadTable560(ACE_Time_Value expiryTime,
                               Table560& table,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASReadEvent(expiryTime, socketScheduler, processScheduler, eventSource)
        , m_table(table)
    {
    }

    void ReadTable560::readTable()
    {
        LOG_SCOPE("ReadTable560::readTable");

        PASConnection::getInstance()->readTable(m_table.TABLE_NUMBER,
                                                m_table.m_buffer,
                                                m_table.BUFFER_SIZE);

        //    ACE_Time_Value nextReadTime(m_timeToExpire + ACE_Time_Value( 0, CachedConfig::getInstance()->getFastPollRateInMSecs() * 1000 ));
        //    if (nextReadTime < ACE_OS::gettimeofday())
        //    {
        //        nextReadTime = ACE_OS::gettimeofday();
        //    }

        //    m_socketScheduler.post( new ReadTable560( nextReadTime,
        //        m_table,
        //        m_socketScheduler,
        //        m_processScheduler,
        //        m_pasEventSource ) );

        m_processScheduler.post(new ProcessTable560(m_table));
    }

    ProcessTable560::ProcessTable560(Table560& table)
        : m_table(table)
    {
    }

    void ProcessTable560::consume()
    {
        LOG_SCOPE("ProcessTable560::consume");

        m_table.processRead();
        PasTableReadCounter::instance().increase(m_table.TABLE_NUMBER);

        if (m_table.m_events->empty())
        {
            LOG_ERROR_IF(m_table.isAnyFlagAset(), "consume(): flags not clear");

            m_table.m_socketScheduler.post(new ReadTable560(getNextTimeToExpire(CachedConfig::getInstance()->getFastPollRateInMSecs()),
                                                            m_table,
                                                            m_table.m_socketScheduler,
                                                            m_table.m_processScheduler,
                                                            m_table.m_pasEventSource));
        }

        delete this;
    }

    void ProcessTable560::cancel()
    {
        LOG_SCOPE("ProcessTable560::cancel");
        delete this;
    }

    WriteTable560::WriteTable560(Table560& table,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASWriteEvent(table, socketScheduler, processScheduler, eventSource)
        , m_table(table)
    {
    }

    WriteTable560::WriteTable560(ACE_Time_Value timeToExpire,
                                 Table560& table,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASWriteEvent(timeToExpire, table, socketScheduler, processScheduler, eventSource)
        , m_table(table)
    {
    }

    void WriteTable560::writeTable()
    {
        LOG_SCOPE("WriteTable560::writeTable");

        // ReEntrant as this will also be guarded in the write() call.
        ThreadGuard guard(m_table.m_lock);

        PASConnection::getInstance()->writeTable(m_table.TABLE_NUMBER,
                                                 m_table.m_buffer,
                                                 m_table.BUFFER_SIZE);

        // m_table.m_writeTable560 = false;  // mission complete

        // Resume reading
        m_socketScheduler.post(new ReadTable560(ACE_OS::gettimeofday(),
                                                m_table,
                                                m_socketScheduler,
                                                m_processScheduler,
                                                m_pasEventSource));
    }

    PASWriteEvent* WriteTable560::recreateEvent(ACE_Time_Value timeToExpire)
    {
        return new WriteTable560(timeToExpire,
                                 m_table,
                                 m_socketScheduler,
                                 m_processScheduler,
                                 m_pasEventSource);
    }
}
