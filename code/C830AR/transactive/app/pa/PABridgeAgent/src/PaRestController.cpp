#include "PaRestController.h"

#include "JsonUtil.h"
#include "PaJsonConverter.h"

#include "core/exceptions/src/ObjectResolutionException.h"
#include "core/utilities/src/DebugUtil.h"

#include <memory>
#include <sstream>

namespace TA_IRS_App
{
    PaRestController::PaRestController(const std::string& paAgentEntityName, SessionIdProvider sessionIdProvider)
        : m_paAgent(paAgentEntityName, true),
          m_sessionIdProvider(sessionIdProvider)
    {
    }

    std::string PaRestController::handle(const std::string& method, const std::string& path, const std::string& body, int& statusCode)
    {
        statusCode = 200;

        try
        {
            if (method == "GET" && path == "/health")
            {
                const bool hasSession = m_sessionIdProvider && !m_sessionIdProvider().empty();
                return ok(JsonUtil::object({
                    JsonUtil::property("status", hasSession ? "ok" : "starting"),
                    JsonUtil::property("sessionReady", hasSession ? "true" : "false", true)
                }));
            }

            const std::string sessionId = m_sessionIdProvider ? m_sessionIdProvider() : "";
            if (sessionId.empty())
            {
                return error(statusCode, 503, "PABridgeAgent has not acquired an Authentication session yet");
            }

            if (method == "GET" && path == "/api/pa/broadcasts")
            {
                TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdTypeSeq_var data = m_paAgent->getBroadcastIdTypes();
                return ok(PaJsonConverter::toJson(data.in()));
            }

            if (method == "GET" && path == "/api/pa/config/station-dva-messages")
            {
                TA_Base_Bus::IPAAgentCorbaDef::StationDvaMessageRecords_var data = m_paAgent->getStationDvaMessageRecords();
                return ok(PaJsonConverter::toJson(data.in()));
            }

            if (method == "GET" && path == "/api/pa/config/train-dva-messages")
            {
                TA_Base_Bus::IPAAgentCorbaDef::TrainDvaMessageRecords_var data = m_paAgent->getTrainDvaMessageRecords();
                return ok(PaJsonConverter::toJson(data.in()));
            }

            if (method == "GET" && path == "/api/pa/config/dva-public-versions")
            {
                TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecords_var data = m_paAgent->getDvaPublicVersionRecords();
                return ok(PaJsonConverter::toJson(data.in()));
            }

            if (method == "GET" && path == "/api/pa/config/dva-private-versions")
            {
                TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecords_var data = m_paAgent->getDvaPrivateVersionRecords();
                return ok(PaJsonConverter::toJson(data.in()));
            }

            if (method == "GET" && path == "/api/pa/config/zones")
            {
                TA_Base_Bus::IPAAgentCorbaDef::PaZoneRecords_var data = m_paAgent->getPaZoneRecords();
                return ok(PaJsonConverter::toJson(data.in()));
            }

            if (method == "GET" && path == "/api/pa/priority-scheme")
            {
                TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme_var data = m_paAgent->getPriorityScheme();
                return ok(PaJsonConverter::toJson(data.in()));
            }

            std::string broadcastId = getPathId(path, "/api/pa/broadcasts/", "/progress");
            if (method == "GET" && !broadcastId.empty())
            {
                TA_Base_Bus::IPAAgentCorbaDef::ProgressReport_var data = m_paAgent->getProgressReport(broadcastId.c_str());
                return ok(PaJsonConverter::toJson(data.in()));
            }

            broadcastId = getPathId(path, "/api/pa/broadcasts/", "/config");
            if (method == "GET" && !broadcastId.empty())
            {
                TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig_var data = m_paAgent->getBroadcastConfig(broadcastId.c_str());
                return ok(PaJsonConverter::toJson(data.in()));
            }

            std::map<std::string, std::string> request = JsonUtil::parseFlatObject(body);

            broadcastId = getPathId(path, "/api/pa/broadcasts/", "/terminate");
            if (method == "POST" && !broadcastId.empty())
            {
                m_paAgent->terminateBroadcast(broadcastId.c_str(), sessionId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("broadcastId", broadcastId), JsonUtil::property("terminated", "true", true) }));
            }

