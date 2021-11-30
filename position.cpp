#include "piece.h"
#include "fen.h"
#include "eval.h"
#include "castle.h"

Position::Position() noexcept
    {
    Moves::InitAttacks();
    Zobrist::Init();

    pieces[PieceColor::White] = Empty;
    pieces[PieceColor::Black] = Empty;
    OccupiedSquares = Empty;
    EmptySquares = Empty;

    for ( uint8_t c = PieceColor::White; c < PieceColor::None; c++ )
        for ( uint8_t f = 0; f < 8; f++ )
            pawnsOnFile[c][f] = 0;
    }

void Position::LoadFen( std::string pos )
    {
    Fen fenString(pos);

    for ( uint8_t c = PieceColor::White; c < PieceColor::None; c++ )
        for ( uint8_t t = PieceType::Pawn; t < PieceType::None; t++ )
            numOfPieces[c][t] = 0;

    for ( uint8_t c = PieceColor::White; c < PieceColor::None; c++ )
        for ( uint8_t f = 0; f < 8; f++ )
            pawnsOnFile[c][f] = 0;

    material[PieceColor::White] = 0;
    material[PieceColor::Black] = 0;
    allowNullMove = true;
    currentPly = 0;
    zobrist = 0;
    initializeCastlingStatus(fenString);
    initializesideToMove(fenString);
    initializePieceSet(fenString);
    initializeEnPassantSquare(fenString);
    initializeBitBoards(fenString);

    pstValue[PieceColor::White] = calculatePST(PieceColor::White);
    pstValue[PieceColor::Black] = calculatePST(PieceColor::Black);
    }

Score Position::calculatePST( uint8_t color ) const
    {
    PieceInfo piece;
    int pst[2][2] =
        {
        { 0 }
        };

    for ( uint8_t sq = Square::A1; sq <= Square::H8; sq++ )
        {
        piece = PieceOnSquare(sq);

        if (piece.Type != PieceType::None)
            {
            Score scores = Eval::PieceSquareValue(piece, sq);
            pst[piece.Color][0] += scores.first;
            pst[piece.Color][1] += scores.second;
            }
        }

    return std::make_pair(pst[color][0], pst[color][1]);
    }

void Position::AddPiece( PieceInfo piece, uint8_t sq )
    {
    pieceSet[sq] = piece;

    if (piece.Type != PieceType::None)
        {
        numOfPieces[piece.Color][piece.Type]++;
        material[piece.Color] += PieceValue[piece.Type];
        zobrist ^= Zobrist::PieceInfo[piece.Color][piece.Type][sq];

        if (piece.Type == PieceType::Pawn)
            pawnsOnFile[piece.Color][Square::GetFileIndex(sq)]++;
        }
    }

void Position::clearPieceSet()
    {
    for ( uint8_t i = 0; i < 64; i++ )
        {
        pieceSet[i] = Null;
        }
    }

void Position::updateGenericBitBoards()
    {
    pieces[PieceColor::White] =
        bitBoardSet[PieceColor::White][PieceType::Pawn] | bitBoardSet[PieceColor::White][PieceType::Knight]
            | bitBoardSet[PieceColor::White][PieceType::Bishop] | bitBoardSet[PieceColor::White][PieceType::Rook]
            | bitBoardSet[PieceColor::White][PieceType::Queen] | bitBoardSet[PieceColor::White][PieceType::King];

    pieces[PieceColor::Black] =
        bitBoardSet[PieceColor::Black][PieceType::Pawn] | bitBoardSet[PieceColor::Black][PieceType::Knight]
            | bitBoardSet[PieceColor::Black][PieceType::Bishop] | bitBoardSet[PieceColor::Black][PieceType::Rook]
            | bitBoardSet[PieceColor::Black][PieceType::Queen] | bitBoardSet[PieceColor::Black][PieceType::King];

    OccupiedSquares = pieces[PieceColor::White] | pieces[PieceColor::Black];
    EmptySquares = ~OccupiedSquares;
    }

