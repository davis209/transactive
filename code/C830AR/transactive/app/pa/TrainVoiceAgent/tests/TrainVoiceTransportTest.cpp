#include "../src/TrainVoiceTransport.h"
#include "../src/TrainEventPublisher.h"
#include "../src/TrainVoiceAccessFactory.h"
#include <cassert>
#include <mutex>
#include <thread>

namespace TA_IRS_App
{
    static std::mutex publishedLock;
    static std::string published;
    static std::vector<TrainVoiceCallRequest> savedRequests;
    TrainEventPublisher::TrainEventPublisher(const std::string&, const std::string&) : m_producer(0) {}
    TrainEventPublisher::~TrainEventPublisher() {}
    bool TrainEventPublisher::publish(const std::string& name, const std::string&)
    { std::lock_guard<std::mutex> guard(publishedLock); published = name; return true; }
    TrainVoiceAccessFactory& TrainVoiceAccessFactory::getInstance() { static TrainVoiceAccessFactory factory; return factory; }
    std::vector<TrainVoiceDevice> TrainVoiceAccessFactory::getDevices()
    {
        TrainVoiceDevice d = {}; d.trainId = 1; d.carId = 1; d.carPosition = 1; d.deviceType = "TCCU";
        d.protocolDeviceId = 0x0201; d.ipAddress = "127.0.0.1"; d.controlPort = 46481; return {d};
    }
    void TrainVoiceAccessFactory::addPendingRequest(unsigned long trainId, const std::string& type, unsigned long sourceId)
    {
        std::lock_guard<std::mutex> guard(publishedLock);
        for (size_t i = 0; i < savedRequests.size(); ++i)
            if (savedRequests[i].trainId == trainId && savedRequests[i].requestType == type && savedRequests[i].sourceDeviceId == sourceId) return;
        TrainVoiceCallRequest r = {}; r.trainId = trainId; r.requestType = type; r.sourceDeviceId = sourceId; savedRequests.push_back(r);
    }
}

