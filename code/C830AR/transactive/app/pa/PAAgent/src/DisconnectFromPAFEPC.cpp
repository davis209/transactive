/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/DisconnectFromPAFEPC.cpp $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "DisconnectFromPAFEPC.h"
#include "PasTableManager.h"
#include "core/utilities/src/DebugUtilEx.h"

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    DisconnectFromPAFEPC::DisconnectFromPAFEPC(TA_IRS_Bus::Scheduler& socketScheduler,
                                               TA_IRS_Bus::Scheduler& processScheduler,
                                               PasTableManager& pasTableManager,
                                               PASConnection& pasConnection)
        : m_socketScheduler(socketScheduler)
        , m_processScheduler(processScheduler)
        , m_pasTableManager(pasTableManager)
        , m_pasConnection(pasConnection)
    {
    }

    DisconnectFromPAFEPC::DisconnectFromPAFEPC(ACE_Time_Value expiryTime,
                                               TA_IRS_Bus::Scheduler& socketScheduler,
                                               TA_IRS_Bus::Scheduler& processScheduler,
                                               PasTableManager& pasTableManager,
                                               PASConnection& pasConnection)
        : TA_IRS_Bus::IEvent(expiryTime)
        , m_socketScheduler(socketScheduler)
        , m_processScheduler(processScheduler)
        , m_pasTableManager(pasTableManager)
        , m_pasConnection(pasConnection)
    {
    }

    DisconnectFromPAFEPC::~DisconnectFromPAFEPC()
    {
    }

    void DisconnectFromPAFEPC::consume()
    {
        LOG_SCOPE("DisconnectFromPAFEPC::consume");
        LOG_DEBUG("Consuming DisconnectFromPAFEPC event.");

        destroyAllWriteablePASTableEvents();
        delete this;
    }

    void DisconnectFromPAFEPC::cancel()
    {
        LOG_SCOPE("DisconnectFromPAFEPC::cancel");
        delete this;
    }

    void DisconnectFromPAFEPC::destroyAllWriteablePASTableEvents()
    {
        LOG_SCOPE("DisconnectFromPAFEPC::destroyAllWriteablePASTableEvents");

        auto writeableTables = m_pasTableManager.getAllWriteableTables();

        // destroy all the table write events
        for (auto writeableTable : writeableTables)
        {
            writeableTable->destroyCurrentWriteEvents(m_socketScheduler);
        }
    }
}
