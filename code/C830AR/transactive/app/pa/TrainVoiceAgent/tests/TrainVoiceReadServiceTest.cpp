#include "../src/TrainVoiceReadService.h"
#include "../src/TrainVoiceAccessFactory.h"
#include "../src/json.hpp"
#include <cassert>
#include <stdexcept>
#include <map>

namespace TA_IRS_App
{
    static bool failQuery = false;
    static std::map<std::string, TrainVoiceCall> storedCalls;
    static std::map<unsigned long long, TrainVoiceCallRequest> storedRequests;
    TrainVoiceAccessFactory& TrainVoiceAccessFactory::getInstance() { static TrainVoiceAccessFactory factory; return factory; }
    std::vector<TrainVoiceDevice> TrainVoiceAccessFactory::getDevices()
    {
        if (failQuery) throw std::runtime_error("db failed");
        TrainVoiceDevice device = {};
        device.trainId = 1; device.trainName = "Train 1"; device.carId = 2; device.carName = "Car 2";
        device.carPosition = 1; device.deviceKey = 3; device.deviceType = "PI"; device.sipExtension = "1002";
        TrainVoiceDevice tccu = device; tccu.deviceKey = 4; tccu.deviceType = "TCCU"; tccu.protocolDeviceId = 0x0201;
        tccu.ipAddress = "127.0.0.1"; tccu.controlPort = 61001;
        TrainVoiceDevice empty = {}; empty.trainId = 4; empty.trainName = "Train 4";
        return {device, tccu, empty};
    }
    std::vector<TrainVoiceCall> TrainVoiceAccessFactory::getActiveCalls() {
        std::vector<TrainVoiceCall> calls;
        for (std::map<std::string, TrainVoiceCall>::const_iterator i = storedCalls.begin(); i != storedCalls.end(); ++i) calls.push_back(i->second);
        return calls;
    }
    bool TrainVoiceAccessFactory::addActiveCall(const TrainVoiceCall& c) {
        if (failQuery) throw std::runtime_error("db failed");
        std::map<std::string, TrainVoiceCall>::const_iterator it = storedCalls.find(c.callId);
        if (it == storedCalls.end()) { storedCalls[c.callId] = c; return true; }
        const TrainVoiceCall& old = it->second;
        return old.trainId == c.trainId && old.carId == c.carId && old.deviceKey == c.deviceKey &&
            old.audioFunction == c.audioFunction && old.sipExtension == c.sipExtension && old.sipCallId == c.sipCallId && old.clientId == c.clientId;
    }
    void TrainVoiceAccessFactory::removeActiveCall(const std::string& id) {
        if (failQuery) throw std::runtime_error("db failed");
        storedCalls.erase(id);
    }
    std::vector<TrainVoiceCallRequest> TrainVoiceAccessFactory::getPendingRequests() {
        std::vector<TrainVoiceCallRequest> requests;
        for (std::map<unsigned long long, TrainVoiceCallRequest>::const_iterator it = storedRequests.begin(); it != storedRequests.end(); ++it) requests.push_back(it->second);
        return requests;
    }
    void TrainVoiceAccessFactory::removePendingRequest(unsigned long long id) { storedRequests.erase(id); }
}

class FakeTransport : public TA_IRS_App::ITrainVoiceTransport
{
public:
    TA_IRS_App::TrainFrame sent = {};
    bool ready() const { return true; }
    bool send(const TA_IRS_App::TrainFrame& frame, const std::string&, unsigned short, std::string&) { sent = frame; return true; }
    TA_IRS_App::TrainRequestManager::WaitResult request(const TA_IRS_App::TrainFrame& frame, const std::string&, unsigned short,
        TA_IRS_App::TrainFrame& reply, std::string&) {
        sent = frame; reply.type = 2; reply.data = {frame.data[0], 1, 1, 2, 0}; return TA_IRS_App::TrainRequestManager::Completed;
    }
};

