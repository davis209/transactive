#pragma once
#include "TrainVoiceData.h"

namespace TA_IRS_App
{
    class TrainVoiceAccessFactory
    {
    public:
        static TrainVoiceAccessFactory& getInstance();
        std::vector<TrainVoiceDevice> getDevices();
        std::vector<TrainVoiceCall> getActiveCalls();
        // Atomic no-op on duplicate; returns false if the same ID has different content.
        bool addActiveCall(const TrainVoiceCall& call);
        void removeActiveCall(const std::string& callId);
        void addPendingRequest(unsigned long trainId, const std::string& type, unsigned long sourceDeviceId);
        std::vector<TrainVoiceCallRequest> getPendingRequests();
        void removePendingRequest(unsigned long long requestId);
    private:
        TrainVoiceAccessFactory() {}
        TrainVoiceAccessFactory(const TrainVoiceAccessFactory&);
        TrainVoiceAccessFactory& operator=(const TrainVoiceAccessFactory&);
    };
}
