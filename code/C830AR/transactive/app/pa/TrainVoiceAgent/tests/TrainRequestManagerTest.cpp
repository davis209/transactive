#include "../src/TrainRequestManager.h"
#include <cassert>

int main()
{
    using namespace TA_IRS_App;
    TrainRequestManager manager(std::chrono::minutes(1));
    const uint8_t device[] = {1, 2};
    TrainRequestManager::TicketPtr first = manager.reserve(1, 2, device);
    TrainRequestManager::TicketPtr other = manager.reserve(2, 2, device);
    assert(first && other && first->sequence == other->sequence);
    TrainFrame frame = {};
    frame.sourceTrainId = 2;
    frame.destinationTrainId = 0;
    frame.sourceDevice[0] = 1;
    frame.sourceDevice[1] = 2;
    frame.type = 2;
    frame.sequence = other->sequence;
    frame.data = {2, 2, 1, 2, 0};
    assert(manager.receive(frame) == TrainRequestManager::ResetEvent);
    frame.data[1] = 1;
    frame.data[0] = 6;
    assert(manager.receive(frame) == TrainRequestManager::Unmatched);
    frame.data[0] = 2;
    assert(manager.receive(frame) == TrainRequestManager::Matched);
    TrainFrame reply = {};
    assert(manager.wait(other, std::chrono::milliseconds(0), reply) == TrainRequestManager::Completed);
    assert(manager.receive(frame) == TrainRequestManager::Unmatched);
    assert(manager.wait(first, std::chrono::milliseconds(0), reply) == TrainRequestManager::TimedOut);
    frame.sourceTrainId = 1;
    assert(manager.receive(frame) == TrainRequestManager::Unmatched);
    TrainRequestManager::TicketPtr next = manager.reserve(1, 2, device);
    assert(next && next->sequence != first->sequence);
    manager.cancelAll();
    assert(manager.wait(next, std::chrono::milliseconds(0), reply) == TrainRequestManager::Cancelled);
    std::vector<TrainRequestManager::TicketPtr> tickets;
    for (uint8_t train = 10; train < 35; ++train) {
        TrainRequestManager::TicketPtr ticket = manager.reserve(train, 6, device);
        assert(ticket); tickets.push_back(ticket);
    }
    frame.data = {6, 1, 1, 2, 0};
    for (std::vector<TrainRequestManager::TicketPtr>::reverse_iterator it = tickets.rbegin(); it != tickets.rend(); ++it) {
        frame.sourceTrainId = (*it)->trainId; frame.sequence = (*it)->sequence;
        assert(manager.receive(frame) == TrainRequestManager::Matched);
    }
    for (size_t i = 0; i < tickets.size(); ++i) {
        assert(manager.wait(tickets[i], std::chrono::milliseconds(0), reply) == TrainRequestManager::Completed);
        assert(reply.sourceTrainId == tickets[i]->trainId);
    }
    return 0;
}
