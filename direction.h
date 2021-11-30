#pragma once
#include "square.h"
#include "files.h"
#include "defines.h"

namespace Direction
    {
    const uint64_t notAFile = ~Files::A;
    const uint64_t notBFile = ~Files::B;
    const uint64_t notCFile = ~Files::C;
    const uint64_t notDFile = ~Files::D;
    const uint64_t notEFile = ~Files::E;
    const uint64_t notFFile = ~Files::F;
    const uint64_t notGFile = ~Files::G;
    const uint64_t notHFile = ~Files::H;
    const uint64_t notABFile = notAFile | notBFile;
    const uint64_t notGHFile = notGFile | notHFile;

    INLINE static uint64_t oneStepSouth(uint64_t bitBoard)
        {
        return bitBoard >> 8;
        }

    INLINE static uint64_t oneStepNorth(uint64_t bitBoard)
        {
        return bitBoard << 8;
        }

    INLINE static uint64_t oneStepWest(uint64_t bitBoard)
        {
        return bitBoard >> 1 & notHFile;
        }

    INLINE static uint64_t oneStepEast(uint64_t bitBoard)
        {
        return bitBoard << 1 & notAFile;
        }

    INLINE static uint64_t oneStepNorthEast(uint64_t bitBoard)
        {
        return bitBoard << 9 & notAFile;
        }

    INLINE static uint64_t oneStepNorthWest(uint64_t bitBoard)
        {
        return bitBoard << 7 & notHFile;
        }

    INLINE static uint64_t oneStepSouthEast(uint64_t bitBoard)
        {
        return bitBoard >> 7 & notAFile;
        }

    INLINE static uint64_t oneStepSouthWest(uint64_t bitBoard)
        {
        return bitBoard >> 9 & notHFile;
        }
    }