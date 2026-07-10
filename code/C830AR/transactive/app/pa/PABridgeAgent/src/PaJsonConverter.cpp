#include "PaJsonConverter.h"

#include "JsonUtil.h"

#include <sstream>
#include <vector>

namespace
{
    std::string number(unsigned long value)
    {
        std::ostringstream out;
        out << value;
        return out.str();
    }

    std::string number(long value)
    {
        std::ostringstream out;
        out << value;
        return out.str();
    }

    std::string boolean(bool value)
    {
        return value ? "true" : "false";
    }
}

namespace TA_IRS_App
{
    std::string PaJsonConverter::broadcastType(TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType value)
    {
        switch (value)
        {
            case TA_Base_Bus::IPAAgentCorbaDef::StationMusic: return "StationMusic";
            case TA_Base_Bus::IPAAgentCorbaDef::StationLive: return "StationLive";
            case TA_Base_Bus::IPAAgentCorbaDef::StationDva: return "StationDva";
            case TA_Base_Bus::IPAAgentCorbaDef::StationRecording: return "StationRecording";
            case TA_Base_Bus::IPAAgentCorbaDef::TrainLive: return "TrainLive";
            case TA_Base_Bus::IPAAgentCorbaDef::TrainDva: return "TrainDva";
        }
        return "Unknown";
    }

    std::string PaJsonConverter::broadcastState(TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState value)
    {
        switch (value)
        {
            case TA_Base_Bus::IPAAgentCorbaDef::Creation: return "Creation";
            case TA_Base_Bus::IPAAgentCorbaDef::Execution: return "Execution";
            case TA_Base_Bus::IPAAgentCorbaDef::Runtime: return "Runtime";
            case TA_Base_Bus::IPAAgentCorbaDef::Termination: return "Termination";
            case TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval: return "PendingRemoval";
        }
        return "Unknown";
    }

    std::string PaJsonConverter::localState(TA_Base_Bus::IPAAgentCorbaDef::ELocalState value)
    {
        std::ostringstream out;
        out << static_cast<int>(value);
        return out.str();
    }

    std::string PaJsonConverter::globalState(TA_Base_Bus::IPAAgentCorbaDef::EGlobalState value)
    {
        std::ostringstream out;
        out << static_cast<int>(value);
        return out.str();
    }

    std::string PaJsonConverter::musicType(TA_Base_Bus::IPAAgentCorbaDef::EMusicType value)
    {
        switch (value)
        {
            case TA_Base_Bus::IPAAgentCorbaDef::NoOccMusic: return "NoOccMusic";
            case TA_Base_Bus::IPAAgentCorbaDef::NoLocalMusic: return "NoLocalMusic";
            case TA_Base_Bus::IPAAgentCorbaDef::LocalMusic: return "LocalMusic";
            case TA_Base_Bus::IPAAgentCorbaDef::OccMusic: return "OccMusic";
        }
        return "Unknown";
    }

    TA_Base_Bus::IPAAgentCorbaDef::EMusicType PaJsonConverter::parseMusicType(const std::string& value)
    {
        if (value == "LocalMusic") return TA_Base_Bus::IPAAgentCorbaDef::LocalMusic;
        if (value == "OccMusic") return TA_Base_Bus::IPAAgentCorbaDef::OccMusic;
        if (value == "NoOccMusic") return TA_Base_Bus::IPAAgentCorbaDef::NoOccMusic;
        return TA_Base_Bus::IPAAgentCorbaDef::NoLocalMusic;
    }

    std::string PaJsonConverter::toJson(const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData& data)
    {
        return JsonUtil::object({
            JsonUtil::property("fromBroadcastId", data.fromBroadcastId.in()),
            JsonUtil::property("toBroadcastId", data.toBroadcastId.in()),
            JsonUtil::property("broadcastType", broadcastType(data.broadcastType)),
            JsonUtil::property("sessionId", data.sessionId.in())
        });
    }

