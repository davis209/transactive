/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/PAAgentNamedObjectStub.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#include "StdAfx.h"
#include "PAAgentNamedObjectStub.h"

PAAgentNamedObject::PAAgentNamedObject()
{

}


void PAAgentNamedObject::clearTrainRetryCache(const std::string& broadcastId) { m_trainRetryCache.clear(); }

bool PAAgentNamedObject::hasTrainRetryBeenAttemptedOn(const std::string& broadcastId, unsigned long trainId)
{
    return m_trainRetryCache[broadcastId].end() != 
        std::find(m_trainRetryCache[broadcastId].begin(), m_trainRetryCache[broadcastId].end(), trainId);
}

void PAAgentNamedObject::clearBeginRequestCount(const std::string& broadcastId) { m_beginRequests[broadcastId] = 0; }
void PAAgentNamedObject::clearContinueRequestCount(const std::string& broadcastId) { m_continueRequests[broadcastId] = 0; }
void PAAgentNamedObject::clearEndRequestCount(const std::string& broadcastId) { m_endRequests[broadcastId] = 0; }
UINT PAAgentNamedObject::getBeginRequestCount(const std::string& broadcastId) { return m_beginRequests[broadcastId]; }
UINT PAAgentNamedObject::getContinueRequestCount(const std::string& broadcastId) { return m_continueRequests[broadcastId]; }
UINT PAAgentNamedObject::getEndRequestCount(const std::string& broadcastId) { return m_endRequests[broadcastId]; }


//////////////////////////////////////////////////
//////////////////////////////////////////////////
// Stubbed out agent functions
char* PAAgentNamedObject::broadcastStationDva(const TA_IRS_Bus::IPAAgentCorbaDef::MessageKeySeq& messages, const TA_IRS_Bus::IPAAgentCorbaDef::ZoneCoverage& zones, CORBA::UShort dwellInSecs, CORBA::Boolean hasChime, TA_Base_Core::DateTime startTime, TA_Base_Core::DateTime stopTime, CORBA::UShort periodInSecs, CORBA::Boolean isSynchronisedWithTis, const char* sessionId) { return 0; }
char* PAAgentNamedObject::broadcastStationLive(CORBA::Octet sourceId, const TA_IRS_Bus::IPAAgentCorbaDef::ZoneCoverage& zones, const char* sessionId) { return 0; }
void PAAgentNamedObject::broadcastStationMusic(TA_IRS_Bus::IPAAgentCorbaDef::EMusicType musicType, const char* sessionId) {}
char* PAAgentNamedObject::recordAdhocMessage(CORBA::Octet sourceId, CORBA::ULong messageKey, const char* sessionId) { return 0; }
void PAAgentNamedObject::retryStationBroadcast(const char* broadcastId, const TA_IRS_Bus::IPAAgentCorbaDef::ZoneCoverage& zones, const char* sessionId) {}
char* PAAgentNamedObject::broadcastTrainDva(const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains, CORBA::ULong messageKey, CORBA::Short libraryVersion, const TA_Base_Bus::IPATrainCommandsCorbaDef::TimeInterval& announcementPeriod, CORBA::Boolean hasOverrideOption, const char* sessionId) { return 0; }
char* PAAgentNamedObject::broadcastTrainLive(const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains, CORBA::Boolean hasOverrideOption, const char* sessionId) { return 0; }

void PAAgentNamedObject::beginTrainLiveAnnouncement(const char* broadcastId) 
{
    m_beginRequests[std::string(broadcastId)] ++;
}


void PAAgentNamedObject::continueTrainLiveAnnouncement(CORBA::Octet trainID, const char* broadcastId)
{
    m_continueRequests[std::string(broadcastId)] ++;
}


void PAAgentNamedObject::endTrainLiveAnnouncement(const char* broadcastId)
{
    m_endRequests[std::string(broadcastId)] ++;
}
    

void PAAgentNamedObject::retryTrainBroadcast(const char* broadcastId, const TA_IRS_Bus::IPAAgentCorbaDef::TrainCoverage& trains, CORBA::Boolean hasOverrideOption, const char* sessionId)
{
    clearTrainRetryCache(broadcastId);

    for (unsigned int i = 0; i < trains.length(); i ++)
    {
        m_trainRetryCache[std::string(broadcastId)].push_back(trains[i]);
    }
}

    
void PAAgentNamedObject::terminateBroadcast(const char* broadcastId, const char* sessionId) {}
void PAAgentNamedObject::removeBroadcast(const char* broadcastId, const char* sessionId) {}
void PAAgentNamedObject::setAdhocType(CORBA::ULong messageKey, CORBA::Char messageType, const char* sessionId) {}
void PAAgentNamedObject::setAdhocLabel(CORBA::ULong messageKey, const char* label, const char* sessionId) {}
TA_IRS_Bus::IPAAgentCorbaDef::BroadcastIdTypeSeq* PAAgentNamedObject::getBroadcastIdTypes() { return 0; }
void PAAgentNamedObject::changeBroadcastId(const char* fromBroadcastId, const char* toBroadcastId, const char* sessionId) {}
TA_IRS_Bus::IPAAgentCorbaDef::BroadcastConfig* PAAgentNamedObject::getBroadcastConfig(const char* broadcastId) { return 0; }


