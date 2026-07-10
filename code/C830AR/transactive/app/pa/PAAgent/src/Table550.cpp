/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table550.cpp $
 * @author:  Ripple
 * @version: $Revision: #5 $
 *
 * Last modification: $DateTime: 2025/06/24 11:25:13 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table550.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "PASConnection.h"
#include "CachedConfig.h"
#include "bus/scada/datapoint_library/src/DataPoint.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include <time.h>

using namespace TA_Base_Core;

const size_t TABLE_550_WORD_OFFSET                          = 0;
const size_t TABLE_550_SUMMARY_OF_MAJOR_ALARMS_BIT_OFFSET   = 0;
const size_t TABLE_550_SUMMARY_OF_MINOR_ALARMS_BIT_OFFSET   = 1;

namespace TA_IRS_App
{
    Table550::Table550()
        : AbstractPASTable(550)
    {
        FUNCTION_ENTRY("Constructor");
        FUNCTION_EXIT;
    }

    void Table550::processRead()
    {
        LOG_SCOPE("Table550::processRead");

        ThreadGuard guard(m_lock);

        std::uint16_t wordData = PasHelpers::instance().get16bit(m_buffer, TABLE_550_WORD_OFFSET);

        {
            // Update major summary alarm datapoint
            TA_Base_Bus::DataPoint* dp = CachedConfig::getInstance()->getMajorSummaryAlarmDP();
            LOG_ERROR_IF_NOT(dp, "processRead(): Major PA equipment alarm detected but datapoint not found");

            if (dp)
            {
                TA_Base_Bus::DpValue dpValue(dp->getDataPointDataType(),
                                             dp->getEnumLabelsMap(),
                                             dp->getBooleanLabels());
                bool alarmState = (wordData & (1 << TABLE_550_SUMMARY_OF_MAJOR_ALARMS_BIT_OFFSET)) != 0;
                dpValue.setBoolean(alarmState);
                // set up DataPointState with the default timestamp and quality
                TA_Base_Bus::DataPointState newState(dpValue, time(NULL), TA_Base_Bus::QUALITY_GOOD_NO_SPECIFIC_REASON);

                dp->updateFieldState(newState);
            }
        }

        {
            // Update minor summary alarm datapoint
            TA_Base_Bus::DataPoint* dp = CachedConfig::getInstance()->getMinorSummaryAlarmDP();
            LOG_ERROR_IF_NOT(dp, "processRead(): Minor PA equipment alarm detected but datapoint not found");

            if (dp)
            {
                TA_Base_Bus::DpValue dpValue(dp->getDataPointDataType(),
                                             dp->getEnumLabelsMap(),
                                             dp->getBooleanLabels());
                bool alarmState = (wordData & (1 << TABLE_550_SUMMARY_OF_MINOR_ALARMS_BIT_OFFSET)) != 0;
                dpValue.setBoolean(alarmState);
                // set up DataPointState with the default timestamp and quality
                TA_Base_Bus::DataPointState newState(dpValue, time(NULL), TA_Base_Bus::QUALITY_GOOD_NO_SPECIFIC_REASON);

                dp->updateFieldState(newState);
            }
        }
    }

    void invalidateDataPoint(TA_Base_Bus::DataPoint&);

    void Table550::invalidateAllDataPoints()
    {
        LOG_SCOPE("Table550::invalidateAllDataPoints");

        try
        {
            if (auto majorDp = CachedConfig::getInstance()->getMajorSummaryAlarmDP())
            {
                invalidateDataPoint(*majorDp);
            }

            if (auto minorDp = CachedConfig::getInstance()->getMinorSummaryAlarmDP())
            {
                invalidateDataPoint(*minorDp);
            }
        }
        catch (...)
        {
            // Carry on
        }
    }

    ReadTable550::ReadTable550(Table550& table,
                               Table560& table560,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASReadEvent(socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_table560(table560)
    {
    }

    void ReadTable550::readTable()
    {
        LOG_SCOPE("ReadTable550::readTable");

        ThreadGuard guard(m_table.m_lock);

        PASConnection::getInstance()->readTable(m_table.TABLE_NUMBER,
                                                m_table.m_buffer,
                                                m_table.BUFFER_SIZE);

        m_processScheduler.post(new ProcessTable550(m_table));

        m_table560.resetFlags(this,
                              Table560::TABLE_560_TABLE_550_FLAG_A_BIT_OFFSET,
                              Table560::TABLE_560_TABLE_550_FLAG_B_BIT_OFFSET);
    }

    ProcessTable550::ProcessTable550(Table550& table)
        : m_table(table)
    {
    }

    void ProcessTable550::consume()
    {
        LOG_SCOPE("ProcessTable550::consume");

        m_table.processRead();
        PasTableReadCounter::instance().increase(m_table.TABLE_NUMBER);

        delete this;
    }

    void ProcessTable550::cancel()
    {
        delete this;
    }

    InvalidateTable550::InvalidateTable550(Table550& table)
        : m_table(table)
    {
    }

    void InvalidateTable550::consume()
    {
        LOG_SCOPE("InvalidateTable550::consume");

        m_table.invalidateAllDataPoints();

        delete this;
    }

    void InvalidateTable550::cancel()
    {
        LOG_SCOPE("InvalidateTable550::cancel");
        delete this;
    }
}