void Position::initializeCastlingStatus( const Fen &fenString )
    {
    castlingStatus = 0;

    if (fenString.WhiteCanCastleShort)
        castlingStatus |= Castle::WhiteCastleOO;

    if (fenString.WhiteCanCastleLong)
        castlingStatus |= Castle::WhiteCastleOOO;

    if (fenString.BlackCanCastleShort)
        castlingStatus |= Castle::BlackCastleOO;

    if (fenString.BlackCanCastleLong)
        castlingStatus |= Castle::BlackCastleOOO;

    zobrist ^= Zobrist::Castling[castlingStatus];
    }

void Position::initializesideToMove( const Fen &fenString )
    {
    sideToMove = fenString.sideToMove;

    if (sideToMove == PieceColor::Black)
        zobrist ^= Zobrist::Color;
    }

void Position::initializePieceSet( const Fen &fenString )
    {
    for ( uint8_t i = 0; i < 64; i++ )
        {
        AddPiece(fenString.PiecePlacement[i], i);
        }
    }

void Position::initializeEnPassantSquare( const Fen &fenString )
    {
    enPassantSquare = fenString.EnPassantSquare;

    if (enPassantSquare != Square::None)
        zobrist ^= Zobrist::Enpassant[Square::GetFileIndex(enPassantSquare)];
    }

void Position::initializeHalfMoveClock( const Fen &fenString )
    {
    halfMoveClock = fenString.HalfMove;
    }

void Position::initializeBitBoards( const Fen &fenString )
    {
    for ( uint8_t i = PieceType::Pawn; i < PieceType::None; i++ )
        for ( uint8_t l = PieceColor::White; l < PieceColor::None; l++ )
            bitBoardSet[l][i] = 0;

    for ( uint8_t i = 0; i < 64; i++ )
        {
        if (fenString.PiecePlacement[i].Type == PieceType::King)
            kingSquare[fenString.PiecePlacement[i].Color] = i;

        if (fenString.PiecePlacement[i].Color != PieceColor::None)
            bitBoardSet[fenString.PiecePlacement[i].Color][fenString.PiecePlacement[i].Type] |= Masks::SquareMask[i];
        }

    updateGenericBitBoards();
    }

Move Position::ParseMove( std::string str ) const
    {
    uint8_t from = Square::Parse(str.substr(0, 2));
    uint8_t to = Square::Parse(str.substr(2));
    Move move;

    if (to == enPassantSquare && pieceSet[from].Type == PieceType::Pawn)
        move = Move(from, to, EnPassant);

    else if (str == "e1g1")
        move = Castle::WhiteCastlingOO;

    else if (str == "e8g8")
        move = Castle::BlackCastlingOO;

    else if (str == "e1c1")
        move = Castle::WhiteCastlingOOO;

    else if (str == "e8c8")
        move = Castle::BlackCastlingOOO;

    else if (str.size() == 5)
        move = Move(from, to, 0x8 | (Piece::GetPiece(str[4]) - 1));

    else
        move = Move(from, to);

    return move;
    }

