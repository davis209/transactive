#include "../src/TrainEventConverter.h"
#include "../src/json.hpp"
#include <cassert>

int main()
{
    using namespace TA_IRS_App;
    TrainFrame frame = {};
    frame.sourceTrainId = 1;
    frame.destinationTrainId = 0;
    frame.sourceDevice[0] = 1;
    frame.sourceDevice[1] = 2;
    frame.sequence = 42;
    std::string event, payload, error;
    frame.type = 6;
    frame.data = {1, 0};
    assert(TrainEventConverter::convert(frame, event, payload, error));
    assert(event == "DriverCallRequested");
    assert(nlohmann::json::parse(payload)["trainId"] == 1);
    frame.data[0] = 0;
    assert(TrainEventConverter::convert(frame, event, payload, error) && event == "DriverCallCancelled");
    frame.type = 2;
    frame.data = {10, 2, 3, 2, 0};
    assert(TrainEventConverter::convert(frame, event, payload, error) && event == "AudioFunctionReset");
    assert(nlohmann::json::parse(payload)["audioFunction"] == 10);
    frame.data[1] = 1;
    assert(!TrainEventConverter::convert(frame, event, payload, error));
    frame.type = 3;
    frame.data = {1, 0, 0};
    assert(TrainEventConverter::convert(frame, event, payload, error) && event == "PassengerIntercomStateChanged");
    frame.type = 0x0c; frame.data = {5, 1, 0};
    assert(TrainEventConverter::convert(frame, event, payload, error) && event == "BaggageDetectionChanged");
    assert(nlohmann::json::parse(payload)["detected"] == true);
    frame.destinationTrainId = 2;
    assert(!TrainEventConverter::convert(frame, event, payload, error));
    return 0;
}
