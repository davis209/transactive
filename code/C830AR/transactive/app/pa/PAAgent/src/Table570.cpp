/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table570.cpp $
 * @author:  Ripple
 * @version: $Revision: #6 $
 *
 * Last modification: $DateTime: 2025/07/10 16:50:01 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table570.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "PASConnection.h"
#include "CachedConfig.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "bus/scada/datapoint_library/src/DataPoint.h"
#include <time.h>

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    Table570::Table570(TA_IRS_Bus::Scheduler& socketScheduler, TA_IRS_Bus::Scheduler& processScheduler, IPasEventSource& eventSource)
        : AbstractWriteablePASTable(570)
        , m_socketScheduler(socketScheduler)
        , m_processScheduler(processScheduler)
        , m_pasEventSource(eventSource)
    {
        FUNCTION_ENTRY("Constructor");

        m_writeBuffer = new std::uint8_t[BUFFER_SIZE]{0};

        FUNCTION_EXIT;
    }

    Table570::~Table570()
    {
        FUNCTION_ENTRY("Destructor");

        delete[] m_writeBuffer;

        FUNCTION_EXIT;
    }

    void Table570::processRead()
    {
        LOG_SCOPE("Table570::processRead");

        ThreadGuard guard(m_lock);

#if 0
        // TD15174
        if (!CachedConfig::getInstance()->getIsDepot() && !CachedConfig::getInstance()->getIsMaster())
#else
        if (!CachedConfig::getInstance()->getIsMaster())
#endif
        {
            TA_Base_Bus::DataPoint* dp = CachedConfig::getInstance()->getFireCountdownActiveDP();

            if (dp)
            {
                TA_Base_Bus::DpValue dpValue(dp->getDataPointDataType(),
                                             dp->getEnumLabelsMap(),
                                             dp->getBooleanLabels());

                if (m_buffer[0] != 0)
                {
                    dpValue.setBoolean(true);
                    // TD18294
                    m_isOnFireAlarm = true;
                    // TD18294
                }
                else
                {
                    dpValue.setBoolean(false);
                    // TD18294
                    m_isOnFireAlarm = false;
                    // TD18294
                }

                // set up DataPointState with the default timestamp and quality
                TA_Base_Bus::DataPointState newState(dpValue, time(NULL), TA_Base_Bus::QUALITY_GOOD_NO_SPECIFIC_REASON);

                dp->updateFieldState(newState);
            }
        }
    }

    // TD18294
    bool Table570::isOnFireAlarmState()
    {
        return m_isOnFireAlarm;
    }

    void Table570::resetFireAlarmStatusAndWrite()
    {
        ThreadGuard guard(m_lock);

        LOG_INFO("Resetting FireAlarmStatus on FEPC");

        // Maochun Sun++
        // bug774 (TD13071)

        //m_buffer[0] = 0;
        m_writeBuffer[0] = 0;

        // ++Maochun Sun
        // bug774 (TD13071)

        WriteTable570* event = new WriteTable570(*this,
                                                 m_socketScheduler,
                                                 m_processScheduler,
                                                 m_pasEventSource);
        addWriteEvent(*event);
        m_socketScheduler.post(event);
    }

    void invalidateDataPoint(TA_Base_Bus::DataPoint&);

    void Table570::invalidateAllDataPoints()
    {
        LOG_SCOPE("Table570::invalidateAllDataPoints");

        try
        {
#if 0
            // TD15174
            if (!CachedConfig::getInstance()->getIsDepot() && !CachedConfig::getInstance()->getIsMaster())
#else
            if (!CachedConfig::getInstance()->getIsMaster())
#endif
            {
                TA_Base_Bus::DataPoint* dp = CachedConfig::getInstance()->getFireCountdownActiveDP();

                if (dp)
                {
                    invalidateDataPoint(*dp);
                }
            }
        }
        catch (...)
        {
            // Carry on
        }
    }

    ReadTable570::ReadTable570(Table570& table,
                               Table560& table560,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASReadEvent(socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_table560(table560)
    {
    }

    void ReadTable570::readTable()
    {
        LOG_SCOPE("ReadTable570::readTable");

        ThreadGuard guard(m_table.m_lock);

        PASConnection::getInstance()->readTable(m_table.TABLE_NUMBER,
                                                m_table.m_buffer,
                                                m_table.BUFFER_SIZE);

        m_processScheduler.post(new ProcessTable570(m_table));

        m_table560.resetFlags(this,
                              Table560::TABLE_560_TABLE_570_FLAG_A_BIT_OFFSET,
                              Table560::TABLE_560_TABLE_570_FLAG_B_BIT_OFFSET);
    }

    ProcessTable570::ProcessTable570(Table570& table)
        : m_table(table)
    {
    }

    void ProcessTable570::consume()
    {
        LOG_SCOPE("ProcessTable570::consume");
        m_table.processRead();
        PasTableReadCounter::instance().increase(m_table.TABLE_NUMBER);

        delete this;
    }

    void ProcessTable570::cancel()
    {
        LOG_SCOPE("ProcessTable570::cancel");
        delete this;
    }

    WriteTable570::WriteTable570(Table570& table,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASWriteEvent(table, socketScheduler, processScheduler, eventSource)
        , m_table(table)
    {
    }

    WriteTable570::WriteTable570(ACE_Time_Value timeToExpire,
                                 Table570& table,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASWriteEvent(timeToExpire, table, socketScheduler, processScheduler, eventSource)
        , m_table(table)
    {
    }

    void WriteTable570::writeTable()
    {
        LOG_SCOPE("WriteTable570::writeTable");

        ThreadGuard guard(m_table.m_lock);

        PASConnection::getInstance()->writeTable(m_table.TABLE_NUMBER,
                                                 // Maochun Sun++
                                                 // bug774 (TD13071)

                                                 //m_table.m_buffer,
                                                 m_table.m_writeBuffer,

                                                 // ++Maochun Sun
                                                 // bug774 (TD13071)
                                                 m_table.BUFFER_SIZE);
    }

    InvalidateTable570::InvalidateTable570(Table570& table)
        : m_table(table)
    {
    }

    void InvalidateTable570::consume()
    {
        LOG_SCOPE("InvalidateTable570::consume");

        m_table.invalidateAllDataPoints();

        delete this;
    }

    void InvalidateTable570::cancel()
    {
        LOG_SCOPE("InvalidateTable570::cancel");
        delete this;
    }

    PASWriteEvent* WriteTable570::recreateEvent(ACE_Time_Value timeToExpire)
    {
        return new WriteTable570(timeToExpire,
                                 m_table,
                                 m_socketScheduler,
                                 m_processScheduler,
                                 m_pasEventSource);
    }
}
