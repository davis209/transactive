#pragma once
#include "TrainProtocol.h"
#include <string>

namespace TA_IRS_App
{
    class TrainEventConverter
    {
    public:
        // Converts supported unsolicited messages; invalid frames are rejected.
        static bool convert(const TrainFrame& frame, std::string& eventType,
                            std::string& payloadJson, std::string& error);
    };
}
