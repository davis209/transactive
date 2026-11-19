#include "TrainEventConverter.h"
#include "json.hpp"

namespace TA_IRS_App
{
    bool TrainEventConverter::convert(const TrainFrame& frame, std::string& eventType,
                                      std::string& payloadJson, std::string& error)
    {
        eventType.clear();
        payloadJson.clear();
        error.clear();
        if (frame.sourceTrainId == 0 || frame.sourceTrainId == 255 || frame.destinationTrainId != 0)
        {
            error = "Invalid train message addressing";
            return false;
        }
        nlohmann::json payload = {
            {"trainId", frame.sourceTrainId},
            {"sequenceNumber", frame.sequence},
            {"sourceDeviceBytes", {frame.sourceDevice[0], frame.sourceDevice[1]}}
        };
        if (frame.type == 0x02)
        {
            uint8_t function, result, deviceId, deviceType;
            if (!TrainProtocol::parseAudioReply(frame, function, result, deviceId, deviceType, error)) return false;
            if (result != 2) { error = "Audio reply is not a reset"; return false; }
            eventType = "AudioFunctionReset";
            payload["audioFunction"] = function;
            payload["deviceId"] = deviceId;
            payload["deviceType"] = deviceType;
        }
        else if (frame.type == 0x06)
        {
            if (frame.data.size() != 2 || frame.data[0] > 1) { error = "Invalid driver callback request"; return false; }
            eventType = frame.data[0] == 1 ? "DriverCallRequested" : "DriverCallCancelled";
        }
        else if (frame.type == 0x03)
        {
            if (frame.data.size() != 3 || frame.data[0] > 4 || frame.data[1] > 3) { error = "Invalid PI state"; return false; }
            eventType = "PassengerIntercomStateChanged";
            payload["state"] = frame.data[0];
            payload["selfTestStatus"] = frame.data[1];
        }
        else if (frame.type == 0x0c)
        {
            if (frame.data.size() != 3 || frame.data[1] > 1) { error = "Invalid baggage detection"; return false; }
            eventType = "BaggageDetectionChanged";
            payload["cameraId"] = frame.data[0];
            payload["detected"] = frame.data[1] == 1;
        }
        else
        {
            error = "Unsupported event type";
            return false;
        }
        payloadJson = payload.dump();
        return true;
    }
}
