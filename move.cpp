#include <cctype>
#include "move.h"
#include "position.h"

std::string Move::toAlgebraic() const
    {
    std::string algebraic;

    if (isNull())
        return "(none)";      

    if (isCastle())
        {
        if (isCastleOO())
            {
            if (fromSquare() == Square::E1)
                algebraic += "e1g1";
            else
                algebraic += "e8g8";
            }
        else if (isCastleOOO())
            {
            if (fromSquare() == Square::E1)
                algebraic += "e1c1";
            else
                algebraic += "e8c8";
            }
        }
    else
        {
        algebraic += Square::toAlgebraic(fromSquare());
        algebraic += Square::toAlgebraic(toSquare());

        if (isPromotion())
            algebraic += Piece::getType(piecePromoted());
        }
    return algebraic;
    }
