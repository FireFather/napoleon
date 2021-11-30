#pragma once
#include "defines.h"
#include <string>

class pieceInfo
	{
public:

	uint8_t Color;
	uint8_t Type;
	pieceInfo(uint8_t, uint8_t);
	pieceInfo() noexcept;
	};

namespace Piece
    {
    uint8_t getPiece(char);
    char getType(uint8_t);
    char getColor(uint8_t);
    char getInitial(pieceInfo);
    uint8_t getOpposite(uint8_t);
    }

INLINE uint8_t Piece::getOpposite(uint8_t color)
    {
    return uint8_t(1 ^ color);
    }

enum pieceType :
    uint8_t
    {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    noType
    };

enum pieceColor :
	uint8_t
	{
	White,
	Black,
	noColor
	};

class Position;

class Knight
    {
    public:
    static uint64_t getAllTargets(uint64_t, Position &);
    static uint64_t getKnightAttacks(uint64_t);
    static uint64_t targetsFrom(uint8_t, uint8_t, Position &);
    };

class Bishop
    {
    public:
    static uint64_t getAllTargets(uint64_t, Position &);
    static uint64_t targetsFrom(uint8_t, uint8_t, Position &);
    };

class Rook
    {
    public:
    static uint64_t getAllTargets(uint64_t, Position &);
    static uint64_t targetsFrom(uint8_t, uint8_t, Position &);
    };

class Queen
    {
    public:
    static uint64_t getAllTargets(uint64_t, Position &);
    static uint64_t targetsFrom(uint8_t, uint8_t, Position &);
    };

class King
    {
    public:
    static uint64_t getAllTargets(uint64_t, Position &);
    static uint64_t getKingAttacks(uint64_t);
    };