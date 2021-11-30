#include <cctype>
#include "move.h"
#include "position.h"

bool Move::IsNull() const
    {
    return (FromSquare() == ToSquare());
    }

bool Move::IsCastleOO() const
    {
    return ((move >> 12) == KingCastle);
    }

bool Move::IsCastleOOO() const
    {
    return ((move >> 12) == QueenCastle);
    }

std::string Move::ToAlgebraic() const
    {
    std::string algebraic;

    if (IsNull())
        return "(none)";      

    if (IsCastle())
        {
        if (IsCastleOO())
            {
            if (FromSquare() == Square::E1)
                algebraic += "e1g1";
            else
                algebraic += "e8g8";
            }
        else if (IsCastleOOO())
            {
            if (FromSquare() == Square::E1)
                algebraic += "e1c1";
            else
                algebraic += "e8c8";
            }
        }
    else
        {
        algebraic += Square::ToAlgebraic(FromSquare());
        algebraic += Square::ToAlgebraic(ToSquare());

        if (IsPromotion())
            algebraic += Piece::GetType(PiecePromoted());
        }
    return algebraic;
    }
