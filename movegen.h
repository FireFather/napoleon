#pragma once

#include "position.h"
#include "move.h"
#include "pawn.h"
#include "piece.h"
#include "castle.h"

namespace MoveGen
    {
    int MoveCount(Position &);
    template <bool>
    void GetPseudoLegalMoves( Move allMoves [], int &pos, uint64_t attackers, Position &position );
    void GetLegalMoves(Move allMoves [], int & pos, Position & position);
    void GetAllMoves(Move allMoves [], int & pos, Position & position);
    template <bool>
    void GetPawnMoves( uint64_t pawns, Position &position, Move moveList [], int &pos, uint64_t target );
    void GetKingMoves(uint64_t king, Position & position, Move moveList [], int & pos, uint64_t target);
    void GetKnightMoves(uint64_t knights, Position & position, Move moveList [], int & pos, uint64_t target);
    void GetRookMoves(uint64_t rooks, Position & position, Move moveList [], int & pos, uint64_t target);
    void GetBishopMoves(uint64_t bishops, Position & position, Move moveList [], int & pos, uint64_t target);
    void GetQueenMoves(uint64_t queens, Position & position, Move moveList [], int & pos, uint64_t target);
    void GetCastleMoves(Position & position, Move moveList [], int & pos);
    template <bool>
    void GetEvadeMoves( Position &position, uint64_t attackers, Move moveList [], int &pos );
    void GetCaptures(Move allMoves [], int & pos, Position & position);
    void GetNonCaptures(Move allMoves [], int & pos, Position & position);
    const int MaxMoves = 256;
    }

inline int MoveGen::MoveCount( Position &position )
    {
    int count = 0;
    Move moves[MaxMoves];
    GetLegalMoves(moves, count, position);
    return count;
    }

template <bool onlyCaptures>
INLINE void MoveGen::GetPseudoLegalMoves( Move allMoves [], int &pos, uint64_t attackers, Position &position )
    {
    if (attackers)
        {
        GetEvadeMoves<onlyCaptures>(position, attackers, allMoves, pos);
        position.SetCheckState(true);
        }
    else if (onlyCaptures)
        {
        GetCaptures(allMoves, pos, position);
        position.SetCheckState(false);
        }
    else
        {
        GetAllMoves(allMoves, pos, position);
        position.SetCheckState(false);
        }
    }

INLINE void MoveGen::GetAllMoves( Move allMoves [], int &pos, Position &position )
    {
    GetCaptures(allMoves, pos, position);
    GetNonCaptures(allMoves, pos, position);
    }

INLINE void MoveGen::GetCaptures( Move allMoves [], int &pos, Position &position )
    {
    uint64_t enemy = position.EnemyPieces();
    GetPawnMoves<true>(position.Pieces(position.SideToMove(), PieceType::Pawn), position, allMoves, pos, enemy);
    GetKnightMoves(position.Pieces(position.SideToMove(), PieceType::Knight), position, allMoves, pos, enemy);
    GetBishopMoves(position.Pieces(position.SideToMove(), PieceType::Bishop), position, allMoves, pos, enemy);
    GetQueenMoves(position.Pieces(position.SideToMove(), PieceType::Queen), position, allMoves, pos, enemy);
    GetKingMoves(position.Pieces(position.SideToMove(), PieceType::King), position, allMoves, pos, enemy);
    GetRookMoves(position.Pieces(position.SideToMove(), PieceType::Rook), position, allMoves, pos, enemy);
    }

INLINE void MoveGen::GetNonCaptures( Move allMoves [], int &pos, Position &position )
    {
    uint64_t enemy = ~position.EnemyPieces();
    GetPawnMoves<false>(position.Pieces(position.SideToMove(), PieceType::Pawn), position, allMoves, pos, enemy);
    GetKnightMoves(position.Pieces(position.SideToMove(), PieceType::Knight), position, allMoves, pos, enemy);
    GetBishopMoves(position.Pieces(position.SideToMove(), PieceType::Bishop), position, allMoves, pos, enemy);
    GetQueenMoves(position.Pieces(position.SideToMove(), PieceType::Queen), position, allMoves, pos, enemy);
    GetKingMoves(position.Pieces(position.SideToMove(), PieceType::King), position, allMoves, pos, enemy);
    GetRookMoves(position.Pieces(position.SideToMove(), PieceType::Rook), position, allMoves, pos, enemy);
    GetCastleMoves(position, allMoves, pos);
    }

