#pragma once
#include "direction.h"
#include "ranks.h"

class Position;

class Pawn
    {
    public:
    static uint64_t GetAllTargets(uint64_t pawns, Position & position);
    INLINE static uint64_t GetAnyAttack(uint64_t pawns, Position& position);
    static uint64_t GetAnyAttack(uint64_t pawns, uint8_t color, uint64_t squares);
    static uint64_t GetQuietTargets(uint8_t color, uint64_t pawns, uint64_t empty);

    private:
    static uint64_t GetSinglePushTargets(uint8_t color, uint64_t pawns, uint64_t empty);
    static uint64_t GetDoublePushTargets(uint8_t color, uint64_t pawns, uint64_t empty);
    INLINE static uint64_t GetPawnsAbleToSinglePush(uint8_t color, uint64_t pawns, uint64_t empty);
    INLINE static uint64_t GetPawnsAbleToDoublePush(uint8_t color, uint64_t pawns, uint64_t empty);
    INLINE static uint64_t GetEastAttacks(uint8_t color, uint64_t pawns);
    INLINE static uint64_t GetWestAttacks(uint8_t color, uint64_t pawns);
    };

INLINE uint64_t Pawn::GetQuietTargets( uint8_t color, uint64_t pawns, uint64_t empty )
    {
    return GetSinglePushTargets(color, pawns, empty) | GetDoublePushTargets(color, pawns, empty);
    }

INLINE uint64_t Pawn::GetSinglePushTargets( uint8_t color, uint64_t pawns, uint64_t empty )
    {
    return color == PieceColor::White ? Direction::OneStepNorth(pawns) & empty : Direction::OneStepSouth(pawns) & empty;
    }

INLINE uint64_t Pawn::GetDoublePushTargets( uint8_t color, uint64_t pawns, uint64_t empty )
    {
    uint64_t singlePush = GetSinglePushTargets(color, pawns, empty);

    return color == PieceColor::White
		? Direction::OneStepNorth(singlePush) & empty & Ranks::Four
        : Direction::OneStepSouth(singlePush) & empty & Ranks::Five;
    }