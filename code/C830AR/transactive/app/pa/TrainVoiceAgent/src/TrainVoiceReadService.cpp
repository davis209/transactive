#include "TrainVoiceReadService.h"
#include "TrainVoiceAccessFactory.h"
#include "json.hpp"
#include "core/utilities/src/DebugUtil.h"
#include <map>
#include <cstdlib>
#include <limits>
#include <cerrno>

namespace
{
    bool validCallId(const std::string& id)
    {
        if (id.size() != 36) return false;
        for (size_t i = 0; i < id.size(); ++i)
        {
            if (i == 8 || i == 13 || i == 18 || i == 23) { if (id[i] != '-') return false; }
            else if (!((id[i] >= '0' && id[i] <= '9') || (id[i] >= 'a' && id[i] <= 'f'))) return false;
        }
        return true;
    }
    bool safeText(const std::string& text, size_t maxLength)
    {
        if (text.empty() || text.size() > maxLength) return false;
        for (size_t i = 0; i < text.size(); ++i)
            if (text[i] == '\\' || static_cast<unsigned char>(text[i]) < 0x20 || text[i] == 0x7f) return false;
        return true;
    }
    bool parseTrainTime(const std::string& value, std::vector<uint8_t>& time)
    {
        if (value.size() != 19 || value[4] != '-' || value[7] != '-' || value[10] != 'T' || value[13] != ':' || value[16] != ':') return false;
        const size_t positions[] = {0,1,2,3,5,6,8,9,11,12,14,15,17,18};
        for (size_t i = 0; i < sizeof(positions) / sizeof(positions[0]); ++i)
            if (value[positions[i]] < '0' || value[positions[i]] > '9') return false;
        const unsigned year = (value[0]-'0')*1000 + (value[1]-'0')*100 + (value[2]-'0')*10 + value[3]-'0';
        const unsigned month = (value[5]-'0')*10 + value[6]-'0';
        const unsigned day = (value[8]-'0')*10 + value[9]-'0';
        const unsigned hour = (value[11]-'0')*10 + value[12]-'0';
        const unsigned minute = (value[14]-'0')*10 + value[15]-'0';
        const unsigned second = (value[17]-'0')*10 + value[18]-'0';
        if (year < 2000 || year > 2099 || month < 1 || month > 12 || hour > 23 || minute > 59 || second > 59) return false;
        const unsigned days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
        const unsigned maximum = days[month-1] + ((month == 2 && year % 4 == 0) ? 1 : 0);
        if (day < 1 || day > maximum) return false;
        time = {static_cast<uint8_t>(year-2000), static_cast<uint8_t>(month), static_cast<uint8_t>(day),
                static_cast<uint8_t>(hour), static_cast<uint8_t>(minute), static_cast<uint8_t>(second)};
        return true;
    }
}

