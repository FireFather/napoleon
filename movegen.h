#pragma once

#include "position.h"
#include "move.h"
#include "pawn.h"
#include "piece.h"
#include "castle.h"

namespace moveGen
    {
	int moveCount(Position &);

    template <bool>
	void getPseudoLegalMoves(Move allMoves [], int &pos, uint64_t attackers, Position &position);
	void getLegalMoves(Move allMoves [], int & pos, Position & position);
	void getAllMoves(Move allMoves [], int & pos, Position & position);

    template <bool>
	void getPawnMoves(uint64_t pawns, Position &position, Move moveList [], int &pos, uint64_t target);
	void getKingMoves(uint64_t king, Position & position, Move moveList [], int & pos, uint64_t target);
	void getKnightMoves(uint64_t knights, Position & position, Move moveList [], int & pos, uint64_t target);
	void getRookMoves(uint64_t rooks, Position & position, Move moveList [], int & pos, uint64_t target);
	void getBishopMoves(uint64_t bishops, Position & position, Move moveList [], int & pos, uint64_t target);
	void getQueenMoves(uint64_t queens, Position & position, Move moveList [], int & pos, uint64_t target);
	void getCastleMoves(Position & position, Move moveList [], int & pos);

    template <bool>
    void getEvadeMoves(Position &position, uint64_t attackers, Move moveList [], int &pos);
	void getCaptures(Move allMoves [], int & pos, Position & position);
	void getNonCaptures(Move allMoves [], int & pos, Position & position);
    const int maxMoves = 256;
    }

INLINE int moveGen::moveCount(Position &position)
    {
    int count = 0;
    Move moves[maxMoves];
    getLegalMoves(moves, count, position);
    return count;
    }

template <bool onlyCaptures>
INLINE void moveGen::getPseudoLegalMoves(Move allMoves [], int &pos, uint64_t attackers, Position &position)
    {
    if (attackers)
        {
        getEvadeMoves<onlyCaptures>(position, attackers, allMoves, pos);
        position.setCheckState(true);
        }
    else if (onlyCaptures)
        {
        getCaptures(allMoves, pos, position);
        position.setCheckState(false);
        }
    else
        {
        getAllMoves(allMoves, pos, position);
        position.setCheckState(false);
        }
    }

INLINE void moveGen::getAllMoves(Move allMoves [], int &pos, Position &position)
    {
    getCaptures(allMoves, pos, position);
    getNonCaptures(allMoves, pos, position);
    }

INLINE void moveGen::getCaptures(Move allMoves [], int &pos, Position &position)
    {
    uint64_t enemy = position.enemyPieces();
    getPawnMoves<true>(position.Pieces(position.getSideToMove(), pieceType::Pawn), position, allMoves, pos, enemy);
    getKnightMoves(position.Pieces(position.getSideToMove(), pieceType::Knight), position, allMoves, pos, enemy);
    getBishopMoves(position.Pieces(position.getSideToMove(), pieceType::Bishop), position, allMoves, pos, enemy);
    getQueenMoves(position.Pieces(position.getSideToMove(), pieceType::Queen), position, allMoves, pos, enemy);
    getKingMoves(position.Pieces(position.getSideToMove(), pieceType::King), position, allMoves, pos, enemy);
    getRookMoves(position.Pieces(position.getSideToMove(), pieceType::Rook), position, allMoves, pos, enemy);
    }

INLINE void moveGen::getNonCaptures(Move allMoves [], int &pos, Position &position)
    {
    uint64_t enemy = ~position.enemyPieces();
    getPawnMoves<false>(position.Pieces(position.getSideToMove(), pieceType::Pawn), position, allMoves, pos, enemy);
    getKnightMoves(position.Pieces(position.getSideToMove(), pieceType::Knight), position, allMoves, pos, enemy);
    getBishopMoves(position.Pieces(position.getSideToMove(), pieceType::Bishop), position, allMoves, pos, enemy);
    getQueenMoves(position.Pieces(position.getSideToMove(), pieceType::Queen), position, allMoves, pos, enemy);
    getKingMoves(position.Pieces(position.getSideToMove(), pieceType::King), position, allMoves, pos, enemy);
    getRookMoves(position.Pieces(position.getSideToMove(), pieceType::Rook), position, allMoves, pos, enemy);
    getCastleMoves(position, allMoves, pos);
    }

