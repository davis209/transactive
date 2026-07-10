/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table1303.cpp $
 * @author:  Ripple
 * @version: $Revision: #5 $
 *
 * Last modification: $DateTime: 2025/06/24 11:25:13 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table1303.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "PASConnection.h"
#include "bus/scada/datapoint_library/src/DataPoint.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include <time.h>

using namespace TA_Base_Core;

const size_t TABLE_1303_RECORD_SIZE                 = 222;

const size_t TABLE_1303_CHIME_SUB_OFFSET            = 0;
const size_t TABLE_1303_DVA_MESSAGE_1_SUB_OFFSET    = 1;
const size_t TABLE_1303_DVA_MESSAGE_2_SUB_OFFSET    = 3;
const size_t TABLE_1303_DVA_MESSAGE_3_SUB_OFFSET    = 5;
const size_t TABLE_1303_DVA_MESSAGE_4_SUB_OFFSET    = 7;
const size_t TABLE_1303_DWELL_SUB_OFFSET            = 9;
const size_t TABLE_1303_PERIOD_SUB_OFFSET           = 11;
const size_t TABLE_1303_START_TIME_SUB_OFFSET       = 13;
const size_t TABLE_1303_STOP_TIME_SUB_OFFSET        = 17;
const size_t TABLE_1303_EVENT_TRIGGERED_SUB_OFFSET  = 21;
const size_t TABLE_1303_GLOBAL_COVERAGE_SUB_OFFSET  = 22;

const size_t PAS_NO_MESSAGE_ID                      = 0;

namespace TA_IRS_App
{
    Table1303::Table1303(unsigned long locationKey)
        : AbstractPASTable(1303),
        m_locationKey(locationKey)
    {
        FUNCTION_ENTRY("Constructor");
        TA_ASSERT(CachedConfig::getInstance()->getIsMaster(), "Master only table created for station");

        m_messageSequenceDescriptions.resize(MAXMSGSEQ);

        FUNCTION_EXIT;
    }

    void Table1303::processRead()
    {
        LOG_SCOPE("Table1303::processRead");

        ThreadGuard guard(m_lock);

        unsigned long offset = 0;
        unsigned long messageId = 0;
        std::uint32_t localCoverageBitmap = 0;

        for (int sequenceIdMinus1 = 0; sequenceIdMinus1 < MAXMSGSEQ; ++sequenceIdMinus1)
        {
            m_messageSequenceDescriptions[sequenceIdMinus1].m_hasChime =
                PasHelpers::instance().get8bit(m_buffer, TABLE_1303_CHIME_SUB_OFFSET + TABLE_1303_RECORD_SIZE * sequenceIdMinus1) ? true : false;

            m_messageSequenceDescriptions[sequenceIdMinus1].m_dwellInSecs =
                PasHelpers::instance().get16bit(m_buffer, TABLE_1303_DWELL_SUB_OFFSET + TABLE_1303_RECORD_SIZE * sequenceIdMinus1);

            m_messageSequenceDescriptions[sequenceIdMinus1].m_periodInSecs =
                PasHelpers::instance().get16bit(m_buffer, TABLE_1303_PERIOD_SUB_OFFSET + TABLE_1303_RECORD_SIZE * sequenceIdMinus1);

            m_messageSequenceDescriptions[sequenceIdMinus1].m_startTime =
                PasHelpers::instance().get32bit(m_buffer, TABLE_1303_START_TIME_SUB_OFFSET + TABLE_1303_RECORD_SIZE * sequenceIdMinus1);

            m_messageSequenceDescriptions[sequenceIdMinus1].m_stopTime =
                PasHelpers::instance().get32bit(m_buffer, TABLE_1303_STOP_TIME_SUB_OFFSET + TABLE_1303_RECORD_SIZE * sequenceIdMinus1);

            m_messageSequenceDescriptions[sequenceIdMinus1].m_isEventTriggered =
                PasHelpers::instance().get8bit(m_buffer, TABLE_1303_EVENT_TRIGGERED_SUB_OFFSET + TABLE_1303_RECORD_SIZE * sequenceIdMinus1) ? true : false;

            // Convert local message Ids to message keys
            m_messageSequenceDescriptions[sequenceIdMinus1].m_messages.length(4);

            messageId = PasHelpers::instance().get16bit(m_buffer, TABLE_1303_DVA_MESSAGE_1_SUB_OFFSET + TABLE_1303_RECORD_SIZE * sequenceIdMinus1);
            m_messageSequenceDescriptions[sequenceIdMinus1].m_messages[0] = getMessageKeyAndValidate(messageId);

            messageId = PasHelpers::instance().get16bit(m_buffer, TABLE_1303_DVA_MESSAGE_2_SUB_OFFSET + TABLE_1303_RECORD_SIZE * sequenceIdMinus1);
            m_messageSequenceDescriptions[sequenceIdMinus1].m_messages[1] = getMessageKeyAndValidate(messageId);

            messageId = PasHelpers::instance().get16bit(m_buffer, TABLE_1303_DVA_MESSAGE_3_SUB_OFFSET + TABLE_1303_RECORD_SIZE * sequenceIdMinus1);
            m_messageSequenceDescriptions[sequenceIdMinus1].m_messages[2] = getMessageKeyAndValidate(messageId);

            messageId = PasHelpers::instance().get16bit(m_buffer, TABLE_1303_DVA_MESSAGE_4_SUB_OFFSET + TABLE_1303_RECORD_SIZE * sequenceIdMinus1);
            m_messageSequenceDescriptions[sequenceIdMinus1].m_messages[3] = getMessageKeyAndValidate(messageId);

            // Copy the global coverage bitmap and also Convert to PA Zone Keys

            unsigned long absoluteZoneId = 1;
            unsigned long offset = TABLE_1303_GLOBAL_COVERAGE_SUB_OFFSET;

            for (int stationId = 1; stationId <= MAXSTNID; ++stationId)
            {
                localCoverageBitmap = PasHelpers::instance().get32bit(m_buffer, offset + TABLE_1303_RECORD_SIZE * sequenceIdMinus1);
                m_messageSequenceDescriptions[sequenceIdMinus1].m_globalCoverageBitmaps[stationId - 1] = localCoverageBitmap;
                offset += 4;
            }

            PasHelpers::instance().globalCoverageBitmapsToCoverage(
                m_messageSequenceDescriptions[sequenceIdMinus1].m_globalCoverageBitmaps,
                m_messageSequenceDescriptions[sequenceIdMinus1].m_zones);
        }
    }

