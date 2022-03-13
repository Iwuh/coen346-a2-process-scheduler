#include "include/process.h"

#include "include/clock.h"

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
    std::lock_guard memberLock(memberMutex);
    return waitingTime;
}

int Process::getRunningTime() const
{
    std::lock_guard memberLock(memberMutex);
    return runningTime;
}

int Process::getRemainingTime() const
{
    std::lock_guard memberLock(memberMutex);
    return burstTime - runningTime;
}

bool Process::isTerminated() const
{
    std::lock_guard memberLock(memberMutex);
    return terminated;
}

int Process::getPriority() const
{
    std::lock_guard memberLock(memberMutex);
    return priority;
}

void Process::setPriority(int newPriority)
{
    std::lock_guard memberLock(memberMutex);
    priority = newPriority;
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
    Clock& clock = Clock::getInstance();
    int previousSlotEndTime = arrivalTime;
    while (getRemainingTime() > 0)
    {
        // Wait until the scheduler tells us to resume
        startSignal.acquire();
        startSignal.release();

        // Update the total time spend waiting
        std::unique_lock memberLock(memberMutex);
        waitingTime += clock.getTime() - previousSlotEndTime;
        memberLock.unlock();

        while (true)
        {
            // We need to track the running time of the process, so we hold the previous time check and the current time check to know when the clock has incremented
            int lastTimeCheck, currentTimeCheck;
            lastTimeCheck = clock.getTime();
            if ((currentTimeCheck = clock.getTime()) > lastTimeCheck)
            {
                memberLock.lock();
                runningTime++;
                memberLock.unlock();
                lastTimeCheck = currentTimeCheck;
            }

            if (getRemainingTime() <= 0)
            {
                memberLock.lock();
                terminated = true;
                memberLock.unlock();
                return;
            }
            else if (stopSignal.try_acquire())
            {
                // If the semaphore is acquired, it means the scheduler has told us to stop execution
                stopSignal.release();
                previousSlotEndTime = currentTimeCheck;
                break;
            }
            else
            {
                // Otherwise, we're still running and the clock increments every 250ms, so wait 125ms before the next iteration of the loop.
                std::this_thread::sleep_for(std::chrono::milliseconds(125));
            }
        }
    }
    // Just in case we don't catch the process terminating inside the loop
    std::lock_guard memberLock(memberMutex);
    terminated = true;
}