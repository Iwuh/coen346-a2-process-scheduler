#include <iostream>
#include <fstream>
#include <vector>
#include "include/process.h"
#include "include/clock.h"

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
    std::vector<Process> processList; // vector of Processes

    processList.reserve(numOfProcess); // Reserves number of processes in vector
    // Reads from file and instantiates objects that are added to the Vector
    for (int i = 0; i < numOfProcess; i++)
    {
        myfile >> id >> arrivalTime >> burstTime >> initialPriority;
        processList.push_back(Process(id, arrivalTime, burstTime, initialPriority));
    }
    return 0;
}