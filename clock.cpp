#include "include/clock.h"

#include <exception>
#include <thread>
#include <mutex>
#include <iostream>

Clock& Clock::getInstance()
{
    // The static instance persists between calls, so the first getInstance() call will create the instance and future calls will return the same instance.
    static Clock instance;
    return instance;
}

Clock::Clock()
    : time(0), started(false)
{
}

int Clock::getTime() const
{
    // Get a shared lock allowing any number of readers, as long as nobody is writing
    std::shared_lock readerLock(mutex);
    return time;
}

void Clock::operator()()
{
    if (started) throw std::runtime_error("Clock has already been started");

    started = true;
    for (unsigned int count = 0; count < maxCycles; count++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(cycleLength));
        // Get a unique lock that blocks all readers while the increment happens
        std::unique_lock writerLock(mutex);
        time++;
        std::cout << time << std::endl;
    }
}