int main()
{
    TA_IRS_App::TrainVoiceReadService service;
    int status = 0;
    nlohmann::json result = nlohmann::json::parse(service.handle("GET", "/api/train-voice/trains", "", status));
    assert(status == 200 && result["trains"].size() == 2);
    assert(result["trains"][0]["cars"][0]["devices"][0]["sipExtension"] == "1002");
    assert(result["trains"][1]["cars"].empty());
    service.handle("GET", "/health", "", status); assert(status == 503);
    service.handle("GET", "/api/train-voice/active-calls", "", status); assert(status == 200);
    TA_IRS_App::TrainVoiceCallRequest pending = {};
    pending.requestId = 9007199254740993ULL; pending.trainId = 1; pending.sourceDeviceId = 0x0302; pending.requestType = "PASSENGER";
    TA_IRS_App::storedRequests[pending.requestId] = pending;
    result = nlohmann::json::parse(service.handle("GET", "/api/train-voice/call-requests", "", status));
    assert(status == 200 && result["requests"][0]["requestId"] == "9007199254740993" && result["requests"][0]["deviceIndex"] == 2);
    service.handle("DELETE", "/api/train-voice/call-requests/9007199254740993", "", status); assert(status == 204);
    service.handle("DELETE", "/api/train-voice/call-requests/9007199254740993", "", status); assert(status == 204);
    service.handle("DELETE", "/api/train-voice/call-requests/18446744073709551616", "", status); assert(status == 400);
    const std::string callId = "550e8400-e29b-41d4-a716-446655440000";
    const std::string callBody = "{\"callId\":\"" + callId + "\",\"trainId\":1,\"carId\":2,\"deviceKey\":3,\"audioFunction\":10,\"sipExtension\":\"1002\",\"sipCallId\":\"sip-1\",\"clientId\":\"web-1\"}";
    service.handle("POST", "/api/train-voice/active-calls", callBody, status); assert(status == 200);
    service.handle("POST", "/api/train-voice/active-calls", callBody, status); assert(status == 200);
    nlohmann::json conflicting = nlohmann::json::parse(callBody); conflicting["carId"] = 8;
    service.handle("POST", "/api/train-voice/active-calls", conflicting.dump(), status); assert(status == 409);
    result = nlohmann::json::parse(service.handle("GET", "/api/train-voice/active-calls", "", status));
    assert(status == 200 && result["calls"].size() == 1 && result["calls"][0]["callId"] == callId);
    service.handle("DELETE", "/api/train-voice/active-calls/" + callId, "", status); assert(status == 204);
    service.handle("DELETE", "/api/train-voice/active-calls/" + callId, "", status); assert(status == 204);
    service.handle("DELETE", "/api/train-voice/active-calls/not-a-uuid", "", status); assert(status == 400);
    conflicting["callId"] = "550e8400-e29b-41d4-a716-44665544000A";
    service.handle("POST", "/api/train-voice/active-calls", conflicting.dump(), status); assert(status == 400);
    conflicting["callId"] = callId; conflicting["sipCallId"] = "bad\\value";
    service.handle("POST", "/api/train-voice/active-calls", conflicting.dump(), status); assert(status == 400);
    TA_IRS_App::failQuery = true;
    service.handle("POST", "/api/train-voice/active-calls", callBody, status); assert(status == 503);
    service.handle("DELETE", "/api/train-voice/active-calls/" + callId, "", status); assert(status == 503);
    TA_IRS_App::failQuery = false;
    FakeTransport transport;
    TA_IRS_App::TrainVoiceReadService control(&transport);
    result = nlohmann::json::parse(control.handle("POST", "/api/train-voice/trains/1/audio-requests",
        "{\"carId\":2,\"function\":10,\"start\":true,\"additional\":1}", status));
    assert(status == 200 && result["result"] == "PROCEED" && transport.sent.data[0] == 10);
    result = nlohmann::json::parse(control.handle("POST", "/api/train-voice/trains/1/live-text",
        "{\"carId\":2,\"text\":\"Hello\"}", status));
    assert(status == 202 && transport.sent.type == 0x25 && transport.sent.data[0] == 5);
    control.handle("POST", "/api/train-voice/trains/1/live-text", "{\"carId\":2,\"text\":\"\"}", status);
    assert(status == 202 && transport.sent.data[0] == 0);
    control.handle("POST", "/api/train-voice/trains/1/live-text", "{\"carId\":2,\"text\":\"\\u4e2d\"}", status);
    assert(status == 202 && transport.sent.data == std::vector<uint8_t>({3, 0xe4, 0xb8, 0xad}));
    control.handle("POST", "/api/train-voice/trains/1/live-text", "{\"carId\":2,\"text\":\"" + std::string(256, 'a') + "\"}", status);
    assert(status == 400);
    control.handle("POST", "/api/train-voice/trains/1/md-commands", "{\"carId\":2,\"do0\":128,\"do1\":2}", status);
    assert(status == 202 && transport.sent.type == 0x0a && transport.sent.data == std::vector<uint8_t>({128, 2, 0}));
    control.handle("POST", "/api/train-voice/trains/1/md-commands", "{\"carId\":2,\"do0\":256,\"do1\":0}", status);
    assert(status == 400);
    control.handle("POST", "/api/train-voice/trains/1/pi-commands", "{\"carId\":2,\"command\":1}", status);
    assert(status == 202 && transport.sent.type == 0x04 && transport.sent.data == std::vector<uint8_t>({1, 0}));
    assert(transport.sent.destinationDevice[0] == 1 && transport.sent.destinationDevice[1] == 2);
    for (int command : {0, 2, 3, 10}) {
        control.handle("POST", "/api/train-voice/trains/1/pi-commands", "{\"carId\":2,\"command\":" + std::to_string(command) + "}", status);
        assert(status == 202 && transport.sent.data[0] == command);
    }
    control.handle("POST", "/api/train-voice/trains/1/pi-commands", "{\"carId\":2,\"command\":4}", status);
    assert(status == 400);
    control.handle("POST", "/api/train-voice/trains/1/pi-commands", "{\"carId\":2,\"command\":256}", status);
    assert(status == 400);
    control.handle("POST", "/api/train-voice/trains/1/train-information",
        "{\"carId\":2,\"timestamp\":\"2026-05-15T20:30:40\",\"carIds\":[101,102]}", status);
    assert(status == 202 && transport.sent.type == 0x0b &&
        transport.sent.data == std::vector<uint8_t>({26,5,15,20,30,40,0,101,102,0,1,0,0}));
    control.handle("POST", "/api/train-voice/trains/1/train-information", "{\"carId\":2}", status);
    assert(status == 400);
    control.handle("POST", "/api/train-voice/trains/1/train-information",
        "{\"carId\":2,\"timestamp\":\"2026-02-29T20:30:40\",\"carIds\":[101]}", status);
    assert(status == 400);
    control.handle("POST", "/api/train-voice/trains/1/train-information",
        "{\"carId\":2,\"timestamp\":\"2024-02-29T20:30:40\",\"carIds\":[101,101]}", status);
    assert(status == 400);
    TA_IRS_App::failQuery = true;
    service.handle("GET", "/api/train-voice/trains", "", status); assert(status == 503);
    return 0;
}
