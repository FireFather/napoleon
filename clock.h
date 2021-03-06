#pragma once
#include <chrono>

using namespace std::chrono;
typedef std::chrono::milliseconds MS;
typedef std::chrono::steady_clock t_clock;

class Clock
    {
    public:
    Clock() noexcept;
	void Restart();
	double elapsedMilliseconds();
	static Clock startNow();

    private:
    t_clock::time_point begin;
    t_clock::time_point end;
    };