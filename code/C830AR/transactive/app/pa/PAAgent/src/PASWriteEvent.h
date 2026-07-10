/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source : $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/PASWriteEvent.h $
 * @author  J. Dalin
 * @version $Revision: #3 $
 * Last modification : $DateTime: 2025/01/06 12:26:20 $
 * Last modified by : $Author: limin.zhu $
 *
 *
 */

#pragma once
#include "app/pa/PAAgent/src/PASEvent.h"
#include "app/pa/PAAgent/src/AbstractWriteablePASTable.h"
#include "ace/Future.h"

namespace TA_IRS_App
{
    class PASWriteEvent : public PASEvent
    {
    public:

        PASWriteEvent(AbstractWriteablePASTable& writeableTable,
                      ACE_Future<int> future,
                      TA_IRS_Bus::Scheduler& socketScheduler,
                      TA_IRS_Bus::Scheduler& processScheduler,
                      IPasEventSource& eventSource);

        PASWriteEvent(AbstractWriteablePASTable& writeableTable,
                      TA_IRS_Bus::Scheduler& socketScheduler,
                      TA_IRS_Bus::Scheduler& processScheduler,
                      IPasEventSource& eventSource);

        PASWriteEvent(ACE_Time_Value timeToExpire,
                      AbstractWriteablePASTable& writeableTable,
                      ACE_Future<int> future,
                      TA_IRS_Bus::Scheduler& socketScheduler,
                      TA_IRS_Bus::Scheduler& processScheduler,
                      IPasEventSource& eventSource);

        PASWriteEvent(ACE_Time_Value timeToExpire,
                      AbstractWriteablePASTable& writeableTable,
                      TA_IRS_Bus::Scheduler& socketScheduler,
                      TA_IRS_Bus::Scheduler& processScheduler,
                      IPasEventSource& eventSource);

        virtual ~PASWriteEvent();
        void consume() override;
        void cancel() override;

        /** This function is called by the consume function and should
          * be overridden in all subclasses to perform a write to a table.
          * Subclasses are assured that they are the only ones writing to (or reading)
          * the PASConnection class at the point this function is executing.
          */
        virtual void writeTable() = 0;

        /* This function is used to recreate the PASWriteEvent with a new
         * timeToExpire.
         */
        virtual PASWriteEvent* recreateEvent(ACE_Time_Value timeToExpire) = 0;

    protected:

        /** The future object to store the result of the write.
          * If the subclass is not interesting in knowing the result of the write
          * then the future can be left empty by using the constructor without
          * the future parameter.
          */
        ACE_Future<int> m_future;

        /** The table that this write event is interested in writing to. */
        AbstractWriteablePASTable& m_writeableTable;

    private:

        /* This future is used if no future is provided. This means that the thread
         * posting the event is not interested in the result. i.e. an asynchronous event
         */
        static ACE_Future<int> ignoredFuture;
    };
}