namespace TA_IRS_App
{
    std::string TrainVoiceReadService::handle(const std::string& method, const std::string& path,
                                               const std::string& body, int& status)
    {
        if (method == "GET" && path == "/health")
        {
            status = m_transport && m_transport->ready() ? 200 : 503;
            return status == 200 ? "{\"status\":\"ok\"}" : "{\"status\":\"starting\",\"reason\":\"train communication not ready\"}";
        }
        try
        {
            const std::string requestsPath = "/api/train-voice/call-requests";
            if (method == "GET" && path == requestsPath)
            {
                const std::vector<TrainVoiceCallRequest> rows = TrainVoiceAccessFactory::getInstance().getPendingRequests();
                nlohmann::json values = nlohmann::json::array();
                for (std::vector<TrainVoiceCallRequest>::const_iterator it = rows.begin(); it != rows.end(); ++it)
                    values.push_back({{"requestId", std::to_string(it->requestId)}, {"trainId", it->trainId},
                        {"requestType", it->requestType}, {"sourceDeviceId", it->sourceDeviceId},
                        {"deviceType", (it->sourceDeviceId >> 8) & 0xff}, {"deviceIndex", it->sourceDeviceId & 0xff},
                        {"receivedAt", it->receivedAt}});
                status = 200;
                return nlohmann::json({{"requests", values}}).dump();
            }
            const std::string requestPrefix = requestsPath + "/";
            if (method == "DELETE" && path.compare(0, requestPrefix.size(), requestPrefix) == 0)
            {
                const std::string id = path.substr(requestPrefix.size());
                if (id.empty() || id.find_first_not_of("0123456789") != std::string::npos)
                { status = 400; return "{\"error\":\"invalid requestId\"}"; }
                char* end = 0;
                errno = 0;
                const unsigned long long requestId = std::strtoull(id.c_str(), &end, 10);
                if (errno == ERANGE || end == id.c_str() || *end || requestId == 0)
                { status = 400; return "{\"error\":\"invalid requestId\"}"; }
                TrainVoiceAccessFactory::getInstance().removePendingRequest(requestId);
                status = 204;
                return "";
            }
            const std::string callsPath = "/api/train-voice/active-calls";
            if (method == "POST" && path == callsPath)
            {
                const nlohmann::json input = nlohmann::json::parse(body);
                if (!input.is_object() || !input.contains("callId") || !input["callId"].is_string() ||
                    !input.contains("trainId") || !input["trainId"].is_number_unsigned() ||
                    !input.contains("carId") || !input["carId"].is_number_unsigned() ||
                    !input.contains("deviceKey") || !input["deviceKey"].is_number_unsigned() ||
                    !input.contains("audioFunction") || !input["audioFunction"].is_number_unsigned() ||
                    !input.contains("sipExtension") || !input["sipExtension"].is_string() ||
                    !input.contains("sipCallId") || !input["sipCallId"].is_string() ||
                    !input.contains("clientId") || !input["clientId"].is_string())
                { status = 400; return "{\"error\":\"missing or invalid call fields\"}"; }
                TrainVoiceCall call;
                call.callId = input["callId"].get<std::string>();
                call.sipExtension = input["sipExtension"].get<std::string>();
                call.sipCallId = input["sipCallId"].get<std::string>();
                call.clientId = input["clientId"].get<std::string>();
                const uint64_t trainId = input["trainId"].get<uint64_t>();
                const uint64_t carId = input["carId"].get<uint64_t>();
                const uint64_t deviceKey = input["deviceKey"].get<uint64_t>();
                const uint64_t function = input["audioFunction"].get<uint64_t>();
                if (!validCallId(call.callId) || trainId == 0 || trainId > 254 || carId == 0 || carId > 0xffffffffULL ||
                    deviceKey == 0 || deviceKey > 0xffffffffULL ||
                    (function != 2 && function != 4 && function != 6 && function != 10 && function != 11) ||
                    !safeText(call.sipExtension, 64) || !safeText(call.sipCallId, 255) || !safeText(call.clientId, 128))
                { status = 400; return "{\"error\":\"invalid call fields\"}"; }
                call.trainId = static_cast<unsigned long>(trainId); call.carId = static_cast<unsigned long>(carId);
                call.deviceKey = static_cast<unsigned long>(deviceKey); call.audioFunction = static_cast<unsigned long>(function);
                if (!TrainVoiceAccessFactory::getInstance().addActiveCall(call))
                { status = 409; return "{\"error\":\"callId already exists with different content\"}"; }
                status = 200;
                return nlohmann::json({{"callId", call.callId}, {"status", "active"}}).dump();
            }
            const std::string callPrefix = callsPath + "/";
            if (method == "DELETE" && path.compare(0, callPrefix.size(), callPrefix) == 0)
            {
                const std::string callId = path.substr(callPrefix.size());
                if (!validCallId(callId)) { status = 400; return "{\"error\":\"invalid callId\"}"; }
                TrainVoiceAccessFactory::getInstance().removeActiveCall(callId);
                status = 204;
                return "";
            }
            const std::string prefix = "/api/train-voice/trains/";
            if (method == "POST" && path.compare(0, prefix.size(), prefix) == 0)
            {
                const size_t slash = path.find('/', prefix.size());
                const std::string id = path.substr(prefix.size(), slash == std::string::npos ? std::string::npos : slash - prefix.size());
                char* end = 0;
                const unsigned long trainId = std::strtoul(id.c_str(), &end, 10);
                if (id.empty() || id.find_first_not_of("0123456789") != std::string::npos || end == id.c_str() || *end || trainId == 0 || trainId > 254 || slash == std::string::npos)
                { status = 400; return "{\"error\":\"invalid train ID\"}"; }
                const std::string action = path.substr(slash);
                if (action != "/audio-requests" && action != "/live-text" && action != "/md-commands" && action != "/train-information" && action != "/pi-commands")
                { status = 404; return "{\"error\":\"unknown endpoint\"}"; }
                const nlohmann::json request = nlohmann::json::parse(body);
                if (!request.is_object() || !request.contains("carId") || !request["carId"].is_number_unsigned())
                { status = 400; return "{\"error\":\"carId required\"}"; }
                const uint64_t carValue = request["carId"].get<uint64_t>();
                if (carValue == 0 || carValue > (std::numeric_limits<unsigned long>::max)())
                { status = 400; return "{\"error\":\"invalid carId\"}"; }
                const unsigned long carId = static_cast<unsigned long>(carValue);
                const std::vector<TrainVoiceDevice> devices = TrainVoiceAccessFactory::getInstance().getDevices();
                const TrainVoiceDevice* target = 0;
                for (std::vector<TrainVoiceDevice>::const_iterator it = devices.begin(); it != devices.end(); ++it)
                {
                    if (it->trainId != trainId || it->carId != carId || it->deviceType != "TCCU") continue;
                    if (target) { status = 409; return "{\"error\":\"multiple TCCU devices\"}"; }
                    target = &*it;
                }
                if (!target || target->ipAddress.empty() || target->controlPort == 0 || target->controlPort > 65535 ||
                    target->protocolDeviceId > 65535 || (target->protocolDeviceId >> 8) != 2 ||
                    (target->protocolDeviceId & 0xff) == 0 || (target->protocolDeviceId & 0xff) == 255)
                { status = 404; return "{\"error\":\"TCCU configuration unavailable\"}"; }
                if (!m_transport || !m_transport->ready()) { status = 503; return "{\"error\":\"train transport unavailable\"}"; }
                TrainFrame frame = {};
                frame.sourceTrainId = 0; frame.destinationTrainId = static_cast<uint8_t>(trainId);
                frame.sourceDevice[0] = 1; frame.sourceDevice[1] = 0x80;
                frame.destinationDevice[0] = static_cast<uint8_t>(target->protocolDeviceId & 0xff);
                frame.destinationDevice[1] = static_cast<uint8_t>(target->protocolDeviceId >> 8);
                std::string error;
                if (action == "/pi-commands")
                {
                    if (!request.contains("command") || !request["command"].is_number_unsigned() ||
                        request["command"].get<uint64_t>() > 255 ||
                        !TrainProtocol::setPiCommand(frame, static_cast<uint8_t>(request["command"].get<uint64_t>()), error))
                    { status = 400; return "{\"error\":\"invalid PI command\"}"; }
                    if (!m_transport->send(frame, target->ipAddress, static_cast<unsigned short>(target->controlPort), error))
                    { status = 503; return nlohmann::json({{"error", error}}).dump(); }
                    status = 202;
                    return "{\"status\":\"SENT\",\"confirmed\":false}";
                }
                if (action == "/train-information")
                {
                    if (!request.contains("timestamp") || !request["timestamp"].is_string() ||
                        !request.contains("carIds") || !request["carIds"].is_array() ||
                        request["carIds"].empty() || request["carIds"].size() > 3)
                    { status = 400; return "{\"error\":\"timestamp and 1 to 3 carIds required\"}"; }
                    std::vector<uint8_t> cars;
                    for (size_t i = 0; i < request["carIds"].size(); ++i)
                    {
                        const nlohmann::json& item = request["carIds"][i];
                        if (!item.is_number_unsigned() || item.get<uint64_t>() == 0 || item.get<uint64_t>() > 255)
                        { status = 400; return "{\"error\":\"carIds must contain integers from 1 to 255\"}"; }
                        const uint8_t car = static_cast<uint8_t>(item.get<uint64_t>());
                        for (size_t j = 0; j < cars.size(); ++j)
                            if (cars[j] == car) { status = 400; return "{\"error\":\"carIds must be unique\"}"; }
                        cars.push_back(car);
                    }
                    std::vector<uint8_t> time;
                    if (!parseTrainTime(request["timestamp"].get<std::string>(), time))
                    { status = 400; return "{\"error\":\"invalid timestamp; expected YYYY-MM-DDTHH:MM:SS\"}"; }
                    if (!TrainProtocol::setTrainInformation(frame, time, cars, static_cast<uint16_t>(trainId), error))
                    { status = 409; return "{\"error\":\"invalid train configuration\"}"; }
                    if (!m_transport->send(frame, target->ipAddress, static_cast<unsigned short>(target->controlPort), error))
                    { status = 503; return nlohmann::json({{"error", error}}).dump(); }
                    status = 202;
                    return "{\"status\":\"SENT\",\"confirmed\":false}";
                }
                if (action == "/md-commands")
                {
                    if (!request.contains("do0") || !request["do0"].is_number_unsigned() ||
                        !request.contains("do1") || !request["do1"].is_number_unsigned() ||
                        request["do0"].get<uint64_t>() > 255 || request["do1"].get<uint64_t>() > 255)
                    { status = 400; return "{\"error\":\"do0 and do1 must be bytes\"}"; }
                    TrainProtocol::setMdCommand(frame, static_cast<uint8_t>(request["do0"].get<uint64_t>()),
                        static_cast<uint8_t>(request["do1"].get<uint64_t>()));
                    if (!m_transport->send(frame, target->ipAddress, static_cast<unsigned short>(target->controlPort), error))
                    { status = 503; return nlohmann::json({{"error", error}}).dump(); }
                    status = 202;
                    return "{\"status\":\"SENT\",\"confirmed\":false}";
                }
                if (action == "/live-text")
                {
                    if (!request.contains("text") || !request["text"].is_string())
                    { status = 400; return "{\"error\":\"text required\"}"; }
                    const std::string text = request["text"].get<std::string>();
                    if (!TrainProtocol::setLiveText(frame, std::vector<uint8_t>(text.begin(), text.end()), error))
                    { status = 400; return nlohmann::json({{"error", error}}).dump(); }
                    if (!m_transport->send(frame, target->ipAddress, static_cast<unsigned short>(target->controlPort), error))
                    { status = 503; return nlohmann::json({{"error", error}}).dump(); }
                    status = 202;
                    return "{\"status\":\"SENT\",\"confirmed\":false}";
                }
                if (!request.contains("function") || !request["function"].is_number_unsigned() ||
                    !request.contains("start") || !request["start"].is_boolean() ||
                    (request.contains("additional") && !request["additional"].is_number_unsigned()))
                { status = 400; return "{\"error\":\"function and start required\"}"; }
                const uint64_t function = request["function"].get<uint64_t>();
                const uint64_t additional = request.contains("additional") ? request["additional"].get<uint64_t>() : 0;
                if (function > 255 || additional > 255 || !TrainProtocol::setAudioRequest(frame,
                    static_cast<uint8_t>(function), request["start"].get<bool>(), static_cast<uint8_t>(additional), error))
                { status = 400; return "{\"error\":\"invalid audio request\"}"; }
                TrainFrame reply = {};
                const TrainRequestManager::WaitResult result = m_transport->request(frame, target->ipAddress,
                    static_cast<unsigned short>(target->controlPort), reply, error);
                if (result == TrainRequestManager::TimedOut) { status = 504; return "{\"error\":\"train response timeout; outcome unknown\"}"; }
                if (result != TrainRequestManager::Completed) { status = 503; return "{\"error\":\"train request unavailable\"}"; }
                uint8_t replyFunction, replyResult, deviceId, deviceType;
                if (!TrainProtocol::parseAudioReply(reply, replyFunction, replyResult, deviceId, deviceType, error) || replyResult > 1)
                { status = 502; return "{\"error\":\"invalid train response\"}"; }
                status = 200;
                nlohmann::json response = {{"trainId", trainId}, {"carId", carId}, {"function", replyFunction},
                    {"result", replyResult == 1 ? "PROCEED" : "REJECTED"},
                    {"calleeDeviceId", deviceId}, {"calleeDeviceType", deviceType}};
                if (replyResult == 1 && deviceId != 0 && deviceType != 0)
                {
                    const unsigned int protocolId = (static_cast<unsigned int>(deviceType) << 8) | deviceId;
                    std::string extension;
                    bool ambiguous = false;
                    for (std::vector<TrainVoiceDevice>::const_iterator it = devices.begin(); it != devices.end(); ++it)
                        if (it->trainId == trainId && it->carId == carId && it->protocolDeviceId == protocolId && !it->sipExtension.empty())
                        { if (!extension.empty()) ambiguous = true; else extension = it->sipExtension; }
                    if (!ambiguous && !extension.empty()) response["sipExtension"] = extension;
                }
                return response.dump();
            }
            if (method == "GET" && path == "/api/train-voice/trains")
            {
                const std::vector<TrainVoiceDevice> devices = TrainVoiceAccessFactory::getInstance().getDevices();
                nlohmann::json trains = nlohmann::json::array();
                std::map<unsigned long, size_t> trainIndexes;
                std::map<unsigned long, std::pair<size_t, size_t> > carIndexes;
                for (std::vector<TrainVoiceDevice>::const_iterator it = devices.begin(); it != devices.end(); ++it)
                {
                    if (trainIndexes.find(it->trainId) == trainIndexes.end())
                    {
                        trainIndexes[it->trainId] = trains.size();
                        trains.push_back({{"trainId", it->trainId}, {"trainName", it->trainName}, {"cars", nlohmann::json::array()}});
                    }
                    const size_t ti = trainIndexes[it->trainId];
                    if (it->carId == 0) continue;
                    if (carIndexes.find(it->carId) == carIndexes.end())
                    {
                        const size_t ci = trains[ti]["cars"].size();
                        carIndexes[it->carId] = std::make_pair(ti, ci);
                        trains[ti]["cars"].push_back({{"carId", it->carId}, {"carName", it->carName},
                            {"position", it->carPosition}, {"devices", nlohmann::json::array()}});
                    }
                    const size_t ci = carIndexes[it->carId].second;
                    if (it->deviceKey == 0) continue;
                    trains[ti]["cars"][ci]["devices"].push_back({{"deviceKey", it->deviceKey},
                        {"deviceType", it->deviceType}, {"deviceName", it->deviceName},
                        {"protocolDeviceId", it->protocolDeviceId}, {"sipExtension", it->sipExtension}});
                }
                status = 200;
                return nlohmann::json({{"trains", trains}}).dump();
            }
            if (method == "GET" && path == "/api/train-voice/active-calls")
            {
                const std::vector<TrainVoiceCall> calls = TrainVoiceAccessFactory::getInstance().getActiveCalls();
                nlohmann::json values = nlohmann::json::array();
                for (std::vector<TrainVoiceCall>::const_iterator it = calls.begin(); it != calls.end(); ++it)
                    values.push_back({{"callId", it->callId}, {"trainId", it->trainId}, {"carId", it->carId},
                        {"deviceKey", it->deviceKey}, {"audioFunction", it->audioFunction}, {"sipExtension", it->sipExtension},
                        {"sipCallId", it->sipCallId}, {"clientId", it->clientId}, {"startedAt", it->startedAt}});
                status = 200;
                return nlohmann::json({{"calls", values}}).dump();
            }
        }
        catch (const nlohmann::json::exception&)
        {
            status = 400;
            return "{\"error\":\"invalid JSON request\"}";
        }
        catch (...)
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "TrainVoiceAgent database query failed");
            status = 503;
            return "{\"error\":\"database unavailable\"}";
        }
        status = 503;
        return "{\"error\":\"train service unavailable\"}";
    }
}
