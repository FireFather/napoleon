#pragma once
#include "defines.h"
#include "piece.h"

enum MoveType
    {
    kingCastle = 0x2,
    queenCastle = 0x3,
    EnPassant = 0x5,
    queenPromotion = 0xB,
    rookPromotion = 0xA,
    bishopPromotion = 0x9,
    knightPromotion = 0x8
    };

class Position;

class Move
    {
    public:
    Move() noexcept;
    Move(uint8_t, uint8_t);
    Move(uint8_t, uint8_t, uint8_t);

    uint8_t fromSquare()const;
    uint8_t toSquare()const;
    uint8_t piecePromoted()const;

    int butterflyIndex()const;

    bool isNull()const;
    bool isCastle()const;
    bool isCastleOO()const;
    bool isCastleOOO()const;
    bool isPromotion()const;
    bool isEnPassant()const;

    bool operator ==(const Move &) const;
    bool operator !=(const Move &) const;

    std::string toAlgebraic()const;

    private:
    unsigned short move;
    };

const Move nullMove(0, 0);

INLINE Move::Move() noexcept
    {
    }

INLINE Move::Move(uint8_t from, uint8_t to)
    {
    move = (from & 0x3f) | ((to & 0x3f) << 6);
    }

INLINE Move::Move(uint8_t from, uint8_t to, uint8_t flag)
    {
    move = (from & 0x3f) | ((to & 0x3f) << 6) | ((flag & 0xf) << 12);
    }

INLINE uint8_t Move::fromSquare() const
    {
    return move & 0x3f;
    }

INLINE uint8_t Move::toSquare() const
    {
    return (move >> 6) & 0x3f;
    }

INLINE int Move::butterflyIndex() const
    {
    return (move & 0xfff);
    }

INLINE uint8_t Move::piecePromoted() const
    {
    if (!isPromotion())
        return pieceType::noType;

    return ((move >> 12) & 0x3) + 1;
    }

INLINE bool Move::isNull() const
    {
    return (fromSquare() == toSquare());
    }

INLINE bool Move::isCastle() const
    {
    return (((move >> 12) == kingCastle) || ((move >> 12) == queenCastle));
    }

INLINE bool Move::isCastleOO() const
    {
    return ((move >> 12) == kingCastle);
    }

INLINE bool Move::isCastleOOO() const
    {
    return ((move >> 12) == queenCastle);
    }

INLINE bool Move::isPromotion() const
    {
    return ((move >> 12) & 0x8);
    }

INLINE bool Move::isEnPassant() const
    {
    return ((move >> 12) == EnPassant);
    }

INLINE bool Move::operator ==(const Move &other) const
    {
    return (move == other.move);
    }

INLINE bool Move::operator !=(const Move &other) const
    {
    return (move != other.move);
    }