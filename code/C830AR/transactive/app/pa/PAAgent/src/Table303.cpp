/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table303.cpp $
 * @author:  Ripple
 * @version: $Revision: #5 $
 *
 * Last modification: $DateTime: 2025/06/24 11:25:13 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table303.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "PASConnection.h"
#include "bus/scada/datapoint_library/src/DataPoint.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include <time.h>

using namespace TA_Base_Core;

namespace
{
    size_t TABLE_303_RECORD_SIZE = 26;

    const size_t TABLE_303_CHIME_SUB_OFFSET             = 0;
    const size_t TABLE_303_DVA_MESSAGE_1_SUB_OFFSET     = 1;
    const size_t TABLE_303_DVA_MESSAGE_2_SUB_OFFSET     = 3;
    const size_t TABLE_303_DVA_MESSAGE_3_SUB_OFFSET     = 5;
    const size_t TABLE_303_DVA_MESSAGE_4_SUB_OFFSET     = 7;
    const size_t TABLE_303_DWELL_SUB_OFFSET             = 9;
    const size_t TABLE_303_PERIOD_SUB_OFFSET            = 11;
    const size_t TABLE_303_START_TIME_SUB_OFFSET        = 13;
    const size_t TABLE_303_STOP_TIME_SUB_OFFSET         = 17;
    const size_t TABLE_303_EVENT_TRIGGERED_SUB_OFFSET   = 21;
    const size_t TABLE_303_LOCAL_COVERAGE_SUB_OFFSET    = 22;

    const size_t PAS_NO_MESSAGE_ID = 0;
}

namespace TA_IRS_App
{
    Table303::Table303(unsigned long locationKey)
        : AbstractPASTable(303),
        m_locationKey(locationKey)
    {
        FUNCTION_ENTRY("Constructor");

        if (is_depot())
        {
            TABLE_303_RECORD_SIZE = 30;
        }

        m_messageSequenceDescriptions.resize(MAXMSGSEQ);

        FUNCTION_EXIT;
    }

    void Table303::processRead()
    {
        LOG_SCOPE("Table303::processRead");
        ThreadGuard guard(m_lock);

        unsigned long offset = 0;
        unsigned long messageId = 0;
        unsigned long messageKey = 0;

        for (int sequenceIdMinus1 = 0; sequenceIdMinus1 < MAXMSGSEQ; ++sequenceIdMinus1)
        {
            m_messageSequenceDescriptions[sequenceIdMinus1].m_hasChime =
                PasHelpers::instance().get8bit(m_buffer, TABLE_303_CHIME_SUB_OFFSET + TABLE_303_RECORD_SIZE * sequenceIdMinus1) ? true : false;

            m_messageSequenceDescriptions[sequenceIdMinus1].m_dwellInSecs =
                PasHelpers::instance().get16bit(m_buffer, TABLE_303_DWELL_SUB_OFFSET + TABLE_303_RECORD_SIZE * sequenceIdMinus1);

            m_messageSequenceDescriptions[sequenceIdMinus1].m_periodInSecs =
                PasHelpers::instance().get16bit(m_buffer, TABLE_303_PERIOD_SUB_OFFSET + TABLE_303_RECORD_SIZE * sequenceIdMinus1);

            m_messageSequenceDescriptions[sequenceIdMinus1].m_startTime =
                PasHelpers::instance().get32bit(m_buffer, TABLE_303_START_TIME_SUB_OFFSET + TABLE_303_RECORD_SIZE * sequenceIdMinus1);

            m_messageSequenceDescriptions[sequenceIdMinus1].m_stopTime =
                PasHelpers::instance().get32bit(m_buffer, TABLE_303_STOP_TIME_SUB_OFFSET + TABLE_303_RECORD_SIZE * sequenceIdMinus1);

            m_messageSequenceDescriptions[sequenceIdMinus1].m_isEventTriggered =
                PasHelpers::instance().get8bit(m_buffer, TABLE_303_EVENT_TRIGGERED_SUB_OFFSET + TABLE_303_RECORD_SIZE * sequenceIdMinus1) ? true : false;

            // Convert local message Ids to message keys
            m_messageSequenceDescriptions[sequenceIdMinus1].m_messages.length(4);

            messageId = PasHelpers::instance().get16bit(m_buffer, TABLE_303_DVA_MESSAGE_1_SUB_OFFSET + TABLE_303_RECORD_SIZE * sequenceIdMinus1);
            m_messageSequenceDescriptions[sequenceIdMinus1].m_messages[0] = getMessageKeyAndValidate(messageId);

            messageId = PasHelpers::instance().get16bit(m_buffer, TABLE_303_DVA_MESSAGE_2_SUB_OFFSET + TABLE_303_RECORD_SIZE * sequenceIdMinus1);
            m_messageSequenceDescriptions[sequenceIdMinus1].m_messages[1] = getMessageKeyAndValidate(messageId);

            messageId = PasHelpers::instance().get16bit(m_buffer, TABLE_303_DVA_MESSAGE_3_SUB_OFFSET + TABLE_303_RECORD_SIZE * sequenceIdMinus1);
            m_messageSequenceDescriptions[sequenceIdMinus1].m_messages[2] = getMessageKeyAndValidate(messageId);

            messageId = PasHelpers::instance().get16bit(m_buffer, TABLE_303_DVA_MESSAGE_4_SUB_OFFSET + TABLE_303_RECORD_SIZE * sequenceIdMinus1);
            m_messageSequenceDescriptions[sequenceIdMinus1].m_messages[3] = getMessageKeyAndValidate(messageId);

            // Convert local coverage bitmap to PA Zone Keys

            if (is_depot())
            {
                auto localCoverageBitmap = PasHelpers::instance().get64bit(m_buffer, TABLE_303_LOCAL_COVERAGE_SUB_OFFSET + TABLE_303_RECORD_SIZE * sequenceIdMinus1);
                PasHelpers::instance().localCoverageBitmapToCoverage(localCoverageBitmap,
                                                                     m_messageSequenceDescriptions[sequenceIdMinus1].m_zones,
                                                                     m_locationKey);
            }
            else
            {
                auto localCoverageBitmap = PasHelpers::instance().get32bit(m_buffer, TABLE_303_LOCAL_COVERAGE_SUB_OFFSET + TABLE_303_RECORD_SIZE * sequenceIdMinus1);
                PasHelpers::instance().localCoverageBitmapToCoverage(localCoverageBitmap,
                                                                     m_messageSequenceDescriptions[sequenceIdMinus1].m_zones,
                                                                     m_locationKey);
            }
        }
    }

