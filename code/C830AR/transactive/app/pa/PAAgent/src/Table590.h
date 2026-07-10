/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table590.h $
 * @author:  Ripple
 * @version: $Revision: #5 $
 *
 * Last modification: $DateTime: 2025/01/13 19:01:23 $
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

    // TABLE 590 (PRESS TO TALK STATUS)
    class Table590 : public AbstractPASTable
    {
        friend class ReadTable590;

    public:

        Table590();

        bool isPttStatusOn(unsigned int pagingConsoleSourceId);
        //throw ( TA_Base_Core::PasConnectionException );

    protected:

        virtual void processRead();
        bool m_is_on = false;
    };

    /**
     * Event that will read Table 590 from PAS.
     */
    class ReadTable590 : public TA_IRS_App::PASReadEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table The reference to Table 590.
         * @param table560 The reference to Table 560.
         */
        ReadTable590(Table590& table,
                     Table560& table560,
                     TA_IRS_Bus::Scheduler& socketScheduler,
                     TA_IRS_Bus::Scheduler& processScheduler,
                     IPasEventSource& eventSource);

        /**
         * readTable called by PASReadEvent::consume().  In this case, Table 590 is read from PAS.
         */
        void readTable() override;

    private:

        /** The reference to Table 590. */
        Table590& m_table;

        /** The reference to Table 560. */
        Table560& m_table560;
    };
}
