/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table202.cpp $
 * @author:  Ripple
 * @version: $Revision: #8 $
 *
 * Last modification: $DateTime: 2025/06/24 11:25:13 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table202.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "PASConnection.h"
#include "AbstractBroadcastCircularList.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    Table202::Table202()
        : AbstractPASTable(202)
    {
        FUNCTION_ENTRY("Constructor");
        FUNCTION_EXIT;
    }

    void Table202::processRead()
    {
        // No processing required
    }

    CORBA::Octet Table202::getAnnounceId(unsigned int sourceId)
        //throw ( TA_Base_Core::PasConnectionException )
    {
        TA_ASSERT(sourceId <= MAXSRCID, "sourceId out of bounds");

        ThreadGuard guard(m_lock);

        return PasHelpers::instance().get8bit(m_buffer, (sourceId - 1));
    }

    unsigned int Table202::getSourceId(CORBA::Octet announceId)
    {
        ThreadGuard guard(m_lock);

        for (unsigned int sourceId = 1; sourceId <= MAXSRCID; ++sourceId)
        {
            if (PasHelpers::instance().get8bit(m_buffer, (sourceId - 1)) == announceId)
            {
                return sourceId;
            }
        }

        return 0;
    }

    ReadTable202::ReadTable202(Table202& table,
                               Table560& table560,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASReadEvent(socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_table560(table560)
    {
    }

    void ReadTable202::readTable()
    {
        LOG_SCOPE("ReadTable202::readTable");
        ThreadGuard guard(m_table.m_lock);

        PASConnection::getInstance()->readTable(m_table.TABLE_NUMBER,
                                                m_table.m_buffer,
                                                m_table.BUFFER_SIZE);

        PasTableReadCounter::instance().increase(m_table.TABLE_NUMBER);

        m_table560.resetFlags(this,
                              Table560::TABLE_560_TABLE_202_FLAG_A_BIT_OFFSET,
                              Table560::TABLE_560_TABLE_202_FLAG_B_BIT_OFFSET);
    }
}
