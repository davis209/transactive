/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/InvalidatePASDataPoints.h $
 * @author:  Ripple
 * @version: $Revision: #3 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "bus/event/src/Scheduler.h"

namespace TA_IRS_App
{
    class PasTableManager;

    /**
     * Event that will connect to the PA FEPC.
     */
    class InvalidatePASDataPoints : public TA_IRS_Bus::IEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param processScheduler The reference to the scheduler responsible for
         *                         processing events.
         * @param pasTableManager The reference to the PAS table manager to get
         *                        the relevent tables if this event should
         *                        succeed.
         */
        InvalidatePASDataPoints(TA_IRS_Bus::Scheduler& processScheduler,
                                PasTableManager& pasTableManager);

        /** Destructs this instance of this class. */
        virtual ~InvalidatePASDataPoints();
        /**
         * Method that is invoked when the event is consumed.
         */
        virtual void consume() = 0;
        /**
         * Method that is invoked when the event is cancelled.
         */
        virtual void cancel() = 0;

    protected:

        /** The scheduler responsible for all processing events. */
        TA_IRS_Bus::Scheduler& m_processScheduler;
        /** The PAS table manager. */
        PasTableManager& m_pasTableManager;
    };

    /**
     * Event that will connect to the master PA FEPC.
     */
    class InvalidateMasterPASDataPoints : public InvalidatePASDataPoints
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param processScheduler The reference to the scheduler responsible for
         *                         processing events.
         * @param pasTableManager The reference to the PAS table manager to get
         *                        the relevent tables if this event should
         *                        succeed.
         */
        InvalidateMasterPASDataPoints(TA_IRS_Bus::Scheduler& processScheduler,
                                      PasTableManager& pasTableManager);

        /** Destructs this instance of this class. */
        virtual ~InvalidateMasterPASDataPoints();
        /**
         * Method that is invoked when the event is consumed.
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
    class InvalidateStationPASDataPoints : public InvalidatePASDataPoints
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param processScheduler The reference to the scheduler responsible for
         *                         processing events.
         * @param pasTableManager The reference to the PAS table manager to get
         *                        the relevent tables if this event should
         *                        succeed.
         */
        InvalidateStationPASDataPoints(TA_IRS_Bus::Scheduler& processScheduler,
                                       PasTableManager& pasTableManager);

        /** Destructs this instance of this class. */
        virtual ~InvalidateStationPASDataPoints();

        /**
         * Method that is invoked when the event is consumed.
         */
        void consume() override;

        /**
         * Method that is invoked when the event is cancelled.
         */
        void cancel() override;
    };
}
