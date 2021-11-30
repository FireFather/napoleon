#pragma once

class Position;

class Benchmark
    {
    public:
    Benchmark(Position &);
    uint64_t Perft(int);

    private:
    Position &position;
    };