void Position::MakeMove( Move move )
    {
    bool incrementClock = true;

    uint8_t from = move.FromSquare();
    uint8_t to = move.ToSquare();
    uint8_t promoted;
    uint8_t captured = move.IsEnPassant() ? static_cast<uint8_t>(PieceType::Pawn) : pieceSet[to].Type;
    uint8_t pieceMoved = pieceSet[from].Type;
    uint8_t enemy = Piece::GetOpposite(sideToMove);

    bool capture = captured != PieceType::None;

    castlingStatusHistory[currentPly] = castlingStatus;       
    enpSquaresHistory[currentPly] = enPassantSquare;        
    halfMoveClockHistory[currentPly] = halfMoveClock;        
    hashHistory[currentPly] = zobrist;            
    capturedPieceHistory[currentPly] = captured;            

    zobrist ^= Zobrist::Color;                               

    pieceSet[to] = pieceSet[from];       
    pieceSet[from] = Null;      

    updatePstvalue<Sub>(sideToMove, Eval::PieceSquareValue(PieceInfo(sideToMove, pieceMoved), from));
    updatePstvalue<Add>(sideToMove, Eval::PieceSquareValue(PieceInfo(sideToMove, pieceMoved), to));

    uint64_t From = Masks::SquareMask[from];
    uint64_t To = Masks::SquareMask[to];
    uint64_t FromTo = From | To;

    bitBoardSet[sideToMove][pieceMoved] ^= FromTo;
    zobrist ^= Zobrist::PieceInfo[sideToMove][pieceMoved][from];        
    zobrist ^= Zobrist::PieceInfo[sideToMove][pieceMoved][to];          

    pieces[sideToMove] ^= FromTo;

    if (pieceMoved == PieceType::King)
        {
        kingSquare[sideToMove] = to;

        if (move.IsCastle())
            {
            makeCastle(from, to);
            }

        if (sideToMove == PieceColor::White)
            castlingStatus &=
                ~(Castle::WhiteCastleOO | Castle::WhiteCastleOOO);         
        else
            castlingStatus &=
                ~(Castle::BlackCastleOO | Castle::BlackCastleOOO);         
        }
    else if (pieceMoved == PieceType::Rook)            
        {
        if (castlingStatus)                                              
            {
            if (sideToMove == PieceColor::White)
                {
                if (from == Square::A1)
                    castlingStatus &= ~Castle::WhiteCastleOOO;

                else if (from == Square::H1)
                    castlingStatus &= ~Castle::WhiteCastleOO;
                }
            else
                {
                if (from == Square::A8)
                    castlingStatus &= ~Castle::BlackCastleOOO;

                else if (from == Square::H8)
                    castlingStatus &= ~Castle::BlackCastleOO;
                }
            }
        }
    else if (move.IsPromotion())
        {
        promoted = move.PiecePromoted();
        pieceSet[to] = PieceInfo(sideToMove, promoted);
        bitBoardSet[sideToMove][PieceType::Pawn] ^= To;
        bitBoardSet[sideToMove][promoted] ^= To;
        numOfPieces[sideToMove][PieceType::Pawn]--;
        numOfPieces[sideToMove][promoted]++;

        material[sideToMove] -= PieceValue[PieceType::Pawn];
        material[sideToMove] += PieceValue[promoted];
        zobrist ^= Zobrist::PieceInfo[sideToMove][PieceType::Pawn][to];
        zobrist ^= Zobrist::PieceInfo[sideToMove][promoted][to];
        updatePstvalue<Sub>(sideToMove, Eval::PieceSquareValue(PieceInfo(sideToMove, PieceType::Pawn), to));
        updatePstvalue<Add>(sideToMove, Eval::PieceSquareValue(PieceInfo(sideToMove, promoted), to));

        if (!capture)
            pawnsOnFile[sideToMove][Square::GetFileIndex(from)]--;
        else
            pawnsOnFile[sideToMove][Square::GetFileIndex(to)]--;
        }

    if (capture)
        {
        if (move.IsEnPassant())
            {
            uint64_t piece;

            if (sideToMove == PieceColor::White)
                {
                piece = Masks::SquareMask[enPassantSquare - 8];
                pieceSet[enPassantSquare - 8] = Null;
                updatePstvalue
                    <Sub>(enemy, Eval::PieceSquareValue(PieceInfo(enemy, PieceType::Pawn), enPassantSquare - 8));
                zobrist ^= Zobrist::PieceInfo[enemy][PieceType::Pawn][enPassantSquare
                    - 8];        
                }
            else
                {
                piece = Masks::SquareMask[enPassantSquare + 8];
                pieceSet[enPassantSquare + 8] = Null;
                updatePstvalue
                    <Sub>(enemy, Eval::PieceSquareValue(PieceInfo(enemy, PieceType::Pawn), enPassantSquare + 8));
                zobrist ^= Zobrist::PieceInfo[enemy][PieceType::Pawn][enPassantSquare
                    + 8];        
                }

            pieces[enemy] ^= piece;
            bitBoardSet[enemy][PieceType::Pawn] ^= piece;
            OccupiedSquares ^= FromTo ^ piece;
            EmptySquares ^= FromTo ^ piece;

            pawnsOnFile[sideToMove][Square::GetFileIndex(from)]--;
            pawnsOnFile[sideToMove][Square::GetFileIndex(to)]++;
            pawnsOnFile[enemy][Square::GetFileIndex(to)]--;
            }
        else
            {
            if (captured == PieceType::Rook)
                {
                if (enemy == PieceColor::White)
                    {
                    if (to == Square::H1)
                        castlingStatus &= ~Castle::WhiteCastleOO;

                    else if (to == Square::A1)
                        castlingStatus &= ~Castle::WhiteCastleOOO;
                    }
                else
                    {
                    if (to == Square::H8)
                        castlingStatus &= ~Castle::BlackCastleOO;

                    else if (to == Square::A8)
                        castlingStatus &= ~Castle::BlackCastleOOO;
                    }
                }
            else if (captured == PieceType::Pawn)
                {
                pawnsOnFile[enemy][Square::GetFileIndex(to)]--;
                }

            if (pieceMoved == PieceType::Pawn)
                {
                pawnsOnFile[sideToMove][Square::GetFileIndex(from)]--;
                pawnsOnFile[sideToMove][Square::GetFileIndex(to)]++;
                }

            updatePstvalue<Sub>(enemy, Eval::PieceSquareValue(PieceInfo(enemy, captured), to));
            bitBoardSet[enemy][captured] ^= To;
            pieces[enemy] ^= To;    
            OccupiedSquares ^= From;
            EmptySquares ^= From;
            zobrist ^= Zobrist::PieceInfo[enemy][captured][to];     
            }

        numOfPieces[enemy][captured]--;
        material[enemy] -= PieceValue[captured];
        incrementClock = false;             
        }
    else
        {
        OccupiedSquares ^= FromTo;
        EmptySquares ^= FromTo;
        }

    if (enPassantSquare != Square::None)
        zobrist ^= Zobrist::Enpassant[Square::GetFileIndex(enPassantSquare)];

    enPassantSquare = Square::None;

    if (pieceMoved == PieceType::Pawn)
        {
        incrementClock = false;                
        int sq = to - from;           

        if (sq == 16 || sq == -16)     
            {
            enPassantSquare = to - sq / 2;
            zobrist ^= Zobrist::Enpassant[Square::GetFileIndex(enPassantSquare)];
            }
        }

    if (castlingStatusHistory[currentPly] != castlingStatus)
        zobrist ^= Zobrist::Castling[castlingStatus];      

    if (incrementClock)
        halfMoveClock++;                                 
    else
        halfMoveClock = 0;                               

    sideToMove = enemy;
    currentPly++;
    }

