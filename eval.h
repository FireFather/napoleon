#pragma once
#include "defines.h"
#include "position.h"
#include "piece.h"
#include "pst.h"

class PieceInfo;

namespace Eval
    {
    int Evaluate(Position &);
    int EvaluatePiece(PieceInfo, uint8_t, uint64_t, Position &);
    Score PieceSquareValue(PieceInfo, uint8_t);
    int interpolate(Score, int);
    inline void updateScore(Score &, int, int);
    inline void updateScore(Score &, int);
    }

INLINE void Eval::updateScore(Score &scores, int openingBonus, int endBonus )
    {
    scores.first += openingBonus;
    scores.second += endBonus;
    }

INLINE void Eval::updateScore(Score &scores, int openingBonus )
    {
    scores.first += openingBonus;
    scores.second += openingBonus;
    }

inline int Eval::interpolate( Score score, int phase )
    {
    return ((score.first * (MaxPhase - phase)) + (score.second * phase)) / MaxPhase;
    }

static int flip[64] =
{
	56,  57,  58,  59,  60,  61,  62,  63,
	48,  49,  50,  51,  52,  53,  54,  55,
	40,  41,  42,  43,  44,  45,  46,  47,
	32,  33,  34,  35,  36,  37,  38,  39,
	24,  25,  26,  27,  28,  29,  30,  31,
	16,  17,  18,  19,  20,  21,  22,  23,
	8,   9,  10,  11,  12,  13,  14,  15,
	0,   1,   2,   3,   4,   5,   6,   7
};

inline Score Eval::PieceSquareValue( PieceInfo piece, uint8_t square )
    {
    square = piece.Color == PieceColor::White ? square : Square::MirrorSquare(square);
    return std::make_pair(PST[piece.Type][Opening][flip[square]], PST[piece.Type][EndGame][flip[square]]);
    }