#include "eval.h"
#include "piece.h"
#include "castle.h"
#include "evalterms.h"

int Eval::Evaluate( Position &position )
    {
    Score scores(0, 0);
    Score wPstValues, bPstValues;
    auto wking_square = position.KingSquare(PieceColor::White);
    auto bking_square = position.KingSquare(PieceColor::Black);

    uint64_t king_proximity[2] =
        {
        Moves::KingProximity[PieceColor::White][wking_square], Moves::KingProximity[PieceColor::Black][bking_square]
        };

    int material = position.MaterialBalance(PieceColor::White);

    // PST evaluation
    wPstValues = position.PstValue(PieceColor::White);
    bPstValues = position.PstValue(PieceColor::Black);

    updateScore(scores, material + (wPstValues.first - bPstValues.first) / 2,
        material + (wPstValues.second - bPstValues.second) / 2);

    // phase dependant piece bonus
    int wPawns = position.NumOfPieces(PieceColor::White, Pawn);
    int bPawns = position.NumOfPieces(PieceColor::Black, Pawn);
    updateScore(scores, PawnBonus[Opening] * wPawns, PawnBonus[EndGame] * wPawns);
    updateScore(scores, -PawnBonus[Opening] * bPawns, -PawnBonus[EndGame] * bPawns);

    int wKnights = position.NumOfPieces(PieceColor::White, Knight);
    int bKnights = position.NumOfPieces(PieceColor::Black, Knight);
    updateScore(scores, KnightBonus[Opening] * wKnights, KnightBonus[EndGame] * wKnights);
    updateScore(scores, -KnightBonus[Opening] * bKnights, -KnightBonus[EndGame] * bKnights);

    int wRooks = position.NumOfPieces(PieceColor::White, Rook);
    int bRooks = position.NumOfPieces(PieceColor::Black, Rook);
    updateScore(scores, RookBonus[Opening] * wRooks, RookBonus[EndGame] * wRooks);
    updateScore(scores, -RookBonus[Opening] * bRooks, -RookBonus[EndGame] * bRooks);

    // premature queen development penalty
    if (!position.IsOnSquare(PieceColor::White, PieceType::Queen, Square::D1))
        updateScore(scores, -QueenPenaltyOpening, -QueenPenaltyEndGame);
    if (!position.IsOnSquare(PieceColor::Black, PieceType::Queen, Square::D8))
        updateScore(scores, QueenPenaltyOpening, QueenPenaltyEndGame);

    // tempo bonus
    if (position.SideToMove() == PieceColor::White)
        updateScore(scores, TempoBonus);
    else
        updateScore(scores, -TempoBonus);

    // bishop pair bonus
    if (position.NumOfPieces(PieceColor::White, PieceType::Bishop) == 2)
        updateScore(scores, BishopPair[Opening], BishopPair[EndGame]);

    if (position.NumOfPieces(PieceColor::Black, PieceType::Bishop) == 2)
        updateScore(scores, -BishopPair[Opening], -BishopPair[EndGame]);

    // doubled/isolated pawns
    uint64_t wpawns = position.Pieces(PieceColor::White, Pawn);
    uint64_t bpawns = position.Pieces(PieceColor::Black, Pawn);

    for ( uint8_t f = 0; f < 8; f++ )
        {
        int pawns;

        if ((pawns = position.PawnsOnFile(PieceColor::White, f)))
            {
            if (!(wpawns &Moves::SideFiles[f]))
                updateScore(scores, -isolatedPawn[f]);

            updateScore(scores, -multiPawn[pawns]);

            if (f < 7)
                updateScore(scores, -multiPawn[position.PawnsOnFile(PieceColor::White, ++f)]);
            }
        }

    for ( uint8_t f = 0; f < 8; f++ )
        {
        int pawns;

        if ((pawns = position.PawnsOnFile(PieceColor::Black, f)))
            {
            if (!(bpawns &Moves::SideFiles[f]))
                updateScore(scores, isolatedPawn[f]);

            updateScore(scores, multiPawn[pawns]);

            if (f < 7)
                updateScore(scores, multiPawn[position.PawnsOnFile(PieceColor::Black, ++f)]);
            }
        }

    // mobility evaluation
    PieceInfo piece;
    auto pieceList = position.PieceList();

    for ( uint8_t sq = Square::A1; sq <= Square::H8; sq++ )
        {
        piece = pieceList[sq];

        if (piece.Type != PieceType::None)
            {
            if (piece.Type == Pawn)
                {
                if ((Moves::PasserSpan[piece.Color][sq] & position.Pieces(Piece::GetOpposite(piece.Color), Pawn)) == 0)
                    {
                    auto rank = Square::GetRankIndex(sq);

                    if (piece.Color == PieceColor::White)
                        updateScore(scores, passedPawn[Opening][rank], passedPawn[EndGame][rank]);
                    else
                        updateScore(scores, -passedPawn[Opening][7 - rank], -passedPawn[EndGame][7 - rank]);
                    }
                }
            else if (piece.Color == PieceColor::White)
                updateScore(scores, EvaluatePiece(piece, sq, king_proximity[PieceColor::Black], position));
            else
                updateScore(scores, -EvaluatePiece(piece, sq, king_proximity[PieceColor::White], position));
            }
        }

    //king safety
    int shelter1 = 0, shelter2 = 0;

    //pawn shelter
    uint64_t pawns = position.Pieces(PieceColor::White, Pawn);

    if (Masks::SquareMask[wking_square] & Castle::WhiteKingSide)
        {
        shelter1 = PopCount(pawns & Castle::WhiteKingShield);
        shelter2 = PopCount(pawns & Direction::OneStepNorth(Castle::WhiteKingShield));
        }
    else if (Masks::SquareMask[wking_square] & Castle::WhiteQueenSide)
        {
        shelter1 = PopCount(pawns & Castle::WhiteQueenShield);
        shelter2 = PopCount(pawns & Direction::OneStepNorth(Castle::WhiteQueenShield));
        }
    updateScore(scores, shelter1 * 4 + shelter2 * 3, shelter1 * 2 + shelter2 * 3); // shelter bonus

    pawns = position.Pieces(PieceColor::Black, Pawn);

    if (Masks::SquareMask[bking_square] & Castle::BlackKingSide)
        {
        shelter1 = PopCount(pawns & Castle::BlackKingShield);
        shelter2 = PopCount(pawns & Direction::OneStepSouth(Castle::BlackKingShield));
        }
    else if (Masks::SquareMask[bking_square] & Castle::BlackQueenSide)
        {
        shelter1 = PopCount(pawns & Castle::BlackQueenShield);
        shelter2 = PopCount(pawns & Direction::OneStepSouth(Castle::BlackQueenShield));
        }
    updateScore(scores, -(shelter1 * 4 + shelter2 * 3), -(shelter1 * 2 + shelter2 * 3)); // shelter bonus

    int opening = scores.first;
    int endgame = scores.second;
    int phase = position.Phase();
    int score = ((opening * (MaxPhase - phase)) + (endgame * phase)) / MaxPhase;
    return score * (1 - (position.SideToMove() * 2)); // score relative to side to move
    }

