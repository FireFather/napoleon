#pragma once
#include "move.h"

namespace Castle
    {
    const uint8_t WhiteCastleOO = 0x1;
    const uint8_t WhiteCastleOOO = 0x2;
    const uint8_t BlackCastleOO = 0x4;
    const uint8_t BlackCastleOOO = 0x8;
    const uint8_t FullCastlingRights = 0xF;

    const uint64_t WhiteCastleMaskOO = 0x0000000000000060;
    const uint64_t WhiteCastleMaskOOO = 0x000000000000000E;
    const uint64_t BlackCastleMaskOO = 0x6000000000000000;
    const uint64_t BlackCastleMaskOOO = 0x0E00000000000000;

    const uint64_t WhiteKingShield = 0xe000;    
    const uint64_t WhiteQueenShield = 0x700;    
    const uint64_t BlackKingShield = 0xe0000000000000;
    const uint64_t BlackQueenShield = 0x7000000000000;

    const uint64_t WhiteKingSide = 0xe0;
    const uint64_t WhiteQueenSide = 0x7;
    const uint64_t BlackKingSide = 0xe000000000000000;
    const uint64_t BlackQueenSide = 0x700000000000000;

    const Move WhiteCastlingOO(Square::E1, Square::G1, KingCastle);
    const Move WhiteCastlingOOO(Square::E1, Square::C1, QueenCastle);

    const Move BlackCastlingOO(Square::E8, Square::G8, KingCastle);
    const Move BlackCastlingOOO(Square::E8, Square::C8, QueenCastle);
    }