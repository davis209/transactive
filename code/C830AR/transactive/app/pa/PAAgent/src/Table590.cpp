/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table590.cpp $
 * @author:  Ripple
 * @version: $Revision: #8 $
 *
 * Last modification: $DateTime: 2025/06/24 11:25:13 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table590.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "PASConnection.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"

using namespace TA_Base_Core;

namespace
{
    const size_t TABLE_590_PTT_STATUS_OFFSET = 0;

#if 0
    std::map<TA_Base_Bus::IPAAgentCorbaDef::ESource, size_t> SOURCE_ID_TO_TABLE_INDEX =
    {
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_1_PAGING_CONSOLE_MFT_STATION_SO,     0},     // STATION MFT (SO) Paging Console
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_2_PAGING_CONSOLE_MFT_DEPOT_DCO,      1},     // DCR MFT (DCO) Paging Console
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_3_PAGING_CONSOLE_MFT_DEPOT_DPSCO,    2},     // DCR MFT (PSCO) Paging Console
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_4_PAGING_CONSOLE_MFT_OCR_CS,         3},     // OCC/BOCC MFT (CS) Paging Console
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_5_PAGING_CONSOLE_MFT_OCR_TCO_1,      4},     // OCC/BOCC MFT (TCO 1) Paging Console
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_6_PAGING_CONSOLE_MFT_OCR_TCO_2,      5},     // OCC/BOCC MFT (TCO 2) Paging Console
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_7_PAGING_CONSOLE_MFT_OCR_TCO_3,      6},     // OCC/BOCC MFT (TCO 3) Paging Console
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_8_PAGING_CONSOLE_MFT_OCR_SCO_1,      7},     // OCC/BOCC MFT (SCO 1) Paging Console
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_9_PAGING_CONSOLE_MFT_OCR_SCO_2,      8},     // OCC/BOCC MFT (SCO 2) Paging Console
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_10_PAGING_CONSOLE_MFT_OCR_MCO_1,     9},     // OCC/BOCC MFT (MCO 1) Paging Console
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_11_PAGING_CONSOLE_MFT_OCR_MCO_2,     10},    // OCC/BOCC MFT (MCO 2) Paging Console
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_12_PAGING_CONSOLE_MFT_OCR_PSCO,      11},     // OCC/BOCC MFT (PSCO) Paging Console
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_13_PAGING_CONSOLE_MFT_OCR_TCO_4,     12},    // OCC/BOCC MFT (TCO 4) Paging Console
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_14_BDCR_BGM,                         13},    // BDCR BGM
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_15_PAGING_CONSOLE_PCP_STATION_1,     14},    // STATION PCP Paging Console 1 (Platform Communication Point 1)
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_28_PAGING_CONSOLE_MFT_BDCR_DCO,      15},    // BDCR MFT (DCO) Paging Console
        {TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_29_PAGING_CONSOLE_MFT_BDCR_PSCO,     16},    // BDCR MFT (PSCO) Paging Console
    };
#endif
}

namespace TA_IRS_App
{
    Table590::Table590()
        : AbstractPASTable(590)
    {
        FUNCTION_ENTRY("Constructor");
        FUNCTION_EXIT;
    }

    void Table590::processRead()
    {
        // No processing required
    }

    bool Table590::isPttStatusOn(unsigned int pagingConsoleSourceId)
        //throw ( TA_Base_Core::PasConnectionException )
    {
        TA_ASSERT(pagingConsoleSourceId > 0 && pagingConsoleSourceId <= TA_Base_Bus::IPAAgentCorbaDef::PA_MAX_MFT_PAGING_CONSOLE, "Paging console source Id out of range");

        ThreadGuard guard(m_lock);

        auto is_on = PasHelpers::instance().getBit(m_buffer, pagingConsoleSourceId - 1);
        LOG_DEBUG_IF(m_is_on != is_on, "isPttStatusOn(): pagingConsoleSourceId=%d, %d => %d", pagingConsoleSourceId, m_is_on, is_on);
        return m_is_on = is_on;
    }

    ReadTable590::ReadTable590(Table590& table,
                               Table560& table560,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASReadEvent(socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_table560(table560)
    {
    }

    void ReadTable590::readTable()
    {
        LOG_SCOPE("ReadTable590::readTable");

        ThreadGuard guard(m_table.m_lock);

        PASConnection::getInstance()->readTable(m_table.TABLE_NUMBER,
                                                m_table.m_buffer,
                                                m_table.BUFFER_SIZE);

        PasTableReadCounter::instance().increase(m_table.TABLE_NUMBER);

        m_table560.resetFlags(this,
                              Table560::TABLE_560_TABLE_590_FLAG_A_BIT_OFFSET,
                              Table560::TABLE_560_TABLE_590_FLAG_B_BIT_OFFSET);
    }
}
