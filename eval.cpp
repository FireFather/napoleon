#include "eval.h"
#include "piece.h"
#include "castle.h"
#include "evalterms.h"

int Eval::evaluate(Position &position)
    {
    Score score(0, 0);
    Score wPstScore, bPstScore;
    auto wking_square = position.getKingSquare(pieceColor::White);
    auto bking_square = position.getKingSquare(pieceColor::Black);

    int material = position.materialScore(pieceColor::White);

	// bishop pair bonus
	if (position.getNumPieces(pieceColor::White, pieceType::Bishop) == 2)
		updateScore(score, bishopPair[OP], bishopPair[EG]);

	if (position.getNumPieces(pieceColor::Black, pieceType::Bishop) == 2)
		updateScore(score, -bishopPair[OP], -bishopPair[EG]);

    // PST evaluation
    wPstScore = position.getPstScore(pieceColor::White);
    bPstScore = position.getPstScore(pieceColor::Black);

    updateScore(score, material + (wPstScore.first - bPstScore.first), material + (wPstScore.second - bPstScore.second));

	// tempo bonus
	if (position.getSideToMove() == pieceColor::White)
		updateScore(score, tempoBonus[OP], tempoBonus[EG]);
	else
		updateScore(score, -tempoBonus[OP], -tempoBonus[EG]);

    // premature queen development penalty
    if (!position.isOnSquare(pieceColor::White, pieceType::Queen, Square::D1))
		updateScore(score, -queenPenalty[OP], -queenPenalty[EG]);
    if (!position.isOnSquare(pieceColor::Black, pieceType::Queen, Square::D8))
		updateScore(score, queenPenalty[OP], queenPenalty[EG]);

    // doubled/isolated pawns
    uint64_t wpawns = position.Pieces(pieceColor::White, Pawn);
    uint64_t bpawns = position.Pieces(pieceColor::Black, Pawn);

    for (uint8_t file = 0; file < 8; file++)
        {
        int pawns;

        if ((pawns = position.getPawnsOnFile(pieceColor::White, file)))
            {
            if (!(wpawns &Moves::adjacentFiles[file]))
				updateScore(score, -isolatedPawn[OP], -isolatedPawn[EG]);

			if (pawns > 1)
				updateScore(score, -doubledPawn[OP], -doubledPawn[EG]);
            }
        }

    for (uint8_t file = 0; file < 8; file++)
        {
        int pawns;

        if ((pawns = position.getPawnsOnFile(pieceColor::Black, file)))
            {
            if (!(bpawns &Moves::adjacentFiles[file]))
				updateScore(score, isolatedPawn[OP], isolatedPawn[EG]);

			if (pawns > 1)
				updateScore(score, doubledPawn[OP], doubledPawn[EG]);
            }
        }

    pieceInfo piece;
    auto pieceList = position.pieceList();

    for (uint8_t sq = Square::A1; sq <= Square::H8; sq++)
        {
        piece = pieceList[sq];

        if (piece.Type != pieceType::noType)
            { // passed pawns
            if (piece.Type == Pawn)
                {
                if ((Moves::passerSpan[piece.Color][sq] & position.Pieces(Piece::getOpposite(piece.Color), Pawn)) == 0)
                    {
                    auto rank = Square::getRankIndex(sq);

                    if (piece.Color == pieceColor::White)
                        updateScore(score, passedPawn[OP][rank], passedPawn[EG][rank]);
                    else
                        updateScore(score, -passedPawn[OP][7 - rank], -passedPawn[EG][7 - rank]);
                    }
                }
			else
				{
				uint64_t allSquares = 0xFFFFFFFFFFFFFFFF;
				uint8_t us = piece.Color;
				uint8_t them = Piece::getOpposite(us);
				uint64_t b = 0;

				// mobility
				switch (piece.Type)
					{
					case pieceType::Knight:
						b = Knight::targetsFrom(sq, us, position) & ~Pawn::getAnyAttack(position.Pieces(them, pieceType::Pawn), them, allSquares);
						break;
					case pieceType::Bishop:
						b = Bishop::targetsFrom(sq, us, position) & ~Pawn::getAnyAttack(position.Pieces(them, pieceType::Pawn), them, allSquares);
						break;
					case pieceType::Rook:
						b = Rook::targetsFrom(sq, us, position) & ~Pawn::getAnyAttack(position.Pieces(them, pieceType::Pawn), them, allSquares);
						break;
					case pieceType::Queen:
						b = Queen::targetsFrom(sq, us, position) & ~Pawn::getAnyAttack(position.Pieces(them, pieceType::Pawn), them, allSquares);
						break;
					}

				int count = popCount(b);
				if (piece.Color == pieceColor::White)
					updateScore(score, mobilityBonus[OP][piece.Type][count], mobilityBonus[EG][piece.Type][count]);
				else
					updateScore(score, -mobilityBonus[OP][piece.Type][count], -mobilityBonus[EG][piece.Type][count]);
				}
            }
        }

    // pawn shelter
	int shelter1 = 0, shelter2 = 0;
    uint64_t pawns = position.Pieces(pieceColor::White, Pawn);

    if (Masks::squareMask[wking_square] & Castle::whiteKingSide)
        {
        shelter1 = popCount(pawns & Castle::whiteKingShield);
        shelter2 = popCount(pawns & Direction::oneStepNorth(Castle::whiteKingShield));
        }
    else if (Masks::squareMask[wking_square] & Castle::whiteQueenSide)
        {
        shelter1 = popCount(pawns & Castle::whiteQueenShield);
        shelter2 = popCount(pawns & Direction::oneStepNorth(Castle::whiteQueenShield));
        }
    updateScore(score,
		shelter1 * pawnShelter[OP][0] + shelter2 * pawnShelter[OP][1],
		shelter1 * pawnShelter[EG][0] + shelter2 * pawnShelter[EG][1]);

    pawns = position.Pieces(pieceColor::Black, Pawn);

    if (Masks::squareMask[bking_square] & Castle::blackKingSide)
        {
        shelter1 = popCount(pawns & Castle::blackKingShield);
        shelter2 = popCount(pawns & Direction::oneStepSouth(Castle::blackKingShield));
        }
    else if (Masks::squareMask[bking_square] & Castle::blackQueenSide)
        {
        shelter1 = popCount(pawns & Castle::blackQueenShield);
        shelter2 = popCount(pawns & Direction::oneStepSouth(Castle::blackQueenShield));
        }
    updateScore(score,
		-(shelter1 * pawnShelter[OP][0] + shelter2 * pawnShelter[OP][1]),
		-(shelter1 * pawnShelter[EG][0] + shelter2 * pawnShelter[EG][1]));

    int opening = score.first;
    int endgame = score.second;
    int phase = position.Phase();
    int finalScore = ((opening * (maxPhase - phase)) + (endgame * phase)) / maxPhase;
    return finalScore * (1 - (position.getSideToMove() * 2));
    }
