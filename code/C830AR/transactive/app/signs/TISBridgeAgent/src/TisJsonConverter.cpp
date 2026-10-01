#include "TisJsonConverter.h"

#include "TisJsonUtil.h"
#include "json.hpp"

#include <cstdlib>
#include <sstream>
#include <vector>

namespace
{
    typedef nlohmann::json Json;

    template <typename T>
    std::string number(T value)
    {
        std::ostringstream out;
        out << value;
        return out.str();
    }

    std::string boolean(bool value)
    {
        return value ? "true" : "false";
    }

    unsigned long enumValue(const std::string& value)
    {
        return static_cast<unsigned long>(std::strtoul(value.c_str(), 0, 10));
    }

    void fillStrings(const Json& values, TA_Base_Core::PIDList& result)
    {
        if (!values.is_array()) return;
        result.length(static_cast<CORBA::ULong>(values.size()));
        for (CORBA::ULong i = 0; i < result.length(); ++i)
        {
            result[i] = values[i].get<std::string>().c_str();
        }
    }

    void fillLevels(const Json& values, TA_Base_Core::LevelList& result)
    {
        if (!values.is_array()) return;
        result.length(static_cast<CORBA::ULong>(values.size()));
        for (CORBA::ULong i = 0; i < result.length(); ++i)
        {
            result[i] = values[i].get<std::string>().c_str();
        }
    }

    TA_Base_Core::STISDestination parseDestination(const Json& value)
    {
        TA_Base_Core::STISDestination result;
        result.station = value.value("station", "").c_str();
        fillLevels(value.value("levels", Json::array()), result.levels);
        fillStrings(value.value("pids", Json::array()), result.pids);
        return result;
    }
}

namespace TA_IRS_App
{
    TA_Base_Core::TrainList TisJsonConverter::toTrainList(const std::map<std::string, std::string>& request)
    {
        std::vector<unsigned long> values = TisJsonUtil::parseUnsignedArray(request, "trains");
        TA_Base_Core::TrainList result;
        result.length(static_cast<CORBA::ULong>(values.size()));
        for (CORBA::ULong i = 0; i < result.length(); ++i) result[i] = static_cast<CORBA::Octet>(values[i]);
        return result;
    }

    TA_Base_Core::TrainPIDList TisJsonConverter::toTrainPidList(const std::map<std::string, std::string>& request)
    {
        std::vector<unsigned long> values = TisJsonUtil::parseUnsignedArray(request, "pids");
        TA_Base_Core::TrainPIDList result;
        result.length(static_cast<CORBA::ULong>(values.size()));
        for (CORBA::ULong i = 0; i < result.length(); ++i) result[i] = static_cast<TA_Base_Core::EPIDSelection>(values[i]);
        return result;
    }

    TA_Base_Core::STISDestinationList TisJsonConverter::toStisDestinations(const std::map<std::string, std::string>& request)
    {
        Json values = Json::parse(TisJsonUtil::getString(request, "destinations", "[]"));
        TA_Base_Core::STISDestinationList result;
        if (!values.is_array()) return result;
        result.length(static_cast<CORBA::ULong>(values.size()));
        for (CORBA::ULong i = 0; i < result.length(); ++i) result[i] = parseDestination(values[i]);
        return result;
    }

    TA_Base_Core::STISDestination TisJsonConverter::toStisDestination(const std::map<std::string, std::string>& request)
    {
        Json value = Json::parse(TisJsonUtil::getString(request, "destination", "{}"));
        return parseDestination(value);
    }

    TA_Base_Core::ELibrarySection TisJsonConverter::librarySection(const std::string& value)
    {
        return value == "EMERGENCY_SECTION" || value == "emergency"
            ? TA_Base_Core::EMERGENCY_SECTION : TA_Base_Core::NORMAL_SECTION;
    }