template <bool ep>
INLINE void MoveGen::GetPawnMoves( uint64_t pawns, Position &position, Move moveList [], int &pos, uint64_t target )
    {
    uint64_t targets;
    uint64_t epTargets;
    uint8_t fromIndex;
    uint8_t toIndex;

    while (pawns != 0)
        {
        fromIndex = BitScanForwardReset(pawns);        
        targets = Pawn::GetAllTargets(Masks::SquareMask[fromIndex], position) & target;

        if (ep)
            {
            if (position.EnPassantSquare() != Square::None)
                {
                epTargets = Moves::PawnAttacks[position.SideToMove()][fromIndex];

                if ((epTargets & Masks::SquareMask[position.EnPassantSquare()]) != 0)
                    moveList[pos++] = Move(fromIndex, position.EnPassantSquare(), EnPassant);
                }
            }

        while (targets != 0)
            {
            toIndex = BitScanForwardReset(targets);        

            if ((Square::GetRankIndex(toIndex) == 7 && position.SideToMove() == PieceColor::White)
                || (Square::GetRankIndex(toIndex) == 0 && position.SideToMove() == PieceColor::Black))
                {
                moveList[pos++] = Move(fromIndex, toIndex, QueenPromotion);
                moveList[pos++] = Move(fromIndex, toIndex, RookPromotion);
                moveList[pos++] = Move(fromIndex, toIndex, BishopPromotion);
                moveList[pos++] = Move(fromIndex, toIndex, KnightPromotion);
                }
            else
                {
                moveList[pos++] = Move(fromIndex, toIndex);   
                }
            }
        }
    }

INLINE void MoveGen::GetKnightMoves( uint64_t knights, Position &position, Move moveList [], int &pos, uint64_t target )
    {
    uint64_t targets;
    uint8_t fromIndex;
    uint8_t toIndex;

    while (knights != 0)
        {
        fromIndex = BitScanForwardReset(knights);        
        targets = Knight::GetAllTargets(Masks::SquareMask[fromIndex], position) & target;

        while (targets != 0)
            {
            toIndex = BitScanForwardReset(targets);        
            moveList[pos++] = Move(fromIndex, toIndex);
            }
        }
    }

INLINE void MoveGen::GetBishopMoves( uint64_t bishops, Position &position, Move moveList [], int &pos, uint64_t target )
    {
    uint64_t targets;
    uint8_t fromIndex;
    uint8_t toIndex;

    while (bishops != 0)
        {
        fromIndex = BitScanForwardReset(bishops);        
        targets = Bishop::GetAllTargets(Masks::SquareMask[fromIndex], position) & target;

        while (targets != 0)
            {
            toIndex = BitScanForwardReset(targets);        
            moveList[pos++] = Move(fromIndex, toIndex);
            }
        }
    }

INLINE void MoveGen::GetKingMoves( uint64_t king, Position &position, Move moveList [], int &pos, uint64_t target )
    {
    uint64_t targets;
    uint8_t fromIndex;
    uint8_t toIndex;

    while (king != 0)
        {
        fromIndex = BitScanForwardReset(king);        
        targets = King::GetAllTargets(Masks::SquareMask[fromIndex], position) & target;

        while (targets != 0)
            {
            toIndex = BitScanForwardReset(targets);        
            moveList[pos++] = Move(fromIndex, toIndex);
            }
        }
    }

INLINE void MoveGen::GetRookMoves( uint64_t rooks, Position &position, Move moveList [], int &pos, uint64_t target )
    {
    uint64_t targets;
    uint8_t fromIndex;
    uint8_t toIndex;

    while (rooks != 0)
        {
        fromIndex = BitScanForwardReset(rooks);        
        targets = Rook::GetAllTargets(Masks::SquareMask[fromIndex], position) & target;

        while (targets != 0)
            {
            toIndex = BitScanForwardReset(targets);        
            moveList[pos++] = Move(fromIndex, toIndex);
            }
        }
    }

