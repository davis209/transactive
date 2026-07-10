/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/DisconnectFromPAFEPC.h $
 * @author:  Ripple
 * @version: $Revision: #3 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "app/pa/PAAgent/src/PASConnection.h"
#include "bus/event/src/Scheduler.h"

namespace TA_IRS_App
{
    class PasTableManager;

    /**
     * Event that will connect to the PA FEPC.
     */
    class DisconnectFromPAFEPC : public TA_IRS_Bus::IEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param socketScheduler The reference to the scheduler responsible for
         *                        socket bound events.
         * @param processScheduler The reference to the scheduler responsible for
         *                         processing events.
         * @param pasTableManager The reference to the PAS table manager to get
         *                        the relevent tables if this event should
         *                        succeed.
         * @param pasConnection The PAS connection instance.
         */
        DisconnectFromPAFEPC(TA_IRS_Bus::Scheduler& socketScheduler,
                             TA_IRS_Bus::Scheduler& processScheduler,
                             PasTableManager& pasTableManager,
                             PASConnection& pasConnection);
        /**
         * Constructs an instance of this class.
         * @param expiryTime The time this event is to expire.
         * @param socketScheduler The reference to the scheduler responsible for
         *                        socket bound events.
         * @param processScheduler The reference to the scheduler responsible for
         *                         processing events.
         * @param pasTableManager The reference to the PAS table manager to get
         *                        the relevent tables if this event should
         *                        succeed.
         * @param pasConnection The PAS connection instance.
         */
        DisconnectFromPAFEPC(ACE_Time_Value expiryTime,
                             TA_IRS_Bus::Scheduler& socketScheduler,
                             TA_IRS_Bus::Scheduler& processScheduler,
                             PasTableManager& pasTableManager,
                             PASConnection& pasConnection);

        /** Destructs this instance of this class. */
        virtual ~DisconnectFromPAFEPC();

        /**
         * Method that is invoked when the event is consumed.  In this case, the
         * event will attempt to disconnect from the PA FEPC.
         * This method will destroy all write events stored in writeable table classes.
         */
        void consume() override;

        /**
         * Method that is invoked when the event is cancelled.
         */
        void cancel() override;

    protected:

        /** Destroy all writeable table events */
        void destroyAllWriteablePASTableEvents();

    protected:

        /** The scheduler responsible for all socket bound events. */
        TA_IRS_Bus::Scheduler& m_socketScheduler;
        /** The scheduler responsible for all processing events. */
        TA_IRS_Bus::Scheduler& m_processScheduler;
        /** The PAS table manager. */
        PasTableManager& m_pasTableManager;
        /** The PAS connection instance. */
        PASConnection& m_pasConnection;
    };
}
