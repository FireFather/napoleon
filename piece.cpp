#include "piece.h"
#include "direction.h"
#include "position.h"

namespace Piece
    {
    uint8_t GetPiece( char initial )
        {
        switch( initial )
            {
            case 'b':
                return PieceType::Bishop;

            case 'n':
                return PieceType::Knight;

            case 'q':
                return PieceType::Queen;

            case 'r':
                return PieceType::Rook;

            default:
                throw std::exception();  
            }
        }

    char GetType( uint8_t type )
        {
        switch( type )
            {
            case PieceType::Bishop:
                return 'b';

            case PieceType::King:
                return 'k';

            case PieceType::Knight:
                return 'n';

            case PieceType::Pawn:
                return 'p';

            case PieceType::Queen:
                return 'q';

            case PieceType::Rook:
                return 'r';

            default:
                throw std::exception();  
            }
        }

	char GetColor(uint8_t color)
	{
		switch (color)
		{
		case PieceColor::White:
			return 'w';

		case PieceColor::Black:
			return 'b';

		default:
			throw std::exception();
		}
	}

    char GetInitial( PieceInfo piece )
        {
        if (piece.Color == PieceColor::White)
            {
            switch( piece.Type )
                {
                case PieceType::Bishop:
                    return 'B';

                case PieceType::King:
                    return 'K';

                case PieceType::Knight:
                    return 'N';

                case PieceType::Pawn:
                    return 'P';

                case PieceType::Queen:
                    return 'Q';

                case PieceType::Rook:
                    return 'R';

                default:
                    throw std::exception();  
                }
            }
        else
            {
            switch( piece.Type )
                {
                case PieceType::Bishop:
                    return 'b';

                case PieceType::King:
                    return 'k';

                case PieceType::Knight:
                    return 'n';

                case PieceType::Pawn:
                    return 'p';

                case PieceType::Queen:
                    return 'q';

                case PieceType::Rook:
                    return 'r';

                default:
                    throw std::exception();  
                }
            }
        }
    }

PieceInfo::PieceInfo( uint8_t color, uint8_t type ) :
    Color(color),
    Type(type)
    {
    }

PieceInfo::PieceInfo()  noexcept :
    Color(PieceColor::None),
    Type(PieceType::None)
    {
    }

uint64_t Knight::GetAllTargets(uint64_t knights, Position &position)
{
	uint64_t targets = Moves::KnightAttacks[(BitScanForward(knights))];
	return targets & ~position.PlayerPieces();
}

uint64_t Knight::TargetsFrom(uint8_t square, uint8_t color, Position &position)
{
	uint64_t targets = Moves::KnightAttacks[square];
	return targets & ~position.Pieces(color);
}

uint64_t Knight::GetKnightAttacks(uint64_t knights)
{
	uint64_t west, east, attacks;
	east = Direction::OneStepEast(knights);
	west = Direction::OneStepWest(knights);
	attacks = (east | west) << 16;
	attacks |= (east | west) >> 16;
	east = Direction::OneStepEast(east);
	west = Direction::OneStepWest(west);
	attacks |= (east | west) << 8;
	attacks |= (east | west) >> 8;

	return attacks;
}

INLINE uint64_t Bishop::GetAllTargets(uint64_t bishops, Position &position)
{
	uint64_t occupiedSquares = position.OccupiedSquares;
	uint64_t targets = Empty;
	uint8_t square = BitScanForward(bishops);
	targets |= Moves::GetA1H8DiagonalAttacks(occupiedSquares, square);
	targets |= Moves::GetH1A8DiagonalAttacks(occupiedSquares, square);
	return targets & ~position.PlayerPieces();
}

INLINE uint64_t Bishop::TargetsFrom(uint8_t square, uint8_t color, Position &position)
{
	uint64_t occupiedSquares = position.OccupiedSquares;
	uint64_t targets = Empty;
	targets |= Moves::GetA1H8DiagonalAttacks(occupiedSquares, square);
	targets |= Moves::GetH1A8DiagonalAttacks(occupiedSquares, square);
	return targets & ~position.Pieces(color);
}

INLINE uint64_t Rook::GetAllTargets(uint64_t rooks, Position &position)
{
	uint64_t occupiedSquares = position.OccupiedSquares;
	uint64_t targets = Empty;
	uint8_t square = BitScanForward(rooks);

#ifdef PEXT
	targets = Moves::GetRookAttacks(occupiedSquares, square);
#else
	targets |= Moves::GetRankAttacks(occupiedSquares, square);
	targets |= Moves::GetFileAttacks(occupiedSquares, square);
#endif
	return targets & ~position.PlayerPieces();
}

INLINE uint64_t Rook::TargetsFrom(uint8_t square, uint8_t color, Position &position)
{
	uint64_t occupiedSquares = position.OccupiedSquares;
	uint64_t targets = Empty;

#ifdef PEXT
	targets = Moves::GetRookAttacks(occupiedSquares, square);
#else
	targets |= Moves::GetRankAttacks(occupiedSquares, square);
	targets |= Moves::GetFileAttacks(occupiedSquares, square);
#endif
	return targets & ~position.Pieces(color);
}

uint64_t Queen::GetAllTargets(uint64_t queens, Position &position)
{
	return Rook::GetAllTargets(queens, position) | Bishop::GetAllTargets(queens, position);
}

uint64_t Queen::TargetsFrom(uint8_t square, uint8_t color, Position &position)
{
	return Rook::TargetsFrom(square, color, position) | Bishop::TargetsFrom(square, color, position);
}

uint64_t King::GetAllTargets(uint64_t king, Position &position)
{
	uint64_t kingMoves = Moves::KingAttacks[(BitScanForward(king))];
	return kingMoves & ~position.PlayerPieces();
}

uint64_t King::GetKingAttacks(uint64_t king)
{
	uint64_t attacks = Direction::OneStepEast(king) | Direction::OneStepWest(king);
	king |= attacks;
	attacks |= Direction::OneStepNorth(king) | Direction::OneStepSouth(king);
	return attacks;
}