void Position::UndoMove( Move move )
    {
    uint8_t from = move.FromSquare();
    uint8_t to = move.ToSquare();
    uint8_t enemy = sideToMove;
    bool promotion = move.IsPromotion();
    bool capture;
    uint8_t promoted;
    uint8_t captured;
    uint8_t pieceMoved;

    currentPly--;

    captured = capturedPieceHistory[currentPly];
    capture = captured != PieceType::None;

    zobrist ^= Zobrist::Color;                             

    if (castlingStatusHistory[currentPly] != castlingStatus)
        zobrist ^= Zobrist::Castling[castlingStatus];      

    if (enPassantSquare != Square::None)
        zobrist ^= Zobrist::Enpassant[Square::GetFileIndex(enPassantSquare)];

    if (enpSquaresHistory[currentPly] != Square::None)
        zobrist ^= Zobrist::Enpassant[Square::GetFileIndex(enpSquaresHistory[currentPly])];

    halfMoveClock = halfMoveClockHistory[currentPly];

    if (promotion)
        pieceMoved = PieceType::Pawn;
    else
        pieceMoved = pieceSet[to].Type;

    sideToMove = Piece::GetOpposite(sideToMove);

    pieceSet[from] = pieceSet[to];    

    if (!promotion)
        {
        updatePstvalue<Sub>(sideToMove, Eval::PieceSquareValue(PieceInfo(sideToMove, pieceMoved), to));
        updatePstvalue<Add>(sideToMove, Eval::PieceSquareValue(PieceInfo(sideToMove, pieceMoved), from));
        }

    uint64_t From = Masks::SquareMask[from];
    uint64_t To = Masks::SquareMask[to];
    uint64_t FromTo = From | To;

    bitBoardSet[sideToMove][pieceMoved] ^= FromTo;
    zobrist ^= Zobrist::PieceInfo[sideToMove][pieceMoved][from];        
    zobrist ^= Zobrist::PieceInfo[sideToMove][pieceMoved][to];          

    pieces[sideToMove] ^= FromTo;

    if (pieceMoved == PieceType::King)
        {
        kingSquare[sideToMove] = from;

        if (move.IsCastle())
            {
            undoCastle(from, to);
            }

        castlingStatus = castlingStatusHistory[currentPly];         
        }
    else if (pieceMoved == PieceType::Rook)
        {
        castlingStatus = castlingStatusHistory[currentPly];
        }
    else if (promotion)
        {
        promoted = move.PiecePromoted();
        numOfPieces[sideToMove][PieceType::Pawn]++;
        numOfPieces[sideToMove][promoted]--;

        material[sideToMove] += PieceValue[PieceType::Pawn];
        material[sideToMove] -= PieceValue[promoted];
        pieceSet[from] = PieceInfo(sideToMove, PieceType::Pawn);
        bitBoardSet[sideToMove][promoted] ^= To;
        bitBoardSet[sideToMove][PieceType::Pawn] ^= To;
        zobrist ^= Zobrist::PieceInfo[sideToMove][PieceType::Pawn][to];
        zobrist ^= Zobrist::PieceInfo[sideToMove][promoted][to];
        updatePstvalue<Add>(sideToMove, Eval::PieceSquareValue(PieceInfo(sideToMove, PieceType::Pawn), from));
        updatePstvalue<Sub>(sideToMove, Eval::PieceSquareValue(PieceInfo(sideToMove, promoted), to));

        if (!capture)
            pawnsOnFile[sideToMove][Square::GetFileIndex(from)]++;
        else
            pawnsOnFile[sideToMove][Square::GetFileIndex(to)]++;
        }

    enPassantSquare = enpSquaresHistory[currentPly];

    if (capture)
        {
        if (move.IsEnPassant())
            {
            pieceSet[to] = Null;           
            uint64_t piece;
			const uint8_t offset = 8;

            if (sideToMove == PieceColor::White)
                {
                piece = Masks::SquareMask[enPassantSquare - offset];
                pieceSet[enPassantSquare - offset] = PieceInfo(PieceColor::Black, PieceType::Pawn);
                updatePstvalue <Add>(enemy, Eval::PieceSquareValue(PieceInfo(enemy, PieceType::Pawn), enPassantSquare - offset));
                zobrist ^= Zobrist::PieceInfo[enemy][PieceType::Pawn][enPassantSquare - offset];
                }
            else
                {
                piece = Masks::SquareMask[enPassantSquare + offset];
                pieceSet[enPassantSquare + offset] = PieceInfo(PieceColor::White, PieceType::Pawn);
                updatePstvalue <Add>(enemy, Eval::PieceSquareValue(PieceInfo(enemy, PieceType::Pawn), enPassantSquare + offset));
                zobrist ^= Zobrist::PieceInfo[enemy][PieceType::Pawn][enPassantSquare + offset];
                }

            pieces[enemy] ^= piece;
            bitBoardSet[enemy][PieceType::Pawn] ^= piece;
            OccupiedSquares ^= FromTo ^ piece;
            EmptySquares ^= FromTo ^ piece;

            pawnsOnFile[sideToMove][Square::GetFileIndex(from)]++;
            pawnsOnFile[sideToMove][Square::GetFileIndex(to)]--;
            pawnsOnFile[enemy][Square::GetFileIndex(to)]++;
            }
        else
            {
            if (captured == PieceType::Rook)
                {
                castlingStatus = castlingStatusHistory[currentPly];
                }
            else if (captured == PieceType::Pawn)
                {
                pawnsOnFile[enemy][Square::GetFileIndex(to)]++;
                }

            if (pieceMoved == PieceType::Pawn)
                {
                pawnsOnFile[sideToMove][Square::GetFileIndex(from)]++;
                pawnsOnFile[sideToMove][Square::GetFileIndex(to)]--;
                }

            updatePstvalue<Add>(enemy, Eval::PieceSquareValue(PieceInfo(enemy, captured), to));

            pieceSet[to] = PieceInfo(enemy, captured);
            bitBoardSet[enemy][captured] ^= To;

            pieces[enemy] ^= To;    
            OccupiedSquares ^= From;
            EmptySquares ^= From;

            zobrist ^= Zobrist::PieceInfo[enemy][captured][to];     
            }

        numOfPieces[enemy][captured]++;
        material[enemy] += PieceValue[captured];
        }
    else
        {
        pieceSet[to] = Null;
        OccupiedSquares ^= FromTo;
        EmptySquares ^= FromTo;
        }
    }

