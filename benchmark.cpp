#include "defines.h"
#include "benchmark.h"
#include "movegen.h"
#include "move.h"
#include "moves.h"

Benchmark::Benchmark( Position &position ) :
    position(position)
    {
    }

uint64_t Benchmark::Perft( int depth )
    {
    int pos = 0;
    Move moves[MoveGen::MaxMoves];

    uint64_t nodes = 0;

    MoveGen::GetLegalMoves(moves, pos, position);

    if (depth == 1)
        {
        return pos;
        }

    if (depth == 0)
        return 1;

    for ( int i = 0; i < pos; i++ )
        {
        position.MakeMove(moves[i]);
        nodes += Perft(depth - 1);
        position.UndoMove(moves[i]);
        }

    return nodes;
    }