#include "position.h"
#include "ranks.h"

uint64_t Pawn::GetAllTargets( uint64_t pawns, Position &position )
    {
    uint64_t empty = position.EmptySquares;
    return GetQuietTargets(position.SideToMove(), pawns, empty) | GetAnyAttack(pawns, position);
    }

uint64_t Pawn::GetAnyAttack( uint64_t pawns, Position &position )
    {
    return (GetEastAttacks(position.SideToMove(), pawns) | GetWestAttacks(position.SideToMove(), pawns)) & position.EnemyPieces();
    }

uint64_t Pawn::GetAnyAttack( uint64_t pawns, uint8_t color, uint64_t squares )
    {
    return (GetEastAttacks(color, pawns) | GetWestAttacks(color, pawns)) & squares;
    }

uint64_t Pawn::GetPawnsAbleToSinglePush( uint8_t color, uint64_t pawns, uint64_t empty )
    {
    switch( color )
        {
        case PieceColor::White:
            return Direction::OneStepSouth(empty) & pawns;
        case PieceColor::Black:
            return Direction::OneStepNorth(empty) & pawns;
        default:
            throw std::exception();
        }
    }

uint64_t Pawn::GetPawnsAbleToDoublePush( uint8_t color, uint64_t pawns, uint64_t empty )
    {
    switch( color )
        {
        case PieceColor::White:
            {
            uint64_t emptyRank3 = Direction::OneStepSouth(empty & Ranks::Four) & empty;
            return GetPawnsAbleToSinglePush(color, pawns, emptyRank3);
            }
        case PieceColor::Black:
            {
            uint64_t emptyRank6 = Direction::OneStepNorth(empty & Ranks::Six) & empty;
            return GetPawnsAbleToSinglePush(color, pawns, emptyRank6);
            }
        default:
            throw std::exception();
        }
    }

uint64_t Pawn::GetEastAttacks( uint8_t color, uint64_t pawns )
    {
    return color == PieceColor::White ? Direction::OneStepNorthEast(pawns) : Direction::OneStepSouthEast(pawns);
    }

uint64_t Pawn::GetWestAttacks( uint8_t color, uint64_t pawns )
    {
    return color == PieceColor::White ? Direction::OneStepNorthWest(pawns) : Direction::OneStepSouthWest(pawns);
    }