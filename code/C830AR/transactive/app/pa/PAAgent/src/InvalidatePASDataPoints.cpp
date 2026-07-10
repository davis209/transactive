/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/InvalidatePASDataPoints.cpp $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "InvalidatePASDataPoints.h"
#include "PasTableManager.h"
#include "core/utilities/src/DebugUtilEx.h"

namespace TA_IRS_App
{
    InvalidatePASDataPoints::InvalidatePASDataPoints(TA_IRS_Bus::Scheduler& processScheduler,
                                                     PasTableManager& pasTableManager)
        : m_processScheduler(processScheduler)
        , m_pasTableManager(pasTableManager)
    {
    }

    InvalidatePASDataPoints::~InvalidatePASDataPoints()
    {
    }

    void InvalidatePASDataPoints::consume()
    {
        LOG_SCOPE("InvalidatePASDataPoints::consume");
        m_processScheduler.post(new InvalidateTable550(m_pasTableManager.getTable550()));
        m_processScheduler.post(new InvalidateTable570(m_pasTableManager.getTable570()));
    }

    InvalidateStationPASDataPoints::InvalidateStationPASDataPoints(TA_IRS_Bus::Scheduler& processScheduler,
                                                                   PasTableManager& pasTableManager)
        : InvalidatePASDataPoints(processScheduler, pasTableManager)
    {
    }

    InvalidateStationPASDataPoints::~InvalidateStationPASDataPoints()
    {
    }

    void InvalidateStationPASDataPoints::consume()
    {
        LOG_SCOPE("InvalidateStationPASDataPoints::consume");
        InvalidatePASDataPoints::consume();

        m_processScheduler.post(new InvalidateTable200(m_pasTableManager.getTable200()));

        delete this;
    }

    void InvalidateStationPASDataPoints::cancel()
    {
        LOG_SCOPE("InvalidateStationPASDataPoints::cancel");
        delete this;
    }

    InvalidateMasterPASDataPoints::InvalidateMasterPASDataPoints(TA_IRS_Bus::Scheduler& processScheduler,
                                                                 PasTableManager& pasTableManager)
        : InvalidatePASDataPoints(processScheduler, pasTableManager)
    {
    }

    InvalidateMasterPASDataPoints::~InvalidateMasterPASDataPoints()
    {
    }

    void InvalidateMasterPASDataPoints::consume()
    {
        LOG_SCOPE("InvalidateMasterPASDataPoints::consume");
        InvalidatePASDataPoints::consume();

        m_processScheduler.post(new InvalidateTable1200(m_pasTableManager.getTable1200()));

        delete this;
    }

    void InvalidateMasterPASDataPoints::cancel()
    {
        LOG_SCOPE("InvalidateMasterPASDataPoints::cancel");
        delete this;
    }
}