    TA_Base_Core::EDisplayMode TisJsonConverter::displayMode(const std::string& value)
    {
        if (value == "SCROLL_RIGHT") return TA_Base_Core::SCROLL_RIGHT;
        if (value == "SCROLL_UP") return TA_Base_Core::SCROLL_UP;
        if (value == "SCROLL_DOWN") return TA_Base_Core::SCROLL_DOWN;
        if (value == "INSTANT_ON") return TA_Base_Core::INSTANT_ON;
        if (value == "BLINKING") return TA_Base_Core::BLINKING;
        if (value == "WIPING") return TA_Base_Core::WIPING;
        if (value == "SNOW") return TA_Base_Core::SNOW;
        if (!value.empty() && value[0] >= '0' && value[0] <= '9') return static_cast<TA_Base_Core::EDisplayMode>(enumValue(value));
        return TA_Base_Core::SCROLL_LEFT;
    }

    TA_Base_Core::EJustification TisJsonConverter::justification(const std::string& value)
    {
        if (value == "CENTRED" || value == "center") return TA_Base_Core::CENTRED;
        if (value == "RIGHT" || value == "right") return TA_Base_Core::RIGHT;
        if (!value.empty() && value[0] >= '0' && value[0] <= '9') return static_cast<TA_Base_Core::EJustification>(enumValue(value));
        return TA_Base_Core::LEFT;
    }

    TA_Base_Core::ETimeScheduleChangeType TisJsonConverter::timeScheduleChangeType(const std::string& value)
    {
        if (value == "Deleted" || value == "deleted") return TA_Base_Core::Deleted;
        if (value == "Modified" || value == "modified") return TA_Base_Core::Modified;
        return TA_Base_Core::Added;
    }

    TA_Base_Core::ERATISMessageType TisJsonConverter::ratisMessageType(const std::string& value)
    {
        if (value == "RATIS_OUT_NEW") return TA_Base_Core::RATIS_OUT_NEW;
        if (value == "RATIS_IN_UPDATE") return TA_Base_Core::RATIS_IN_UPDATE;
        if (value == "RATIS_OUT_UPDATE") return TA_Base_Core::RATIS_OUT_UPDATE;
        if (value == "RATIS_IN_CLEAR") return TA_Base_Core::RATIS_IN_CLEAR;
        if (value == "RATIS_OUT_CLEAR") return TA_Base_Core::RATIS_OUT_CLEAR;
        return TA_Base_Core::RATIS_IN_NEW;
    }

    TA_Base_Core::EPIDControl TisJsonConverter::pidControl(const std::string& value)
    {
        return value == "TURN_OFF" || value == "off" ? TA_Base_Core::TURN_OFF : TA_Base_Core::TURN_ON;
    }

    std::string TisJsonConverter::commandType(TA_Base_Core::TisCommandType value)
    {
        switch (value)
        {
            case TA_Base_Core::TisFreeTextMessageCommand: return "TisFreeTextMessageCommand";
            case TA_Base_Core::TisPredefinedMessageCommand: return "TisPredefinedMessageCommand";
            case TA_Base_Core::TisClearCommand: return "TisClearCommand";
        }
        return "Unknown";
    }

    std::string TisJsonConverter::downloadType(TA_Base_Core::EDownloadChangeType value)
    {
        switch (value)
        {
            case TA_Base_Core::LibraryDownloadStart: return "LibraryDownloadStart";
            case TA_Base_Core::LibraryDownloadFinish: return "LibraryDownloadFinish";
            case TA_Base_Core::LibraryUpgrade: return "LibraryUpgrade";
            case TA_Base_Core::ScheduleDownloadStart: return "ScheduleDownloadStart";
            case TA_Base_Core::ScheduleDownloadFinish: return "ScheduleDownloadFinish";
            case TA_Base_Core::ScheduleUpgrade: return "ScheduleUpgrade";
        }
        return "Unknown";
    }

    std::string TisJsonConverter::scheduleChangeType(TA_Base_Core::ETimeScheduleChangeType value)
    {
        switch (value)
        {
            case TA_Base_Core::Added: return "Added";
            case TA_Base_Core::Deleted: return "Deleted";
            case TA_Base_Core::Modified: return "Modified";
        }
        return "Unknown";
    }

