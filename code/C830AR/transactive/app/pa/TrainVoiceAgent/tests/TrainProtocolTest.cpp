#include "../src/TrainProtocol.h"
#include <cassert>

int main()
{
    using namespace TA_IRS_App;
    const uint8_t example[] = {0x02, 0x01, 0x00, 0x01, 0x02, 0x01, 0x80,
        0x04, 0x00, 0x50, 0x01, 0x0a, 0x0b, 0x03, 0xd7};
    std::vector<uint8_t> bytes(example, example + sizeof(example));
    TrainFrame frame;
    std::string error;
    assert(TrainProtocol::decode(bytes, frame, error));
    assert(frame.sourceTrainId == 1 && frame.type == 0x50 && frame.sequence == 1);
    assert(frame.data.size() == 2 && frame.data[0] == 0x0a && frame.data[1] == 0x0b);
    std::vector<uint8_t> encoded;
    assert(TrainProtocol::encode(frame, encoded, error));
    assert(encoded == bytes);
    encoded.back() ^= 1;
    assert(!TrainProtocol::decode(encoded, frame, error));
    encoded = bytes;
    encoded[7] = 3;
    assert(!TrainProtocol::decode(encoded, frame, error));
    encoded = bytes;
    encoded[encoded.size() - 2] = 0;
    assert(!TrainProtocol::decode(encoded, frame, error));
    encoded = bytes;
    encoded.pop_back();
    assert(!TrainProtocol::decode(encoded, frame, error));
    frame.data.assign(65534, 0);
    assert(!TrainProtocol::encode(frame, encoded, error));
    assert(TrainProtocol::setAudioRequest(frame, 10, true, 7, error));
    assert(frame.type == 1 && frame.data.size() == 4 && frame.data[0] == 10 && frame.data[1] == 1 && frame.data[2] == 7);
    assert(!TrainProtocol::setAudioRequest(frame, 3, true, 0, error));
    assert(!TrainProtocol::setAudioRequest(frame, 2, true, 1, error));
    frame.type = 2;
    frame.data = {10, 1, 7, 2, 0};
    uint8_t function, result, deviceId, deviceType;
    assert(TrainProtocol::parseAudioReply(frame, function, result, deviceId, deviceType, error));
    assert(function == 10 && result == 1 && deviceId == 7 && deviceType == 2);
    frame.data[1] = 3;
    assert(!TrainProtocol::parseAudioReply(frame, function, result, deviceId, deviceType, error));
    const std::vector<uint8_t> text = {0x41, 0x00};
    assert(TrainProtocol::setLiveText(frame, text, error));
    assert(frame.type == 0x25 && frame.data == std::vector<uint8_t>({2, 0x41, 0}));
    assert(TrainProtocol::setLiveText(frame, std::vector<uint8_t>(), error) && frame.data[0] == 0);
    assert(!TrainProtocol::setLiveText(frame, std::vector<uint8_t>(256), error));
    assert(TrainProtocol::setPiCommand(frame, 10, error));
    assert(frame.type == 4 && frame.data == std::vector<uint8_t>({10, 0}));
    assert(!TrainProtocol::setPiCommand(frame, 4, error));
    TrainProtocol::setMdCommand(frame, 0x80, 0x02);
    assert(frame.type == 0x0a && frame.data == std::vector<uint8_t>({0x80, 2, 0}));
    assert(TrainProtocol::setTrainInformation(frame, {26, 5, 15, 20, 30, 40}, {1, 2}, 1, error));
    assert(frame.type == 0x0b && frame.data == std::vector<uint8_t>({26, 5, 15, 20, 30, 40, 0, 1, 2, 0, 1, 0, 0}));
    assert(!TrainProtocol::setTrainInformation(frame, {26}, {1}, 1, error));
    return 0;
}
