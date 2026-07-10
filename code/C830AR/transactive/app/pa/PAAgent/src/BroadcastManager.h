/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source : $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/BroadcastManager.h $
 * @author  A. Parker
 * @version $Revision: #3 $
 * Last modification : $DateTime: 2025/01/06 12:26:20 $
 * Last modified by : $Author: limin.zhu $
 *
 *
 */

#pragma once
#include "app/pa/PAAgent/src/AbstractBroadcastCircularList.h"
#include "app/pa/PAAgent/src/PasTableManager.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "core/corba/src/ServantBase.h"
#include "core/exceptions/src/GenericAgentException.h"
#include "core/naming/src/FlexibleTimeoutNamedObject.h"
#include "core/synchronisation/src/ReEntrantThreadLockable.h"

namespace TA_Base_Bus
{
    class RightsLibrary;
}

namespace TA_IRS_App
{
    class BroadcastWorker;

    /**
     * This class manages the state of broadcasts within the PA Agent.
     */
    class BroadcastManager :
        public virtual POA_TA_Base_Bus::IPAAgentCorbaDef,
        public virtual TA_Base_Core::ServantBase
    {
    public:

        /**
         * Constructs an instance of this class.
         * @param tableManager The table manager instance that will allow access
         *                     to the table instances.
         */
        BroadcastManager(PasTableManager& tableManager);
        virtual ~BroadcastManager();

        void enable();
        void disable();

        void setModeChangingFlag(bool flag);

        // IDL Interface functions
        // jeffrey++ TD10377
        virtual void agentLinkTest();
        // ++jeffrey TD10377
        // jeffrey++ TES736
        virtual bool hardwareLinkTest(const char* broadcastId, const char* sessionId);
        // ++jeffrey TES736
        virtual char* broadcastStationDva(const TA_Base_Bus::IPAAgentCorbaDef::MessageKeySeq& messages, const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones, CORBA::UShort dwellInSecs, CORBA::Boolean hasChime, TA_Base_Core::DateTime startTime, TA_Base_Core::DateTime stopTime, CORBA::UShort periodInSecs, CORBA::Boolean isSynchronisedWithTis, const char* groupName, const char* sessionId);
        virtual char* broadcastStationLive(CORBA::Octet sourceId, const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones, const char* groupName, const char* sessionId);
        virtual void broadcastStationMusic(TA_Base_Bus::IPAAgentCorbaDef::EMusicType noneLocalOrOcc, const char* sessionId);
        virtual char* recordAdhocMessage(CORBA::Octet sourceId, CORBA::ULong messageKey, const char* sessionId);
        virtual void retryStationBroadcast(const char* broadcastId, const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones, const char* sessionId);
        virtual char* broadcastTrainDva(const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains, CORBA::ULong messageKey, CORBA::Short libraryVersion, const TA_Base_Bus::ITrainPaCorba::MessageSchedule& announcementPeriod, CORBA::Boolean hasOverrideOption, const char* sessionId);
        virtual char* broadcastTrainLive(const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains, CORBA::Boolean hasOverrideOption, const char* staticGroup, const char* sessionId);
        virtual void beginTrainLiveAnnouncement(const char* broadcastId);
        virtual void continueTrainLiveAnnouncement(const char* broadcastId);
        virtual void endTrainLiveAnnouncement(const char* broadcastId);
        virtual void retryTrainBroadcast(const char* broadcastId, const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains, CORBA::Boolean hasOverrideOption, const char* sessionId);
        virtual void terminateBroadcast(const char* broadcastId, const char* sessionId);
        virtual void removeBroadcast(const char* broadcastId, const char* sessionId);
        virtual void forceTerminateAllExistPA(); //wuzhongyi CL-19136
        // jeffrey++ TD10853
        //virtual void setAdhocType(CORBA::ULong messageKey, CORBA::Char messageType, const char* sessionId);
        virtual bool setAdhocType(CORBA::ULong messageKey, CORBA::Char messageType, const char* sessionId);
        // ++jeffrey TD10853
        virtual void setAdhocLabel(CORBA::ULong messageKey, const char* label, const char* sessionId);
        virtual TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdTypeSeq* getBroadcastIdTypes();
        virtual void changeBroadcastId(const char* fromBroadcastId, const char* toBroadcastId, const char* sessionId);
        virtual TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* getBroadcastConfig(const char* broadcastId);
        virtual TA_Base_Bus::IPAAgentCorbaDef::ProgressReport* getProgressReport(const char* broadcastId);
        virtual TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme* getPriorityScheme();
        virtual TA_Base_Bus::IPAAgentCorbaDef::StationDvaMessageRecords* getStationDvaMessageRecords();
        virtual TA_Base_Bus::IPAAgentCorbaDef::TrainDvaMessageRecords* getTrainDvaMessageRecords();
        virtual TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords* getDvaVersionRecords();
        virtual TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecords* getDvaPublicVersionRecords();
        virtual TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecords* getDvaPrivateVersionRecords();
        virtual TA_Base_Bus::IPAAgentCorbaDef::PaZoneRecords* getPaZoneRecords();
        // song toan++
        // TD10409
        virtual TA_Base_Bus::IPAAgentCorbaDef::EMusicType getMusicStatus(const char* sessionId);
        // ++song toan
        // TD10409
        virtual void broadcastEventTriggeredStationDva(CORBA::ULong messageKey, const char* messageVersionNumber, CORBA::Boolean hasChime, CORBA::Boolean isSynchronisedWithTis, CORBA::ULong durationInSecs, CORBA::UShort periodInSecs, const char* sessionId, const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones);
        virtual void broadcastEventTriggeredStationDvaForStation(CORBA::ULong messageKey, const char* messageVersionNumber, CORBA::Boolean hasChime, CORBA::Boolean isSynchronisedWithTis, CORBA::ULong durationInSecs, CORBA::UShort periodInSecs, const char* sessionId);
        virtual void broadcastEventTriggeredStationDvaForArea(CORBA::ULong messageKey, const char* messageVersionNumber, CORBA::Boolean hasChime, CORBA::Boolean isSynchronisedWithTis, CORBA::ULong durationInSecs, CORBA::UShort periodInSecs, const char* sessionId, const char* zoneArea);
        virtual void broadcastEventTriggeredStationDvaForGroup(CORBA::ULong messageKey, const char* messageVersionNumber, CORBA::Boolean hasChime, CORBA::Boolean isSynchronisedWithTis, CORBA::ULong durationInSecs, CORBA::UShort periodInSecs, const char* sessionId, const char* zoneGroupLabel);
        virtual void broadcastEventTriggeredTrainDva(CORBA::ULong messageKey, CORBA::Short libraryVersion, const TA_Base_Bus::ITrainPaCorba::MessageSchedule& announcementPeriod, const char* sessionId, const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains);
        virtual void broadcastEventTriggeredTrainDvaForStation(CORBA::ULong messageKey, CORBA::Short libraryVersion, const TA_Base_Bus::ITrainPaCorba::MessageSchedule& announcementPeriod, const char* sessionId);
        virtual void broadcastEventTriggeredTrainDvaForInbound(CORBA::ULong messageKey, CORBA::Short libraryVersion, const TA_Base_Bus::ITrainPaCorba::MessageSchedule& announcementPeriod, const char* sessionId);
        virtual void broadcastEventTriggeredTrainDvaForOutbound(CORBA::ULong messageKey, CORBA::Short libraryVersion, const TA_Base_Bus::ITrainPaCorba::MessageSchedule& announcementPeriod, const char* sessionId);

        TA_Base_Bus::IPAAgentCorbaDef::CompleteBroadcastStateSeq* getCompleteBroadcastStateSeqCopy();

        void processPAAgentNewBroadcastUpdate(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* updateData);
        // throw CorruptInternalStateDataException
        void processPAAgentRemovedBroadcastUpdate(const char* updateData);
        // throw CorruptInternalStateDataException
        void processPAAgentRenamedBroadcastUpdate(const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData* updateData);
        // throw CorruptInternalStateDataException
        void processPAAgentBroadcastParameterUpdate(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameter* updateData);
        // throw CorruptInternalStateDataException
        void processPAAgentBroadcastProgressUpdate(const TA_Base_Bus::IPAAgentCorbaDef::ProgressReport* updateData);
        // throw CorruptInternalStateDataException
        void processPAAgentEntireAgentStateUpdate(const TA_Base_Bus::IPAAgentCorbaDef::CompleteBroadcastStateSeq* updateData);
        // throw CorruptInternalStateDataException

    public:

        static const std::string ATS_AGENT_SESSION_ID;
        // TD17372
        static const std::string MANAGER_LAUNCHING_BROADCAST_ID;
        // TD17372

    protected:

        // used by all event triggered station broadcast methods
        void broadcastEventTriggeredStationDvaInternal(
            const std::string& broadcastId,
            const CORBA::ULong& messageKey,
            const char* messageVersionNumber,
            const CORBA::Boolean& hasChime,
            const CORBA::Boolean& isSynchronisedWithTis,
            const CORBA::ULong& durationInSecs,
            const CORBA::UShort& periodInSecs,
            const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones,
            const char* sessionId);

        std::string getNewBroadcastId(TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType broadcastType,
                                      const std::string& extraInformation = "");
        // TA_THROW CorruptInternalStateDataException

        bool isRightsCheckingAvailable();

        void throwAccessDeniedExceptionIfCannotPerformAction(unsigned long action, const std::string& sessionId, const std::string& broadcastId = "");
        //throw(TA_Base_Core::AccessDeniedException);

        TA_Base_Core::ReEntrantThreadLockable                    m_lock;
        AbstractBroadcastCircularList* m_allBroadcasts;
        BroadcastWorker* m_broadcastWorker;
        TA_Base_Bus::RightsLibrary* m_rightsLibrary;
        unsigned long                                       m_resourceId;
        /**
         * The reference to the PasTableManager instance that will allow access to
         * the table instances.  This will allow the broadcasts to monitor the
         * state of various tables.
         */
        PasTableManager& m_tableManager;

        // song toan++
        // TD 10409

        // m_MusicType: One of following values
        //  NoOccMusic or NoLocalMusic if no music is playing
        //  LocalMusic if local music is playing
        //  OccMusic if OCC music is playing
        TA_Base_Bus::IPAAgentCorbaDef::EMusicType m_MusicType;
        // ++song toan
        // TD10409
    };
}
