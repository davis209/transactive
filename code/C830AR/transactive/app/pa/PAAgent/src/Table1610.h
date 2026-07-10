/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table1610.h $
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

namespace TA_Base_Core
{
    class CommsMessageSender;
}

namespace TA_IRS_App
{
    class IPasEventSource;

    class Table1610 : public AbstractPASTable
    {
        friend class ReadTable1610;
        friend class ProcessTable1610;

    public:

        Table1610();

        virtual void notifyIscs()
        {
            processPublicSetVersionConflicts();
        }

        //Maochun Sun++
        //TD13407
        /*
        bool isPublicSetSynchronised()
        {
            return (getFirstStationInConflict().empty());
        }
        */

        bool isPublicSetSynchronised(std::vector<unsigned int> stationIdList)
        {
            return (getFirstStationInConflict(stationIdList).empty());
        }

        //++Maochun Sun
        //TD13407

    protected:

        virtual void processRead();

        bool processOneVersionId(CORBA::String_member& dvaVersionId, unsigned long offset);
        bool processLocalDvaVersionRecord(TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecord& localDvaVersionRecord, unsigned long stationOffset);
        TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecords* getDvaVersionRecordsCopy();
        //throw ( TA_Base_Core::PasConnectionException );

        void processPublicSetVersionConflicts();

        //Maochun Sun++
        //TD13407
        std::string Table1610::getFirstStationInConflict(std::vector<unsigned int> stationIdList);
        //++Maochun Sun
        //TD13407
        std::string Table1610::getFirstStationInConflict();
        bool isStationInConflict(unsigned long locationKey);

    protected:

        typedef std::map<unsigned long, TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecord> LocationKeyToDvaVersionRecordMap;
        typedef LocationKeyToDvaVersionRecordMap::iterator                          LocationKeyToDvaVersionRecordMapIt;

        LocationKeyToDvaVersionRecordMap                                            m_dvaVersionRecordMap;
        unsigned long                                                               m_depotLocationKey = -1;
    };

    /**
     * Event encapsulation to read Table 1610.
     */
    class ReadTable1610 : public TA_IRS_App::PASReadEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table The reference to Table 1610.
         * @param table The reference to Table 560.
         * @param processScheduler The reference to the scheduler responsible for
         *                         process events.
         */
        ReadTable1610(Table1610& table,
                      Table560& table560,
                      TA_IRS_Bus::Scheduler& socketScheduler,
                      TA_IRS_Bus::Scheduler& processScheduler,
                      IPasEventSource& eventSource);

        /**
         * readTable called by PASReadEvent::consume().  This will simply read the table from PAS and
         * then post a ProcessTable1610 event to the process scheduler.
         */
        void readTable() override;

    private:

        /** The reference to Table1610. */
        Table1610& m_table;

        /** The reference to Table560. */
        Table560& m_table560;
    };

    /**
     * Event encapsulation to process Table 1610.
     */
    class ProcessTable1610 : public TA_IRS_Bus::IEvent
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param table Table 1610 instance.
         */
        ProcessTable1610(Table1610& table);

        /**
         * Consumes the event.  In this case, the event will send the appropriate
         * comms message.
         */
        void consume() override;

        /** Cancels the event.  In this case, the event deletes itself. */
        void cancel() override;

    private:

        /** The table that this is processing. */
        Table1610& m_table;
    };
}
