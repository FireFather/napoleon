#pragma once
#include "defines.h"
#include "position.h"
#include "piece.h"
#include "pst.h"

class pieceInfo;

namespace Eval
    {
    int evaluate(Position &);
	Score pieceSquareScore(pieceInfo, uint8_t);
	void updateScore(Score &, int, int);
    }

INLINE void Eval::updateScore(Score &scores, int openingBonus, int endBonus)
    {
    scores.first += openingBonus;
    scores.second += endBonus;
    }

inline Score Eval::pieceSquareScore(pieceInfo piece, uint8_t square)
    {
    square = piece.Color == pieceColor::White ? square : Square::mirrorSquare(square);
    return std::make_pair(PST[piece.Type][OP][square], PST[piece.Type][EG][square]);
    }