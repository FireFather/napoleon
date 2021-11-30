#pragma once
#include "move.h"

enum class ScoreType : uint8_t { Exact, Alpha, Beta };

class HashEntry
	{
    public:
	uint64_t Key;
	uint8_t Depth;
	ScoreType Bound;
	Move BestMove;
	int Score;
	HashEntry() noexcept;
	HashEntry(uint64_t, uint8_t, int, Move, ScoreType);
    };


