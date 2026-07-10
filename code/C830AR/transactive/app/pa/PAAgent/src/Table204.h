/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table204.h $
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

    class Table204 : public AbstractPASTable
    {
        friend class ReadTable204;

    public:

        Table204();

        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage getLocalCoverage(unsigned int sourceId);
        //throw ( TA_Base_Core::PasConnectionException );

    protected:

        virtual void processRead();
    };

    /**
     * Event that will read Table 204 from PAS.
     */
    class ReadTable204 : public TA_IRS_App::PASReadEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table The reference to Table 204.
         * @param table560 The reference to Table 560.
         * @param socketScheduler The scheduler responsible for socket bound events.
         * @param processScheduler The scheduler responsible for process events.
         * @param eventSource The source of Pas events.
         */
        ReadTable204(Table204& table,
                     Table560& table560,
                     TA_IRS_Bus::Scheduler& socketScheduler,
                     TA_IRS_Bus::Scheduler& processScheduler,
                     IPasEventSource& eventSource);

        /**
         * readTable called by PASReadEvent::consume().  In this case, this will simply read from PAS.
         */
        void readTable() override;

    private:

        /** The reference to Table204. */
        Table204& m_table;

        /** The reference to Table560. */
        Table560& m_table560;
    };
}