int main()
{
    using namespace TA_IRS_App;
    TrainEventPublisher publisher("", "");
    TrainVoiceTransport bridge(publisher, "127.0.0.1", 46480);
    TrainUdpSocket train;
    std::string error;
    assert(train.open("127.0.0.1", 46481, error));
    // The transport starts without requiring a Control-mode transition.
    bridge.start();
    for (int i = 0; i < 100 && !bridge.ready(); ++i) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    assert(bridge.ready());
    std::vector<uint8_t> unsolicited; std::string unsolicitedAddress; uint16_t unsolicitedPort;
    assert(!train.receive(unsolicited, unsolicitedAddress, unsolicitedPort, 100, error) && error.empty());
    TrainFrame request = {};
    request.destinationTrainId = 1; request.sourceDevice[0] = 1; request.sourceDevice[1] = 0x80;
    request.destinationDevice[0] = 1; request.destinationDevice[1] = 2;
    assert(TrainProtocol::setAudioRequest(request, 2, true, 0, error));
    std::thread simulator([&] {
        std::vector<uint8_t> bytes; std::string address, failure; uint16_t port;
        assert(train.receive(bytes, address, port, 2000, failure));
        TrainFrame incoming = {}; assert(TrainProtocol::decode(bytes, incoming, failure));
        TrainFrame response = {}; response.sourceTrainId = 1; response.sourceDevice[0] = 1; response.sourceDevice[1] = 2;
        response.destinationDevice[0] = 1; response.destinationDevice[1] = 0x80;
        response.type = 2; response.sequence = incoming.sequence; response.data = {2, 1, 1, 2, 0};
        assert(TrainProtocol::encode(response, bytes, failure));
        assert(train.send("127.0.0.1", 46480, bytes, failure));
    });
    TrainFrame reply = {};
    assert(bridge.request(request, "127.0.0.1", 46481, reply, error) == TrainRequestManager::Completed);
    simulator.join();
    TrainFrame command = request;
    TrainProtocol::setMdCommand(command, 0x80, 2);
    assert(bridge.send(command, "127.0.0.1", 46481, error));
    std::vector<uint8_t> commandBytes; std::string commandAddress; uint16_t commandPort;
    assert(train.receive(commandBytes, commandAddress, commandPort, 2000, error));
    TrainFrame decodedCommand = {}; assert(TrainProtocol::decode(commandBytes, decodedCommand, error));
    assert(decodedCommand.type == 0x0a && decodedCommand.data == std::vector<uint8_t>({0x80, 2, 0}));
    assert(TrainProtocol::setPiCommand(command, 1, error));
    assert(bridge.send(command, "127.0.0.1", 46481, error));
    assert(train.receive(commandBytes, commandAddress, commandPort, 2000, error));
    assert(TrainProtocol::decode(commandBytes, decodedCommand, error));
    assert(decodedCommand.type == 0x04 && decodedCommand.data == std::vector<uint8_t>({1, 0}));
    assert(decodedCommand.destinationDevice[0] == 1 && decodedCommand.destinationDevice[1] == 2);
    assert(TrainProtocol::setLiveText(command, {0xe4, 0xb8, 0xad}, error));
    assert(bridge.send(command, "127.0.0.1", 46481, error));
    assert(train.receive(commandBytes, commandAddress, commandPort, 2000, error));
    assert(TrainProtocol::decode(commandBytes, decodedCommand, error));
    assert(decodedCommand.type == 0x25 && decodedCommand.data == std::vector<uint8_t>({3, 0xe4, 0xb8, 0xad}));
    TrainFrame event = {}; event.sourceTrainId = 1; event.sourceDevice[0] = 1; event.sourceDevice[1] = 1;
    event.destinationDevice[0] = 1; event.destinationDevice[1] = 0x80; event.type = 6; event.data = {1, 0};
    std::vector<uint8_t> bytes; assert(TrainProtocol::encode(event, bytes, error));
    assert(train.send("127.0.0.1", 46480, bytes, error));
    for (int i = 0; i < 100; ++i) {
        { std::lock_guard<std::mutex> guard(publishedLock); if (savedRequests.size() == 1) break; }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    { std::lock_guard<std::mutex> guard(publishedLock); assert(savedRequests.size() == 1 && published.empty()); }
    bridge.setKafkaPublishingEnabled(true);
    assert(train.send("127.0.0.1", 46480, bytes, error));
    for (int i = 0; i < 100; ++i) {
        { std::lock_guard<std::mutex> guard(publishedLock); if (published == "DriverCallRequested") break; }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    { std::lock_guard<std::mutex> guard(publishedLock); assert(published == "DriverCallRequested");
      assert(savedRequests.size() == 1 && savedRequests[0].sourceDeviceId == 0x0101 && savedRequests[0].requestType == "DRIVER"); }
    event.type = 3; event.sourceDevice[0] = 2; event.sourceDevice[1] = 3; event.data = {1, 0, 0};
    assert(TrainProtocol::encode(event, bytes, error) && train.send("127.0.0.1", 46480, bytes, error));
    for (int i = 0; i < 100; ++i) {
        { std::lock_guard<std::mutex> guard(publishedLock); if (savedRequests.size() == 2) break; }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    { std::lock_guard<std::mutex> guard(publishedLock); assert(savedRequests.size() == 2 && savedRequests[1].sourceDeviceId == 0x0302); }
    assert(train.send("127.0.0.1", 46480, bytes, error));
    event.type = 0x0c; event.data = {2, 1, 0};
    assert(TrainProtocol::encode(event, bytes, error) && train.send("127.0.0.1", 46480, bytes, error));
    for (int i = 0; i < 100; ++i) {
        { std::lock_guard<std::mutex> guard(publishedLock); if (published == "BaggageDetectionChanged") break; }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    { std::lock_guard<std::mutex> guard(publishedLock); assert(published == "BaggageDetectionChanged" && savedRequests.size() == 2); }
    event.type = 0x02; event.data = {10, 2, 1, 3, 0};
    assert(TrainProtocol::encode(event, bytes, error) && train.send("127.0.0.1", 46480, bytes, error));
    for (int i = 0; i < 100; ++i) {
        { std::lock_guard<std::mutex> guard(publishedLock); if (published == "AudioFunctionReset") break; }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    { std::lock_guard<std::mutex> guard(publishedLock); assert(published == "AudioFunctionReset"); }
    bridge.setKafkaPublishingEnabled(false);
    assert(bridge.ready());
    event.type = 6; event.sourceDevice[0] = 3; event.sourceDevice[1] = 1; event.data = {1, 0};
    assert(TrainProtocol::encode(event, bytes, error) && train.send("127.0.0.1", 46480, bytes, error));
    for (int i = 0; i < 100; ++i) {
        { std::lock_guard<std::mutex> guard(publishedLock); if (savedRequests.size() == 3) break; }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    { std::lock_guard<std::mutex> guard(publishedLock); assert(savedRequests.size() == 3 && published == "AudioFunctionReset"); }
    TrainProtocol::setMdCommand(command, 1, 2);
    assert(bridge.send(command, "127.0.0.1", 46481, error));
    assert(train.receive(commandBytes, commandAddress, commandPort, 2000, error));
    bridge.terminateAndWait();
    assert(!bridge.ready());
    return 0;
}