            broadcastId = getPathId(path, "/api/pa/broadcasts/", "/remove");
            if (method == "POST" && !broadcastId.empty())
            {
                m_paAgent->removeBroadcast(broadcastId.c_str(), sessionId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("broadcastId", broadcastId), JsonUtil::property("removed", "true", true) }));
            }

            broadcastId = getPathId(path, "/api/pa/broadcasts/", "/change-id");
            if (method == "POST" && !broadcastId.empty())
            {
                std::string toBroadcastId = JsonUtil::getString(request, "toBroadcastId");
                m_paAgent->changeBroadcastId(
                    broadcastId.c_str(),
                    toBroadcastId.c_str(),
                    sessionId.c_str());
                return ok(JsonUtil::object({
                    JsonUtil::property("fromBroadcastId", broadcastId),
                    JsonUtil::property("toBroadcastId", toBroadcastId)
                }));
            }

            broadcastId = getPathId(path, "/api/pa/broadcasts/", "/retry-station");
            if (method == "POST" && !broadcastId.empty())
            {
                TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones = toZoneCoverage(request, "zones");
                m_paAgent->retryStationBroadcast(
                    broadcastId.c_str(),
                    zones,
                    sessionId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("broadcastId", broadcastId), JsonUtil::property("accepted", "true", true) }));
            }

            broadcastId = getPathId(path, "/api/pa/broadcasts/", "/retry-train");
            if (method == "POST" && !broadcastId.empty())
            {
                TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage trains = toTrainCoverage(request, "trains");
                m_paAgent->retryTrainBroadcast(
                    broadcastId.c_str(),
                    trains,
                    JsonUtil::parseBool(request, "hasOverrideOption", false),
                    sessionId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("broadcastId", broadcastId), JsonUtil::property("accepted", "true", true) }));
            }

            broadcastId = getPathId(path, "/api/pa/train-live/", "/begin");
            if (method == "POST" && !broadcastId.empty())
            {
                m_paAgent->beginTrainLiveAnnouncement(broadcastId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("broadcastId", broadcastId), JsonUtil::property("accepted", "true", true) }));
            }

            broadcastId = getPathId(path, "/api/pa/train-live/", "/continue");
            if (method == "POST" && !broadcastId.empty())
            {
                m_paAgent->continueTrainLiveAnnouncement(broadcastId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("broadcastId", broadcastId), JsonUtil::property("accepted", "true", true) }));
            }

            broadcastId = getPathId(path, "/api/pa/train-live/", "/end");
            if (method == "POST" && !broadcastId.empty())
            {
                m_paAgent->endTrainLiveAnnouncement(broadcastId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("broadcastId", broadcastId), JsonUtil::property("accepted", "true", true) }));
            }

            if (method == "POST" && path == "/api/pa/station/music")
            {
                m_paAgent->broadcastStationMusic(
                    PaJsonConverter::parseMusicType(JsonUtil::getString(request, "musicType")),
                    sessionId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("accepted", "true", true) }));
            }

            if (method == "GET" && path.find("/api/pa/station/music-status") == 0)
            {
                TA_Base_Bus::IPAAgentCorbaDef::EMusicType value = m_paAgent->getMusicStatus(sessionId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("musicType", PaJsonConverter::musicType(value)) }));
            }

            if (method == "POST" && path == "/api/pa/station/dva")
            {
                TA_Base_Bus::IPAAgentCorbaDef::MessageKeySeq messages;
                std::vector<unsigned long> messageKeys = JsonUtil::parseUnsignedArray(request, "messages");
                messages.length(messageKeys.size());
                for (CORBA::ULong i = 0; i < messageKeys.size(); ++i)
                {
                    messages[i] = messageKeys[i];
                }

                TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones = toZoneCoverage(request, "zones");
                CORBA::String_var id = m_paAgent->broadcastStationDva(
                    messages,
                    zones,
                    static_cast<CORBA::UShort>(JsonUtil::parseUnsigned(request, "dwellInSecs", 0)),
                    JsonUtil::parseBool(request, "hasChime", false),
                    static_cast<TA_Base_Core::DateTime>(JsonUtil::parseUnsigned(request, "startTime", 0)),
                    static_cast<TA_Base_Core::DateTime>(JsonUtil::parseUnsigned(request, "stopTime", 0)),
                    static_cast<CORBA::UShort>(JsonUtil::parseUnsigned(request, "periodInSecs", 0)),
                    JsonUtil::parseBool(request, "isSynchronisedWithTis", false),
                    JsonUtil::getString(request, "groupName").c_str(),
                    sessionId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("broadcastId", id.in()) }));
            }

            if (method == "POST" && path == "/api/pa/station/live")
            {
                TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones = toZoneCoverage(request, "zones");
                CORBA::String_var id = m_paAgent->broadcastStationLive(
                    static_cast<CORBA::Octet>(JsonUtil::parseUnsigned(request, "sourceId", 0)),
                    zones,
                    JsonUtil::getString(request, "groupName").c_str(),
                    sessionId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("broadcastId", id.in()) }));
            }

            if (method == "POST" && path == "/api/pa/station/record-adhoc")
            {
                CORBA::String_var id = m_paAgent->recordAdhocMessage(
                    static_cast<CORBA::Octet>(JsonUtil::parseUnsigned(request, "sourceId", 0)),
                    JsonUtil::parseUnsigned(request, "messageKey", 0),
                    sessionId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("broadcastId", id.in()) }));
            }

            if (method == "POST" && path == "/api/pa/station/adhoc-type")
            {
                std::string type = JsonUtil::getString(request, "messageType");
                CORBA::Boolean result = m_paAgent->setAdhocType(
                    JsonUtil::parseUnsigned(request, "messageKey", 0),
                    type.empty() ? 'N' : type[0],
                    sessionId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("updated", result ? "true" : "false", true) }));
            }

            if (method == "POST" && path == "/api/pa/station/adhoc-label")
            {
                m_paAgent->setAdhocLabel(
                    JsonUtil::parseUnsigned(request, "messageKey", 0),
                    JsonUtil::getString(request, "label").c_str(),
                    sessionId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("updated", "true", true) }));
            }

            if (method == "POST" && path == "/api/pa/train/dva")
            {
                TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage trains = toTrainCoverage(request, "trains");
                TA_Base_Bus::ITrainPaCorba::MessageSchedule schedule;
                schedule.period = static_cast<CORBA::UShort>(JsonUtil::parseUnsigned(request, "announcementPeriod", 0));
                schedule.duration = static_cast<CORBA::UShort>(JsonUtil::parseUnsigned(request, "announcementDuration", 0));

                CORBA::String_var id = m_paAgent->broadcastTrainDva(
                    trains,
                    JsonUtil::parseUnsigned(request, "messageKey", 0),
                    static_cast<CORBA::Short>(JsonUtil::parseUnsigned(request, "libraryVersion", 0)),
                    schedule,
                    JsonUtil::parseBool(request, "hasOverrideOption", false),
                    sessionId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("broadcastId", id.in()) }));
            }

            if (method == "POST" && path == "/api/pa/train/live")
            {
                TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage trains = toTrainCoverage(request, "trains");
                CORBA::String_var id = m_paAgent->broadcastTrainLive(
                    trains,
                    JsonUtil::parseBool(request, "hasOverrideOption", false),
                    JsonUtil::getString(request, "staticGroup").c_str(),
                    sessionId.c_str());
                return ok(JsonUtil::object({ JsonUtil::property("broadcastId", id.in()) }));
            }

            return error(statusCode, 404, "Unknown endpoint");
        }
        catch (const CORBA::Exception&)
        {
            return error(statusCode, 502, "PAAgent CORBA call failed");
        }
        catch (const std::exception& e)
        {
            return error(statusCode, 500, e.what());
        }
        catch (...)
        {
            return error(statusCode, 500, "Unknown error");
        }
    }

    std::string PaRestController::ok(const std::string& json)
    {
        return json;
    }

    std::string PaRestController::error(int& statusCode, int code, const std::string& message)
    {
        statusCode = code;
        std::ostringstream status;
        status << code;
        return JsonUtil::object({
            JsonUtil::property("error", message),
            JsonUtil::property("status", status.str(), true)
        });
    }

    std::string PaRestController::getPathId(const std::string& path, const std::string& prefix, const std::string& suffix) const
    {
        if (path.size() <= prefix.size() + suffix.size())
        {
            return "";
        }
        if (path.compare(0, prefix.size(), prefix) != 0)
        {
            return "";
        }
        if (path.compare(path.size() - suffix.size(), suffix.size(), suffix) != 0)
        {
            return "";
        }
        return path.substr(prefix.size(), path.size() - prefix.size() - suffix.size());
    }

    TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage PaRestController::toZoneCoverage(const std::map<std::string, std::string>& body, const std::string& key) const
    {
        std::vector<unsigned long> values = JsonUtil::parseUnsignedArray(body, key);
        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage result;
        result.length(values.size());
        for (CORBA::ULong i = 0; i < values.size(); ++i)
        {
            result[i] = values[i];
        }
        return result;
    }

    TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage PaRestController::toTrainCoverage(const std::map<std::string, std::string>& body, const std::string& key) const
    {
        std::vector<unsigned long> values = JsonUtil::parseUnsignedArray(body, key);
        TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage result;
        result.length(values.size());
        for (CORBA::ULong i = 0; i < values.size(); ++i)
        {
            result[i] = values[i];
        }
        return result;
    }
}
