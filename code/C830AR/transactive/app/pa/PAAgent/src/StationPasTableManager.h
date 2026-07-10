/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/StationPasTableManager.h $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/10 11:18:50 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "app/pa/PAAgent/src/PasTableManager.h"

namespace TA_IRS_App
{
    /**
     * Station specific PAS table manager.
     *
     * Will provide access to the following PAS tables:
     *    - Table 200
     *    - Table 201
     *    - Table 204
     *    - Table 301
     *    - Table 303
     *    - Table 610
     */
    class StationPasTableManager : public PasTableManager
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param locationKey The location key that this manager is instantiated
         *                    for.  This is not retained by this instance but
         *                    rather used in the construction of certain tables.
         */
        StationPasTableManager(unsigned long locationKey);

        virtual ~StationPasTableManager();

        /** Sets this PAS Table manager to monitor mode. */
        void setMonitor();
        /** Sets this PAS Table manager to control mode. */
        void setControl();

        /** Returns the reference to the Table 200 instance.
        @return the reference to the Table 200 instance. */
        virtual Table200& getTable200();
        /** Returns the reference to the Table 1200 instance.
        @return the reference to the Table 1200 instance. */
        virtual Table1200& getTable1200();
        /** Returns the reference to the Table 201 instance.
        @return the reference to the Table 201 instance. */
        virtual Table201& getTable201();
        /** Returns the reference to the Table 1201 instance.
        @return the reference to the Table 1201 instance. */
        virtual Table1201& getTable1201();
        /** Returns the reference to the Table 204 instance.
        @return the reference to the Table 204 instance. */
        virtual Table204& getTable204();
        /** Returns the reference to the Table 1204 instance.
        @return the reference to the Table 1204 instance. */
        virtual Table1204& getTable1204();
        /** Returns the reference to the Table 301 instance.
        @return the reference to the Table 301 instance. */
        virtual Table301& getTable301();
        /** Returns the reference to the Table 1301 instance.
        @return the reference to the Table 1301 instance. */
        virtual Table1301& getTable1301();
        /** Returns the reference to the Table 303 instance.
        @return the reference to the Table 303 instance. */
        virtual Table303& getTable303();
        /** Returns the reference to the Table 1303 instance.
        @return the reference to the Table 1303 instance. */
        virtual Table1303& getTable1303();
        /** Returns the reference to the Table 610 instance.
        @return the reference to the Table 610 instance. */
        virtual Table610& getTable610();
        /** Returns the reference to the Table 1610 instance.
        @return the reference to the Table 1610 instance. */
        virtual Table1610& getTable1610();

        /**
         * Returns the appropriate ConnectToPAFEPC event to be scheduled.
         * @return The appropriate ConnectToPAFEPC event to be scheduled.
         */
        virtual ConnectToPAFEPC* getConnectEvent();

        /**
         * Returns a pointer to the correct InvalidatePASDataPoints event instance.
         * This event is to be scheduled and dispatched by the scheduler
         * responsible for processing events.
         * @return A pointer to the correct InvalidatePASDataPoints event instance.
         */
        virtual InvalidatePASDataPoints* getInvalidateEvent();

        /**
         * Returns the tables that can be written to.
         * @return The tables that are writeable.
         */
        virtual std::vector<AbstractWriteablePASTable*> getAllWriteableTables();

        //wuzhongyi CL-19136
        virtual void forceTerminateAllPA();

    protected:

    private:
        /** Private copy constructor. */
        StationPasTableManager(const StationPasTableManager& theStationPasTableManager);
    };
}
