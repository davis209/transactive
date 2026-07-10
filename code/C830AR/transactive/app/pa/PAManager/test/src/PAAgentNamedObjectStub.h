/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/PAAgentNamedObjectStub.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// Provides a stub for the PA Agent
#pragma once

#include "core\naming\src\INamedObject.h"


class PAAgentNamedObject
{
public:
    PAAgentNamedObject();

    void setCorbaName(const TA_Base_Core::CorbaName& name) {}
    void setEntityName(const std::string& name) {}

    void clearTrainRetryCache(const std::string& broadcastId);
    bool hasTrainRetryBeenAttemptedOn(const std::string& broadcastId, unsigned long trainId);
    
    ///////
    // Train live broadcast states
    void clearBeginRequestCount(const std::string& broadcastId);
    void clearContinueRequestCount(const std::string& broadcastId);
    void clearEndRequestCount(const std::string& broadcastId);
    UINT getBeginRequestCount(const std::string& broadcastId);
    UINT getContinueRequestCount(const std::string& broadcastId);
    UINT getEndRequestCount(const std::string& broadcastId);
    //////

    PAAgentNamedObject* operator->() { return this; }

    /// Actual stubbed out functions (matching IDL)
    char* broadcastStationDva(const TA_IRS_Bus::IPAAgentCorbaDef::MessageKeySeq& messages, const TA_IRS_Bus::IPAAgentCorbaDef::ZoneCoverage& zones, CORBA::UShort dwellInSecs, CORBA::Boolean hasChime, TA_Base_Core::DateTime startTime, TA_Base_Core::DateTime stopTime, CORBA::UShort periodInSecs, CORBA::Boolean isSynchronisedWithTis, const char* sessionId);
    char* broadcastStationLive(CORBA::Octet sourceId, const TA_IRS_Bus::IPAAgentCorbaDef::ZoneCoverage& zones, const char* sessionId);
    void broadcastStationMusic(TA_IRS_Bus::IPAAgentCorbaDef::EMusicType musicType, const char* sessionId);
    char* recordAdhocMessage(CORBA::Octet sourceId, CORBA::ULong messageKey, const char* sessionId);
    void retryStationBroadcast(const char* broadcastId, const TA_IRS_Bus::IPAAgentCorbaDef::ZoneCoverage& zones, const char* sessionId);
    char* broadcastTrainDva(const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains, CORBA::ULong messageKey, CORBA::Short libraryVersion, const TA_Base_Bus::IPATrainCommandsCorbaDef::TimeInterval& announcementPeriod, CORBA::Boolean hasOverrideOption, const char* sessionId);
    char* broadcastTrainLive(const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains, CORBA::Boolean hasOverrideOption, const char* sessionId);
    void beginTrainLiveAnnouncement(const char* broadcastId);
    void continueTrainLiveAnnouncement(CORBA::Octet trainID, const char* broadcastId);
    void endTrainLiveAnnouncement(const char* broadcastId);
    
    // Stub tracks train retry requests
    void retryTrainBroadcast(const char* broadcastId, const TA_IRS_Bus::IPAAgentCorbaDef::TrainCoverage& trains, CORBA::Boolean hasOverrideOption, const char* sessionId);
    
    void terminateBroadcast(const char* broadcastId, const char* sessionId);
    void removeBroadcast(const char* broadcastId, const char* sessionId);
    void setAdhocType(CORBA::ULong messageKey, CORBA::Char messageType, const char* sessionId);
    void setAdhocLabel(CORBA::ULong messageKey, const char* label, const char* sessionId);
    TA_IRS_Bus::IPAAgentCorbaDef::BroadcastIdTypeSeq* getBroadcastIdTypes();
    void changeBroadcastId(const char* fromBroadcastId, const char* toBroadcastId, const char* sessionId);
    TA_IRS_Bus::IPAAgentCorbaDef::BroadcastConfig* getBroadcastConfig(const char* broadcastId);
    TA_IRS_Bus::IPAAgentCorbaDef::ProgressReport* getProgressReport(const char* broadcastId);
    TA_IRS_Bus::IPAAgentCorbaDef::PriorityScheme* getPriorityScheme();
    TA_IRS_Bus::IPAAgentCorbaDef::StationDvaMessageRecords* getStationDvaMessageRecords();
    TA_IRS_Bus::IPAAgentCorbaDef::TrainDvaMessageRecords* getTrainDvaMessageRecords();
    TA_IRS_Bus::IPAAgentCorbaDef::DvaVersionRecords* getDvaVersionRecords();
    TA_IRS_Bus::IPAAgentCorbaDef::PaZoneRecords* getPaZoneRecords();
    void broadcastEventTriggeredStationDva(CORBA::ULong messageKey, const char* messageVersionNumber, CORBA::Boolean hasChime, CORBA::Boolean isSynchronisedWithTis, CORBA::ULong durationInSecs, CORBA::UShort periodInSecs, const char* sessionId, const TA_IRS_Bus::IPAAgentCorbaDef::ZoneCoverage& zones);
    void broadcastEventTriggeredStationDvaForStation(CORBA::ULong messageKey, const char* messageVersionNumber, CORBA::Boolean hasChime, CORBA::Boolean isSynchronisedWithTis, CORBA::ULong durationInSecs, CORBA::UShort periodInSecs, const char* sessionId);
    void broadcastEventTriggeredStationDvaForArea(CORBA::ULong messageKey, const char* messageVersionNumber, CORBA::Boolean hasChime, CORBA::Boolean isSynchronisedWithTis, CORBA::ULong durationInSecs, CORBA::UShort periodInSecs, const char* sessionId, const char* zoneArea);
    void broadcastEventTriggeredStationDvaForGroup(CORBA::ULong messageKey, const char* messageVersionNumber, CORBA::Boolean hasChime, CORBA::Boolean isSynchronisedWithTis, CORBA::ULong durationInSecs, CORBA::UShort periodInSecs, const char* sessionId, const char* zoneGroupLabel);
    void broadcastEventTriggeredTrainDva(CORBA::ULong messageKey, CORBA::Short libraryVersion, const TA_Base_Bus::IPATrainCommandsCorbaDef::TimeInterval& announcementPeriod, const char* sessionId, const TA_IRS_Bus::IPAAgentCorbaDef::TrainCoverage& trains);
    void broadcastEventTriggeredTrainDvaForStation(CORBA::ULong messageKey, CORBA::Short libraryVersion, const TA_Base_Bus::IPATrainCommandsCorbaDef::TimeInterval& announcementPeriod, const char* sessionId);
    void broadcastEventTriggeredTrainDvaForInbound(CORBA::ULong messageKey, CORBA::Short libraryVersion, const TA_Base_Bus::IPATrainCommandsCorbaDef::TimeInterval& announcementPeriod, const char* sessionId);
    void broadcastEventTriggeredTrainDvaForOutbound(CORBA::ULong messageKey, CORBA::Short libraryVersion, const TA_Base_Bus::IPATrainCommandsCorbaDef::TimeInterval& announcementPeriod, const char* sessionId);


private:
    
    std::map<std::string, std::vector<unsigned long> > m_trainRetryCache;
    std::map<std::string, unsigned int > m_beginRequests;
    std::map<std::string, unsigned int > m_continueRequests;
    std::map<std::string, unsigned int > m_endRequests;

};
