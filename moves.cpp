#include "defines.h"
#include "direction.h"
#include "piece.h"
#include "position.h"

uint64_t Moves::PawnAttacks[2][64];            
uint64_t Moves::KingAttacks[64];              
uint64_t Moves::KnightAttacks[64];            

#ifdef PEXT
uint64_t Moves::RookMask[64];                 
#endif

uint64_t Moves::RankAttacks[64][64];           
uint64_t Moves::FileAttacks[64][64];           

uint64_t Moves::A1H8DiagonalAttacks[64][64];    
uint64_t Moves::H1A8DiagonalAttacks[64][64];    
uint64_t Moves::PseudoRookAttacks[64];        
uint64_t Moves::PseudoBishopAttacks[64];      

uint64_t Moves::ObstructedTable[64][64];
uint64_t Moves::KingProximity[2][64];          
uint64_t Moves::SideFiles[8];                 
uint64_t Moves::FrontSpan[2][64];              
uint64_t Moves::PasserSpan[2][64];            

int Moves::Distance[64][64];                   

void Moves::InitAttacks()
    {
    initPawnAttacks();
    initKnightAttacks();
    initKingAttacks();
    initRankAttacks();
    initFileAttacks();
    initDiagonalAttacks();
    initAntiDiagonalAttacks();
    initPseudoAttacks();
    initObstructedTable();

#ifdef PEXT
    initRookAttacks();
    for ( auto sq = 0; sq < 64; sq++ )
        {
        auto f = Square::GetFileIndex(sq);
        auto r = Square::GetRankIndex(sq);
        RookMask[sq] = Masks::SixBitRankMask[r] | Masks::SixBitFileMask[f];
        }
#endif
    for ( auto sq1 = 0; sq1 < 64; sq1++ )
        for ( auto sq2 = 0; sq2 < 64; sq2++ )
            Distance[sq1][sq2] = Square::Distance(sq1, sq2);

    for ( auto sq = 0; sq < 64; sq++ )
        {
        uint64_t king_ring = King::GetKingAttacks(Masks::SquareMask[sq]);
        KingProximity[PieceColor::White][sq] = king_ring | Direction::OneStepNorth(king_ring);
        KingProximity[PieceColor::Black][sq] = king_ring | Direction::OneStepSouth(king_ring);
        }

    for ( uint8_t f = 0; f < 8; f++ )
        {
        uint64_t file = Masks::FileMask[f];
        SideFiles[f] = Direction::OneStepWest(file);
        SideFiles[f] |= Direction::OneStepEast(file);
        }

    for ( auto sq = 0; sq < 64; sq++ )
        {
        uint64_t wspan = Masks::SquareMask[sq];
        uint64_t bspan = Masks::SquareMask[sq];

        wspan |= wspan << 8;
        wspan |= wspan << 16;
        wspan |= wspan << 32;
        wspan = Direction::OneStepNorth(wspan);

        bspan |= bspan >> 8;
        bspan |= bspan >> 16;
        bspan |= bspan >> 32;
        bspan = Direction::OneStepSouth(bspan);

        PasserSpan[PieceColor::White][sq] = FrontSpan[PieceColor::White][sq] = wspan;
        PasserSpan[PieceColor::Black][sq] = FrontSpan[PieceColor::Black][sq] = bspan;

        PasserSpan[PieceColor::White][sq] |= Direction::OneStepWest(wspan);
        PasserSpan[PieceColor::White][sq] |= Direction::OneStepEast(wspan);

        PasserSpan[PieceColor::Black][sq] |= Direction::OneStepWest(bspan);
        PasserSpan[PieceColor::Black][sq] |= Direction::OneStepEast(bspan);
        }
    }

void Moves::initPawnAttacks()
    {
    for ( int sq = 0; sq < 64; sq++ )
        {
        PawnAttacks[PieceColor::White][sq] =
            Direction::OneStepNorthEast(Masks::SquareMask[sq]) | Direction::OneStepNorthWest(Masks::SquareMask[sq]);
        PawnAttacks[PieceColor::Black][sq] =
            Direction::OneStepSouthEast(Masks::SquareMask[sq]) | Direction::OneStepSouthWest(Masks::SquareMask[sq]);
        }
    }

void Moves::initKnightAttacks()
    {
    for ( int sq = 0; sq < 64; sq++ )
        {
        KnightAttacks[sq] = Knight::GetKnightAttacks(Masks::SquareMask[sq]);
        }
    }

void Moves::initKingAttacks()
    {
    for ( int sq = 0; sq < 64; sq++ )
        {
        KingAttacks[sq] = King::GetKingAttacks(Masks::SquareMask[sq]);
        }
    }

void Moves::initRankAttacks()
    {
    for ( int sq = 0; sq < 64; sq++ )
        {
        for (uint64_t occ = 0; occ < 64; occ++ )
            {
            int rank = Square::GetRankIndex(sq);
            int file = Square::GetFileIndex(sq);

            uint64_t occupancy = (occ << 1);
            uint64_t targets = Empty;

            int blocker = file + 1;

            while (blocker <= 7)
                {
                targets |= Masks::SquareMask[blocker];

                if (IsBitSet(occupancy, blocker))
                    break;

                blocker++;
                }

            blocker = file - 1;

            while (blocker >= 0)
                {
                targets |= Masks::SquareMask[blocker];

                if (IsBitSet(occupancy, blocker))
                    break;

                blocker--;
                }

            RankAttacks[sq][occ] = targets << (8 * rank);
            }
        }
    }

