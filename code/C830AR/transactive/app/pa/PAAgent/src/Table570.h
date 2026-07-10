/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table570.h $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "app/pa/PAAgent/src/PASReadEvent.h"
#include "app/pa/PAAgent/src/PASWriteEvent.h"
#include "app/pa/PAAgent/src/AbstractWriteablePASTable.h"
#include "app/pa/PAAgent/src/Table560.h"
#include "ace/Future.h"

namespace TA_IRS_App
{
    class IPasEventSource;

    class Table570 : public TA_IRS_App::AbstractWriteablePASTable
    {
        friend class ReadTable570;
        friend class ProcessTable570;
        friend class InvalidateTable570;
        friend class WriteTable570;

    public:

        Table570(TA_IRS_Bus::Scheduler& socketScheduler,
                 TA_IRS_Bus::Scheduler& processScheduler,
                 IPasEventSource& eventSource);

        void resetFireAlarmStatusAndWrite();

        // TD18294
        bool isOnFireAlarmState();
        // TD18294

    protected:

        virtual void processRead();
        virtual void invalidateAllDataPoints();

    private:

        Table570(const Table570& theTable570);
        virtual ~Table570();

        TA_IRS_Bus::Scheduler& m_socketScheduler;
        /** Scheduler to add new process event instance to. */
        TA_IRS_Bus::Scheduler& m_processScheduler;
        IPasEventSource& m_pasEventSource;

        // TD18294
        bool m_isOnFireAlarm = false;
        // TD18294

        // Maochun Sun++
        // bug774 (TD13071)
        std::uint8_t* m_writeBuffer = nullptr;
        // ++Maochun Sun
        // bug774 (TD13071)
    };

    /**
     * Event encapsulation to read Table 570 from PAS.
     */
    class ReadTable570 : public TA_IRS_App::PASReadEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Table 570 instance.
         * @param table560 Table 560 instance.
         * @param socketScheduler The scheduler to resubmit a read event.
         */
        ReadTable570(Table570& table,
                     Table560& table560,
                     TA_IRS_Bus::Scheduler& socketScheduler,
                     TA_IRS_Bus::Scheduler& processScheduler,
                     IPasEventSource& eventSource);

        /**
         * readTable called by PASReadEvent::consume().  In this case, the event will merely read the table
         * and then submit another instance of this event to expire the configured
         * poll cycle.
         */
        void readTable() override;

    private:

        /** The table that this is reading for. */
        Table570& m_table;

        /** The reference to Table560. */
        Table560& m_table560;
    };

    /**
     * Event encapsulation to process Table 570.
     */
    class ProcessTable570 : public TA_IRS_Bus::IEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Table 570 instance.
         */
        ProcessTable570(Table570& table);

        /**
         * Consumes the event.  In this case, the event will update the
         * appropriate datapoints.
         */
        void consume() override;

        /** Cancels the event.  In this case, the event deletes itself. */
        void cancel() override;

    private:

        /** The table that this is processing. */
        Table570& m_table;
    };

    /**
     * Event that will write this table to PAS.
     */
    class WriteTable570 : public TA_IRS_App::PASWriteEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table The reference to Table 570 instance.
         */
        WriteTable570(Table570& table,
                      TA_IRS_Bus::Scheduler& socketScheduler,
                      TA_IRS_Bus::Scheduler& processScheduler,
                      IPasEventSource& eventSource);
        /**
         * Constructs an instance of this class.
         * @param timeToExpire The time the event is consumed in the scheduler.
         * @param table The reference to Table 570 instance.
         */
        WriteTable570(ACE_Time_Value timeToExpire,
                      Table570& table,
                      TA_IRS_Bus::Scheduler& socketScheduler,
                      TA_IRS_Bus::Scheduler& processScheduler,
                      IPasEventSource& eventSource);

        /**
         * writeTable called by PASWriteEvent::consume().
         * In this case, the event will write out the necessary data.
         */
        virtual void writeTable();

        /* This function is used to recreate the PASWriteEvent with a new
         * timeToExpire.
         */
        virtual PASWriteEvent* recreateEvent(ACE_Time_Value timeToExpire);

    private:

        /** The reference to the Table 201 instance. */
        Table570& m_table;
    };

    /**
     * Event encapsulation to invalidate all datapoints associated with Table 570.
     */
    class InvalidateTable570 : public TA_IRS_Bus::IEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Table 570 instance.
         */
        InvalidateTable570(Table570& table);

        /**
         * Consumes the event.  In this case, the event will update the
         * appropriate datapoints.
         */
        void consume() override;

        /** Cancels the event.  In this case, the event deletes itself. */
        void cancel() override;

    private:

        /** The table that this is processing. */
        Table570& m_table;
    };
}
