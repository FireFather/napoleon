#pragma once
#include <string>
#include <vector>

using namespace std;
typedef std::pair<int, int> Score;

//#define PEXT

#pragma warning(disable : 6326) //Potential comparison of a constant with another constant
#pragma warning(disable : 6386) //Buffer overrun while writing to 'this->kingSquare'

#ifdef __GNUC__
#define INLINE __inline __attribute__ ((__always_inline__))
#elif defined(_MSC_VER) && defined(_WIN64)
#ifdef PEXT
#include <immintrin.h>
#endif
#include <intrin.h>
#define INLINE __forceinline
#else
#define INLINE inline
#include <intrin.h>
#pragma warning(disable : 4146)
#endif

#if defined(__GNUC__) && defined(__LP64__)
INLINE int BitScanForward( uint64_t bitBoard )
    {
    __asm__ ("bsfq %0, %0": "=r"(bitBoard) : "0" (bitBoard));
    return bitBoard;
    }

INLINE int BitScanForwardReset( uint64_t &bitBoard )
    {
    uint64_t index;
    __asm__ ("bsfq %1, %0": "=r"(index) : "rm"(bitBoard));
    bitBoard &= (bitBoard - 1);
    return index;
    }
#elif defined(__GNUC__)
INLINE int BitScanForward( uint64_t bitBoard )
    {
    return __builtin_ctzll(bitBoard);
    }

INLINE int BitScanForwardReset( uint64_t &bitBoard )
    {
    uint64_t bb = bitBoard;
    bitBoard &= (bitBoard - 1);
    return __builtin_ctzll(bb);
    }
#elif defined(_MSC_VER) && defined(_WIN64)
INLINE int BitScanForward( uint64_t bitBoard )
    {
    unsigned long index;
    _BitScanForward64(&index, bitBoard);
    return (int)index;
    }
INLINE int BitScanForwardReset( uint64_t &bitBoard )
    {
    unsigned long index;
    _BitScanForward64(&index, bitBoard);
    bitBoard &= (bitBoard - 1);
    return (int)index;
    }
#else
const uint64_t DeBrujinValue = 0x07EDD5E59A4E28C2;
const int DeBrujinTable [] =
    {
	63,  0, 58,  1, 59, 47, 53,  2,
	60, 39, 48, 27, 54, 33, 42,  3,
	61, 51, 37, 40, 49, 18, 28, 20,
	55, 30, 34, 11, 43, 14, 22,  4,
	62, 57, 46, 52, 38, 26, 32, 41,
	50, 36, 17, 19, 29, 10, 13, 21,
	56, 45, 25, 31, 35, 16,  9, 12,
	44, 24, 15,  8, 23,  7,  6,  5
    };

INLINE int BitScanForward( uint64_t bitBoard )
    {
    return DeBrujinTable[((bitBoard & -bitBoard)*DeBrujinValue) >> 58];
    }

INLINE int BitScanForwardReset( uint64_t &bitBoard )
    {
    uint64_t bb = bitBoard;
    bitBoard &= (bitBoard - 1);

    return DeBrujinTable[((bb & -bb)*DeBrujinValue) >> 58];
    }
#endif
INLINE int PopCount( uint64_t bitBoard )
    {
#if defined(__GNUC__)
    return __builtin_popcountll(bitBoard);
#elif defined(_MSC_VER) && defined(_WIN64)
    return _mm_popcnt_u64(bitBoard);
#elif defined(_MSC_VER)
    uint32_t lo = (uint32_t)bitBoard;
    uint32_t hi = (uint32_t)(bitBoard >> 32);
    return _mm_popcnt_u32(lo) + _mm_popcnt_u32(hi);
#else
    bitBoard -= ((bitBoard >> 1) & 0x5555555555555555UL);
    bitBoard = ((bitBoard >> 2) & 0x3333333333333333UL) + (bitBoard & 0x3333333333333333UL);
    bitBoard = ((bitBoard >> 4) + bitBoard) & 0x0F0F0F0F0F0F0F0FUL;
    return (int)((bitBoard * 0x0101010101010101UL) >> 56);
#endif
    }

INLINE bool IsBitSet( uint64_t bitBoard, int bitPos )
    {
    return (bitBoard &(static_cast<uint64_t>(1) << bitPos)) != 0;
    }