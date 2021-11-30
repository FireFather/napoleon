#pragma once
#include <algorithm>
#include "move.h"

namespace Square
    {
    int getA1H8DiagonalIndex(int, int);
    int getA1H8DiagonalIndex(int);
    int getH1A8AntiDiagonalIndex(int, int);
    int getH1A8AntiDiagonalIndex(int);
    int getFileIndex(int);
    int getRankIndex(int);
    int getSquareIndex(int, int);
    int mirrorSquare(int);
    int Distance(int, int);
    int Parse(std::string);
    std::string toAlgebraic(uint8_t);

	enum Square : uint8_t
		{
		A1, B1, C1, D1, E1, F1, G1, H1,
		A2, B2, C2, D2, E2, F2, G2, H2,
		A3, B3, C3, D3, E3, F3, G3, H3,
		A4, B4, C4, D4, E4, F4, G4, H4,
		A5, B5, C5, D5, E5, F5, G5, H5,
		A6, B6, C6, D6, E6, F6, G6, H6,
		A7, B7, C7, D7, E7, F7, G7, H7,
		A8, B8, C8, D8, E8, F8, G8, H8,
		noSquare
		};
	}

inline int Square::getA1H8DiagonalIndex(int file, int rank)
    {
    return 7 + rank - file;
    }

inline int Square::getA1H8DiagonalIndex(int squareIndex)
    {
    return 7 + getRankIndex(squareIndex) - getFileIndex(squareIndex);
    }

inline int Square::getH1A8AntiDiagonalIndex(int file, int rank)
    {
    return rank + file;
    }

inline int Square::getH1A8AntiDiagonalIndex(int squareIndex)
    {
    return getRankIndex(squareIndex) + getFileIndex(squareIndex);
    }

inline int Square::getFileIndex(int squareIndex)
    {
    return squareIndex & 7;
    }

inline int Square::getRankIndex(int squareIndex)
    {
    return squareIndex >> 3;
    }

inline int Square::getSquareIndex(int file, int rank)
    {
    return file + 8*rank;
    }

inline int Square::mirrorSquare(int square)
    {
    return square ^ 56;
    }

inline int Square::Distance(int sq1, int sq2)
    {
    int f1, f2, r1, r2;
    f1 = getFileIndex(sq1);
    f2 = getFileIndex(sq2);
    r1 = getRankIndex(sq1);
    r2 = getRankIndex(sq2);
    return std::max(std::abs(r1 - r2), std::abs(f1 - f2));
    }