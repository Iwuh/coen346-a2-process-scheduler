#include "include/scheduler.h"
#include "include/clock.h"
#include "include/process.h"
#include <map>
#include <algorithm>
#include <thread>


void Scheduler::operator()()
  {
    std::priority_queue<Process*> *active = &q1;
    std::priority_queue<Process*> *expired = &q2;
    
    while (!active->empty())
    {
        Process* CpuProcess = active->top(); //sees highest priority
        active->pop(); //pops highest priority

    
        int timeSlotLength;
        // Figure out the time slot length
        if (CpuProcess->getPriority() < 100) {
            timeSlotLength = (140 - CpuProcess->getPriority()) * 20;
        }
        if (CpuProcess->getPriority() >= 100) {
            timeSlotLength = (140 - CpuProcess->getPriority()) * 5;
        }
        
        Clock& clock = Clock::getInstance();
        int currentTime = clock.getTime();
        int endTime = currentTime + timeSlotLength;

        // To start a process for the first time
        if (CpuProcess->getRunningTime() == 0) {
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

        // To pause a process that's running
        CpuProcess->setState(Process::State::Paused);
        CpuProcess->update();

        //running time of process in the cpu
        while (clock.getTime() < endTime)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));


        //update priority, key value pair with string (name of process) and int (number of time slots)
        priorityUpdator[CpuProcess->getName()]++;

        if(priorityUpdator[CpuProcess->getName()] % 2 && priorityUpdator[CpuProcess->getName()] > 0 ) {
            int bonus = ((10* CpuProcess->getWaitingTime()) / (clock.getTime() - CpuProcess->getArrivalTime()));
            CpuProcess->setPriority(std::max(100, std::min(CpuProcess->getPriority() - bonus + 5,139)));
        }
        

        // Check if the process is terminated, if it is then delete it, otherwise move it into the expired queue
        if (!CpuProcess->isTerminated())
            expired->push(CpuProcess);
        }

        // Once the active queue is empty, swap the pointers to swap the queues
        std::priority_queue<Process*>* temp = active;
        active = expired;
        expired = temp;
  }