#include "defines.h"
#include "direction.h"
#include "piece.h"
#include "position.h"

uint64_t Moves::pawnAttacks[2][64];            
uint64_t Moves::kingAttacks[64];              
uint64_t Moves::knightAttacks[64];            

#ifdef PEXT
uint64_t Moves::RookMask[64];                 
#endif

uint64_t Moves::rankAttacks[64][64];           
uint64_t Moves::fileAttacks[64][64];           

uint64_t Moves::A1H8diagonalAttacks[64][64];    
uint64_t Moves::H1A8diagonalAttacks[64][64];    
uint64_t Moves::pseudoRookAttacks[64];        
uint64_t Moves::pseudoBishopAttacks[64];      

uint64_t Moves::obstructedTable[64][64];
uint64_t Moves::kingProximity[2][64];          
uint64_t Moves::adjacentFiles[8];                 
uint64_t Moves::frontSpan[2][64];              
uint64_t Moves::passerSpan[2][64];            

int Moves::Distance[64][64];                   

void Moves::initAttacks()
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
    for (auto sq = 0; sq < 64; sq++)
        {
        auto f = Square::getFileIndex(sq);
        auto r = Square::getRankIndex(sq);
        RookMask[sq] = Masks::sixBitRankMask[r] | Masks::sixBitFileMask[f];
        }
#endif
    for (auto sq1 = 0; sq1 < 64; sq1++)
        for (auto sq2 = 0; sq2 < 64; sq2++)
            Distance[sq1][sq2] = Square::Distance(sq1, sq2);

    for (auto sq = 0; sq < 64; sq++)
        {
        uint64_t king_ring = King::getKingAttacks(Masks::squareMask[sq]);
        kingProximity[pieceColor::White][sq] = king_ring | Direction::oneStepNorth(king_ring);
        kingProximity[pieceColor::Black][sq] = king_ring | Direction::oneStepSouth(king_ring);
        }

    for (uint8_t f = 0; f < 8; f++)
        {
        uint64_t file = Masks::fileMask[f];
        adjacentFiles[f] = Direction::oneStepWest(file);
        adjacentFiles[f] |= Direction::oneStepEast(file);
        }

    for (auto sq = 0; sq < 64; sq++)
        {
        uint64_t wspan = Masks::squareMask[sq];
        uint64_t bspan = Masks::squareMask[sq];

        wspan |= wspan << 8;
        wspan |= wspan << 16;
        wspan |= wspan << 32;
        wspan = Direction::oneStepNorth(wspan);

        bspan |= bspan >> 8;
        bspan |= bspan >> 16;
        bspan |= bspan >> 32;
        bspan = Direction::oneStepSouth(bspan);

        passerSpan[pieceColor::White][sq] = frontSpan[pieceColor::White][sq] = wspan;
        passerSpan[pieceColor::Black][sq] = frontSpan[pieceColor::Black][sq] = bspan;

        passerSpan[pieceColor::White][sq] |= Direction::oneStepWest(wspan);
        passerSpan[pieceColor::White][sq] |= Direction::oneStepEast(wspan);

        passerSpan[pieceColor::Black][sq] |= Direction::oneStepWest(bspan);
        passerSpan[pieceColor::Black][sq] |= Direction::oneStepEast(bspan);
        }
    }

void Moves::initPawnAttacks()
    {
    for (int sq = 0; sq < 64; sq++)
        {
        pawnAttacks[pieceColor::White][sq] =
            Direction::oneStepNorthEast(Masks::squareMask[sq]) | Direction::oneStepNorthWest(Masks::squareMask[sq]);
        pawnAttacks[pieceColor::Black][sq] =
            Direction::oneStepSouthEast(Masks::squareMask[sq]) | Direction::oneStepSouthWest(Masks::squareMask[sq]);
        }
    }

void Moves::initKnightAttacks()
    {
    for (int sq = 0; sq < 64; sq++)
        {
        knightAttacks[sq] = Knight::getKnightAttacks(Masks::squareMask[sq]);
        }
    }

void Moves::initKingAttacks()
    {
    for (int sq = 0; sq < 64; sq++)
        {
        kingAttacks[sq] = King::getKingAttacks(Masks::squareMask[sq]);
        }
    }