void Position::makeCastle( uint8_t from, uint8_t to )
    {
    uint64_t rook;
    uint8_t fromR;
    uint8_t toR;

    if (from < to)   
        {
        if (sideToMove == PieceColor::White)
            {
            fromR = Square::H1;
            toR = Square::F1;
            }
        else
            {
            fromR = Square::H8;
            toR = Square::F8;
            }
        }
    else   
        {
        if (sideToMove == PieceColor::White)
            {
            fromR = Square::A1;
            toR = Square::D1;
            }
        else
            {
            fromR = Square::A8;
            toR = Square::D8;
            }
        }

    rook = Masks::SquareMask[fromR] | Masks::SquareMask[toR];
    pieces[sideToMove] ^= rook;
    bitBoardSet[sideToMove][PieceType::Rook] ^= rook;
    OccupiedSquares ^= rook;
    EmptySquares ^= rook;
    pieceSet[fromR] = Null;                         
    pieceSet[toR] = PieceInfo(sideToMove, PieceType::Rook);    

    updatePstvalue<Sub>(sideToMove, Eval::PieceSquareValue(PieceInfo(sideToMove, PieceType::Rook), fromR));
    updatePstvalue<Add>(sideToMove, Eval::PieceSquareValue(PieceInfo(sideToMove, PieceType::Rook), toR));

    zobrist ^= Zobrist::PieceInfo[sideToMove][PieceType::Rook][fromR];
    zobrist ^= Zobrist::PieceInfo[sideToMove][PieceType::Rook][toR];
    castled[sideToMove] = true;
    }

