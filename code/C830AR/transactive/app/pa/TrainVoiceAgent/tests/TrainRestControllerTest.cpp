#include "../src/TrainRestController.h"
#include <cassert>

class FakeService : public TA_IRS_App::ITrainVoiceService
{
public:
    std::string handle(const std::string&, const std::string&, const std::string&, int& status)
    { status = 200; return "{\"ok\":true}"; }
};

int main()
{
    int status = 0;
    TA_IRS_App::TrainRestController unavailable(0);
    assert(unavailable.handle("GET", "/health", "", status) == "{\"status\":\"starting\"}" && status == 503);
    assert(unavailable.handle("POST", "/api/train-voice/trains/1/audio-requests", "{}", status).find("unavailable") != std::string::npos && status == 503);
    FakeService service;
    TA_IRS_App::TrainRestController ready(&service);
    assert(ready.handle("GET", "/health", "", status) == "{\"ok\":true}" && status == 200);
    return 0;
}
