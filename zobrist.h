#pragma once

namespace Zobrist
    {
    extern uint64_t pieceInfo[2][7][74];
    extern uint64_t Castling[16];
    extern uint64_t Enpassant[8];
    extern uint64_t Color;
    void Init();
    }