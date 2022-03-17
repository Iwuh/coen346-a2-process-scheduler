#ifndef PRIORITY_COMPARE_H
#define PRIORITY_COMPARE_H

#include "process.h"

// Compares two process pointers by the underlying objects' priorities.
class PriorityCompare
{
public:
    bool operator()(Process* p1, Process* p2)
    {
        return *p1 < *p2;
    }
};

#endif