    std::string TisJsonConverter::ratisType(TA_Base_Core::ERATISMessageType value)
    {
        switch (value)
        {
            case TA_Base_Core::RATIS_IN_NEW: return "RATIS_IN_NEW";
            case TA_Base_Core::RATIS_OUT_NEW: return "RATIS_OUT_NEW";
            case TA_Base_Core::RATIS_IN_UPDATE: return "RATIS_IN_UPDATE";
            case TA_Base_Core::RATIS_OUT_UPDATE: return "RATIS_OUT_UPDATE";
            case TA_Base_Core::RATIS_IN_CLEAR: return "RATIS_IN_CLEAR";
            case TA_Base_Core::RATIS_OUT_CLEAR: return "RATIS_OUT_CLEAR";
        }
        return "Unknown";
    }

    std::string TisJsonConverter::ratisStatus(TA_Base_Core::ERATISMessageStatus value)
    {
        switch (value)
        {
            case TA_Base_Core::APPROVED: return "APPROVED";
            case TA_Base_Core::NOT_APPROVED: return "NOT_APPROVED";
            case TA_Base_Core::REJECTED: return "REJECTED";
            case TA_Base_Core::APPROVE_FAILED: return "APPROVE_FAILED";
        }
        return "Unknown";
    }

    std::string TisJsonConverter::toJson(const TA_Base_Core::TTISDisplayResult& data)
    {
        return TisJsonUtil::object({
            TisJsonUtil::property("trainId", number(static_cast<unsigned long>(data.trainId)), true),
            TisJsonUtil::property("timestamp", number(data.timestamp), true),
            TisJsonUtil::property("originalCommand", commandType(data.originalCommand)),
            TisJsonUtil::property("success", boolean(data.success), true),
            TisJsonUtil::property("errorDetails", data.errorDetails.in())
        });
    }

    std::string TisJsonConverter::toJson(const TA_Base_Core::TrainDownloadStatus& data)
    {
        return TisJsonUtil::object({
            TisJsonUtil::property("trainNumber", number(static_cast<unsigned long>(data.trainNumber)), true),
            TisJsonUtil::property("type", downloadType(data.type)),
            TisJsonUtil::property("success", boolean(data.success), true),
            TisJsonUtil::property("errorDetails", data.errorDetails.in())
        });
    }

    std::string TisJsonConverter::toJson(const TA_Base_Core::TrainDataVersion& data)
    {
        return TisJsonUtil::object({
            TisJsonUtil::property("trainNumber", number(static_cast<unsigned long>(data.trainNumber)), true),
            TisJsonUtil::property("predefinedLibraryVersion", number(data.predefinedLibraryVersion), true),
            TisJsonUtil::property("nextPredefinedLibraryVersion", number(data.nextPredefinedLibraryVersion), true),
            TisJsonUtil::property("trainTimeScheduleVersion", number(data.trainTimeScheduleVersion), true),
            TisJsonUtil::property("nextTrainTimeScheduleVersion", number(data.nextTrainTimeScheduleVersion), true)
        });
    }

    std::string TisJsonConverter::toJson(const TA_Base_Core::TimeScheduleChange& data)
    {
        return TisJsonUtil::object({
            TisJsonUtil::property("timeSchedulePkey", number(data.timeSchedulePkey), true),
            TisJsonUtil::property("changeType", scheduleChangeType(data.changeType))
        });
    }

    std::string TisJsonConverter::toJson(const TA_Base_Bus::ISTISManagerCorbaDef::IncomingRATISEvent& data)
    {
        return TisJsonUtil::object({
            TisJsonUtil::property("messageId", number(data.messageID), true),
            TisJsonUtil::property("sessionRef", number(data.sessionRef), true),
            TisJsonUtil::property("requiresVetting", boolean(data.requiresVetting), true),
            TisJsonUtil::property("type", ratisType(data.type))
        });
    }

    std::string TisJsonConverter::toJson(const TA_Base_Bus::ISTISManagerCorbaDef::RATISMessageApprovalDetails& data)
    {
        return TisJsonUtil::object({
            TisJsonUtil::property("messageId", number(data.messageID), true),
            TisJsonUtil::property("sessionRef", number(data.sessionRef), true),
            TisJsonUtil::property("status", ratisStatus(data.status))
        });
    }

