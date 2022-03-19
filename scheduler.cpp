#include "include/scheduler.h"

#include <algorithm>

#include "include/clock.h"
#include <fstream>
Scheduler::Scheduler(ProcessArrivalQueue &queue)
    : arrivalQueue(queue)
{
}

void Scheduler::operator()(std::atomic_bool &stopFlag)
{
    Clock &clock = Clock::getInstance();
    std::ofstream outFile;
    outFile.open("output.txt");
    // Initialize active and expired queues as pointers to class members
    auto active = &q1;
    auto expired = &q2;

    // Initialize the active queue with any processes that have an arrival time of 0
    while (arrivalQueue.peek()->getArrivalTime() == 0)
    {
        Process *p = arrivalQueue.pop();
        outFile << "Time " << clock.getTime() << ", " << p->getName() << ", Arrived" << std::endl;
        expired->push(p);
    }

    // Run the scheduler until we tell it to stop from the main thread
    while (!stopFlag)
    {
        // Check for new processes and then sleep for 50 milliseconds if both queues are empty to avoid constantly swapping the queues
        if (active->empty() && expired->empty())
        {
            while (arrivalQueue.peek()->getArrivalTime() <= clock.getTime())
            {
                Process *p = arrivalQueue.pop();
                outFile << "Time " << clock.getTime() << ", " << p->getName() << ", Arrived" << std::endl;
                expired->push(p);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        while (!active->empty())
        {
            // Check for new processes and put them in the expired queue
            while (arrivalQueue.peek()->getArrivalTime() <= clock.getTime())
            {
                Process *p = arrivalQueue.pop();
                outFile << "Time " << clock.getTime() << ", " << p->getName() << ", Arrived" << std::endl;
                expired->push(p);
            }

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

            int currentTime = clock.getTime();
            int endTime = currentTime + timeSlotLength;

            // To start a process for the first time
            if (CpuProcess->getRunningTime() == 0)
            {
                // We can't start the thread directly with the Process object, because std::thread wants to copy it.
                // We can't let the Process be copied or it will invalidate the synchronization objects used for multithreading.
                // Instead we run a lambda function that takes a pointer to a Process and then calls it.
                outFile << "Time " << clock.getTime() << ", " << CpuProcess->getName() << ", " << CpuProcess->getState() << ", Granted" << timeSlotLength << std::endl;
                processThreads[CpuProcess->getName()] = new std::thread(
                    [](Process *p)
                    {
                        (*p)();
                    },
                    CpuProcess);

                CpuProcess->setState(Process::State::Started);
                CpuProcess->update();
                priorityUpdator[CpuProcess->getName()] = 0;
            }

            // To resume a process that's already been started
            if (CpuProcess->getRunningTime() != 0)
            {
                CpuProcess->setState(Process::State::Resumed);
                CpuProcess->update();
                outFile << "Time " << clock.getTime() << ", " << CpuProcess->getName() << ", " << CpuProcess->getState() << ", Granted" << timeSlotLength << std::endl;
            }

            // running time of process in the cpu
            while (clock.getTime() < endTime)
                std::this_thread::sleep_for(std::chrono::milliseconds(50));

            // To pause a process that's running
            CpuProcess->setState(Process::State::Paused);
            CpuProcess->update();
            outFile << "Time " << clock.getTime() << ", " << CpuProcess->getName() << ", " << CpuProcess->getState() << std::endl;

            if (CpuProcess->isTerminated())
            {
                // Join and delete the thread created to run the process
                outFile << "Time " << clock.getTime() << ", " << CpuProcess->getName() << ", Terminated" << std::endl;
                processThreads[CpuProcess->getName()]->join();
                delete processThreads[CpuProcess->getName()];
                processThreads.erase(CpuProcess->getName());

                // Delete the process itself
                delete CpuProcess;
            }
            else
            {
                // Update number of time slots process has recieved, key value pair with string (name of process) and int (number of time slots)
                priorityUpdator[CpuProcess->getName()]++;

                // Update priority after every second time slot
                if (priorityUpdator[CpuProcess->getName()] % 2 && priorityUpdator[CpuProcess->getName()] > 0)
                {
                    int bonus = ((10 * CpuProcess->getWaitingTime()) / (clock.getTime() - CpuProcess->getArrivalTime()));
                    CpuProcess->setPriority(std::max(100, std::min(CpuProcess->getPriority() - bonus + 5, 139)));
                    outFile << "Time " << clock.getTime() << ", " << CpuProcess->getName() << ", priority updated to " << CpuProcess->getPriority() << std::endl;
                }

                expired->push(CpuProcess);
            }
        }

        // Once the active queue is empty, swap the pointers to swap the queues
        auto temp = active;
        active = expired;
        expired = temp;
    }
}