INLINE void MoveGen::GetCastleMoves( Position &position, Move moveList [], int &pos )
    {
    if (position.SideToMove() == PieceColor::White)
        {
        if (position.CastlingStatus() & Castle::WhiteCastleOO)
            {
            if ((Castle::WhiteCastleMaskOO & position.OccupiedSquares) == 0)
                {
                if (!position.IsAttacked(Castle::WhiteCastleMaskOO, position.SideToMove()))
                    moveList[pos++] = Castle::WhiteCastlingOO;
                }
            }

        if (position.CastlingStatus() & Castle::WhiteCastleOOO)
            {
            if ((Castle::WhiteCastleMaskOOO & position.OccupiedSquares) == 0)
                {
                if (!position.IsAttacked(Castle::WhiteCastleMaskOOO ^ Masks::SquareMask[8], position.SideToMove()))
                    moveList[pos++] = Castle::WhiteCastlingOOO;
                }
            }
        }
    else if (position.SideToMove() == PieceColor::Black)
        {
        if (position.CastlingStatus() & Castle::BlackCastleOO)
            {
            if ((Castle::BlackCastleMaskOO & position.OccupiedSquares) == 0)
                {
                if (!position.IsAttacked(Castle::BlackCastleMaskOO, position.SideToMove()))
                    moveList[pos++] = Castle::BlackCastlingOO;
                }
            }

        if (position.CastlingStatus() & Castle::BlackCastleOOO)
            {
            if ((Castle::BlackCastleMaskOOO & position.OccupiedSquares) == 0)
                {
                if (!position.IsAttacked(Castle::BlackCastleMaskOOO ^ Masks::SquareMask[15], position.SideToMove()))
                    moveList[pos++] = Castle::BlackCastlingOOO;
                }
            }
        }
    }

template <bool onlyCaptures>
void MoveGen::GetEvadeMoves( Position &position, uint64_t checkers, Move moveList [], int &pos )
    {
    uint64_t b;
    uint8_t to;
    uint8_t from, checksq;
    uint8_t ksq = position.KingSquare(position.SideToMove());
    int checkersCnt = 0;
    uint64_t sliderAttacks = 0;

    b = checkers;

    do
        {
        checkersCnt++;
        checksq = BitScanForwardReset(b);

        switch( position.PieceOnSquare(checksq).Type )
            {
            case PieceType::Bishop:
                sliderAttacks |= Moves::PseudoBishopAttacks[checksq];
                break;

            case PieceType::Rook:
                sliderAttacks |= Moves::PseudoRookAttacks[checksq];
                break;

            case PieceType::Queen:
                if (Moves::ObstructedTable[ksq][checksq]
                    || (Moves::PseudoBishopAttacks[checksq] & Masks::SquareMask[ksq]) == 0)
                    sliderAttacks |= Moves::PseudoBishopAttacks[checksq] | Moves::PseudoRookAttacks[checksq];

                else

#ifdef PEXT
                    sliderAttacks |= Moves::PseudoBishopAttacks[checksq] | Moves::GetRookAttacks(position.OccupiedSquares, checksq);
#else
                sliderAttacks |= Moves::PseudoBishopAttacks[checksq] | (Moves::GetRankAttacks(position.OccupiedSquares, checksq)
                        | Moves::GetFileAttacks(position.OccupiedSquares, checksq));
#endif

                break;

            default:
                break;
            }
        } while ( b );

    if (onlyCaptures)
        b = Moves::KingAttacks[ksq] & checkers;
    else
        b = Moves::KingAttacks[ksq] & ~position.PlayerPieces() & ~sliderAttacks;

    from = ksq;

    while (b)
        {
        to = BitScanForwardReset(b);        
        moveList[pos++] = Move(from, to);
        }

    if (checkersCnt > 1)
        return;

    uint64_t target;

    if (onlyCaptures)
        target = checkers;
    else
        target = Moves::ObstructedTable[checksq][ksq] | checkers;

    GetPawnMoves<true>(position.Pieces(position.SideToMove(), PieceType::Pawn), position, moveList, pos, target);
    GetKnightMoves(position.Pieces(position.SideToMove(), PieceType::Knight), position, moveList, pos, target);
    GetBishopMoves(position.Pieces(position.SideToMove(), PieceType::Bishop), position, moveList, pos, target);
    GetRookMoves(position.Pieces(position.SideToMove(), PieceType::Rook), position, moveList, pos, target);
    GetQueenMoves(position.Pieces(position.SideToMove(), PieceType::Queen), position, moveList, pos, target);
    }

INLINE void MoveGen::GetLegalMoves( Move allMoves [], int &pos, Position &position )
    {
    uint64_t pinned = position.PinnedPieces();
    uint64_t attackers = position.KingAttackers(position.KingSquare(position.SideToMove()), position.SideToMove());

    if (attackers)
        GetEvadeMoves<false>(position, attackers, allMoves, pos);
    else
        GetAllMoves(allMoves, pos, position);

    int last = pos;
    int cur = 0;

    while (cur != last)
        {
        if (!position.IsMoveLegal(allMoves[cur], pinned))
            {
            allMoves[cur] = allMoves[--last];
            }
        else
            {
            cur++;
            }
        }
    pos = last;
    }