int Eval::EvaluatePiece( PieceInfo piece, uint8_t square, uint64_t king_proxy, Position &position )
    {
	uint64_t Universe = 0xFFFFFFFFFFFFFFFF;
    uint8_t us = piece.Color;
    uint8_t enemy = Piece::GetOpposite(us);
    uint64_t b = 0;
    int tropism = 0;
    int distance = 7; 
    uint8_t ksq = position.KingSquare(enemy);

    switch( piece.Type )
        {
        case PieceType::Knight:
            b = Knight::TargetsFrom(square, us, position) & ~Pawn::GetAnyAttack(position.Pieces(enemy, PieceType::Pawn), enemy, Universe);
				// exclude squares controlled by enemy pawns
            tropism = KnightTropism;
            distance = Moves::Distance[square][ksq];
            break;
        case PieceType::Bishop:
            b = Bishop::TargetsFrom(square, us, position);
            tropism = BishopTropism; // TO TEST: divide by distance to king
            distance = Moves::Distance[square][ksq] * 2;
            break;
        case PieceType::Rook:
            b = Rook::TargetsFrom(square, us, position);
            tropism = RookTropism;
            distance = Moves::Distance[square][ksq] * 2;
            break;
        case PieceType::Queen:
            b = Queen::TargetsFrom(square, us, position);
            tropism = QueenTropism;
            distance = Moves::Distance[square][ksq] / 2;
            break;
        }

    int count = PopCount(b);
    tropism = tropism * PopCount(king_proxy & b) + (7 - distance);
    tropism = int(((float)tropism*position.Material(us)) / (float)MaxPlayerMat); // tropism scaling

    return mobilityBonus[piece.Type][count] + tropism;
    }