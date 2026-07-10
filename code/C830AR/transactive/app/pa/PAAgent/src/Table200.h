/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table200.h $
 * @author:  Ripple
 * @version: $Revision: #5 $
 *
 * Last modification: $DateTime: 2025/01/13 19:01:23 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "app/pa/PAAgent/src/PASReadEvent.h"
#include "app/pa/PAAgent/src/AbstractPASTable.h"
#include "core/synchronisation/src/Condition.h"
#include <boost/describe.hpp>

namespace TA_Base_Bus
{
    class DataPoint;
}

namespace TA_IRS_Bus
{
    class CachedMaps;
}

class PasHelpers;

namespace TA_IRS_App
{
    class IPasEventSource;
    class Table202;
    class Table302;
    class Table303;

    struct ZoneStatus
    {
        TA_Base_Bus::IPAAgentCorbaDef::ESource m_sourceId = TA_Base_Bus::IPAAgentCorbaDef::PA_SRC_0;
        bool m_status = false;
        CORBA::Octet m_priority = 0;
    };

    BOOST_DESCRIBE_STRUCT(TA_IRS_App::ZoneStatus, (), (m_sourceId, m_status, m_priority));

    class Table200 : public AbstractPASTable
    {
        friend class ReadTable200;
        friend class ProcessTable200;
        friend class InvalidateTable200;

    public:

        // jeffrey++ Bug457
        int m_readIndex;
        // ++jeffrey Bug457

        Table200(Table202* table202, Table302* table302, Table303* table303);

        ZoneStatus getZoneStatus(unsigned int zoneId);

        //liuyu++ TD11035
        TA_Base_Core::Condition& getCondition()
        {
            return m_readWait;
        }

        //++liuyu TD11035

        // ++ Bug456
        void readTimedWait();
        // ++ Bug456

        // TD 17013
        void setInitalThreadValue(bool initialThreadValue);
        // TD 17013

    private:

        Table200(const Table200& theTable200);
        virtual ~Table200();

    protected:

        virtual void processRead();
        virtual void invalidateAllDataPoints();

        std::vector<ZoneStatus> m_zoneStatuses;
        TA_IRS_Bus::CachedMaps* m_cachedMapsInstance = nullptr;

        bool m_initialThread = true; // TD17013

        // These tables are needed for cross-referencing to determine the 'cyclic' status of a zone
        Table202* m_table202 = nullptr;
        Table302* m_table302 = nullptr;
        Table303* m_table303 = nullptr;

        TA_Base_Core::Condition m_readWait;
    };

    /**
     * Event encapsulation to read Table 200 from PAS.
     */
    class ReadTable200 : public TA_IRS_App::PASReadEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Table 200 instance.
         * @param socketScheduler The scheduler to resubmit a read event.
         * @param processScheduler The scheduler to resubmit a process event.
         */
        ReadTable200(Table200& table,
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
         * @param table Table 200 instance.
         * @param socketScheduler The scheduler to resubmit a read event.
         * @param processScheduler The scheduler to resubmit a process event.
         */
        ReadTable200(ACE_Time_Value expiryTime,
                     Table200& table,
                     TA_IRS_Bus::Scheduler& socketScheduler,
                     TA_IRS_Bus::Scheduler& processScheduler,
                     IPasEventSource& eventSource);

        /** The table that this is reading for. */
        Table200& m_table;
    };

    /**
     * Event encapsulation to process Table 200.
     */
    class ProcessTable200 : public TA_IRS_Bus::IEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Table 200 instance.
         */
        ProcessTable200(Table200& table);

        /**
         * Consumes the event.  In this case, the event will update the
         * appropriate datapoints.
         */
        void consume() override;

        /** Cancels the event.  In this case, the event deletes itself. */
        void cancel() override;

    private:

        /** The table that this is processing. */
        Table200& m_table;
    };

    /**
     * Event encapsulation to invalidate all datapoints associated with Table 200.
     */
    class InvalidateTable200 : public TA_IRS_Bus::IEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Table 200 instance.
         */
        InvalidateTable200(Table200& table);

        /**
         * Consumes the event.  In this case, the event will update the
         * appropriate datapoints.
         */
        void consume() override;

        /** Cancels the event.  In this case, the event deletes itself. */
        void cancel() override;

    private:

        /** The table that this is processing. */
        Table200& m_table;
    };
}
