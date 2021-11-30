#pragma once
#include <string>

class PieceInfo
    {
    public:

    uint8_t Color;
    uint8_t Type;
    PieceInfo(uint8_t, uint8_t);
    PieceInfo() noexcept;
    };

namespace Piece
    {
    uint8_t GetPiece(char);
    char GetType(uint8_t);
	char GetColor(uint8_t);
    char GetInitial(PieceInfo);
    uint8_t GetOpposite(uint8_t);
    }

inline uint8_t Piece::GetOpposite( uint8_t color )
    {
    return uint8_t(1 ^ color);
    }

enum PieceType :
    uint8_t
    {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King,
    None
    };

namespace PieceColor
    {
    const uint8_t White = 0;
    const uint8_t Black = 1;
    const uint8_t None = 2;
    }

class Position;

class Knight
{
public:
	static uint64_t GetAllTargets(uint64_t, Position &);
	static uint64_t GetKnightAttacks(uint64_t);
	static uint64_t TargetsFrom(uint8_t, uint8_t, Position &);
};

class Bishop
{
public:
	static uint64_t GetAllTargets(uint64_t, Position &);
	static uint64_t TargetsFrom(uint8_t, uint8_t, Position &);
};

class Rook
{
public:
	static uint64_t GetAllTargets(uint64_t, Position &);
	static uint64_t TargetsFrom(uint8_t, uint8_t, Position &);
};

class Queen
{
public:
	static uint64_t GetAllTargets(uint64_t, Position &);
	static uint64_t TargetsFrom(uint8_t, uint8_t, Position &);
};

class King
{
public:
	static uint64_t GetAllTargets(uint64_t, Position &);
	static uint64_t GetKingAttacks(uint64_t);
};