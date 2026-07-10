#pragma once

#include <string>

#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"

namespace TA_IRS_App
{
    class PaJsonConverter
    {
    public:
        static std::string toJson(const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData& data);
        static std::string toJson(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData& data);
        static std::string toJson(const TA_Base_Bus::IPAAgentCorbaDef::DvaMessagesUpdateData& data);
        static std::string toJson(const TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecords& data);
        static std::string toJson(const TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecords& data);
        static std::string toJson(const TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme& data);
        static std::string toJson(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdTypeSeq& data);
        static std::string toJson(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig& data);
        static std::string toJson(const TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords& data);
        static std::string toJson(const TA_Base_Bus::IPAAgentCorbaDef::TrainDvaMessageRecords& data);
        static std::string toJson(const TA_Base_Bus::IPAAgentCorbaDef::PaZoneRecords& data);

        static std::string broadcastType(TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType value);
        static std::string broadcastState(TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState value);
        static std::string localState(TA_Base_Bus::IPAAgentCorbaDef::ELocalState value);
        static std::string globalState(TA_Base_Bus::IPAAgentCorbaDef::EGlobalState value);
        static std::string musicType(TA_Base_Bus::IPAAgentCorbaDef::EMusicType value);
        static TA_Base_Bus::IPAAgentCorbaDef::EMusicType parseMusicType(const std::string& value);
    };
}