void Position::undoCastle( uint8_t from, uint8_t to )
    {
    uint64_t rook;
    uint8_t fromR;
    uint8_t toR;

    if (from < to)   
        {
        if (sideToMove == PieceColor::White)
            {
            fromR = Square::H1;
            toR = Square::F1;
            }
        else
            {
            fromR = Square::H8;
            toR = Square::F8;
            }
        }
    else   
        {
        if (sideToMove == PieceColor::White)
            {
            fromR = Square::A1;
            toR = Square::D1;
            }
        else
            {
            fromR = Square::A8;
            toR = Square::D8;
            }
        }

    rook = Masks::SquareMask[fromR] | Masks::SquareMask[toR];
    pieces[sideToMove] ^= rook;
    bitBoardSet[sideToMove][PieceType::Rook] ^= rook;
    OccupiedSquares ^= rook;
    EmptySquares ^= rook;
    pieceSet[fromR] = PieceInfo(sideToMove, PieceType::Rook);    
    pieceSet[toR] = Null;                             

    updatePstvalue<Add>(sideToMove, Eval::PieceSquareValue(PieceInfo(sideToMove, PieceType::Rook), fromR));
    updatePstvalue<Sub>(sideToMove, Eval::PieceSquareValue(PieceInfo(sideToMove, PieceType::Rook), toR));

    castlingStatus = castlingStatusHistory[currentPly];         

    zobrist ^= Zobrist::PieceInfo[sideToMove][PieceType::Rook][fromR];
    zobrist ^= Zobrist::PieceInfo[sideToMove][PieceType::Rook][toR];
    castled[sideToMove] = false;
    }

