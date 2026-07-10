/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table610.h $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "app/pa/PAAgent/src/PASReadEvent.h"
#include "app/pa/PAAgent/src/AbstractPASTable.h"
#include "app/pa/PAAgent/src/Table560.h"
#include "bus/pa/cachedmaps/src/CachedMapDefinitions.h"

namespace TA_Base_Core
{
    class CommsMessageSender;
}

namespace TA_IRS_App
{
    class IPasEventSource;

    class Table610 : public AbstractPASTable
    {
        friend class ReadTable610;
        friend class ProcessTable610;

    public:

        Table610();

    protected:

        virtual void processRead();

        bool processOneVersionId(std::string&, unsigned long offset);

        void updateDatabaseAndBroadcastUpdates(TA_IRS_Bus::DvaPrivateVersionRecord& currentDvaVersionRecord);
        //throw (TA_Base_Core::DatabaseException);

    protected:

        TA_IRS_Bus::DvaVersionRecord                m_localDvaVersionRecord;
    };

    /**
     * Event encapsulation to read Table 610.
     */
    class ReadTable610 : public TA_IRS_App::PASReadEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table The reference to Table 610.
         * @param table The reference to Table 560.
         * @param processScheduler The reference to the scheduler responsible for
         *                         process events.
         */
        ReadTable610(Table610& table,
                     Table560& table560,
                     TA_IRS_Bus::Scheduler& socketScheduler,
                     TA_IRS_Bus::Scheduler& processScheduler,
                     IPasEventSource& eventSource);

        /**
         * readTable called by PASReadEvent::consume(). This will simply read the table from PAS and
         * then post a ProcessTable610 event to the process scheduler.
         */
        void readTable() override;

    private:

        /** The reference to Table610. */
        Table610& m_table;

        /** The reference to Table560. */
        Table560& m_table560;
    };

    /**
     * Event encapsulation to process Table 610.
     */
    class ProcessTable610 : public TA_IRS_Bus::IEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Table 610 instance.
         */
        ProcessTable610(Table610& table);

        /**
         * Consumes the event.  In this case, the event will send the appropriate
         * comms message.
         */
        void consume() override;

        /** Cancels the event.  In this case, the event deletes itself. */
        void cancel() override;

    private:

        /** The table that this is processing. */
        Table610& m_table;
    };
}
