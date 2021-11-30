#include "piece.h"
#include "direction.h"
#include "position.h"

namespace Piece
    {
    uint8_t getPiece(char initial)
        {
        switch(initial)
            {
            case 'b':
                return pieceType::Bishop;
            case 'n':
                return pieceType::Knight;
            case 'q':
                return pieceType::Queen;
            case 'r':
                return pieceType::Rook;
            default:
                throw std::exception();
            }
        }
    char getType(uint8_t type)
        {
        switch(type)
            {
            case pieceType::Bishop:
                return 'b';
            case pieceType::King:
                return 'k';
            case pieceType::Knight:
                return 'n';
            case pieceType::Pawn:
                return 'p';
            case pieceType::Queen:
                return 'q';
            case pieceType::Rook:
                return 'r';
            default:
                throw std::exception();
            }
        }
    char getColor(uint8_t color)
        {
        switch(color)
            {
            case pieceColor::White:
                return 'w';
            case pieceColor::Black:
                return 'b';
            default:
                throw std::exception();
            }
        }

    char getInitial(pieceInfo piece)
        {
        if (piece.Color == pieceColor::White)
            {
            switch(piece.Type)
                {
                case pieceType::Bishop:
                    return 'B';
                case pieceType::King:
                    return 'K';
                case pieceType::Knight:
                    return 'N';
                case pieceType::Pawn:
                    return 'P';
                case pieceType::Queen:
                    return 'Q';
                case pieceType::Rook:
                    return 'R';
                default:
                    throw std::exception();
                }
            }
        else
            {
            switch(piece.Type)
                {
                case pieceType::Bishop:
                    return 'b';
                case pieceType::King:
                    return 'k';
                case pieceType::Knight:
                    return 'n';
                case pieceType::Pawn:
                    return 'p';
                case pieceType::Queen:
                    return 'q';
                case pieceType::Rook:
                    return 'r';
                default:
                    throw std::exception();
                }
            }
        }
    }

pieceInfo::pieceInfo(uint8_t color, uint8_t type) :
    Color(color),
    Type(type)
    {
    }

pieceInfo::pieceInfo() noexcept :
    Color(pieceColor::noColor),
    Type(pieceType::noType)
    {
    }

uint64_t Knight::getAllTargets(uint64_t knights, Position &position)
    {
    uint64_t targets = Moves::knightAttacks[(bitScanForward(knights))];
    return targets & ~position.ourPieces();
    }

uint64_t Knight::targetsFrom(uint8_t square, uint8_t color, Position &position)
    {
    uint64_t targets = Moves::knightAttacks[square];
    return targets & ~position.Pieces(color);
    }

uint64_t Knight::getKnightAttacks(uint64_t knights)
    {
    uint64_t west, east, attacks;
    east = Direction::oneStepEast(knights);
    west = Direction::oneStepWest(knights);
    attacks = (east | west) << 16;
    attacks |= (east | west) >> 16;
    east = Direction::oneStepEast(east);
    west = Direction::oneStepWest(west);
    attacks |= (east | west) << 8;
    attacks |= (east | west) >> 8;
    return attacks;
    }

uint64_t Bishop::getAllTargets(uint64_t bishops, Position &position)
    {
    uint64_t occupiedSquares = position.occupiedSquares;
    uint64_t targets = Empty;
    uint8_t square = bitScanForward(bishops);
    targets |= Moves::getA1H8DiagonalAttacks(occupiedSquares, square);
    targets |= Moves::getH1A8DiagonalAttacks(occupiedSquares, square);
    return targets & ~position.ourPieces();
    }

uint64_t Bishop::targetsFrom(uint8_t square, uint8_t color, Position &position)
    {
    uint64_t occupiedSquares = position.occupiedSquares;
    uint64_t targets = Empty;
    targets |= Moves::getA1H8DiagonalAttacks(occupiedSquares, square);
    targets |= Moves::getH1A8DiagonalAttacks(occupiedSquares, square);
    return targets & ~position.Pieces(color);
    }

uint64_t Rook::getAllTargets(uint64_t rooks, Position &position)
    {
    uint64_t occupiedSquares = position.occupiedSquares;
    uint64_t targets = Empty;
    uint8_t square = bitScanForward(rooks);
#ifdef PEXT
    targets = Moves::getRookAttacks(occupiedSquares, square);
#else
    targets |= Moves::getRankAttacks(occupiedSquares, square);
    targets |= Moves::getFileAttacks(occupiedSquares, square);
#endif
    return targets & ~position.ourPieces();
    }

INLINE uint64_t Rook::targetsFrom(uint8_t square, uint8_t color, Position &position)
    {
    uint64_t occupiedSquares = position.occupiedSquares;
    uint64_t targets = Empty;
#ifdef PEXT
    targets = Moves::getRookAttacks(occupiedSquares, square);
#else
    targets |= Moves::getRankAttacks(occupiedSquares, square);
    targets |= Moves::getFileAttacks(occupiedSquares, square);
#endif
    return targets & ~position.Pieces(color);
    }

uint64_t Queen::getAllTargets(uint64_t queens, Position &position)
    {
    return Rook::getAllTargets(queens, position) | Bishop::getAllTargets(queens, position);
    }

uint64_t Queen::targetsFrom(uint8_t square, uint8_t color, Position &position)
    {
    return Rook::targetsFrom(square, color, position) | Bishop::targetsFrom(square, color, position);
    }

uint64_t King::getAllTargets(uint64_t king, Position &position)
    {
    uint64_t kingMoves = Moves::kingAttacks[(bitScanForward(king))];
    return kingMoves & ~position.ourPieces();
    }

uint64_t King::getKingAttacks(uint64_t king)
    {
    uint64_t attacks = Direction::oneStepEast(king) | Direction::oneStepWest(king);
    king |= attacks;
    attacks |= Direction::oneStepNorth(king) | Direction::oneStepSouth(king);
    return attacks;
    }