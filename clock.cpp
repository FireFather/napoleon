#include "clock.h"

Clock::Clock() noexcept :
    begin(t_clock::now())
    {
    }

void Clock::Restart()
    {
    begin = t_clock::now();
    }

double Clock::elapsedMilliseconds()
    {
    return double(duration_cast<MS>(t_clock::now() - begin).count());
    }

Clock Clock::startNow()
    {
    Clock watch;
    watch.Restart();
    return watch;
    }