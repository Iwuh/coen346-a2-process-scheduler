#include "include/scheduler.h"

#include <algorithm>

#include "include/clock.h"

void Scheduler::operator()()
{
    auto active = &q1;
    auto expired = &q2;

    while (!active->empty())
    {
        Process *CpuProcess = active->top(); // sees highest priority
        active->pop();                       // pops highest priority

        int timeSlotLength;
        // Figure out the time slot length
        if (CpuProcess->getPriority() < 100)
        {
            timeSlotLength = (140 - CpuProcess->getPriority()) * 20;
        }
        if (CpuProcess->getPriority() >= 100)
        {
            timeSlotLength = (140 - CpuProcess->getPriority()) * 5;
        }

        Clock &clock = Clock::getInstance();
        int currentTime = clock.getTime();
        int endTime = currentTime + timeSlotLength;

        // To start a process for the first time
        if (CpuProcess->getRunningTime() == 0)
        {
            // We can't start the thread directly with the Process object, because std::thread wants to copy it. 
            // We can't let the Process be copied or it will invalidate the synchronization objects used for multithreading.
            // Instead we run a lambda function that takes a pointer to a Process and then calls it.
            processThreads[CpuProcess->getName()] = new std::thread([](Process *p){ (*p)(); }, CpuProcess);

            CpuProcess->setState(Process::State::Started);
            CpuProcess->update();
            priorityUpdator[CpuProcess->getName()] = 0;
        }

        // To resume a process that's already been started
        if (CpuProcess->getRunningTime() != 0)
        {
            CpuProcess->setState(Process::State::Resumed);
            CpuProcess->update();
        }



        // running time of process in the cpu
        while (clock.getTime() < endTime && !CpuProcess->isTerminated())
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // To pause a process that's running
        CpuProcess->setState(Process::State::Paused);
        CpuProcess->update();

        if (CpuProcess->isTerminated())
        {
            // Join and delete the thread created to run the process
            processThreads[CpuProcess->getName()]->join();
            delete processThreads[CpuProcess->getName()];
            processThreads.erase(CpuProcess->getName());

            // Delete the process itself
            delete CpuProcess;

            // Skip updating the time slot and count if the process is terminated
            continue;
        }

        // Update number of time slots process has recieved, key value pair with string (name of process) and int (number of time slots)
        priorityUpdator[CpuProcess->getName()]++;

        // Update priority after every second time slot
        if (priorityUpdator[CpuProcess->getName()] % 2 && priorityUpdator[CpuProcess->getName()] > 0)
        {
            int bonus = ((10 * CpuProcess->getWaitingTime()) / (clock.getTime() - CpuProcess->getArrivalTime()));
            CpuProcess->setPriority(std::max(100, std::min(CpuProcess->getPriority() - bonus + 5, 139)));
        }

        expired->push(CpuProcess);           
    }

    // Once the active queue is empty, swap the pointers to swap the queues
    auto temp = active;
    active = expired;
    expired = temp;
}