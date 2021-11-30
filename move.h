#pragma once
#include "defines.h"
#include "piece.h"

enum MoveType
    {
    KingCastle = 0x2,
    QueenCastle = 0x3,
    EnPassant = 0x5,
    QueenPromotion = 0xB,
    RookPromotion = 0xA,
    BishopPromotion = 0x9,
    KnightPromotion = 0x8
    };

class Position;

class Move
    {
    public:
    Move() noexcept;
    Move(uint8_t, uint8_t);
    Move(uint8_t, uint8_t, uint8_t);

    uint8_t FromSquare()const;
    uint8_t ToSquare()const;
    uint8_t PiecePromoted()const;

    int ButterflyIndex()const;
    bool IsNull()const;
    bool IsCastle()const;
    bool IsCastleOO()const;
    bool IsCastleOOO()const;
    bool IsPromotion()const;
    bool IsEnPassant()const;
    bool operator ==( const Move & ) const;
    bool operator !=( const Move & ) const;
    std::string ToAlgebraic()const;
    private:
    unsigned short move;
    };

const Move NullMove( 0, 0 );

INLINE Move::Move() noexcept
    {
    }

INLINE Move::Move( uint8_t from, uint8_t to )
    {
    move = (from & 0x3f) | ((to & 0x3f) << 6);
    }

INLINE Move::Move( uint8_t from, uint8_t to, uint8_t flag )
    {
    move = (from & 0x3f) | ((to & 0x3f) << 6) | ((flag & 0xf) << 12);
    }

INLINE uint8_t Move::FromSquare() const
    {
    return move & 0x3f;
    }

INLINE uint8_t Move::ToSquare() const
    {
    return (move >> 6) & 0x3f;
    }

INLINE int Move::ButterflyIndex() const       
    {
    return (move & 0xfff);
    }

INLINE uint8_t Move::PiecePromoted() const
    {
    if (!IsPromotion())
        return PieceType::None;

    return ((move >> 12) & 0x3) + 1;
    }

INLINE bool Move::IsEnPassant() const
    {
    return ((move >> 12) == EnPassant);     
    }

INLINE bool Move::IsCastle() const
    {
    return (((move >> 12) == KingCastle) || ((move >> 12) == QueenCastle));
    }

INLINE bool Move::IsPromotion() const
    {
    return ((move >> 12) & 0x8);
    }

INLINE bool Move::operator ==( const Move &other ) const
    {
    return (move == other.move);
    }

INLINE bool Move::operator !=( const Move &other ) const
    {
    return (move != other.move);
    }