#include "include/processarrivalqueue.h"

ProcessArrivalQueue::ProcessArrivalQueue(std::vector<Process*> processes)
{
    // No point in using a priority queue when we only add values once. Instead we can sort the input array and then populate an unsorted queue.
    std::sort(processes.begin(), processes.end(), ArrivalTimeCompare());
    for (auto& i : processes)
    {
        queue.push(i);
    }
}

bool ProcessArrivalQueue::empty() const
{
    return queue.empty();
}

Process* ProcessArrivalQueue::peek() const
{
    return queue.front();
}

Process* ProcessArrivalQueue::pop()
{
    auto next = peek();
    queue.pop();
    return next;
}