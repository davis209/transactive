#pragma once
#include <string>
#include <vector>

namespace TA_IRS_App
{
    struct TrainVoiceDevice
    {
        unsigned long trainId, carId, carPosition, deviceKey, protocolDeviceId, controlPort;
        std::string trainName, carName, deviceType, deviceName, ipAddress, sipExtension;
    };
    struct TrainVoiceCall
    {
        unsigned long trainId, carId, deviceKey, audioFunction;
        std::string callId, sipExtension, sipCallId, clientId, startedAt;
    };
    struct TrainVoiceCallRequest
    {
        unsigned long long requestId;
        unsigned long trainId, sourceDeviceId;
        std::string requestType, receivedAt;
    };
}
