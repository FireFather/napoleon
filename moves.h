#pragma once
#include "magics.h"
#include "masks.h"
#include "square.h"

class Moves
    {
    public:
    static uint64_t PawnAttacks[2][64];      // color, square
    static uint64_t KingAttacks[64];         // square
    static uint64_t KnightAttacks[64];       // square
    static uint64_t PseudoRookAttacks[64];   // square
    static uint64_t PseudoBishopAttacks[64]; // square
    static uint64_t ObstructedTable[64][64]; // square, square
    static uint64_t KingProximity[2][64];    // color, square
    static uint64_t SideFiles[8];            // file
    static uint64_t FrontSpan[2][64];        // color, square
    static uint64_t PasserSpan[2][64];       // color, square
    static int Distance[64][64];             // square, square
    static uint64_t GetA1H8DiagonalAttacks(uint64_t, uint8_t);
    static uint64_t GetH1A8DiagonalAttacks(uint64_t, uint8_t);
    static bool AreSquareAligned(uint8_t, uint8_t, uint8_t);
    static void InitAttacks();

#ifdef PEXT
    static uint64_t RookAttacks[64][64 * 64]; // square, occupancy (12 bits)
    static uint64_t GetRookAttacks(uint64_t, uint8_t);
#else
    static uint64_t GetRankAttacks(uint64_t, uint8_t);
    static uint64_t GetFileAttacks(uint64_t, uint8_t);
#endif

    private:
    static uint64_t RankAttacks[64][64];         // square , occupancy
    static uint64_t FileAttacks[64][64];         // square , occupancy
    static uint64_t A1H8DiagonalAttacks[64][64]; // square , occupancy
    static uint64_t H1A8DiagonalAttacks[64][64]; // square , occupancy

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
INLINE uint64_t Moves::GetRookAttacks( uint64_t occupiedSquares, uint8_t square )
    {
    auto pext = _pext_u64(occupiedSquares, Moves::RookMask[square]);
    return Moves::RookAttacks[square][pext];
    }
#else
INLINE uint64_t Moves::GetRankAttacks( uint64_t occupiedSquares, uint8_t square )
    {
    int rank = Square::GetRankIndex(square);
    int occupancy = (int)((occupiedSquares & Masks::SixBitRankMask[rank]) >> (8 * rank));
    return RankAttacks[square][(occupancy >> 1) & 63];
    }

INLINE uint64_t Moves::GetFileAttacks( uint64_t occupiedSquares, uint8_t square )
    {
    int file = Square::GetFileIndex(square);
    int occupancy = (int)((occupiedSquares & Masks::SixBitFileMask[file]) * Magics::FileMagic[file] >> 56);
    return FileAttacks[square][(occupancy >> 1) & 63];
    }
#endif

INLINE uint64_t Moves::GetA1H8DiagonalAttacks( uint64_t occupiedSquares, uint8_t square )
    {
    int diag = Square::GetA1H8DiagonalIndex(square);

#ifdef PEXT
    auto occupancy = _pext_u64(occupiedSquares, Masks::SixBitA1H8DiagonalMask[diag]);
#else
    int occupancy = (int)((occupiedSquares & Masks::A1H8DiagonalMask[diag]) * Magics::A1H8DiagonalMagic[diag] >> 56);
#endif
    return A1H8DiagonalAttacks[square][(occupancy >> 1) & 63];
    }

INLINE uint64_t Moves::GetH1A8DiagonalAttacks( uint64_t occupiedSquares, uint8_t square )
    {
    int diag = Square::GetH1A8AntiDiagonalIndex(square);
#ifdef PEXT
    auto occupancy = _pext_u64(occupiedSquares, Masks::SixBitH1A8DiagonalMask[diag]);
#else
    int occupancy = (int)((occupiedSquares & Masks::H1A8DiagonalMask[diag]) * Magics::H1A8DiagonalMagic[diag] >> 56);
#endif
    return H1A8DiagonalAttacks[square][(occupancy >> 1) & 63];
    }

INLINE bool Moves::AreSquareAligned( uint8_t s1, uint8_t s2, uint8_t s3 )
    {
    return (ObstructedTable[s1][s2] | ObstructedTable[s1][s3] | ObstructedTable[s2][s3])
		& (Masks::SquareMask[s1] | Masks::SquareMask[s2] | Masks::SquareMask[s3]);
    }