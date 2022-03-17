#ifndef PROCESS_ARRIVAL_QUEUE_H
#define PROCESS_ARRIVAL_QUEUE_H

#include <queue>

#include "process.h"
#include "arrivaltimecompare.h"

class ProcessArrivalQueue
{
public:
    ProcessArrivalQueue(std::vector<Process*> processes);

    bool empty() const;
    Process* peek() const;
    Process* pop();

private:
    std::queue<Process*> queue;
};

#endif