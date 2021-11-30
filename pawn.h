#pragma once
#include "direction.h"
#include "ranks.h"

class Position;

class Pawn
    {
    public:
    static uint64_t getAllTargets(uint64_t pawns, Position & position);
    static uint64_t getAnyAttack(uint64_t pawns, Position& position);
    static uint64_t getAnyAttack(uint64_t pawns, uint8_t color, uint64_t squares);
    static uint64_t getQuietTargets(uint8_t color, uint64_t pawns, uint64_t empty);

    private:
    static uint64_t getSinglePushTargets(uint8_t color, uint64_t pawns, uint64_t empty);
    static uint64_t getDoublePushTargets(uint8_t color, uint64_t pawns, uint64_t empty);
    static uint64_t getPawnsAbleToSinglePush(uint8_t color, uint64_t pawns, uint64_t empty);
    static uint64_t getPawnsAbleToDoublePush(uint8_t color, uint64_t pawns, uint64_t empty);
    static uint64_t getEastAttacks(uint8_t color, uint64_t pawns);
    static uint64_t getWestAttacks(uint8_t color, uint64_t pawns);
    };

INLINE uint64_t Pawn::getQuietTargets(uint8_t color, uint64_t pawns, uint64_t empty)
    {
    return getSinglePushTargets(color, pawns, empty) | getDoublePushTargets(color, pawns, empty);
    }

INLINE uint64_t Pawn::getSinglePushTargets(uint8_t color, uint64_t pawns, uint64_t empty)
    {
    return color == pieceColor::White ? Direction::oneStepNorth(pawns) & empty : Direction::oneStepSouth(pawns) & empty;
    }

INLINE uint64_t Pawn::getDoublePushTargets(uint8_t color, uint64_t pawns, uint64_t empty)
    {
    uint64_t singlePush = getSinglePushTargets(color, pawns, empty);

    return color == pieceColor::White
		? Direction::oneStepNorth(singlePush) & empty & Ranks::Four
        : Direction::oneStepSouth(singlePush) & empty & Ranks::Five;
    }