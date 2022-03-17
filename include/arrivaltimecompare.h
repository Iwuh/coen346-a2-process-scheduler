#ifndef ARRIVAL_TIME_COMPARE_H
#define ARRIVAL_TIME_COMPARE_H

#include "process.h"

// Compares two process pointers by the underlying objects' arrival times for use in std::sort. Returns true when p1 arrives sooner than p2.
class ArrivalTimeCompare
{
public:
    bool operator()(Process* p1, Process* p2)
    {
        return p1->getArrivalTime() < p2->getArrivalTime();
    }
};

#endif