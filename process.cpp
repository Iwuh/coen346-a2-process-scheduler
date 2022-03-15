#include "include/process.h"

#include "include/clock.h"

Process::Process(std::string name, int arrivalTime, int burstTime, int priority)
    : name(name)
    , arrivalTime(arrivalTime)
    , burstTime(burstTime)
    , priority(priority)
    , waitingTime(0)
    , runningTime(0)
    , terminated(false)
    , state(Process::State::Paused)
{
}

Process::Process(const Process& other)
    : name(other.name)
    , arrivalTime(other.arrivalTime)
    , burstTime(other.burstTime)
    , priority(other.priority)
    , waitingTime(other.waitingTime)
    , runningTime(other.runningTime)
    , terminated(other.terminated)
    , state(other.state)
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

Process::State Process::getState() const
{
    std::lock_guard stateLock(stateMutex);
    return state;
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
        // Block until the state is no longer paused.
        std::unique_lock stateLock(stateMutex);
        stateSignal.wait(stateLock, 
            [this](Process& p)
            {
                return p.getState() != State::Paused;
            }
        );

        // We own the state lock once the condition variable finishes waiting.
        if (state == State::Started)
        {
            waitingTime = clock.getTime() - arrivalTime;
        }
        else // Resumed
        {
            waitingTime += clock.getTime() - lastBurstEndTime;
        }

        // Release the state lock so that the scheduler can signal us when to stop.
        stateLock.unlock();

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

            // Wait up to 50 milliseconds for a signal from the scheduler.
            // If wait_for returns true, the predicate condition was met when the wait ended.
            // If wait_for returns false, the wait ended because of the timeout but the predicate was not met, so we should keep on running for now.
            if (stateSignal.wait_for(stateLock, std::chrono::milliseconds(50),
                [this](Process& p)
                {
                    return p.getState() == State::Paused;
                }))
            {
                lastBurstEndTime = currentTimeCheck;
                stateLock.unlock();
                break;
            }
        }

        // Set the terminated flag and end process execution if we're done our entire burst time. Otherwise, return to the top of the loop.
        if (getRemainingTime() <= 0)
        {
            std::lock_guard memberLock(memberMutex);
            terminated = true;
            return;
        }
    }
}