    unsigned long Table1303::getMessageKeyAndValidate(unsigned long messageId)
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
                LOG_ERROR("Invalid message Id in table 1303 returned from PAS");
            }
        }

        // Not set
        return -1;
    }

    bool Table1303::hasStartTimeElapsed(unsigned long sequenceId)
    {
        ThreadGuard guard(m_lock);

        return
            (m_messageSequenceDescriptions[sequenceId - 1].m_startTime == 0 ||
             time(NULL) >= m_messageSequenceDescriptions[sequenceId - 1].m_startTime);
    }

    TA_Base_Core::DateTime Table1303::getStartTime(unsigned long sequenceId)
    {
        ThreadGuard guard(m_lock);

        return m_messageSequenceDescriptions[sequenceId - 1].m_startTime;
    }

    TA_Base_Core::DateTime Table1303::getStopTime(unsigned long sequenceId)
    {
        ThreadGuard guard(m_lock);

        return m_messageSequenceDescriptions[sequenceId - 1].m_stopTime;
    }

    bool Table1303::isMessageSequenceCyclic(unsigned long sequenceId)
    {
        ThreadGuard guard(m_lock);

        if (m_messageSequenceDescriptions[sequenceId - 1].m_startTime != 0 ||
            m_messageSequenceDescriptions[sequenceId - 1].m_stopTime != 0)
        {
            return true;
        }

        return false;
    }

    ReadTable1303::ReadTable1303(Table1303& table,
                                 Table560& table560,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASReadEvent(socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_table560(table560)
    {
    }

    void ReadTable1303::readTable()
    {
        LOG_SCOPE("ReadTable1303::readTable");

        ThreadGuard guard(m_table.m_lock);

        PASConnection::getInstance()->readTable(m_table.TABLE_NUMBER,
                                                m_table.m_buffer,
                                                m_table.BUFFER_SIZE);
        m_processScheduler.post(new ProcessTable1303(m_table));

        m_table560.resetFlags(this,
                              Table560::TABLE_560_TABLE_1303_FLAG_A_BIT_OFFSET,
                              Table560::TABLE_560_TABLE_1303_FLAG_B_BIT_OFFSET);
    }

    ProcessTable1303::ProcessTable1303(Table1303& table)
        : m_table(table)
    {
    }

    void ProcessTable1303::consume()
    {
        LOG_SCOPE("ProcessTable1303::consume");

        m_table.processRead();
        PasTableReadCounter::instance().increase(m_table.TABLE_NUMBER);

        delete this;
    }

    void ProcessTable1303::cancel()
    {
        LOG_SCOPE("ProcessTable1303::cancel");
        delete this;
    }
}
