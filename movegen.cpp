#include "movegen.h"
#include "piece.h"

void MoveGen::GetQueenMoves( uint64_t queens, Position &position, Move moveList [], int &pos, uint64_t target )
    {
    uint64_t targets;
    uint8_t fromIndex;
    uint8_t toIndex;

    while (queens != 0)
        {
        fromIndex = BitScanForwardReset(queens);        
        targets = Queen::GetAllTargets(Masks::SquareMask[fromIndex], position) & target;

        while (targets != 0)
            {
            toIndex = BitScanForwardReset(targets);        
            moveList[pos++] = Move(fromIndex, toIndex);
            }
        }
    }