// Return dummy object pointer
TA_IRS_Bus::IPAAgentCorbaDef::ProgressReport* PAAgentNamedObject::getProgressReport(const char* broadcastId)
{
    TA_IRS_Bus::IPAAgentCorbaDef::ProgressReport* pr = new TA_IRS_Bus::IPAAgentCorbaDef::ProgressReport();
    // Provide some suitable default states
    pr->broadcastState = TA_IRS_Bus::IPAAgentCorbaDef::Creation;
    pr->broadcastStateFailure = TA_IRS_Bus::IPAAgentCorbaDef::BROADCAST_STATE_NO_FAILURE;
    pr->broadcastId = CORBA::string_dup("");    
    return pr;
}


TA_IRS_Bus::IPAAgentCorbaDef::PriorityScheme* PAAgentNamedObject::getPriorityScheme() { return 0; }
TA_IRS_Bus::IPAAgentCorbaDef::StationDvaMessageRecords* PAAgentNamedObject::getStationDvaMessageRecords() { return 0; }
TA_IRS_Bus::IPAAgentCorbaDef::TrainDvaMessageRecords* PAAgentNamedObject::getTrainDvaMessageRecords() { return 0; }
TA_IRS_Bus::IPAAgentCorbaDef::DvaVersionRecords* PAAgentNamedObject::getDvaVersionRecords() { return 0; }
TA_IRS_Bus::IPAAgentCorbaDef::PaZoneRecords* PAAgentNamedObject::getPaZoneRecords() { return 0; }
void PAAgentNamedObject::broadcastEventTriggeredStationDva(CORBA::ULong messageKey, const char* messageVersionNumber, CORBA::Boolean hasChime, CORBA::Boolean isSynchronisedWithTis, CORBA::ULong durationInSecs, CORBA::UShort periodInSecs, const char* sessionId, const TA_IRS_Bus::IPAAgentCorbaDef::ZoneCoverage& zones) {}
void PAAgentNamedObject::broadcastEventTriggeredStationDvaForStation(CORBA::ULong messageKey, const char* messageVersionNumber, CORBA::Boolean hasChime, CORBA::Boolean isSynchronisedWithTis, CORBA::ULong durationInSecs, CORBA::UShort periodInSecs, const char* sessionId) {}
void PAAgentNamedObject::broadcastEventTriggeredStationDvaForArea(CORBA::ULong messageKey, const char* messageVersionNumber, CORBA::Boolean hasChime, CORBA::Boolean isSynchronisedWithTis, CORBA::ULong durationInSecs, CORBA::UShort periodInSecs, const char* sessionId, const char* zoneArea) {}
void PAAgentNamedObject::broadcastEventTriggeredStationDvaForGroup(CORBA::ULong messageKey, const char* messageVersionNumber, CORBA::Boolean hasChime, CORBA::Boolean isSynchronisedWithTis, CORBA::ULong durationInSecs, CORBA::UShort periodInSecs, const char* sessionId, const char* zoneGroupLabel) {}
void PAAgentNamedObject::broadcastEventTriggeredTrainDva(CORBA::ULong messageKey, CORBA::Short libraryVersion, const TA_Base_Bus::IPATrainCommandsCorbaDef::TimeInterval& announcementPeriod, const char* sessionId, const TA_IRS_Bus::IPAAgentCorbaDef::TrainCoverage& trains) {}
void PAAgentNamedObject::broadcastEventTriggeredTrainDvaForStation(CORBA::ULong messageKey, CORBA::Short libraryVersion, const TA_Base_Bus::IPATrainCommandsCorbaDef::TimeInterval& announcementPeriod, const char* sessionId) {}
void PAAgentNamedObject::broadcastEventTriggeredTrainDvaForInbound(CORBA::ULong messageKey, CORBA::Short libraryVersion, const TA_Base_Bus::IPATrainCommandsCorbaDef::TimeInterval& announcementPeriod, const char* sessionId) {}
void PAAgentNamedObject::broadcastEventTriggeredTrainDvaForOutbound(CORBA::ULong messageKey, CORBA::Short libraryVersion, const TA_Base_Bus::IPATrainCommandsCorbaDef::TimeInterval& announcementPeriod, const char* sessionId) {}