    std::string PaJsonConverter::toJson(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData& data)
    {
        std::vector<std::string> progresses;
        for (CORBA::ULong i = 0; i < data.localProgresses.length(); ++i)
        {
            const TA_Base_Bus::IPAAgentCorbaDef::LocalProgress& item = data.localProgresses[i];
            progresses.push_back(JsonUtil::object({
                JsonUtil::property("zoneKeyOrTrainId", number(item.zoneKeyOrTrainId), true),
                JsonUtil::property("localState", localState(item.localState), true),
                JsonUtil::property("overridingSourcePriorityForZonesOnly", number(item.overridingSourcePriorityForZonesOnly), true),
                JsonUtil::property("errorOccurred", boolean(item.errorOccurred), true)
            }));
        }

        return JsonUtil::object({
            JsonUtil::property("broadcastId", data.broadcastId.in()),
            JsonUtil::property("broadcastState", broadcastState(data.broadcastState)),
            JsonUtil::property("globalState", globalState(data.globalState), true),
            JsonUtil::property("localProgresses", JsonUtil::array(progresses), true)
        });
    }

    std::string PaJsonConverter::toJson(const TA_Base_Bus::IPAAgentCorbaDef::DvaMessagesUpdateData& data)
    {
        std::vector<std::string> records;
        for (CORBA::ULong i = 0; i < data.length(); ++i)
        {
            const TA_Base_Bus::IPAAgentCorbaDef::StationDvaMessageRecord& item = data[i];
            records.push_back(JsonUtil::object({
                JsonUtil::property("pKey", number(item.m_pKey), true),
                JsonUtil::property("locationKey", number(item.m_locationKey), true),
                JsonUtil::property("id", number(item.m_id), true),
                JsonUtil::property("label", item.m_label.in()),
                JsonUtil::property("type", std::string(1, item.m_type)),
                JsonUtil::property("tisMessageTag", number(item.m_tisMessageTag), true),
                JsonUtil::property("tisLibrarySection", number(item.m_tisLibrarySection), true),
                JsonUtil::property("tisLibraryVersion", number(item.m_tisLibraryVersion), true)
            }));
        }
        return JsonUtil::array(records);
    }

    std::string PaJsonConverter::toJson(const TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecords& data)
    {
        std::vector<std::string> records;
        for (CORBA::ULong i = 0; i < data.length(); ++i)
        {
            const TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecord& item = data[i];
            records.push_back(JsonUtil::object({
                JsonUtil::property("pKey", number(item.m_pKey), true),
                JsonUtil::property("locationKey", number(item.m_locationKey), true),
                JsonUtil::property("publicAdhoc1", item.m_publicAdhoc1.in()),
                JsonUtil::property("publicAdhoc2", item.m_publicAdhoc2.in()),
                JsonUtil::property("publicAdhoc3", item.m_publicAdhoc3.in()),
                JsonUtil::property("publicAdhoc4", item.m_publicAdhoc4.in()),
                JsonUtil::property("publicAdhoc5", item.m_publicAdhoc5.in()),
                JsonUtil::property("publicAdhoc6", item.m_publicAdhoc6.in()),
                JsonUtil::property("publicPreRecorded", item.m_publicPreRecorded.in())
            }));
        }
        return JsonUtil::array(records);
    }

    std::string PaJsonConverter::toJson(const TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecords& data)
    {
        std::vector<std::string> records;
        for (CORBA::ULong i = 0; i < data.length(); ++i)
        {
            const TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecord& item = data[i];
            records.push_back(JsonUtil::object({
                JsonUtil::property("pKey", number(item.m_pKey), true),
                JsonUtil::property("locationKey", number(item.m_locationKey), true),
                JsonUtil::property("privateAdhoc1", item.m_privateAdhoc1.in()),
                JsonUtil::property("privateAdhoc2", item.m_privateAdhoc2.in()),
                JsonUtil::property("privateAdhoc3", item.m_privateAdhoc3.in()),
                JsonUtil::property("privateAdhoc4", item.m_privateAdhoc4.in()),
                JsonUtil::property("privateAdhoc5", item.m_privateAdhoc5.in()),
                JsonUtil::property("privateAdhoc6", item.m_privateAdhoc6.in()),
                JsonUtil::property("privatePreRecorded", item.m_privatePreRecorded.in())
            }));
        }
        return JsonUtil::array(records);
    }

