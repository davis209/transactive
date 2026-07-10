/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/ConnectToPAFEPC.h $
 * @author:  Ripple
 * @version: $Revision: #3 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "bus/event/src/Scheduler.h"
#include "app/pa/PAAgent/src/PASConnection.h"

namespace TA_IRS_App
{
    class PasTableManager;

    /**
     * Event that will connect to the PA FEPC.
     */
    class ConnectToPAFEPC : public TA_IRS_Bus::IEvent
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
        ConnectToPAFEPC(TA_IRS_Bus::Scheduler& socketScheduler,
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
        ConnectToPAFEPC(ACE_Time_Value expiryTime,
                        TA_IRS_Bus::Scheduler& socketScheduler,
                        TA_IRS_Bus::Scheduler& processScheduler,
                        PasTableManager& pasTableManager,
                        PASConnection& pasConnection);

        /** Destructs this instance of this class. */
        virtual ~ConnectToPAFEPC();
        /**
         * Method that is invoked when the event is consumed.  In this case, the
         * event will attempt to connect to the PA FEPC.  If this is successful,
         * this will schedule reads for ALL tables.  If this is unsuccessful, this
         * method will throw a PasConnectionException.  Concrete instances must
         * catch this and reschedule themselves.
         */
        virtual void consume() = 0;
        /**
         * Method that is invoked when the event is cancelled.
         */
        virtual void cancel() = 0;

    protected:

        /** Repost all writeable table events */
        void repostAllWriteablePASTableEvents();

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

    /**
     * Event that will connect to the master PA FEPC.
     */
    class ConnectToMasterPAFEPC : public ConnectToPAFEPC
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
        ConnectToMasterPAFEPC(TA_IRS_Bus::Scheduler& socketScheduler,
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
        ConnectToMasterPAFEPC(ACE_Time_Value expiryTime,
                              TA_IRS_Bus::Scheduler& socketScheduler,
                              TA_IRS_Bus::Scheduler& processScheduler,
                              PasTableManager& pasTableManager,
                              PASConnection& pasConnection);

        /** Destructs this instance of this class. */
        virtual ~ConnectToMasterPAFEPC();

        /**
         * Method that is invoked when the event is consumed.  In this case, the
         * event will attempt to connect to the PA FEPC.  If this is successful,
         * this will schedule reads for ALL tables.  If this is unsuccessful, this
         * method will throw a PasConnectionException.  Concrete instances must
         * catch this and reschedule themselves.
         */
        void consume() override;

        /**
         * Method that is invoked when the event is cancelled.
         */
        void cancel() override;
    };

    /**
     * Event that will connect to the station PA FEPC.
     */
    class ConnectToStationPAFEPC : public ConnectToPAFEPC
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
        ConnectToStationPAFEPC(TA_IRS_Bus::Scheduler& socketScheduler,
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
        ConnectToStationPAFEPC(ACE_Time_Value expiryTime,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               PasTableManager& pasTableManager,
                               PASConnection& pasConnection);

        /** Destructs this instance of this class. */
        virtual ~ConnectToStationPAFEPC();
        /**
         * Method that is invoked when the event is consumed.  In this case, the
         * event will attempt to connect to the PA FEPC.  If this is successful,
         * this will schedule reads for ALL tables.  If this is unsuccessful, this
         * method will throw a PasConnectionException.  Concrete instances must
         * catch this and reschedule themselves.
         */
        void consume() override;
        /**
         * Method that is invoked when the event is cancelled.
         */
        void cancel() override;
    };
}
