/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table1204.cpp $
 * @author:  Ripple
 * @version: $Revision: #8 $
 *
 * Last modification: $DateTime: 2025/06/24 11:25:13 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table1204.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "PASConnection.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"

using namespace TA_Base_Core;

const size_t TABLE_1204_RECORD_SIZE = 200;

namespace TA_IRS_App
{
    Table1204::Table1204()
        : AbstractPASTable(1204)
    {
        FUNCTION_ENTRY("Constructor");
        FUNCTION_EXIT;
    }

    void Table1204::processRead()
    {
        // Nothing to process immediately
    }

    TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage Table1204::getLocalCoverage(unsigned int sourceId)
        //throw ( TA_Base_Core::PasConnectionException )
    {
        TA_ASSERT(sourceId <= TA_Base_Bus::IPAAgentCorbaDef::PA_MAXSRCID, "sourceId out of bounds");

        ThreadGuard guard(m_lock);

        unsigned long offset = TABLE_1204_RECORD_SIZE * (sourceId - 1);

        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage coverage;

        PasHelpers::instance().globalCoverageBitmapsToCoverage(reinterpret_cast<std::uint32_t*>(&m_buffer[offset]), coverage);

        return coverage;
    }

    ReadTable1204::ReadTable1204(Table1204& table,
                                 Table560& table560,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASReadEvent(socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_table560(table560)
    {
    }

    void ReadTable1204::readTable()
    {
        LOG_SCOPE("ReadTable1204::readTable");

        ThreadGuard guard(m_table.m_lock);

        PASConnection::getInstance()->readTable(m_table.TABLE_NUMBER,
                                                m_table.m_buffer,
                                                m_table.BUFFER_SIZE);

        PasTableReadCounter::instance().increase(m_table.TABLE_NUMBER);

        m_table560.resetFlags(this,
                              Table560::TABLE_560_TABLE_1204_FLAG_A_BIT_OFFSET,
                              Table560::TABLE_560_TABLE_1204_FLAG_B_BIT_OFFSET);
    }
}
