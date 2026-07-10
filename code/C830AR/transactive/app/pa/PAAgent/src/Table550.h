/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table550.h $
 * @author:  Ripple
 * @version: $Revision: #3 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "app/pa/PAAgent/src/PASReadEvent.h"
#include "app/pa/PAAgent/src/AbstractPASTable.h"
#include "app/pa/PAAgent/src/Table560.h"

namespace TA_IRS_App
{
    class IPasEventSource;

    class Table550 : public AbstractPASTable
    {
        friend class ReadTable550;
        friend class ProcessTable550;
        friend class InvalidateTable550;

    public:

        Table550();

    protected:

        virtual void processRead();
        virtual void invalidateAllDataPoints();
    };

    /**
     * Event that will read Table 550 from PAS.
     */
    class ReadTable550 : public TA_IRS_App::PASReadEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Reference to Table 550.
         * @param table560 Reference to Table 560.
         * @param processScheduler The reference to the scheduler responsible for
         *                         process events.
         */
        ReadTable550(Table550& table,
                     Table560& table560,
                     TA_IRS_Bus::Scheduler& socketScheduler,
                     TA_IRS_Bus::Scheduler& processScheduler,
                     IPasEventSource& eventSource);

        /**
         * readTable called by PASReadEvent::consume().  In this case, this event will read table 550 from
         * the PAS and then delete itself.
         */
        void readTable() override;

    private:

        /** Reference to Table 550. */
        Table550& m_table;

        /** Reference to Table 560. */
        Table560& m_table560;
    };

    /**
     * Event that will process Table 550.
     */
    class ProcessTable550 : public TA_IRS_Bus::IEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Reference to Table 550.
         */
        ProcessTable550(Table550& table);

        /**
         * Consumes this event.  In this case, this event will process table 550
         * and then delete itself.
         */
        void consume() override;

        /** Cancels this event.  In this case, the event will delete itself. */
        void cancel() override;

    private:

        /** Reference to Table 550. */
        Table550& m_table;
    };

    /**
     * Event that will invalidate datapoints associated with Table 550.
     */
    class InvalidateTable550 : public TA_IRS_Bus::IEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Reference to Table 550.
         */
        InvalidateTable550(Table550& table);

        /**
         * Consumes this event.  In this case, this event will process table 550
         * and then delete itself.
         */
        void consume() override;

        /** Cancels this event.  In this case, the event will delete itself. */
        void cancel() override;

    private:

        /** Reference to Table 550. */
        Table550& m_table;
    };
}
