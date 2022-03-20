#include <iostream>
#include <fstream>
#include <vector>
#include "include/process.h"
#include "include/clock.h"
#include "include/scheduler.h"

int main()
{
    std::ifstream myfile;
    myfile.open("input.txt");
    if (!myfile)
    {
        std::cout << "Can not open file.";
        exit(1);
    }
    int numOfProcess, arrivalTime, burstTime, initialPriority;
    std::string id;

    myfile >> numOfProcess;           // read number of processes
    std::vector<Process*> processList; // vector of Processes

    processList.reserve(numOfProcess); // Reserves number of processes in vector
    // Reads from file and instantiates objects that are added to the Vector
    for (int i = 0; i < numOfProcess; i++)
    {
        myfile >> id >> arrivalTime >> burstTime >> initialPriority;
        processList.push_back(new Process(id, arrivalTime, burstTime, initialPriority));
    }
    myfile.close();

    Clock& clock = Clock::getInstance();
    ProcessArrivalQueue queue(processList);
    Scheduler scheduler(queue);

    std::atomic_bool stopScheduler = false;

    std::thread clockThread(
        [](Clock* c)
        {
            (*c)();
        }, 
        &clock);

    std::thread schedulerThread(
        [](Scheduler* s, std::atomic_bool& stopFlag)
        {
            (*s)(stopFlag);
        },
        &scheduler, std::ref(stopScheduler));

    clockThread.join();
    stopScheduler = true;
    schedulerThread.join();

    return 0;
}