template <bool ep>
INLINE void moveGen::getPawnMoves(uint64_t pawns, Position &position, Move moveList [], int &pos, uint64_t target)
    {
    uint64_t targets;
    uint64_t epTargets;
    uint8_t fromIndex;
    uint8_t toIndex;

    while (pawns != 0)
        {
        fromIndex = bitScanForwardReset(pawns);        
        targets = Pawn::getAllTargets(Masks::squareMask[fromIndex], position) & target;

        if (ep)
            {
            if (position.getPassantSquare() != Square::noSquare)
                {
                epTargets = Moves::pawnAttacks[position.getSideToMove()][fromIndex];

                if ((epTargets & Masks::squareMask[position.getPassantSquare()]) != 0)
                    moveList[pos++] = Move(fromIndex, position.getPassantSquare(), EnPassant);
                }
            }

        while (targets != 0)
            {
            toIndex = bitScanForwardReset(targets);        

            if ((Square::getRankIndex(toIndex) == 7 && position.getSideToMove() == pieceColor::White)
                || (Square::getRankIndex(toIndex) == 0 && position.getSideToMove() == pieceColor::Black))
                {
                moveList[pos++] = Move(fromIndex, toIndex, queenPromotion);
                moveList[pos++] = Move(fromIndex, toIndex, rookPromotion);
                moveList[pos++] = Move(fromIndex, toIndex, bishopPromotion);
                moveList[pos++] = Move(fromIndex, toIndex, knightPromotion);
                }
            else
                {
                moveList[pos++] = Move(fromIndex, toIndex);   
                }
            }
        }
    }

INLINE void moveGen::getKnightMoves(uint64_t knights, Position &position, Move moveList [], int &pos, uint64_t target)
    {
    uint64_t targets;
    uint8_t fromIndex;
    uint8_t toIndex;

    while (knights != 0)
        {
        fromIndex = bitScanForwardReset(knights);        
        targets = Knight::getAllTargets(Masks::squareMask[fromIndex], position) & target;

        while (targets != 0)
            {
            toIndex = bitScanForwardReset(targets);        
            moveList[pos++] = Move(fromIndex, toIndex);
            }
        }
    }

INLINE void moveGen::getBishopMoves(uint64_t bishops, Position &position, Move moveList [], int &pos, uint64_t target)
    {
    uint64_t targets;
    uint8_t fromIndex;
    uint8_t toIndex;

    while (bishops != 0)
        {
        fromIndex = bitScanForwardReset(bishops);        
        targets = Bishop::getAllTargets(Masks::squareMask[fromIndex], position) & target;

        while (targets != 0)
            {
            toIndex = bitScanForwardReset(targets);        
            moveList[pos++] = Move(fromIndex, toIndex);
            }
        }
    }

INLINE void moveGen::getKingMoves(uint64_t king, Position &position, Move moveList [], int &pos, uint64_t target)
    {
    uint64_t targets;
    uint8_t fromIndex;
    uint8_t toIndex;

    while (king != 0)
        {
        fromIndex = bitScanForwardReset(king);        
        targets = King::getAllTargets(Masks::squareMask[fromIndex], position) & target;

        while (targets != 0)
            {
            toIndex = bitScanForwardReset(targets);        
            moveList[pos++] = Move(fromIndex, toIndex);
            }
        }
    }

INLINE void moveGen::getRookMoves(uint64_t rooks, Position &position, Move moveList [], int &pos, uint64_t target)
    {
    uint64_t targets;
    uint8_t fromIndex;
    uint8_t toIndex;

    while (rooks != 0)
        {
        fromIndex = bitScanForwardReset(rooks);        
        targets = Rook::getAllTargets(Masks::squareMask[fromIndex], position) & target;

        while (targets != 0)
            {
            toIndex = bitScanForwardReset(targets);        
            moveList[pos++] = Move(fromIndex, toIndex);
            }
        }
    }

