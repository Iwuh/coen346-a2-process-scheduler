#ifndef PROCESS_H
#define PROCESS_H

#include <mutex>
#include <semaphore>

class Process
{
public:
    Process(int arrivalTime, int burstTime);

    int getArrivalTime() const;
    int getBurstTime() const;
    int getWaitingTime() const;
    int getRunningTime() const;
    int getRemainingTime() const;
    bool isTerminated() const;

    int getPriority() const;
    void setPriority(int newPriority);

    // Gets the binary semaphore used to start the process while it's waiting.
    std::binary_semaphore& getStartSignal();
    // Gets the binary semaphore used to stop the process while it's running.
    std::binary_semaphore& getStopSignal();

    bool operator<(const Process& other);
    bool operator>(const Process& other);
    bool operator==(const Process& other);
    void operator()();

private:
    const int arrivalTime;
    const int burstTime;
    int waitingTime;
    int runningTime;
    int priority;
    std::binary_semaphore startSignal;
    std::binary_semaphore stopSignal;
    bool terminated;
    mutable std::mutex memberMutex;
};

#endif