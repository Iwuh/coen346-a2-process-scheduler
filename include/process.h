#ifndef PROCESS_H
#define PROCESS_H

#include <mutex>
#include <condition_variable>
#include <string>

class Process
{
public:
    enum State
    {
        Started,
        Paused,
        Resumed
    };

    Process(std::string name, int arrivalTime, int burstTime, int priority);
    Process(const Process& other);

    std::string getName() const;
    int getArrivalTime() const;
    int getBurstTime() const;
    int getWaitingTime() const;
    int getRunningTime() const;
    int getRemainingTime() const;
    bool isTerminated() const;

    int getPriority() const;
    void setPriority(int newPriority);

    State getState() const;
    void setState(State newState);
    // Tells the process to update itself based on its current state set by the scheduler.
    void update();

    bool operator<(const Process& other);
    bool operator>(const Process& other);
    bool operator==(const Process& other);
    void operator()();

private:
    const std::string name;
    const int arrivalTime;
    const int burstTime;
    int waitingTime;
    int runningTime;
    int priority;
    bool terminated;
    mutable std::mutex memberMutex;
    State state;
    mutable std::mutex stateMutex;
    std::condition_variable stateSignal;
};

#endif