    std::string PaJsonConverter::toJson(const TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords& data)
    {
        std::vector<std::string> records;
        for (CORBA::ULong i = 0; i < data.length(); ++i)
        {
            const TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecord& item = data[i];
            records.push_back(JsonUtil::object({
                JsonUtil::property("pKey", number(item.m_pKey), true),
                JsonUtil::property("locationKey", number(item.m_locationKey), true),
                JsonUtil::property("privateAdhoc1", item.m_privateAdhoc1.in()),
                JsonUtil::property("privateAdhoc2", item.m_privateAdhoc2.in()),
                JsonUtil::property("privateAdhoc3", item.m_privateAdhoc3.in()),
                JsonUtil::property("privateAdhoc4", item.m_privateAdhoc4.in()),
                JsonUtil::property("privateAdhoc5", item.m_privateAdhoc5.in()),
                JsonUtil::property("privateAdhoc6", item.m_privateAdhoc6.in()),
                JsonUtil::property("privatePreRecorded", item.m_privatePreRecorded.in()),
                JsonUtil::property("publicAdhoc1", item.m_publicAdhoc1.in()),
                JsonUtil::property("publicAdhoc2", item.m_publicAdhoc2.in()),
                JsonUtil::property("publicAdhoc3", item.m_publicAdhoc3.in()),
                JsonUtil::property("publicAdhoc4", item.m_publicAdhoc4.in()),
                JsonUtil::property("publicAdhoc5", item.m_publicAdhoc5.in()),
                JsonUtil::property("publicAdhoc6", item.m_publicAdhoc6.in()),
                JsonUtil::property("publicPreRecorded", item.m_publicPreRecorded.in())
            }));
        }
        return JsonUtil::array(records);
    }

    std::string PaJsonConverter::toJson(const TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme& data)
    {
        std::vector<std::string> stations;
        for (CORBA::ULong station = 0; station < data.length(); ++station)
        {
            std::vector<std::string> zones;
            for (CORBA::ULong zone = 0; zone < data[station].length(); ++zone)
            {
                std::vector<std::string> modes;
                for (CORBA::ULong mode = 0; mode < data[station][zone].length(); ++mode)
                {
                    modes.push_back(number(data[station][zone][mode]));
                }
                zones.push_back(JsonUtil::array(modes));
            }
            stations.push_back(JsonUtil::array(zones));
        }
        return JsonUtil::array(stations);
    }

    std::string PaJsonConverter::toJson(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdTypeSeq& data)
    {
        std::vector<std::string> records;
        for (CORBA::ULong i = 0; i < data.length(); ++i)
        {
            records.push_back(JsonUtil::object({
                JsonUtil::property("broadcastId", data[i].broadcastId.in()),
                JsonUtil::property("broadcastType", broadcastType(data[i].broadcastType)),
                JsonUtil::property("sessionId", data[i].sessionId.in())
            }));
        }
        return JsonUtil::array(records);
    }

    std::string PaJsonConverter::toJson(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig& data)
    {
        return JsonUtil::object({
            JsonUtil::property("broadcastId", data.broadcastId.in()),
            JsonUtil::property("sessionId", data.sessionId.in()),
            JsonUtil::property("broadcastType", broadcastType(data.broadcastType)),
            JsonUtil::property("isEventTriggered", boolean(data.isEventTriggered), true)
        });
    }

    std::string PaJsonConverter::toJson(const TA_Base_Bus::IPAAgentCorbaDef::TrainDvaMessageRecords& data)
    {
        std::vector<std::string> records;
        for (CORBA::ULong i = 0; i < data.length(); ++i)
        {
            records.push_back(JsonUtil::object({
                JsonUtil::property("pKeyOrId", number(data[i].m_pKeyOrId), true),
                JsonUtil::property("label", data[i].m_label.in())
            }));
        }
        return JsonUtil::array(records);
    }

    std::string PaJsonConverter::toJson(const TA_Base_Bus::IPAAgentCorbaDef::PaZoneRecords& data)
    {
        std::vector<std::string> records;
        for (CORBA::ULong i = 0; i < data.length(); ++i)
        {
            records.push_back(JsonUtil::object({
                JsonUtil::property("pKey", number(data[i].m_pKey), true),
                JsonUtil::property("locationKey", number(data[i].m_locationKey), true),
                JsonUtil::property("locationName", data[i].m_locationName.in()),
                JsonUtil::property("id", number(data[i].m_id), true),
                JsonUtil::property("label", data[i].m_label.in()),
                JsonUtil::property("area", data[i].m_area.in())
            }));
        }
        return JsonUtil::array(records);
    }
}
