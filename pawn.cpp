#include "position.h"
#include "ranks.h"

uint64_t Pawn::getAllTargets(uint64_t pawns, Position &position)
    {
    uint64_t empty = position.emptySquares;
    return getQuietTargets(position.getSideToMove(), pawns, empty) | getAnyAttack(pawns, position);
    }

uint64_t Pawn::getAnyAttack(uint64_t pawns, Position &position)
    {
    return (getEastAttacks(position.getSideToMove(), pawns) | getWestAttacks(position.getSideToMove(), pawns)) & position.enemyPieces();
    }

uint64_t Pawn::getAnyAttack(uint64_t pawns, uint8_t color, uint64_t squares)
    {
    return (getEastAttacks(color, pawns) | getWestAttacks(color, pawns)) & squares;
    }

uint64_t Pawn::getPawnsAbleToSinglePush(uint8_t color, uint64_t pawns, uint64_t empty)
    {
    switch(color)
        {
        case pieceColor::White:
            return Direction::oneStepSouth(empty) & pawns;
        case pieceColor::Black:
            return Direction::oneStepNorth(empty) & pawns;
        default:
            throw std::exception();
        }
    }

uint64_t Pawn::getPawnsAbleToDoublePush(uint8_t color, uint64_t pawns, uint64_t empty)
    {
    switch(color)
        {
        case pieceColor::White:
            {
            uint64_t emptyRank3 = Direction::oneStepSouth(empty & Ranks::Four) & empty;
            return getPawnsAbleToSinglePush(color, pawns, emptyRank3);
            }
        case pieceColor::Black:
            {
            uint64_t emptyRank6 = Direction::oneStepNorth(empty & Ranks::Six) & empty;
            return getPawnsAbleToSinglePush(color, pawns, emptyRank6);
            }
        default:
            throw std::exception();
        }
    }

uint64_t Pawn::getEastAttacks(uint8_t color, uint64_t pawns)
    {
    return color == pieceColor::White ? Direction::oneStepNorthEast(pawns) : Direction::oneStepSouthEast(pawns);
    }

uint64_t Pawn::getWestAttacks(uint8_t color, uint64_t pawns)
    {
    return color == pieceColor::White ? Direction::oneStepNorthWest(pawns) : Direction::oneStepSouthWest(pawns);
    }