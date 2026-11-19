#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace TA_IRS_App
{
    struct TrainFrame
    {
        uint8_t sourceTrainId;
        uint8_t destinationTrainId;
        // Device IDs are kept as the two on-wire bytes; their encoding must not be guessed.
        uint8_t sourceDevice[2];
        uint8_t destinationDevice[2];
        uint8_t type;
        uint8_t sequence;
        std::vector<uint8_t> data;
    };

    class TrainProtocol
    {
    public:
        static bool encode(const TrainFrame& frame, std::vector<uint8_t>& bytes, std::string& error);
        static bool decode(const std::vector<uint8_t>& bytes, TrainFrame& frame, std::string& error);
        static bool setAudioRequest(TrainFrame& frame, uint8_t function, bool start,
                                    uint8_t additional, std::string& error);
        static bool parseAudioReply(const TrainFrame& frame, uint8_t& function, uint8_t& result,
                                    uint8_t& deviceId, uint8_t& deviceType, std::string& error);
        static bool setLiveText(TrainFrame& frame, const std::vector<uint8_t>& encodedText, std::string& error);
        static bool setPiCommand(TrainFrame& frame, uint8_t command, std::string& error);
        static void setMdCommand(TrainFrame& frame, uint8_t do0, uint8_t do1);
        static bool setTrainInformation(TrainFrame& frame, const std::vector<uint8_t>& localTime,
                                        const std::vector<uint8_t>& cars, uint16_t trainId, std::string& error);
    };
}