    unsigned long Table303::getMessageKeyAndValidate(unsigned long messageId)
    {
        if (messageId != PAS_NO_MESSAGE_ID)
        {
            try
            {
                return TA_IRS_Bus::CachedMaps::getInstance()->getKeyFromStationDvaMessageIdAndLocation(messageId, m_locationKey);
            }
            catch (TA_Base_Core::CachedMappingNotFoundException& e)
            {
                // Received invalid data
                LOG_ERROR("Invalid message Id in table 303 returned from PAS");
            }
        }

        // Not set
        return -1;
    }

    bool Table303::hasStartTimeElapsed(unsigned long sequenceId)
    {
        ThreadGuard guard(m_lock);

        return
            (m_messageSequenceDescriptions[sequenceId - 1].m_startTime == 0 ||
             time(NULL) >= m_messageSequenceDescriptions[sequenceId - 1].m_startTime);
    }

    TA_Base_Core::DateTime Table303::getStartTime(unsigned long sequenceId)
    {
        ThreadGuard guard(m_lock);

        return m_messageSequenceDescriptions[sequenceId - 1].m_startTime;
    }

    TA_Base_Core::DateTime Table303::getStopTime(unsigned long sequenceId)
    {
        ThreadGuard guard(m_lock);

        return m_messageSequenceDescriptions[sequenceId - 1].m_stopTime;
    }

    bool Table303::isMessageSequenceCyclic(unsigned long sequenceId)
    {
        ThreadGuard guard(m_lock);

        if (m_messageSequenceDescriptions[sequenceId - 1].m_startTime != 0 ||
            m_messageSequenceDescriptions[sequenceId - 1].m_stopTime != 0)
        {
            return true;
        }

        return false;
    }

    ReadTable303::ReadTable303(Table303& table,
                               Table560& table560,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASReadEvent(socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_table560(table560)
    {
    }

    void ReadTable303::readTable()
    {
        LOG_SCOPE("ReadTable303::readTable");
        ThreadGuard guard(m_table.m_lock);

        PASConnection::getInstance()->readTable(m_table.TABLE_NUMBER,
                                                m_table.m_buffer,
                                                m_table.BUFFER_SIZE);

        m_processScheduler.post(new ProcessTable303(m_table));

        m_table560.resetFlags(this,
                              Table560::TABLE_560_TABLE_303_FLAG_A_BIT_OFFSET,
                              Table560::TABLE_560_TABLE_303_FLAG_B_BIT_OFFSET);
    }

    ProcessTable303::ProcessTable303(Table303& table)
        : m_table(table)
    {
    }

    void ProcessTable303::consume()
    {
        LOG_SCOPE("ProcessTable303::consume");
        m_table.processRead();
        PasTableReadCounter::instance().increase(m_table.TABLE_NUMBER);

        delete this;
    }

    void ProcessTable303::cancel()
    {
        LOG_SCOPE("ProcessTable303::cancel");
        delete this;
    }
}
