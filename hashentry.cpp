#include "hashentry.h"

HashEntry::HashEntry() noexcept
    {
    }

HashEntry::HashEntry( uint64_t hash, uint8_t depth, int score, Move bestMove, ScoreType bound )
    {
    Key = hash;
    Depth = depth;
    Score = score;
    Bound = bound;
    BestMove = bestMove;
    }