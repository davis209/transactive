/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table1200.h $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "app/pa/PAAgent/src/Table200.h"
#include "app/pa/PAAgent/src/PASReadEvent.h"
#include "core/synchronisation/src/Condition.h"

class PasHelpers;

namespace TA_IRS_Bus
{
    class CachedMaps;
}

namespace TA_IRS_App
{
    class IPasEventSource;
    class Table202;
    class Table302;
    class Table1303;

    class Table1200 : public AbstractPASTable
    {
        friend class ReadTable1200;
        friend class ProcessTable1200;
        friend class InvalidateTable1200;

    public:

        // jeffrey++ Bug457
        int m_readIndex = 0;
        // ++jeffrey Bug457

        bool m_initialThread = true;// TD17013

        Table1200(Table202* table202, Table302* table302, Table1303* table1303);

        ZoneStatus getZoneStatus(unsigned int zoneId, unsigned int stationId);
        //throw ( TA_Base_Core::PasConnectionException );

        //liuyu++ TD11035
        TA_Base_Core::Condition& getCondition()
        {
            return m_readWait;
        }

        //++liuyu TD11035

        // ++ Bug456
        void Table1200::readTimedWait();
        // ++ Bug456

        // TD 17013
        void setInitalThreadValue(bool initialThreadValue);
        // TD 17013

    protected:

        virtual void processRead();
        virtual void invalidateAllDataPoints();

        typedef std::vector<ZoneStatus>     ZoneStatuses;
        typedef std::vector<ZoneStatuses>   StationStatuses;

        StationStatuses                     m_stationStatuses;
        TA_IRS_Bus::CachedMaps* m_cachedMapsInstance;

        // These tables are needed for cross-referencing to determine the 'cyclic' status of a zone
        Table202* m_table202;
        Table302* m_table302;
        Table1303* m_table1303;

    private:

        TA_Base_Core::Condition m_readWait;
    };

    /**
     * Event encapsulation to read Table 1200 from PAS.
     */
    class ReadTable1200 : public TA_IRS_App::PASReadEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Table 1200 instance.
         * @param socketScheduler The scheduler to resubmit a read event.
         * @param processScheduler The scheduler to resubmit a process event.
         * @param eventSource The source of PAS events.
         */
        ReadTable1200(Table1200& table,
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

        /**
         * Constructs an instance of this class.
         * @param expiryTime The time the event will expire.
         * @param table Table 1200 instance.
         * @param socketScheduler The scheduler to resubmit a read event.
         * @param processScheduler The scheduler to resubmit a process event.
         * @param eventSource The source of PAS events.
         */
        ReadTable1200(ACE_Time_Value expiryTime,
                      Table1200& table,
                      TA_IRS_Bus::Scheduler& socketScheduler,
                      TA_IRS_Bus::Scheduler& processScheduler,
                      IPasEventSource& eventSource);

        /** The table that this is reading for. */
        Table1200& m_table;
    };

    /**
     * Event encapsulation to process Table 200.
     */
    class ProcessTable1200 : public TA_IRS_Bus::IEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Table 1200 instance.
         */
        ProcessTable1200(Table1200& table);

        /**
         * Consumes the event.  In this case, the event will update the
         * appropriate datapoints.
         */
        void consume() override;

        /** Cancels the event.  In this case, the event deletes itself. */
        void cancel() override;

    private:

        /** The table that this is processing. */
        Table1200& m_table;
    };

    /**
     * Event encapsulation to invalidate all datapoints associated with Table 200.
     */
    class InvalidateTable1200 : public TA_IRS_Bus::IEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Table 1200 instance.
         */
        InvalidateTable1200(Table1200& table);

        /**
         * Consumes the event.  In this case, the event will update the
         * appropriate datapoints.
         */
        void consume() override;

        /** Cancels the event.  In this case, the event deletes itself. */
        void cancel() override;

    private:

        /** The table that this is processing. */
        Table1200& m_table;
    };
}