    std::string TisJsonConverter::toJson(const TA_Base_Bus::ISTISManagerCorbaDef::RATISMessageDetails& data)
    {
        return TisJsonUtil::object({
            TisJsonUtil::property("messageId", number(data.messageID), true),
            TisJsonUtil::property("sessionRef", number(data.sessionRef), true),
            TisJsonUtil::property("startTime", data.startTime.in()),
            TisJsonUtil::property("endTime", data.endTime.in()),
            TisJsonUtil::property("requiresVetting", boolean(data.requiresVetting), true),
            TisJsonUtil::property("overridable", boolean(data.overridable), true),
            TisJsonUtil::property("type", ratisType(data.type)),
            TisJsonUtil::property("status", ratisStatus(data.status)),
            TisJsonUtil::property("timeCreated", number(data.timeCreated), true),
            TisJsonUtil::property("destination", data.destination.in()),
            TisJsonUtil::property("tag", data.tag.in()),
            TisJsonUtil::property("priority", number(data.priority), true),
            TisJsonUtil::property("messageText", data.messageText.in()),
            TisJsonUtil::property("isTTIS", boolean(data.isTTIS), true)
        });
    }

    std::string TisJsonConverter::toJson(const TA_Base_Bus::ISTISManagerCorbaDef::RATISMessageList& data)
    {
        std::vector<std::string> values;
        for (CORBA::ULong i = 0; i < data.length(); ++i) values.push_back(toJson(data[i]));
        return TisJsonUtil::array(values);
    }

    std::string TisJsonConverter::toJson(const TA_Base_Core::TrainDownloadList& data)
    {
        std::vector<std::string> values;
        for (CORBA::ULong i = 0; i < data.length(); ++i)
        {
            values.push_back(TisJsonUtil::object({
                TisJsonUtil::property("trainNumber", number(static_cast<unsigned long>(data[i].trainNumber)), true),
                TisJsonUtil::property("predefinedDownloadInProgress", boolean(data[i].predefinedDownloadInProgress), true),
                TisJsonUtil::property("timeScheduleDownloadInProgress", boolean(data[i].timeScheduleDownloadInProgress), true)
            }));
        }
        return TisJsonUtil::array(values);
    }

    std::string TisJsonConverter::toJson(const TA_Base_Core::TrainDataVersionList& data)
    {
        std::vector<std::string> values;
        for (CORBA::ULong i = 0; i < data.length(); ++i) values.push_back(toJson(data[i]));
        return TisJsonUtil::array(values);
    }

    std::string TisJsonConverter::toJson(const TA_Base_Core::TrainDataVersionAlarmList& data)
    {
        std::vector<std::string> values;
        for (CORBA::ULong i = 0; i < data.length(); ++i)
        {
            values.push_back(TisJsonUtil::object({
                TisJsonUtil::property("trainNumber", number(static_cast<unsigned long>(data[i].trainNumber)), true),
                TisJsonUtil::property("messageLibraryMismatchAlarm", data[i].messageLibraryMistmatchAlarm.in()),
                TisJsonUtil::property("iscsLibraryVersion", number(data[i].iscsLibraryVersion), true),
                TisJsonUtil::property("trainLibraryVersion", number(data[i].trainLibraryVersion), true),
                TisJsonUtil::property("timeScheduleMismatchAlarm", data[i].timeScheduleMistmatchAlarm.in()),
                TisJsonUtil::property("iscsScheduleVersion", number(data[i].iscsScheduleVersion), true),
                TisJsonUtil::property("trainScheduleVersion", number(data[i].trainScheduleVersion), true)
            }));
        }
        return TisJsonUtil::array(values);
    }

    std::string TisJsonConverter::toJson(const TA_Base_Core::CurrentDisplayingMessage& data)
    {
        return TisJsonUtil::object({
            TisJsonUtil::property("messageContent", data.messageContent.in()),
            TisJsonUtil::property("startTime", data.startTime.in()),
            TisJsonUtil::property("endTime", data.endTime.in()),
            TisJsonUtil::property("priority", number(data.priority), true)
        });
    }
}
