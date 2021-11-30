#pragma once

class Position;

class Benchmark
    {
    public:
    Benchmark(Position &);
	void runPerft(int);
    uint64_t Perft(int);
	void runDivide(int);
	uint64_t Divide(int);
	uint64_t Loop(int);
	void perftTest(void);
	void ttdTest(int);

    private:
    Position &position;
    };
