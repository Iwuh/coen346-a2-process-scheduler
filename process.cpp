#include "include/process.h"

#include "include/clock.h"

Process::Process(std::string name, int arrivalTime, int burstTime, int priority)
    : name(name)
    , arrivalTime(arrivalTime)
    , burstTime(burstTime)
    , priority(priority)
    , waitingTime(0)
    , runningTime(0)
    , state(Process::State::Paused)
{
}

const std::string& Process::getName() const
{
    return name;
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

Process::State Process::getState() const
{
    std::lock_guard stateLock(stateMutex);
    return state;
}

std::string Process::getStateString() const
{
    switch (getState())
    {
        case State::Started:
            return "Started";
        case State::Paused:
            return "Paused";
        case State::Resumed:
            return "Resumed";
        default:
            return "Unknown";
    }
}

void Process::setState(Process::State newState)
{
    std::lock_guard stateLock(stateMutex);
    state = newState;
}

void Process::update()
{
    stateSignal.notify_one();
}

bool Process::operator<(const Process& other)
{
    // Lower priority integer => Higher actual priority
    return getPriority() > other.getPriority();
}

bool Process::operator>(const Process& other)
{
    return getPriority() < other.getPriority();
}

bool Process::operator==(const Process& other)
{
    return getPriority() == other.getPriority();
}

void Process::operator()()
{
    Clock& clock = Clock::getInstance();
    int lastBurstEndTime;
    while (true)
    {
        // The process thread isn't started until the process is given its first time slot, so there's no need to wait before running
        if (getState() == State::Started)
        {
            waitingTime = clock.getTime() - arrivalTime;
        }
        else
        {
            // Otherwise, we wait until the scheduler sets us to Resumed
            std::unique_lock stateLock(stateMutex);
            stateSignal.wait(stateLock, 
                [this]()
                {
                    return state != State::Paused;
                }
            );
            waitingTime += clock.getTime() - lastBurstEndTime;
            stateLock.unlock();
        }

        int lastTimeCheck, currentTimeCheck;
        lastTimeCheck = clock.getTime();
        while (getRemainingTime() > 0)
        {
            // First, check if the clock has incremented and increase the running time if so.
            if ((currentTimeCheck = clock.getTime()) > lastTimeCheck)
            {
                std::lock_guard memberLock(memberMutex);
                runningTime++;
                lastTimeCheck = currentTimeCheck;
            }

            // Wait up to one fifth of the clock interval for a signal from the scheduler.
            // If wait_for returns true, the predicate condition was met when the wait ended.
            // If wait_for returns false, the wait ended because of the timeout but the predicate was not met, so we should keep on running for now.
            std::unique_lock stateLock(stateMutex);
            if (stateSignal.wait_for(stateLock, std::chrono::milliseconds(Clock::pollingInterval),
                [this]()
                {
                    return state == State::Paused;
                }))
            {
                // Update the running time one last time as we exit just in case the scheduler has told us to stop but we've missed a clock increment.
                if ((currentTimeCheck = clock.getTime()) > lastTimeCheck)
                {
                    std::lock_guard memberLock(memberMutex);
                    runningTime++;
                }
                lastBurstEndTime = currentTimeCheck;
                stateLock.unlock();
                break;
            }
        }

        // End process execution if we're done our entire burst time. Otherwise, return to the top of the loop.
        if (getRemainingTime() <= 0)
        {
            return;
        }
    }
}