INLINE void moveGen::getCastleMoves(Position &position, Move moveList [], int &pos)
    {
    if (position.getSideToMove() == pieceColor::White)
        {
        if (position.getCastlingStatus() & Castle::whiteCastleOO)
            {
            if ((Castle::whiteCastleMaskOO & position.occupiedSquares) == 0)
                {
                if (!position.isAttacked(Castle::whiteCastleMaskOO, position.getSideToMove()))
                    moveList[pos++] = Castle::whiteCastlingOO;
                }
            }

        if (position.getCastlingStatus() & Castle::whiteCastleOOO)
            {
            if ((Castle::whiteCastleMaskOOO & position.occupiedSquares) == 0)
                {
                if (!position.isAttacked(Castle::whiteCastleMaskOOO ^ Masks::squareMask[8], position.getSideToMove()))
                    moveList[pos++] = Castle::whiteCastlingOOO;
                }
            }
        }
    else if (position.getSideToMove() == pieceColor::Black)
        {
        if (position.getCastlingStatus() & Castle::blackCastleOO)
            {
            if ((Castle::blackCastleMaskOO & position.occupiedSquares) == 0)
                {
                if (!position.isAttacked(Castle::blackCastleMaskOO, position.getSideToMove()))
                    moveList[pos++] = Castle::blackCastlingOO;
                }
            }

        if (position.getCastlingStatus() & Castle::blackCastleOOO)
            {
            if ((Castle::blackCastleMaskOOO & position.occupiedSquares) == 0)
                {
                if (!position.isAttacked(Castle::blackCastleMaskOOO ^ Masks::squareMask[15], position.getSideToMove()))
                    moveList[pos++] = Castle::blackCastlingOOO;
                }
            }
        }
    }

template <bool onlyCaptures>
void moveGen::getEvadeMoves(Position &position, uint64_t checkers, Move moveList [], int &pos)
    {
    uint64_t b;
    uint8_t to;
    uint8_t from, checksq;
    uint8_t ksq = position.getKingSquare(position.getSideToMove());
    int checkersCnt = 0;
    uint64_t sliderAttacks = 0;

    b = checkers;

    do
        {
        checkersCnt++;
        checksq = bitScanForwardReset(b);

        switch(position.pieceOnSquare(checksq).Type)
            {
            case pieceType::Bishop:
                sliderAttacks |= Moves::pseudoBishopAttacks[checksq];
                break;

            case pieceType::Rook:
                sliderAttacks |= Moves::pseudoRookAttacks[checksq];
                break;

            case pieceType::Queen:
                if (Moves::obstructedTable[ksq][checksq]
                    || (Moves::pseudoBishopAttacks[checksq] & Masks::squareMask[ksq]) == 0)
                    sliderAttacks |= Moves::pseudoBishopAttacks[checksq] | Moves::pseudoRookAttacks[checksq];

                else

#ifdef PEXT
                    sliderAttacks |= Moves::pseudoBishopAttacks[checksq] | Moves::getRookAttacks(position.occupiedSquares, checksq);
#else
                sliderAttacks |= Moves::pseudoBishopAttacks[checksq] | (Moves::getRankAttacks(position.occupiedSquares, checksq)
                        | Moves::getFileAttacks(position.occupiedSquares, checksq));
#endif

                break;

            default:
                break;
            }
        } while (b);

    if (onlyCaptures)
        b = Moves::kingAttacks[ksq] & checkers;
    else
        b = Moves::kingAttacks[ksq] & ~position.ourPieces() & ~sliderAttacks;

    from = ksq;

    while (b)
        {
        to = bitScanForwardReset(b);        
        moveList[pos++] = Move(from, to);
        }

    if (checkersCnt > 1)
        return;

    uint64_t target;

    if (onlyCaptures)
        target = checkers;
    else
        target = Moves::obstructedTable[checksq][ksq] | checkers;

    getPawnMoves<true>(position.Pieces(position.getSideToMove(), pieceType::Pawn), position, moveList, pos, target);
    getKnightMoves(position.Pieces(position.getSideToMove(), pieceType::Knight), position, moveList, pos, target);
    getBishopMoves(position.Pieces(position.getSideToMove(), pieceType::Bishop), position, moveList, pos, target);
    getRookMoves(position.Pieces(position.getSideToMove(), pieceType::Rook), position, moveList, pos, target);
    getQueenMoves(position.Pieces(position.getSideToMove(), pieceType::Queen), position, moveList, pos, target);
    }

INLINE void moveGen::getLegalMoves(Move allMoves [], int &pos, Position &position)
    {
    uint64_t pinned = position.pinnedPieces();
    uint64_t attackers = position.kingAttackers(position.getKingSquare(position.getSideToMove()), position.getSideToMove());

    if (attackers)
        getEvadeMoves<false>(position, attackers, allMoves, pos);
    else
        getAllMoves(allMoves, pos, position);

    int last = pos;
    int cur = 0;

    while (cur != last)
        {
        if (!position.isMoveLegal(allMoves[cur], pinned))
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