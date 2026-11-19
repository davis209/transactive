#include "TrainProtocol.h"

namespace TA_IRS_App
{
    bool TrainProtocol::setPiCommand(TrainFrame& frame, uint8_t command, std::string& error)
    {
        error.clear();
        if (command != 0 && command != 1 && command != 2 && command != 3 && command != 10)
        { error = "Invalid PI command"; return false; }
        frame.type = 0x04;
        frame.data = {command, 0};
        return true;
    }
    void TrainProtocol::setMdCommand(TrainFrame& frame, uint8_t do0, uint8_t do1)
    {
        frame.type = 0x0a;
        frame.data = {do0, do1, 0};
    }
    bool TrainProtocol::setTrainInformation(TrainFrame& frame, const std::vector<uint8_t>& localTime,
        const std::vector<uint8_t>& cars, uint16_t trainId, std::string& error)
    {
        error.clear();
        if (localTime.size() != 6 || cars.empty() || cars.size() > 3 || trainId == 0 || trainId > 254)
        { error = "Invalid train information"; return false; }
        for (size_t i = 0; i < cars.size(); ++i)
            if (cars[i] == 0) { error = "Invalid car ID"; return false; }
        frame.type = 0x0b;
        frame.data = localTime;
        frame.data.push_back(0);
        frame.data.insert(frame.data.end(), cars.begin(), cars.end());
        while (frame.data.size() < 10) frame.data.push_back(0);
        frame.data.push_back(static_cast<uint8_t>(trainId & 0xff));
        frame.data.push_back(static_cast<uint8_t>(trainId >> 8));
        frame.data.push_back(0);
        return true;
    }
    bool TrainProtocol::setLiveText(TrainFrame& frame, const std::vector<uint8_t>& encodedText, std::string& error)
    {
        error.clear();
        if (encodedText.size() > 255) { error = "Live text exceeds 255 bytes"; return false; }
        frame.type = 0x25;
        frame.data.clear();
        frame.data.push_back(static_cast<uint8_t>(encodedText.size()));
        frame.data.insert(frame.data.end(), encodedText.begin(), encodedText.end());
        return true;
    }
    bool TrainProtocol::setAudioRequest(TrainFrame& frame, uint8_t function, bool start,
                                        uint8_t additional, std::string& error)
    {
        error.clear();
        if (function != 2 && function != 4 && function != 6 && function != 10 && function != 11)
        {
            error = "Unsupported CCW audio function";
            return false;
        }
        if (function != 4 && function != 10 && function != 11 && additional != 0)
        {
            error = "Unexpected audio function additional value";
            return false;
        }
        frame.type = 0x01;
        frame.data.clear();
        frame.data.push_back(function);
        frame.data.push_back(start ? 1 : 0);
        frame.data.push_back(additional);
        frame.data.push_back(0);
        return true;
    }

    bool TrainProtocol::parseAudioReply(const TrainFrame& frame, uint8_t& function, uint8_t& result,
                                        uint8_t& deviceId, uint8_t& deviceType, std::string& error)
    {
        error.clear();
        if (frame.type != 0x02 || frame.data.size() != 5 || frame.data[1] > 2)
        {
            error = "Invalid audio reply";
            return false;
        }
        function = frame.data[0];
        result = frame.data[1];
        deviceId = frame.data[2];
        deviceType = frame.data[3];
        return true;
    }

    bool TrainProtocol::encode(const TrainFrame& frame, std::vector<uint8_t>& bytes, std::string& error)
    {
        bytes.clear();
        error.clear();
        if (frame.data.size() > 65533)
        {
            error = "Message data exceeds protocol length";
            return false;
        }
        const unsigned int length = static_cast<unsigned int>(frame.data.size()) + 2;
        bytes.push_back(0x02);
        bytes.push_back(frame.sourceTrainId);
        bytes.push_back(frame.destinationTrainId);
        bytes.push_back(frame.sourceDevice[0]);
        bytes.push_back(frame.sourceDevice[1]);
        bytes.push_back(frame.destinationDevice[0]);
        bytes.push_back(frame.destinationDevice[1]);
        bytes.push_back(static_cast<uint8_t>(length & 0xff));
        bytes.push_back(static_cast<uint8_t>(length >> 8));
        bytes.push_back(frame.type);
        bytes.push_back(frame.sequence);
        bytes.insert(bytes.end(), frame.data.begin(), frame.data.end());
        uint8_t bcc = 0;
        for (size_t i = 1; i < bytes.size(); ++i) bcc ^= bytes[i];
        bytes.push_back(0x03);
        bytes.push_back(bcc);
        return true;
    }

    bool TrainProtocol::decode(const std::vector<uint8_t>& bytes, TrainFrame& frame, std::string& error)
    {
        error.clear();
        if (bytes.size() < 13 || bytes[0] != 0x02)
        {
            error = "Invalid frame header or size";
            return false;
        }
        const size_t length = static_cast<size_t>(bytes[7]) | (static_cast<size_t>(bytes[8]) << 8);
        if (length < 2 || bytes.size() != 11 + length || bytes[bytes.size() - 2] != 0x03)
        {
            error = "Invalid frame length or terminator";
            return false;
        }
        uint8_t bcc = 0;
        for (size_t i = 1; i < bytes.size() - 2; ++i) bcc ^= bytes[i];
        if (bcc != bytes.back())
        {
            error = "Invalid frame checksum";
            return false;
        }
        TrainFrame result;
        result.sourceTrainId = bytes[1];
        result.destinationTrainId = bytes[2];
        result.sourceDevice[0] = bytes[3];
        result.sourceDevice[1] = bytes[4];
        result.destinationDevice[0] = bytes[5];
        result.destinationDevice[1] = bytes[6];
        result.type = bytes[9];
        result.sequence = bytes[10];
        result.data.assign(bytes.begin() + 11, bytes.end() - 2);
        frame = result;
        return true;
    }
}
