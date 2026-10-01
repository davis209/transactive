#include "TisRestController.h"

#include "TisJsonConverter.h"
#include "TisJsonUtil.h"

#include "core/utilities/src/DebugUtil.h"

#include <cstdlib>
#include <ctime>
#include <sstream>
#include <vector>

namespace
{
    template <typename T>
    std::string number(T value)
    {
        std::ostringstream out;
        out << value;
        return out.str();
    }

    std::string accepted(long timestamp = 0)
    {
        std::vector<std::string> fields;
        fields.push_back(TA_IRS_App::TisJsonUtil::property("accepted", "true", true));
        if (timestamp != 0)
        {
            fields.push_back(TA_IRS_App::TisJsonUtil::property("timestamp", number(timestamp), true));
        }
        return TA_IRS_App::TisJsonUtil::object(fields);
    }
}

namespace TA_IRS_App
{
    TisRestController::TisRestController(const std::string& stisEntityName,
                                         const std::string& ttisEntityName,
                                         SessionIdProvider sessionIdProvider)
        : m_stisAgent(stisEntityName, true),
          m_ttisAgent(ttisEntityName, true),
          m_sessionIdProvider(sessionIdProvider)
    {
    }

    std::string TisRestController::handle(const std::string& method, const std::string& path, const std::string& body, int& statusCode)
    {
        statusCode = 200;
        try
        {
            if (method == "GET" && path == "/health")
            {
                const bool ready = m_sessionIdProvider && !m_sessionIdProvider().empty();
                return TisJsonUtil::object({ TisJsonUtil::property("status", ready ? "ok" : "starting") });
            }

            const std::string sessionId = m_sessionIdProvider ? m_sessionIdProvider() : "";
            if (sessionId.empty())
            {
                return error(statusCode, 503, "TISBridgeAgent has not acquired an Authentication session yet");
            }

            if (method == "GET" && path == "/api/tis/stis/library-versions")
            {
                TA_Base_Core::TimeScheduleVersion schedule = m_stisAgent->getCurrentTrainTimeScheduleVersion();
                return TisJsonUtil::object({
                    TisJsonUtil::property("currentStis", number(m_stisAgent->getCurrentSTISMessageLibraryVersion()), true),
                    TisJsonUtil::property("nextStis", number(m_stisAgent->getNextSTISMessageLibraryVersion()), true),
                    TisJsonUtil::property("currentTtis", number(m_stisAgent->getCurrentTTISMessageLibraryVersion()), true),
                    TisJsonUtil::property("nextTtis", number(m_stisAgent->getNextTTISMessageLibraryVersion()), true),
                    TisJsonUtil::property("stationSynchronised", m_stisAgent->isStationLibrarySynchronisationComplete() ? "true" : "false", true),
                    TisJsonUtil::property("trainSynchronised", m_stisAgent->isTrainLibrarySynchronisationComplete() ? "true" : "false", true),
                    TisJsonUtil::property("timeScheduleVersion", number(schedule.version), true),
                    TisJsonUtil::property("timeScheduleKey", number(schedule.timeScheduleKey), true)
                });
            }

            if (method == "GET" && path == "/api/tis/stis/ratis")
            {
                TA_Base_Bus::ISTISManagerCorbaDef::RATISMessageList_var data = m_stisAgent->getAllIncomingRATISMessages();
                return TisJsonConverter::toJson(data.in());
            }

            if (method == "GET" && path == "/api/tis/stis/ratis-vetting")
            {
                return TisJsonUtil::object({
                    TisJsonUtil::property("enabled", m_stisAgent->isRATISVettingOn() ? "true" : "false", true)
                });
            }

            if (method == "GET" && path == "/api/tis/ttis/downloads")
            {
                TA_Base_Core::TrainDownloadList_var data = m_ttisAgent->getCurrentTrainDownloads();
                return TisJsonConverter::toJson(data.in());
            }

            if (method == "GET" && path == "/api/tis/ttis/versions")
            {
                TA_Base_Core::TrainDataVersionList_var data = m_ttisAgent->getTrainDataVersions();
                return TisJsonConverter::toJson(data.in());
            }

            if (method == "GET" && path == "/api/tis/ttis/version-alarms")
            {
                TA_Base_Core::TrainDataVersionAlarmList_var data = m_ttisAgent->getTrainDataAlarms();
                return TisJsonConverter::toJson(data.in());
            }

            std::string pathId = getPathId(path, "/api/tis/stis/ratis/");
            if (method == "GET" && !pathId.empty())
            {
                TA_Base_Bus::ISTISManagerCorbaDef::RATISMessageDetails_var data =
                    m_stisAgent->getIncomingRATISMessage(std::strtol(pathId.c_str(), 0, 10));
                return TisJsonConverter::toJson(data.in());
            }

            const std::map<std::string, std::string> request = TisJsonUtil::parseFlatObject(body);

            if (method == "POST" && path == "/api/tis/stis/current-display/query")
            {
                TA_Base_Core::STISDestination destination = TisJsonConverter::toStisDestination(request);
                TA_Base_Core::CurrentDisplayingMessage_var data = m_stisAgent->getCurrentDisplayingMessage(destination);
                return TisJsonConverter::toJson(data.in());
            }

            if (method == "POST" && path == "/api/tis/stis/display/predefined")
            {
                TA_Base_Core::STISDestinationList destinations = TisJsonConverter::toStisDestinations(request);
                m_stisAgent->submitPredefinedDisplayRequest(
                    destinations,
                    TisJsonConverter::librarySection(TisJsonUtil::getString(request, "librarySection")),
                    static_cast<CORBA::UShort>(TisJsonUtil::parseUnsigned(request, "libraryVersion", 0)),
                    static_cast<CORBA::UShort>(TisJsonUtil::parseUnsigned(request, "messageTag", 0)),
                    TisJsonUtil::getString(request, "startTime").c_str(),
                    TisJsonUtil::getString(request, "endTime").c_str(),
                    static_cast<CORBA::UShort>(TisJsonUtil::parseUnsigned(request, "priority", 0)),
                    sessionId.c_str());
                return accepted();
            }

            if (method == "POST" && path == "/api/tis/stis/display/free-text")
            {
                TA_Base_Core::STISDestinationList destinations = TisJsonConverter::toStisDestinations(request);
                TA_Base_Core::DisplayAttributes display;
                display.displayMode = TisJsonConverter::displayMode(TisJsonUtil::getString(request, "displayMode"));
                display.scrollSpeed = static_cast<TA_Base_Core::EScrollSpeed>(TisJsonUtil::parseUnsigned(request, "scrollSpeed", 0));
                display.repeatInterval = static_cast<CORBA::Short>(TisJsonUtil::parseUnsigned(request, "repeatInterval", 0));
                display.displayTime = static_cast<CORBA::Short>(TisJsonUtil::parseUnsigned(request, "displayTime", 0));
                display.justification = TisJsonConverter::justification(TisJsonUtil::getString(request, "justification"));

                TA_Base_Core::PlasmaAttributes plasma;
                plasma.fontType = static_cast<TA_Base_Core::EFontType>(TisJsonUtil::parseUnsigned(request, "plasmaFontType", 0));
                plasma.fontSize = static_cast<TA_Base_Core::EPlasmaFontSize>(TisJsonUtil::parseUnsigned(request, "plasmaFontSize", 0));
                plasma.fontColour = static_cast<TA_Base_Core::EPlasmaColour>(TisJsonUtil::parseUnsigned(request, "plasmaFontColour", 0));
                plasma.backgroundColour = static_cast<TA_Base_Core::EPlasmaColour>(TisJsonUtil::parseUnsigned(request, "plasmaBackgroundColour", 0));

                TA_Base_Core::LEDAttributes led;
                led.fontSize = static_cast<TA_Base_Core::ELEDFontSize>(TisJsonUtil::parseUnsigned(request, "ledFontSize", 0));
                led.intensity = static_cast<TA_Base_Core::ELEDIntensity>(TisJsonUtil::parseUnsigned(request, "ledIntensity", 0));
                led.fontColour = static_cast<TA_Base_Core::ELEDColour>(TisJsonUtil::parseUnsigned(request, "ledFontColour", 0));
                led.backgroundColour = static_cast<TA_Base_Core::ELEDColour>(TisJsonUtil::parseUnsigned(request, "ledBackgroundColour", 0));

                m_stisAgent->submitAdHocDisplayRequest(destinations,
                    TisJsonUtil::getString(request, "messageContent").c_str(),
                    TisJsonUtil::getString(request, "startTime").c_str(),
                    TisJsonUtil::getString(request, "endTime").c_str(),
                    static_cast<CORBA::UShort>(TisJsonUtil::parseUnsigned(request, "priority", 0)),
                    display, plasma, led, sessionId.c_str());
                return accepted();
            }

            if (method == "POST" && path == "/api/tis/stis/display/clear")
            {
                TA_Base_Core::STISDestinationList destinations = TisJsonConverter::toStisDestinations(request);
                m_stisAgent->submitClearRequest(destinations,
                    static_cast<CORBA::UShort>(TisJsonUtil::parseUnsigned(request, "upperPriority", 0)),
                    static_cast<CORBA::UShort>(TisJsonUtil::parseUnsigned(request, "lowerPriority", 0)),
                    sessionId.c_str());
                return accepted();
            }

            if (method == "POST" && path == "/api/tis/stis/pid/control")
            {
                m_stisAgent->submitPIDControlRequest(
                    TisJsonUtil::getString(request, "destination").c_str(),
                    TisJsonConverter::pidControl(TisJsonUtil::getString(request, "command")),
                    sessionId.c_str());
                return accepted();
            }

            if (method == "POST" && path == "/api/tis/stis/pid/lock")
            {
                m_stisAgent->setLockStatus(
                    TisJsonUtil::getString(request, "destination").c_str(),
                    TisJsonUtil::parseBool(request, "locked", true),
                    sessionId.c_str());
                return accepted();
            }

            if (method == "POST" && path == "/api/tis/stis/library/station/upgrade")
            {
                m_stisAgent->upgradePredefinedStationMessageLibrary(
                    static_cast<CORBA::UShort>(TisJsonUtil::parseUnsigned(request, "version", 0)), sessionId.c_str());
                return accepted();
            }

            if (method == "POST" && path == "/api/tis/stis/library/train/upgrade")
            {
                m_stisAgent->upgradePredefinedTrainMessageLibrary(
                    static_cast<CORBA::UShort>(TisJsonUtil::parseUnsigned(request, "version", 0)), sessionId.c_str());
                return accepted();
            }

            if (method == "POST" && path == "/api/tis/stis/ratis")
            {
                m_stisAgent->submitRATISDisplayRequest(
                    TisJsonUtil::getString(request, "messageContent").c_str(),
                    static_cast<CORBA::UShort>(TisJsonUtil::parseUnsigned(request, "priority", 0)),
                    TisJsonUtil::getString(request, "tag").c_str(),
                    TisJsonUtil::getString(request, "destination").c_str(),
                    TisJsonUtil::getString(request, "startTime").c_str(),
                    TisJsonUtil::getString(request, "endTime").c_str(),
                    TisJsonConverter::ratisMessageType(TisJsonUtil::getString(request, "type")),
                    TisJsonUtil::parseBool(request, "overridable", false),
                    TisJsonUtil::parseBool(request, "vetting", false),
                    sessionId.c_str());
                return accepted();
            }

            pathId = getPathId(path, "/api/tis/stis/ratis/", "/vetting-response");
            if (method == "POST" && !pathId.empty())
            {
                m_stisAgent->submitRATISVettingResponse(
                    std::strtol(pathId.c_str(), 0, 10),
                    TisJsonUtil::parseBool(request, "approved", false),
                    static_cast<CORBA::UShort>(TisJsonUtil::parseUnsigned(request, "priority", 0)),
                    TisJsonUtil::getString(request, "content").c_str(),
                    sessionId.c_str());
                return accepted();
            }

            if (method == "POST" && path == "/api/tis/stis/ratis-vetting")
            {
                m_stisAgent->setRATISVetting(TisJsonUtil::parseBool(request, "enabled", false), sessionId.c_str());
                return accepted();
            }

            if (method == "POST" && path == "/api/tis/ttis/display/predefined")
            {
                const long timestamp = TisJsonUtil::parseLong(request, "timestamp", static_cast<long>(std::time(0)));
                TA_Base_Core::TrainList trains = TisJsonConverter::toTrainList(request);
                TA_Base_Core::TTISPredefinedMessageParameters parameters = toTtisPredefined(request);
                m_ttisAgent->submitPredefinedDisplayRequest(trains, parameters, timestamp, sessionId.c_str());
                return accepted(timestamp);
            }

            if (method == "POST" && path == "/api/tis/ttis/display/free-text")
            {
                const long timestamp = TisJsonUtil::parseLong(request, "timestamp", static_cast<long>(std::time(0)));
                TA_Base_Core::TrainList trains = TisJsonConverter::toTrainList(request);
                TA_Base_Core::TTISFreeTextMessageParameters parameters = toTtisFreeText(request);
                m_ttisAgent->submitAdHocDisplayRequest(trains, parameters, timestamp, sessionId.c_str());
                return accepted(timestamp);
            }

            if (method == "POST" && path == "/api/tis/ttis/display/clear")
            {
                const long timestamp = TisJsonUtil::parseLong(request, "timestamp", static_cast<long>(std::time(0)));
                TA_Base_Core::TrainList trains = TisJsonConverter::toTrainList(request);
                TA_Base_Core::TTISMessageResetParameters parameters = toTtisReset(request);
                m_ttisAgent->submitClearRequest(trains, parameters, timestamp, sessionId.c_str());
                return accepted(timestamp);
            }

            TA_Base_Core::TrainList trains = TisJsonConverter::toTrainList(request);
            if (method == "POST" && path == "/api/tis/ttis/message-library/download")
            {
                m_ttisAgent->downloadNextMessageLibrary(trains, sessionId.c_str());
                return accepted();
            }
            if (method == "POST" && path == "/api/tis/ttis/message-library/upgrade")
            {
                m_ttisAgent->upgradeMessageLibrary(trains, sessionId.c_str());
                return accepted();
            }
            if (method == "POST" && path == "/api/tis/ttis/time-schedule/download")
            {
                m_ttisAgent->downloadCurrentTimeSchedule(trains, sessionId.c_str());
                return accepted();
            }
            if (method == "POST" && path == "/api/tis/ttis/time-schedule/upgrade")
            {
                m_ttisAgent->upgradeTimeSchedule(trains, sessionId.c_str());
                return accepted();
            }
            if (method == "POST" && path == "/api/tis/ttis/time-schedule/change")
            {
                m_ttisAgent->timeScheduleChanged(
                    TisJsonUtil::parseUnsigned(request, "timeSchedulePkey", 0),
                    TisJsonConverter::timeScheduleChangeType(TisJsonUtil::getString(request, "changeType")),
                    sessionId.c_str());
                return accepted();
            }

            return error(statusCode, 404, "Unknown endpoint");
        }
        catch (const CORBA::Exception&)
        {
            return error(statusCode, 502, "TISAgent CORBA call failed");
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

    TA_Base_Core::TTISPredefinedMessageParameters TisRestController::toTtisPredefined(const std::map<std::string, std::string>& request) const
    {
        TA_Base_Core::TTISPredefinedMessageParameters result;
        result.pidList = TisJsonConverter::toTrainPidList(request);
        result.libraryVersion = static_cast<CORBA::UShort>(TisJsonUtil::parseUnsigned(request, "libraryVersion", 0));
        result.librarySection = TisJsonConverter::librarySection(TisJsonUtil::getString(request, "librarySection"));
        result.messageId = static_cast<CORBA::Octet>(TisJsonUtil::parseUnsigned(request, "messageId", 0));
        result.priority = static_cast<TA_Base_Core::ETTISMessagePriority>(TisJsonUtil::parseUnsigned(request, "priority", 0));
        result.startTime = TisJsonUtil::getString(request, "startTime").c_str();
        result.endTime = TisJsonUtil::getString(request, "endTime").c_str();
        return result;
    }

    TA_Base_Core::TTISFreeTextMessageParameters TisRestController::toTtisFreeText(const std::map<std::string, std::string>& request) const
    {
        TA_Base_Core::TTISFreeTextMessageParameters result;
        result.pidList = TisJsonConverter::toTrainPidList(request);
        result.fontSize = static_cast<TA_Base_Core::ETTISLEDFontSize>(TisJsonUtil::parseUnsigned(request, "fontSize", 0));
        result.justification = TisJsonConverter::justification(TisJsonUtil::getString(request, "justification"));
        result.intensity = static_cast<TA_Base_Core::ETTISLEDIntensity>(TisJsonUtil::parseUnsigned(request, "intensity", 0));
        result.displayMode = TisJsonConverter::displayMode(TisJsonUtil::getString(request, "displayMode"));
        result.priority = static_cast<TA_Base_Core::ETTISMessagePriority>(TisJsonUtil::parseUnsigned(request, "priority", 0));
        result.startTime = TisJsonUtil::getString(request, "startTime").c_str();
        result.endTime = TisJsonUtil::getString(request, "endTime").c_str();
        result.repeatInterval = static_cast<CORBA::Octet>(TisJsonUtil::parseUnsigned(request, "repeatInterval", 0));
        result.message = TisJsonUtil::getString(request, "message").c_str();
        return result;
    }

    TA_Base_Core::TTISMessageResetParameters TisRestController::toTtisReset(const std::map<std::string, std::string>& request) const
    {
        TA_Base_Core::TTISMessageResetParameters result;
        result.pidList = TisJsonConverter::toTrainPidList(request);
        result.messageType = static_cast<TA_Base_Core::ETTISClearType>(TisJsonUtil::parseUnsigned(request, "clearType", 0));
        return result;
    }

    std::string TisRestController::error(int& statusCode, int code, const std::string& message)
    {
        statusCode = code;
        return TisJsonUtil::object({
            TisJsonUtil::property("error", message),
            TisJsonUtil::property("status", number(code), true)
        });
    }

    std::string TisRestController::getPathId(const std::string& path, const std::string& prefix, const std::string& suffix) const
    {
        if (path.size() <= prefix.size() + suffix.size()) return "";
        if (path.compare(0, prefix.size(), prefix) != 0) return "";
        if (!suffix.empty() && path.compare(path.size() - suffix.size(), suffix.size(), suffix) != 0) return "";
        const std::string value = path.substr(prefix.size(), path.size() - prefix.size() - suffix.size());
        return value.find('/') == std::string::npos ? value : "";
    }
}
