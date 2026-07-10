/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table202.h $
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

    class Table202 : public AbstractPASTable
    {
        friend class ReadTable202;
        friend class ProcessTable202;

    public:

        Table202();

        CORBA::Octet getAnnounceId(unsigned int sourceId);
        //throw ( TA_Base_Core::PasConnectionException );

        /**
         * Returns the source id associated with the specified announceId.  A
         * return of 0 means the announce id does not exist in this table.
         * @param announceId The announce id to search for.
         * @return source id associated with provided announce id.
         */
        unsigned int getSourceId(CORBA::Octet announceId);

    protected:

        virtual void processRead();
    };

    /**
     * Event that will read Table 202 from PAS.
     */
    class ReadTable202 : public TA_IRS_App::PASReadEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table The reference to Table 202.
         * @param table560 The reference to Table 560.
         * @param socketScheduler The scheduler responsible for socket bound events.
         * @param socketScheduler The scheduler responsible for process events.
         * @param eventSource The source of Pas events.
         */
        ReadTable202(Table202& table,
                     Table560& table560,
                     TA_IRS_Bus::Scheduler& socketScheduler,
                     TA_IRS_Bus::Scheduler& processScheduler,
                     IPasEventSource& eventSource);

        /**
         * readTable called by PASReadEvent::consume().  In this case, this will simply read from PAS.
         */
        void readTable() override;

    private:

        /** The reference to Table202. */
        Table202& m_table;

        /** The reference to Table560. */
        Table560& m_table560;
    };
}
