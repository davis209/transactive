/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table1303.h $
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

namespace TA_IRS_App
{
    class IPasEventSource;

    struct GlobalMessageSequenceDescription
    {
        GlobalMessageSequenceDescription()
            : m_hasChime(false),
            m_dwellInSecs(0),
            m_periodInSecs(0),
            m_startTime(0),
            m_stopTime(0),
            m_isEventTriggered(false),
            m_messages(),
            m_zones(),
            m_globalCoverageBitmaps(0)
        {
            m_globalCoverageBitmaps = reinterpret_cast<std::uint32_t*>(::calloc(TA_Base_Bus::IPAAgentCorbaDef::PA_MAXSTNID, sizeof(std::uint32_t)));
        }

        // Copy constructor for templates
        GlobalMessageSequenceDescription(const GlobalMessageSequenceDescription& copy)
            : m_hasChime(copy.m_hasChime),
            m_dwellInSecs(copy.m_dwellInSecs),
            m_periodInSecs(copy.m_periodInSecs),
            m_startTime(copy.m_startTime),
            m_stopTime(copy.m_stopTime),
            m_isEventTriggered(copy.m_isEventTriggered),
            m_messages(copy.m_messages),
            m_zones(copy.m_zones),
            m_globalCoverageBitmaps(0)
        {
            m_globalCoverageBitmaps = reinterpret_cast<std::uint32_t*>(::calloc(TA_Base_Bus::IPAAgentCorbaDef::PA_MAXSTNID, 4));
            ::memcpy(m_globalCoverageBitmaps, copy.m_globalCoverageBitmaps, TA_Base_Bus::IPAAgentCorbaDef::PA_MAXSTNID * sizeof(std::uint32_t));
        }

        virtual ~GlobalMessageSequenceDescription()
        {
            ::free(m_globalCoverageBitmaps);
            m_globalCoverageBitmaps = NULL;
        }

        bool                                            m_hasChime = 0;
        TA_Base_Bus::IPAAgentCorbaDef::MessageKeySeq    m_messages;
        unsigned short                                  m_dwellInSecs = 0;
        unsigned short                                  m_periodInSecs = 0;
        TA_Base_Core::DateTime                          m_startTime = 0;
        TA_Base_Core::DateTime                          m_stopTime = 0;
        bool                                            m_isEventTriggered = false;
        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage     m_zones;
        std::uint32_t* m_globalCoverageBitmaps;
    };

    class Table1303 : public AbstractPASTable
    {
        friend class ReadTable1303;
        friend class ProcessTable1303;

    public:

        Table1303(unsigned long locationKey);

        bool hasStartTimeElapsed(unsigned long sequenceId);

        TA_Base_Core::DateTime getStartTime(unsigned long sequenceId);

        TA_Base_Core::DateTime getStopTime(unsigned long sequenceId);

        bool isMessageSequenceCyclic(unsigned long sequenceId);

    protected:

        virtual void processRead();
        unsigned long getMessageKeyAndValidate(unsigned long messageId);

    protected:

        std::vector<GlobalMessageSequenceDescription>   m_messageSequenceDescriptions;
        unsigned long                                   m_locationKey;
    };

    /**
     * Event that will read Table 1303 from PAS.
     */
    class ReadTable1303 : public TA_IRS_App::PASReadEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table The reference to Table 1303.
         * @param table560 The reference to Table 560.
         * @param processScheduler The reference to the scheduler responsible for process events.
         */
        ReadTable1303(Table1303& table,
                      Table560& table560,
                      TA_IRS_Bus::Scheduler& socketScheduler,
                      TA_IRS_Bus::Scheduler& processScheduler,
                      IPasEventSource& eventSource);

        /**
         * readTable called by PASReadEvent::consume().  In this case, this will simply read from PAS.
         */
        void readTable() override;

    private:

        /** The reference to Table1303. */
        Table1303& m_table;

        /** The reference to Table560. */
        Table560& m_table560;
    };

    /**
     * Event encapsulation to process Table 1303.
     */
    class ProcessTable1303 : public TA_IRS_Bus::IEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Table 1303 instance.
         */
        ProcessTable1303(Table1303& table);

        /**
         * Consumes the event.  In this case, the event will update the
         * appropriate datapoints.
         */
        void consume() override;

        /** Cancels the event.  In this case, the event deletes itself. */
        void cancel() override;

    private:

        /** The table that this is processing. */
        Table1303& m_table;
    };
}
