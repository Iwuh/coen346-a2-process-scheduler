#include "include/process.h"

Process::Process(int arrivalTime, int burstTime)
    : arrivalTime(arrivalTime)
    , burstTime(burstTime)
    , waitingTime(0)
    , runningTime(0)
    , startSignal(0)
    , stopSignal(0)
{
}

int Process::getArrivalTime() const
{
    return arrivalTime;
}

int Process::getBurstTime() const
{
    return burstTime;
}

int Process::getWaitingTime() const
{
    std::lock_guard(memberMutex);
    return waitingTime;
}

int Process::getRunningTime() const
{
    std::lock_guard(memberMutex);
    return runningTime;
}

int Process::getRemainingTime() const
{
    std::lock_guard(memberMutex);
    return burstTime - runningTime;
}

bool Process::isTerminated() const
{
    std::lock_guard(memberMutex);
    return terminated;
}

std::binary_semaphore& Process::getStartSignal()
{
    return startSignal;
}

std::binary_semaphore& Process::getStopSignal()
{
    return stopSignal;
}

void Process::operator()()
{
    while (getRemainingTime() > 0)
    {
        // Wait until the scheduler tells us to resume
        startSignal.acquire();
        startSignal.release();

        // TODO: Update the waiting time based on the clock

        // TODO: Update running time and check if stop signal is available, set terminated and return if remaining time = 0

    }
}