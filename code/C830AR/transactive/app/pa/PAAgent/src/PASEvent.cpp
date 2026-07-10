/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/PASEvent.cpp $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "PASEvent.h"
#include "core/data_access_interface/src/LocationAccessFactoryEx.h"

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    PASEvent::PASEvent(TA_IRS_Bus::Scheduler& socketScheduler,
                       TA_IRS_Bus::Scheduler& processScheduler,
                       IPasEventSource& eventSource)
        : m_socketScheduler(socketScheduler),
        m_processScheduler(processScheduler),
        m_pasEventSource(eventSource)
    {
    }

    PASEvent::PASEvent(ACE_Time_Value timeToExpire,
                       TA_IRS_Bus::Scheduler& socketScheduler,
                       TA_IRS_Bus::Scheduler& processScheduler,
                       IPasEventSource& eventSource)
        : IEvent(timeToExpire),
        m_socketScheduler(socketScheduler),
        m_processScheduler(processScheduler),
        m_pasEventSource(eventSource)
    {
    }

    void PASEvent::cancel()
    {
        delete this;
    }

    bool PASEvent::is_depot()
    {
        return ThisLocation::is_depot();
    }

    bool PASEvent::is_station()
    {
        return ThisLocation::is_station();
    }

    bool PASEvent::is_station_or_depot()
    {
        return is_station() || is_depot();
    }
}
