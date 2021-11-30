#pragma once
#include "square.h"
#include "files.h"
#include "defines.h"

namespace Direction
    {
    const uint64_t NotAFile = ~Files::A;
    const uint64_t NotBFile = ~Files::B;
    const uint64_t NotCFile = ~Files::C;
    const uint64_t NotDFile = ~Files::D;
    const uint64_t NotEFile = ~Files::E;
    const uint64_t NotFFile = ~Files::F;
    const uint64_t NotGFile = ~Files::G;
    const uint64_t NotHFile = ~Files::H;
    const uint64_t NotABFile = NotAFile | NotBFile;
    const uint64_t NotGHFile = NotGFile | NotHFile;

    INLINE static uint64_t OneStepSouth(uint64_t bitBoard)
        {
        return bitBoard >> 8;
        }

    INLINE static uint64_t OneStepNorth(uint64_t bitBoard)
        {
        return bitBoard << 8;
        }

    INLINE static uint64_t OneStepWest(uint64_t bitBoard)
        {
        return bitBoard >> 1 & NotHFile;
        }

    INLINE static uint64_t OneStepEast(uint64_t bitBoard)
        {
        return bitBoard << 1 & NotAFile;
        }

    INLINE static uint64_t OneStepNorthEast(uint64_t bitBoard)
        {
        return bitBoard << 9 & NotAFile;
        }

    INLINE static uint64_t OneStepNorthWest(uint64_t bitBoard)
        {
        return bitBoard << 7 & NotHFile;
        }

    INLINE static uint64_t OneStepSouthEast(uint64_t bitBoard)
        {
        return bitBoard >> 7 & NotAFile;
        }

    INLINE static uint64_t OneStepSouthWest(uint64_t bitBoard)
        {
        return bitBoard >> 9 & NotHFile;
        }
    }