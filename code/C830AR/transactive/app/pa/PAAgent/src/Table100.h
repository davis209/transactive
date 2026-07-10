/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table100.h $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 * TABLE 100 [PRIORITY FOR PA PHYSICAL SOURCE]
 */

#pragma once
#include "app/pa/PAAgent/src/AbstractPASTable.h"
#include "app/pa/PAAgent/src/Table560.h"
#include "app/pa/PAAgent/src/PASReadEvent.h"

namespace TA_Base_Core
{
    class CommsMessageSender;
}

namespace TA_IRS_App
{
    class IPasEventSource;

    class Table100 : public AbstractPASTable
    {
        friend class ReadTable100;
        friend class ProcessTable100;

    public:

        Table100();
        TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme* getPrioritySchemeCopy();

    private:

        void processRead();
        void notifyIscs();

    protected:

        TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme    m_priorityScheme;
    };

    /**
     * An event that will read Table 100 from the PAS.
     */
    class ReadTable100 : public TA_IRS_App::PASReadEvent
    {
    public:

        /** Constructs an instance of this class. */
        ReadTable100(Table100& table,
                     Table560& table560,
                     TA_IRS_Bus::Scheduler& socketScheduler,
                     TA_IRS_Bus::Scheduler& processScheduler,
                     IPasEventSource& eventSource);

        /**
         * readTable called by PASReadEvent::consume(), in this case it will read Table100 from PAS.
         * Once this is done, a ProcessTable100 event will be scheduled.
         */
        void readTable() override;

    private:

        /** Table 100 reference. */
        Table100& m_table;

        /** Table560 reference. */
        Table560& m_table560;
    };

    /**
     * An event that will process Table 100 and notify ISCS as appropriate.
     */
    class ProcessTable100 : public TA_IRS_Bus::IEvent
    {
    public:

        /** Constructs an instance of this class. */
        ProcessTable100(Table100& table);

        /**
         * Consumes the event, in this case it will process Table100 that ISCS has
         * stored.  This event will then delete itself.
         */
        void consume() override;

        /** Cancels the event.  In this case, the event will be deleted. */
        void cancel() override;

    private:

        /** Table 100 reference. */
        Table100& m_table;
    };
}
