#include "../src/TrainUdpSocket.h"
#include <cassert>

int main()
{
    TA_IRS_App::TrainUdpSocket sender, receiver;
    std::string error, address;
    uint16_t sourcePort = 0;
    assert(receiver.open("127.0.0.1", 46479, error));
    assert(sender.open("127.0.0.1", 0, error));
    std::vector<uint8_t> bytes = {2, 0, 3};
    assert(sender.send("127.0.0.1", 46479, bytes, error));
    std::vector<uint8_t> received;
    assert(receiver.receive(received, address, sourcePort, 1000, error));
    assert(received == bytes && address == "127.0.0.1" && sourcePort != 0);
    assert(!receiver.receive(received, address, sourcePort, 10, error) && error.empty());
    assert(!sender.send("invalid", 46479, bytes, error));
    return 0;
}