void Moves::initFileAttacks()
    {
    for ( int sq = 0; sq < 64; sq++ )
        {
        for ( int occ = 0; occ < 64; occ++ )
            {
            uint64_t targets = Empty;
            uint64_t rankTargets = RankAttacks[7 - (sq / 8)][occ];         

            for ( int bit = 0; bit < 8; bit++ )                           
                {
                int rank = 7 - bit;
                int file = Square::GetFileIndex(sq);

                if (IsBitSet(rankTargets, bit))
                    {
                    targets |= Masks::SquareMask[Square::GetSquareIndex(file, rank)];
                    }
                }
            FileAttacks[sq][occ] = targets;
            }
        }
    }

void Moves::initDiagonalAttacks()
    {
    for ( int sq = 0; sq < 64; sq++ )
        {
        for ( int occ = 0; occ < 64; occ++ )
            {
            int diag = Square::GetRankIndex(sq) - Square::GetFileIndex(sq);
            uint64_t targets = Empty;
            uint64_t rankTargets = diag > 0 ? RankAttacks[sq % 8][occ] : RankAttacks[sq / 8][occ];
            for ( int bit = 0; bit < 8; bit++ )        
                {
                int rank;
                int file;

                if (IsBitSet(rankTargets, bit))
                    {
                    if (diag >= 0)
                        {
                        rank = diag + bit;
                        file = bit;
                        }
                    else
                        {
                        file = bit - diag;
                        rank = bit;
                        }

                    if ((file >= 0) && (file <= 7) && (rank >= 0) && (rank <= 7))
                        {
                        targets |= Masks::SquareMask[Square::GetSquareIndex(file, rank)];
                        }
                    }
                }

            A1H8DiagonalAttacks[sq][occ] = targets;
            }
        }
    }

void Moves::initAntiDiagonalAttacks()
    {
    for ( int sq = 0; sq < 64; sq++ )
        {
        for ( int occ = 0; occ < 64; occ++ )
            {
            int diag = Square::GetH1A8AntiDiagonalIndex(sq);

            uint64_t targets = Empty;
            uint64_t rankTargets = diag > 7 ? RankAttacks[7 - sq / 8][occ] : RankAttacks[sq % 8][occ];
            for ( int bit = 0; bit < 8; bit++ )        
                {
                int rank;
                int file;

                if (IsBitSet(rankTargets, bit))
                    {
                    if (diag >= 7)
                        {
                        rank = 7 - bit;
                        file = (diag - 7) + bit;
                        }
                    else
                        {
                        rank = diag - bit;
                        file = bit;
                        }

                    if ((file >= 0) && (file <= 7) && (rank >= 0) && (rank <= 7))
                        {
                        targets |= Masks::SquareMask[Square::GetSquareIndex(file, rank)];
                        }
                    }
                }

            H1A8DiagonalAttacks[sq][occ] = targets;
            }
        }
    }

void Moves::initPseudoAttacks()
    {
    for ( int i = 0; i < 64; i++ )
        {
        PseudoRookAttacks[i] = RankAttacks[i][0] | FileAttacks[i][0];
        PseudoBishopAttacks[i] = A1H8DiagonalAttacks[i][0] | H1A8DiagonalAttacks[i][0];
        }
    }

void Moves::initObstructedTable()
    {
    for ( int s1 = 0; s1 < 64; s1++ )
        {
        for ( int s2 = 0; s2 < 64; s2++ )
            {
            ObstructedTable[s1][s2] = 0;

            if ((PseudoRookAttacks[s1] | PseudoBishopAttacks[s1]) & Masks::SquareMask[s2])
                {
                int delta = (s2 - s1) / std::max(abs(Square::GetFileIndex(s1) - Square::GetFileIndex(s2)),
                    abs(Square::GetRankIndex(s1) - Square::GetRankIndex(s2)));

                for ( int s = s1 + delta; s != s2; s += delta )
                    ObstructedTable[s1][s2] |= Masks::SquareMask[s];
                }
            }
        }
    }

#ifdef PEXT
uint64_t Moves::RookAttacks[64][64 * 64];     

void Moves::initRookAttacks()
    {
    for ( auto sq = 0; sq < 64; sq++ )
        {
        auto f = Square::GetFileIndex(sq);
        auto r = Square::GetRankIndex(sq);

        for ( auto occ = 0; occ < 64*64; occ++ )
            {
            int f_occ = 0, r_occ = 0;
            int word = occ;
            int f_bits = 0;

            if (r >= 1 && r <= 6 && f >= 1 && f <= 6)
                {
                f_occ = word &((1 << (r - 1)) - 1);    
                word >>= r - 1;
                f_bits += r - 1;
                f_bits++;                            
                r_occ = word &((1 << 6) - 1);    
                word >>= 6;
                f_occ |= word << f_bits;        
                }

            else if (r == 0)
                {
                r_occ = word &((1 << 6) - 1);    
                word >>= 6;                     
                f_occ = word;
                }

            else if (r == 7)
                {
                f_occ = word &((1 << 6) - 1);    
                word >>= 6;
                r_occ = word;
                }

            else if (f == 0)
                {
                f_occ = word &((1 << r) - 1);    
                word >>= r;                      
                f_bits += r;                      
                r_occ = word &((1 << 6) - 1);    
                word >>= 6;                      
                f_occ |= word << f_bits;        
                }

            else if (f == 7)
                {
                f_occ = word &((1 << (r - 1)) - 1);    
                word >>= r - 1;
                f_bits += r - 1;
                r_occ = word &((1 << 6) - 1);    
                word >>= 6;
                f_occ |= word << f_bits;        
                }

            RookAttacks[sq][occ] = RankAttacks[sq][r_occ] | FileAttacks[sq][f_occ];
            }
        }
    }
#endif