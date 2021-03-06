#include "movegen.h"
#include "piece.h"

void moveGen::getQueenMoves(uint64_t queens, Position &position, Move moveList [], int &pos, uint64_t target)
    {
    uint64_t targets;
    uint8_t fromIndex;
    uint8_t toIndex;

    while (queens != 0)
        {
        fromIndex = bitScanForwardReset(queens);        
        targets = Queen::getAllTargets(Masks::squareMask[fromIndex], position) & target;

        while (targets != 0)
            {
            toIndex = bitScanForwardReset(targets);        
            moveList[pos++] = Move(fromIndex, toIndex);
            }
        }
    }