void Moves::initRankAttacks()
    {
    for (int sq = 0; sq < 64; sq++)
        {
        for (uint64_t occ = 0; occ < 64; occ++)
            {
            int rank = Square::getRankIndex(sq);
            int file = Square::getFileIndex(sq);

            uint64_t occupancy = (occ << 1);
            uint64_t targets = Empty;

            int blocker = file + 1;

            while (blocker <= 7)
                {
                targets |= Masks::squareMask[blocker];

                if (isBitSet(occupancy, blocker))
                    break;

                blocker++;
                }

            blocker = file - 1;

            while (blocker >= 0)
                {
                targets |= Masks::squareMask[blocker];

                if (isBitSet(occupancy, blocker))
                    break;

                blocker--;
                }

            rankAttacks[sq][occ] = targets << (8 * rank);
            }
        }
    }

void Moves::initFileAttacks()
    {
    for (int sq = 0; sq < 64; sq++)
        {
        for (int occ = 0; occ < 64; occ++)
            {
            uint64_t targets = Empty;
            uint64_t rankTargets = rankAttacks[7 - (sq / 8)][occ];         

            for (int bit = 0; bit < 8; bit++)                           
                {
                int rank = 7 - bit;
                int file = Square::getFileIndex(sq);

                if (isBitSet(rankTargets, bit))
                    {
                    targets |= Masks::squareMask[Square::getSquareIndex(file, rank)];
                    }
                }
            fileAttacks[sq][occ] = targets;
            }
        }
    }

void Moves::initDiagonalAttacks()
    {
    for (int sq = 0; sq < 64; sq++)
        {
        for (int occ = 0; occ < 64; occ++)
            {
            int diag = Square::getRankIndex(sq) - Square::getFileIndex(sq);
            uint64_t targets = Empty;
            uint64_t rankTargets = diag > 0 ? rankAttacks[sq % 8][occ] : rankAttacks[sq / 8][occ];
            for (int bit = 0; bit < 8; bit++)        
                {
                int rank;
                int file;

                if (isBitSet(rankTargets, bit))
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
                        targets |= Masks::squareMask[Square::getSquareIndex(file, rank)];
                        }
                    }
                }

            A1H8diagonalAttacks[sq][occ] = targets;
            }
        }
    }

void Moves::initAntiDiagonalAttacks()
    {
    for (int sq = 0; sq < 64; sq++)
        {
        for (int occ = 0; occ < 64; occ++)
            {
            int diag = Square::getH1A8AntiDiagonalIndex(sq);

            uint64_t targets = Empty;
            uint64_t rankTargets = diag > 7 ? rankAttacks[7 - sq / 8][occ] : rankAttacks[sq % 8][occ];
            for (int bit = 0; bit < 8; bit++)        
                {
                int rank;
                int file;

                if (isBitSet(rankTargets, bit))
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
                        targets |= Masks::squareMask[Square::getSquareIndex(file, rank)];
                        }
                    }
                }

            H1A8diagonalAttacks[sq][occ] = targets;
            }
        }
    }

void Moves::initPseudoAttacks()
    {
    for (int i = 0; i < 64; i++)
        {
        pseudoRookAttacks[i] = rankAttacks[i][0] | fileAttacks[i][0];
        pseudoBishopAttacks[i] = A1H8diagonalAttacks[i][0] | H1A8diagonalAttacks[i][0];
        }
    }

void Moves::initObstructedTable()
    {
    for (int s1 = 0; s1 < 64; s1++)
        {
        for (int s2 = 0; s2 < 64; s2++)
            {
            obstructedTable[s1][s2] = 0;

            if ((pseudoRookAttacks[s1] | pseudoBishopAttacks[s1]) & Masks::squareMask[s2])
                {
                int delta = (s2 - s1) / std::max(abs(Square::getFileIndex(s1) - Square::getFileIndex(s2)),
                    abs(Square::getRankIndex(s1) - Square::getRankIndex(s2)));

                for (int s = s1 + delta; s != s2; s += delta)
                    obstructedTable[s1][s2] |= Masks::squareMask[s];
                }
            }
        }
    }

#ifdef PEXT
uint64_t Moves::RookAttacks[64][64 * 64];     

void Moves::initRookAttacks()
    {
    for (auto sq = 0; sq < 64; sq++)
        {
        auto f = Square::getFileIndex(sq);
        auto r = Square::getRankIndex(sq);

        for (auto occ = 0; occ < 64*64; occ++)
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

            RookAttacks[sq][occ] = rankAttacks[sq][r_occ] | fileAttacks[sq][f_occ];
            }
        }
    }
#endif