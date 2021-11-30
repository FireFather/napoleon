#pragma once
#include "magics.h"
#include "masks.h"
#include "square.h"

class Moves
    {
    public:
    static uint64_t pawnAttacks[2][64];      // color, square
    static uint64_t kingAttacks[64];         // square
    static uint64_t knightAttacks[64];       // square
    static uint64_t pseudoRookAttacks[64];   // square
    static uint64_t pseudoBishopAttacks[64]; // square
    static uint64_t obstructedTable[64][64]; // square, square
    static uint64_t kingProximity[2][64];    // color, square
    static uint64_t adjacentFiles[8];		 // file
    static uint64_t frontSpan[2][64];        // color, square
    static uint64_t passerSpan[2][64];       // color, square
    static int Distance[64][64];             // square, square
    static uint64_t getA1H8DiagonalAttacks(uint64_t, uint8_t);
    static uint64_t getH1A8DiagonalAttacks(uint64_t, uint8_t);
    static bool AreSquaresAligned(uint8_t, uint8_t, uint8_t);
    static void initAttacks();

#ifdef PEXT
    static uint64_t RookAttacks[64][64 * 64]; // square, occupancy (12 bits)
    static uint64_t getRookAttacks(uint64_t, uint8_t);
#else
    static uint64_t getRankAttacks(uint64_t, uint8_t);
    static uint64_t getFileAttacks(uint64_t, uint8_t);
#endif

    private:
    static uint64_t rankAttacks[64][64];         // square , occupancy
    static uint64_t fileAttacks[64][64];         // square , occupancy
    static uint64_t A1H8diagonalAttacks[64][64]; // square , occupancy
    static uint64_t H1A8diagonalAttacks[64][64]; // square , occupancy

    static void initPawnAttacks();
    static void initKnightAttacks();
    static void initKingAttacks();
    static void initRankAttacks();
    static void initFileAttacks();
    static void initDiagonalAttacks();
    static void initAntiDiagonalAttacks();
    static void initPseudoAttacks();
    static void initObstructedTable();

#ifdef PEXT
    static uint64_t RookMask[64];
    static void initRookAttacks();
#endif
    };

#ifdef PEXT
INLINE uint64_t Moves::getRookAttacks(uint64_t occupiedSquares, uint8_t square)
    {
    auto pext = _pext_u64(occupiedSquares, Moves::RookMask[square]);
    return Moves::RookAttacks[square][pext];
    }
#else
INLINE uint64_t Moves::getRankAttacks(uint64_t occupiedSquares, uint8_t square)
    {
    int rank = Square::getRankIndex(square);
    int occupancy = (int)((occupiedSquares & Masks::sixBitRankMask[rank]) >> (8 * rank));
    return rankAttacks[square][(occupancy >> 1) & 63];
    }

INLINE uint64_t Moves::getFileAttacks(uint64_t occupiedSquares, uint8_t square)
    {
    int file = Square::getFileIndex(square);
    int occupancy = (int)((occupiedSquares & Masks::sixBitFileMask[file]) * Magics::fileMagic[file] >> 56);
    return fileAttacks[square][(occupancy >> 1) & 63];
    }
#endif

INLINE uint64_t Moves::getA1H8DiagonalAttacks(uint64_t occupiedSquares, uint8_t square)
    {
    int diag = Square::getA1H8DiagonalIndex(square);

#ifdef PEXT
    auto occupancy = _pext_u64(occupiedSquares, Masks::sixBitA1H8diagMask[diag]);
#else
    int occupancy = (int)((occupiedSquares & Masks::A1H8diagMask[diag]) * Magics::A1H8diagMagic[diag] >> 56);
#endif
    return A1H8diagonalAttacks[square][(occupancy >> 1) & 63];
    }

INLINE uint64_t Moves::getH1A8DiagonalAttacks(uint64_t occupiedSquares, uint8_t square)
    {
    int diag = Square::getH1A8AntiDiagonalIndex(square);
#ifdef PEXT
    auto occupancy = _pext_u64(occupiedSquares, Masks::sixBitH1A8DdiagMask[diag]);
#else
    int occupancy = (int)((occupiedSquares & Masks::H1A8diagMask[diag]) * Magics::H1A8diagMagic[diag] >> 56);
#endif
    return H1A8diagonalAttacks[square][(occupancy >> 1) & 63];
    }

INLINE bool Moves::AreSquaresAligned(uint8_t s1, uint8_t s2, uint8_t s3)
    {
    return (obstructedTable[s1][s2] | obstructedTable[s1][s3] | obstructedTable[s2][s3])
		& (Masks::squareMask[s1] | Masks::squareMask[s2] | Masks::squareMask[s3]);
    }