bool Position::IsAttacked( uint64_t target, uint8_t side ) const
    {
    uint64_t slidingAttackers;
    uint64_t pawnAttacks;
    uint8_t enemyColor = Piece::GetOpposite(side);
    uint8_t to;

    while (target != 0)
        {
        to = BitScanForwardReset(target);
        pawnAttacks = Moves::PawnAttacks[side][to];

        if ((Pieces(enemyColor, PieceType::Pawn) & pawnAttacks) != 0)
            return true;

        if ((Pieces(enemyColor, PieceType::Knight) & Moves::KnightAttacks[to]) != 0)
            return true;

        if ((Pieces(enemyColor, PieceType::King) & Moves::KingAttacks[to]) != 0)
            return true;

        slidingAttackers = Pieces(enemyColor, PieceType::Queen) | Pieces(enemyColor, PieceType::Rook);

        if (slidingAttackers != 0)
            {
#ifdef PEXT
            if ((Moves::GetRookAttacks(OccupiedSquares, to) & slidingAttackers) != 0)
                return true;
#else
            if ((Moves::GetRankAttacks(OccupiedSquares, to) & slidingAttackers) != 0)
                return true;

            if ((Moves::GetFileAttacks(OccupiedSquares, to) & slidingAttackers) != 0)
                return true;
#endif
            }

        slidingAttackers = Pieces(enemyColor, PieceType::Queen) | Pieces(enemyColor, PieceType::Bishop);

        if (slidingAttackers != 0)
            {
            if ((Moves::GetH1A8DiagonalAttacks(OccupiedSquares, to) & slidingAttackers) != 0)
                return true;

            if ((Moves::GetA1H8DiagonalAttacks(OccupiedSquares, to) & slidingAttackers) != 0)
                return true;
            }
        }
    return false;
    }

std::string Position::GetFen() const
    {
    std::string fen = "";

    for ( int r = 7; r >= 0; r-- )
        {
        int empty = 0;

        for ( int c = 0; c < 8; c++ )
            {
            if (pieceSet[Square::GetSquareIndex(c, r)].Type == PieceType::None)
                empty++;
            else
                {
                if (empty != 0)
                    {
                    fen += (char)empty + '0';
                    empty = 0;
                    }

                fen += Piece::GetInitial(pieceSet[Square::GetSquareIndex(c, r)]);
                }
            }

        if (empty != 0)
            fen += (char)empty + '0';

        if (r > 0)
            fen += '/';
        }

    fen += " ";

    if (sideToMove == PieceColor::White)
        fen += "w";
    else
        fen += "b";

    fen += " ";

    if (castlingStatus)
        {
        fen += (castlingStatus & Castle::WhiteCastleOO ? "K" : "");
        fen += (castlingStatus & Castle::WhiteCastleOOO ? "Q" : "");
        fen += (castlingStatus & Castle::BlackCastleOO ? "k" : "");
        fen += (castlingStatus & Castle::BlackCastleOOO ? "q" : "");
        }
    else
        fen += '-';

    fen += " ";

    if (enPassantSquare != Square::None)
        fen += Square::ToAlgebraic(enPassantSquare);
    else
        fen += '-';

    fen += " ";
    fen += "0 1";

    return fen;
    }

void Position::Display() const
	{
	PieceInfo piece;

	for (int r = 7; r >= 0; r--)
		{
		for (int c = 0; c <= 7; c++)
			{
			piece = pieceSet[Square::GetSquareIndex(c, r)];
			std::cout << '[';
			if (piece.Type != PieceType::None)
				{
				std::cout << Piece::GetColor(pieceSet[Square::GetSquareIndex(c, r)].Color);
				std::cout << Piece::GetType(pieceSet[Square::GetSquareIndex(c, r)].Type);
				}
			else
				std::cout << "  ";
			std::cout << ']';
			}
		std::cout << std::endl;
		}
	}