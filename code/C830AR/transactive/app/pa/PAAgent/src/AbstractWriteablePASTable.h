/*
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source : $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/AbstractWriteablePASTable.h $
 * @author  J. Dalin
 * @version $Revision: #4 $
 * Last modification : $DateTime: 2025/01/06 12:26:20 $
 * Last modified by : $Author: limin.zhu $
 *
 *
 */

#pragma once
#include "bus/event/src/Scheduler.h"
#include "app/pa/PAAgent/src/AbstractPASTable.h"

namespace TA_IRS_App
{
    class PASWriteEvent;

    class AbstractWriteablePASTable : public AbstractPASTable
    {
    public:

        /** Destructs an instance of this class. */
        virtual ~AbstractWriteablePASTable() {}

        /** Destroys all the write events that this table had sent to the scheduler */
        void destroyCurrentWriteEvents(TA_IRS_Bus::Scheduler& socketScheduler);

        /** This function will repost the current write events into the socket scheduler */
        void repostCurrentWriteEvents(TA_IRS_Bus::Scheduler& socketScheduler);

        /** This function adds an event to the list of current write events that this
          * table is currently taking care of.
          * @return If the event already exists within the list, the function will return FALSE,
          *         otherwise TRUE.
          */
        bool addWriteEvent(PASWriteEvent& event);

        /** This function removes an event from the list of current write events that this
         * table is currently taking care of.
         * @return If the event existed within the list, the function will return TRUE,
         *         otherwise FALSE.
         */
        bool removeWriteEvent(PASWriteEvent& event);

    protected:

        /**
         * Constructs an instance of this class.
         * @param tableNumber The table number of the table the instance will store.
         * @param bufferSize  The size of the buffer for this table.
         */
        AbstractWriteablePASTable(unsigned long tableNumber);

    private:

        AbstractWriteablePASTable(const AbstractWriteablePASTable& theAbstractPASTable);

    protected:

        /* The current write events that this table posted to the socket scheduler */
        std::vector<PASWriteEvent*> m_currentWriteEvents;

        /* Lockable to protect m_currentWriteEvents */
        TA_Base_Core::NonReEntrantThreadLockable m_writeEventsLock;
    };
}
