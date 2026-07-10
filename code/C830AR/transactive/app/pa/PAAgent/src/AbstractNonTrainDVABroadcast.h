/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/AbstractNonTrainDVABroadcast.h $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "AbstractStationBroadcast.h"
#include "PasHelpers.h"
#include "Table200.h"
#include "Table202.h"
#include "Table302.h"
#include "core/data_access_interface/tis_agent_4669/IDL/src/TISMessageDataDefinitions.h"
#include "core/utilities/src/Timer.h"

namespace TA_IRS_App
{
    class Table301;
    class Table302;
    class Table350;

    class AbstractNonTrainDVABroadcast : public virtual AbstractStationBroadcast
    {
    public:

        AbstractNonTrainDVABroadcast(Table202& table202,
                                     Table302& table302,
                                     Table350& table350,
                                     TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* broadcastConfigP,
                                     const std::string& sessionId);
        ~AbstractNonTrainDVABroadcast();

        TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameterSeq* getBroadcastParametersCopy() override;
        void setBroadcastParameter(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameter& parameter) override;

        void validateBroadcastConfig() override; //throw(TA_Base_Core::BadParameterException);
        virtual void validateBroadcastConfigSpecifics() = 0; //throw(TA_Base_Core::BadParameterException);

        void execute() override;

        virtual void writeToMessagesSequenceBroadcastRequestTable(unsigned int announceId) = 0;
        // throws (PasWriteErrorException, PasConnectionException)

        virtual void performVersionCheck()
        {
        }

        virtual void disconnectSourceId() = 0;
        virtual void disconnectSequenceId() = 0;

        void updateCurrentlyPlayingZones();
        void updateAllZones(TA_Base_Bus::IPAAgentCorbaDef::ELocalState localState);

        //Maochun Sun++
        //TD13411
        //virtual void updateCacheWithLatestZoneStatus(unsigned long zoneKey)=0;
        virtual void updateCacheWithLatestZoneStatus() = 0;
        // throws (PasConnectionException)

        //++Maochun Sun
        //TD13411

        virtual void sendRequestToTisAgent(const TA_IRS_Bus::StationDvaMessageRecord& record,
                                           const char* startTimeString,
                                           const char* stopTimeString,
                                           CORBA::UShort tisPriority) = 0;
        // throws (...)

        void terminate() override;

        virtual TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage getZoneCoverage()
        {
            TA_Base_Core::ThreadGuard guard(m_lock);
            return m_theSpecifics.zones;
        }

        // Agent-synchronised BroadcastParameter
        CORBA::Octet getMessageSeqId();
        void setMessageSeqId(CORBA::Octet value, bool sendSynchronisation = true);

        virtual void updateFinalProgressReport();

        // jeffrey++ Bug457
        virtual int getZoneStatusTableIndex();
        // ++jeffrey Bug457

    protected:

        virtual void runtimeSpecifics();
        bool isCyclic();

        virtual void setupAndValidateTisParameters(const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones) = 0;
        //throw(TA_Base_Core::BadParameterException);

        void executeTisBroadcast();

    private:

        AbstractNonTrainDVABroadcast(const AbstractNonTrainDVABroadcast& theAbstractNonTrainDVABroadcast);
        // ++ Bug456
        void executePhase1();
        void executePhase2();
        bool m_timerStarted;

    private:

        TA_Base_Bus::IPAAgentCorbaDef::EExecutePhase m_executePhase;
        TA_Base_Core::Timer m_executePhaseTime;
        // ++ Bug456

    protected:

        typedef std::map<unsigned long, TA_IRS_App::ZoneStatus>     ZoneKeyToZoneStatusMap;

        static const std::string                        EXPECTED_STATION_SYSTEM;

        TA_Base_Bus::IPAAgentCorbaDef::StationDvaConfig& m_theSpecifics; // For convenience

        TA_IRS_App::Table302& m_table302;
        TA_IRS_App::Table202& m_table202;
        TA_IRS_App::Table350& m_table350;

        TA_Base_Core::STISDestination                        m_tisCoverage;

        CORBA::Octet                                    m_messageSeqId;

        ZoneKeyToZoneStatusMap                          m_lastZoneStatuses;

        // jeffrey++ Bug457
        ZoneKeyToZoneStatusMap m_previouslastZoneStatuses;
        MessageSequenceStatus m_previousmsgSeqStatus;
        int m_zoneStatusTableIndex;
        // ++jeffrey Bug457
    };
}
