/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table303.h $
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
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"

namespace TA_Base_Bus
{
    class DataPoint;
}

namespace TA_IRS_Bus
{
    class CachedMaps;
}

namespace TA_IRS_App
{
    class IPasEventSource;

    struct MessageSequenceDescription
    {
        bool                                            m_hasChime = false;
        TA_Base_Bus::IPAAgentCorbaDef::MessageKeySeq    m_messages;
        unsigned short                                  m_dwellInSecs = 0;
        unsigned short                                  m_periodInSecs = 0;
        TA_Base_Core::DateTime                          m_startTime = 0;
        TA_Base_Core::DateTime                          m_stopTime = 0;
        bool                                            m_isEventTriggered = false;
        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage     m_zones;
        // std::uint32_t                                       m_localCoverageBitmap = 0;
    };

    class Table303 : public AbstractPASTable
    {
        friend class ReadTable303;
        friend class ProcessTable303;

    public:

        Table303(unsigned long locationKey);

        bool hasStartTimeElapsed(unsigned long sequenceId);

        TA_Base_Core::DateTime getStartTime(unsigned long sequenceId);

        TA_Base_Core::DateTime getStopTime(unsigned long sequenceId);

        bool isMessageSequenceCyclic(unsigned long sequenceId);

    protected:

        virtual void processRead();
        unsigned long getMessageKeyAndValidate(unsigned long messageId);

    protected:

        std::vector<MessageSequenceDescription> m_messageSequenceDescriptions;
        unsigned long                           m_locationKey = 0;
    };

    /**
     * Event that will read Table 303 from PAS.
     */
    class ReadTable303 : public TA_IRS_App::PASReadEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table The reference to Table 303.
         * @param table560 The reference to Table 560.
         * @param processScheduler The reference to the scheduler responsible for process events.
         */
        ReadTable303(Table303& table,
                     Table560& table560,
                     TA_IRS_Bus::Scheduler& socketScheduler,
                     TA_IRS_Bus::Scheduler& processScheduler,
                     IPasEventSource& eventSource);

        /**
         * readTable called by PASReadEvent::consume().  In this case, this will simply read from PAS.
         */
        void readTable() override;

    private:

        /** The reference to Table303. */
        Table303& m_table;

        /** The reference to Table560. */
        Table560& m_table560;
    };

    /**
     * Event encapsulation to process Table 303.
     */
    class ProcessTable303 : public TA_IRS_Bus::IEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Table 303 instance.
         */
        ProcessTable303(Table303& table);

        /**
         * Consumes the event.  In this case, the event will update the
         * appropriate datapoints.
         */
        void consume() override;

        /** Cancels the event.  In this case, the event deletes itself. */
        void cancel() override;

    private:

        /** The table that this is processing. */
        Table303& m_table;
    };
}
