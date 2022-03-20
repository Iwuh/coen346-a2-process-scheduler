#ifndef CLOCK_H
#define CLOCK_H

#include <shared_mutex>

// Simulates the clock used by the OS when scheduling processes. Thread-safe, uses the singleton pattern to share one instance through getInstance().
// Singleton pattern based on: https://stackoverflow.com/a/1008289
class Clock
{
public:
    static constexpr unsigned int maxCycles = 4000U;
    static constexpr unsigned int cycleLength = 50U;
    static constexpr unsigned int pollingInterval = 10U;

    static Clock& getInstance();

public:
    Clock(const Clock& other) = delete;
    void operator=(const Clock& other) = delete;

    int getTime() const;
    void operator()();

private:
    Clock();
    mutable std::shared_mutex mutex;
    int time;
    bool started;
};

#endif
