/*
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source : $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/PASEvent.h $
 * @author  J. Dalin
 * @version $Revision: #4 $
 * Last modification : $DateTime: 2025/01/06 12:26:20 $
 * Last modified by : $Author: limin.zhu $
 *
 *
 */

#pragma once
#include "bus/event/src/Scheduler.h"

namespace TA_IRS_App
{
    class IPasEventSource;

    class PASEvent : public TA_IRS_Bus::IEvent
    {
    public:

        PASEvent(TA_IRS_Bus::Scheduler& socketScheduler,
                 TA_IRS_Bus::Scheduler& processScheduler,
                 IPasEventSource& eventSource);

        PASEvent(ACE_Time_Value timeToExpire,
                 TA_IRS_Bus::Scheduler& socketScheduler,
                 TA_IRS_Bus::Scheduler& processScheduler,
                 IPasEventSource& eventSource);

        void cancel() override;

    protected:

        static bool is_depot();
        static bool is_station();
        static bool is_station_or_depot();

        /** The scheduler responsible for socket bound events. */
        TA_IRS_Bus::Scheduler& m_socketScheduler;

        /** The scheduler responsible for process events. */
        TA_IRS_Bus::Scheduler& m_processScheduler;

        /** The source of the ConnectToPAFEPC event instance. */
        IPasEventSource& m_pasEventSource;
    };
}
