/*
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source : $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/PASReadEvent.h $
 * @author  J. Dalin
 * @version $Revision: #3 $
 * Last modification : $DateTime: 2025/01/06 12:26:20 $
 * Last modified by : $Author: limin.zhu $
 *
 *
 */

#pragma once
#include "app/pa/PAAgent/src/PASEvent.h"

namespace TA_IRS_App
{
    class PASReadEvent : public PASEvent
    {
    public:

        PASReadEvent(TA_IRS_Bus::Scheduler& socketScheduler,
                     TA_IRS_Bus::Scheduler& processScheduler,
                     IPasEventSource& eventSource);

        PASReadEvent(ACE_Time_Value timeToExpire,
                     TA_IRS_Bus::Scheduler& socketScheduler,
                     TA_IRS_Bus::Scheduler& processScheduler,
                     IPasEventSource& eventSource);

        void consume() override;

        /** This function is called by the consume function and should
          * be overridden in all subclasses to perform a write to a table.
          * Subclasses are assured that they are the only ones reading from (or writing)
          * the PASConnection class at the point this function is executing.
          */
        virtual void readTable() = 0;

        using IEvent::getNextTimeToExpire;
        ACE_Time_Value getNextTimeToExpire();
    };
}
