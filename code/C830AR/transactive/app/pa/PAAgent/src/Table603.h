/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table603.h $
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

namespace TA_Base_Core
{
    class CommsMessageSender;
}

namespace TA_IRS_App
{
    class IPasEventSource;

    struct MessageLabelType
    {
        std::string m_label;
        char        m_type = 'N';
    };

    class Table603 : public AbstractPASTable
    {
        friend class ReadTable603;
        friend class ProcessTable603;

    public:

        Table603();

    protected:

        virtual void processRead();
        //throw (TA_Base_Core::DatabaseException, TA_Base_Core::PasConnectionException);

        void updateDatabaseAndBroadcastUpdates();
        //throw (TA_Base_Core::DatabaseException, TA_Base_Core::PasConnectionException);

    protected:

        std::vector<MessageLabelType>   m_messageLabelTypes;
        unsigned long                   m_minMessageId = 0;
        unsigned long                   m_maxMessageId = 0;
        unsigned long                   m_locationKey = 0;
    };

    /**
     * Event that will read Table 603 from PAS.
     */
    class ReadTable603 : public TA_IRS_App::PASReadEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table The reference to Table 603.
         * @param table The reference to Table 560.
         * @param processScheduler The reference to the scheduler that will deal with process
         *                         events.
         */
        ReadTable603(Table603& table,
                     Table560& table560,
                     TA_IRS_Bus::Scheduler& socketScheduler,
                     TA_IRS_Bus::Scheduler& processScheduler,
                     IPasEventSource& eventSource);

        /**
         * readTable called by PASReadEvent::consume().  In this case, the table will be read from PAS.
         */
        void readTable() override;

    private:

        /** The reference to Table 603. */
        Table603& m_table;

        /** The reference to Table 560. */
        Table560& m_table560;
    };

    /**
     * Event that will process Table 603.
     */
    class ProcessTable603 : public TA_IRS_Bus::IEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table The reference to Table 603.
         */
        ProcessTable603(Table603& table);

        /**
         * Consumes this event.  In this case, the table will be read from PAS.
         * The event will then delete itself.
         */
        void consume() override;

        /** Cancels the event.  In this case, the event will delete itself. */
        void cancel() override;

    private:

        /** The reference to Table 603. */
        Table603& m_table;
    };
}
