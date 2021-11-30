#include "clock.h"

Clock::Clock() noexcept :
    begin(t_clock::now())
    {
    }

void Clock::Restart()
    {
    begin = t_clock::now();
    }

double Clock::ElapsedMilliseconds()
    {
    return double(duration_cast<MS>(t_clock::now() - begin).count());
    }

double Clock::ElapsedSeconds()
    {
    return double(duration_cast<std::chrono::seconds>(t_clock::now() - begin).count());
    }

Clock Clock::StartNew()
    {
    Clock watch;
    watch.Restart();
    return watch;
    }