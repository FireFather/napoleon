#include "piece.h"
#include "fen.h"
#include "eval.h"
#include "castle.h"

Position::Position() noexcept
    {
    Moves::initAttacks();
    Zobrist::Init();

    pieces[pieceColor::White] = Empty;
    pieces[pieceColor::Black] = Empty;
    occupiedSquares = Empty;
    emptySquares = Empty;

    for (uint8_t c = pieceColor::White; c < pieceColor::noColor; c++)
        for (uint8_t f = 0; f < 8; f++)
            pawnsOnFile[c][f] = 0;
    }

void Position::loadFen(std::string pos)
    {
    Fen fenString(pos);

    for (uint8_t c = pieceColor::White; c < pieceColor::noColor; c++)
        for (uint8_t t = pieceType::Pawn; t < pieceType::noType; t++)
            numPieces[c][t] = 0;

    for (uint8_t c = pieceColor::White; c < pieceColor::noColor; c++)
        for (uint8_t f = 0; f < 8; f++)
            pawnsOnFile[c][f] = 0;

    material[pieceColor::White] = 0;
    material[pieceColor::Black] = 0;
    allowNullMove = true;
    currentPly = 0;
    zobrist = 0;
    initializeCastlingStatus(fenString);
    initializesideToMove(fenString);
    initializePieceSet(fenString);
    initializeEnPassantSquare(fenString);
    initializeBitBoards(fenString);

    pstScore[pieceColor::White] = calculatePST(pieceColor::White);
    pstScore[pieceColor::Black] = calculatePST(pieceColor::Black);
    }

Score Position::calculatePST(uint8_t color) const
    {
    pieceInfo piece;
    int pst[2][2] =
        {
        { 0 }
        };

    for (uint8_t sq = Square::A1; sq <= Square::H8; sq++)
        {
        piece = pieceOnSquare(sq);

        if (piece.Type != pieceType::noType)
            {
            Score scores = Eval::pieceSquareScore(piece, sq);
            pst[piece.Color][0] += scores.first;
            pst[piece.Color][1] += scores.second;
            }
        }

    return std::make_pair(pst[color][0], pst[color][1]);
    }

void Position::addPiece(pieceInfo piece, uint8_t sq)
    {
    pieceSet[sq] = piece;

    if (piece.Type != pieceType::noType)
        {
        numPieces[piece.Color][piece.Type]++;
        material[piece.Color] += pieceValue[piece.Type];
        zobrist ^= Zobrist::pieceInfo[piece.Color][piece.Type][sq];

        if (piece.Type == pieceType::Pawn)
            pawnsOnFile[piece.Color][Square::getFileIndex(sq)]++;
        }
    }

void Position::clearPieceSet()
    {
    for (uint8_t i = 0; i < 64; i++)
        {
        pieceSet[i] = Null;
        }
    }

void Position::updateGenericBitBoards()
    {
    pieces[pieceColor::White] =
        bitBoardSet[pieceColor::White][pieceType::Pawn] | bitBoardSet[pieceColor::White][pieceType::Knight]
            | bitBoardSet[pieceColor::White][pieceType::Bishop] | bitBoardSet[pieceColor::White][pieceType::Rook]
            | bitBoardSet[pieceColor::White][pieceType::Queen] | bitBoardSet[pieceColor::White][pieceType::King];

    pieces[pieceColor::Black] =
        bitBoardSet[pieceColor::Black][pieceType::Pawn] | bitBoardSet[pieceColor::Black][pieceType::Knight]
            | bitBoardSet[pieceColor::Black][pieceType::Bishop] | bitBoardSet[pieceColor::Black][pieceType::Rook]
            | bitBoardSet[pieceColor::Black][pieceType::Queen] | bitBoardSet[pieceColor::Black][pieceType::King];

    occupiedSquares = pieces[pieceColor::White] | pieces[pieceColor::Black];
    emptySquares = ~occupiedSquares;
    }

void Position::initializeCastlingStatus(const Fen &fenString)
    {
    castlingStatus = 0;

    if (fenString.whiteCanCastleShort)
        castlingStatus |= Castle::whiteCastleOO;

    if (fenString.whiteCanCastleLong)
        castlingStatus |= Castle::whiteCastleOOO;

    if (fenString.blackCanCastleShort)
        castlingStatus |= Castle::blackCastleOO;

    if (fenString.blackCanCastleLong)
        castlingStatus |= Castle::blackCastleOOO;

    zobrist ^= Zobrist::Castling[castlingStatus];
    }

void Position::initializesideToMove(const Fen &fenString)
    {
    sideToMove = fenString.sideToMove;

    if (sideToMove == pieceColor::Black)
        zobrist ^= Zobrist::Color;
    }

void Position::initializePieceSet(const Fen &fenString)
    {
    for (uint8_t i = 0; i < 64; i++)
        {
        addPiece(fenString.piecePlacement[i], i);
        }
    }

void Position::initializeEnPassantSquare(const Fen &fenString)
    {
    enPassantSquare = fenString.getPassantSquare;

    if (enPassantSquare != Square::noSquare)
        zobrist ^= Zobrist::Enpassant[Square::getFileIndex(enPassantSquare)];
    }

void Position::initializeHalfMoveClock(const Fen &fenString)
    {
    halfMoveClock = fenString.halfMove;
    }

void Position::initializeBitBoards(const Fen &fenString)
    {
    for (uint8_t i = pieceType::Pawn; i < pieceType::noType; i++)
        for (uint8_t l = pieceColor::White; l < pieceColor::noColor; l++)
            bitBoardSet[l][i] = 0;

    for (uint8_t i = 0; i < 64; i++)
        {
        if (fenString.piecePlacement[i].Type == pieceType::King)
            kingSquare[fenString.piecePlacement[i].Color] = i;

        if (fenString.piecePlacement[i].Color != pieceColor::noColor)
            bitBoardSet[fenString.piecePlacement[i].Color][fenString.piecePlacement[i].Type] |= Masks::squareMask[i];
        }

    updateGenericBitBoards();
    }

Move Position::parseMove(std::string str) const
    {
    uint8_t from = Square::Parse(str.substr(0, 2));
    uint8_t to = Square::Parse(str.substr(2));
    Move move;

    if (to == enPassantSquare && pieceSet[from].Type == pieceType::Pawn)
        move = Move(from, to, EnPassant);

    else if (str == "e1g1")
        move = Castle::whiteCastlingOO;

    else if (str == "e8g8")
        move = Castle::blackCastlingOO;

    else if (str == "e1c1")
        move = Castle::whiteCastlingOOO;

    else if (str == "e8c8")
        move = Castle::blackCastlingOOO;

    else if (str.size() == 5)
        move = Move(from, to, 0x8 | (Piece::getPiece(str[4]) - 1));

    else
        move = Move(from, to);

    return move;
    }

void Position::makeMove(Move move)
    {
    bool incrementClock = true;

    uint8_t from = move.fromSquare();
    uint8_t to = move.toSquare();
    uint8_t promoted;
    uint8_t captured = move.isEnPassant() ? static_cast<uint8_t>(pieceType::Pawn) : pieceSet[to].Type;
    uint8_t pieceMoved = pieceSet[from].Type;
    uint8_t enemy = Piece::getOpposite(sideToMove);

    bool capture = captured != pieceType::noType;

    castlingStatusHistory[currentPly] = castlingStatus;       
    enpSquaresHistory[currentPly] = enPassantSquare;        
    halfMoveClockHistory[currentPly] = halfMoveClock;        
    hashHistory[currentPly] = zobrist;            
    capturedPieceHistory[currentPly] = captured;            

    zobrist ^= Zobrist::Color;                               

    pieceSet[to] = pieceSet[from];       
    pieceSet[from] = Null;      

    updatePstScore<Sub>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, pieceMoved), from));
    updatePstScore<Add>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, pieceMoved), to));

    uint64_t From = Masks::squareMask[from];
    uint64_t To = Masks::squareMask[to];
    uint64_t FromTo = From | To;

    bitBoardSet[sideToMove][pieceMoved] ^= FromTo;
    zobrist ^= Zobrist::pieceInfo[sideToMove][pieceMoved][from];        
    zobrist ^= Zobrist::pieceInfo[sideToMove][pieceMoved][to];          

    pieces[sideToMove] ^= FromTo;

    if (pieceMoved == pieceType::King)
        {
        kingSquare[sideToMove] = to;

        if (move.isCastle())
            {
            makeCastle(from, to);
            }

        if (sideToMove == pieceColor::White)
            castlingStatus &=
                ~(Castle::whiteCastleOO | Castle::whiteCastleOOO);         
        else
            castlingStatus &=
                ~(Castle::blackCastleOO | Castle::blackCastleOOO);         
        }
    else if (pieceMoved == pieceType::Rook)            
        {
        if (castlingStatus)                                              
            {
            if (sideToMove == pieceColor::White)
                {
                if (from == Square::A1)
                    castlingStatus &= ~Castle::whiteCastleOOO;

                else if (from == Square::H1)
                    castlingStatus &= ~Castle::whiteCastleOO;
                }
            else
                {
                if (from == Square::A8)
                    castlingStatus &= ~Castle::blackCastleOOO;

                else if (from == Square::H8)
                    castlingStatus &= ~Castle::blackCastleOO;
                }
            }
        }
    else if (move.isPromotion())
        {
        promoted = move.piecePromoted();
        pieceSet[to] = pieceInfo(sideToMove, promoted);
        bitBoardSet[sideToMove][pieceType::Pawn] ^= To;
        bitBoardSet[sideToMove][promoted] ^= To;
        numPieces[sideToMove][pieceType::Pawn]--;
        numPieces[sideToMove][promoted]++;

        material[sideToMove] -= pieceValue[pieceType::Pawn];
        material[sideToMove] += pieceValue[promoted];
        zobrist ^= Zobrist::pieceInfo[sideToMove][pieceType::Pawn][to];
        zobrist ^= Zobrist::pieceInfo[sideToMove][promoted][to];
        updatePstScore<Sub>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, pieceType::Pawn), to));
        updatePstScore<Add>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, promoted), to));

        if (!capture)
            pawnsOnFile[sideToMove][Square::getFileIndex(from)]--;
        else
            pawnsOnFile[sideToMove][Square::getFileIndex(to)]--;
        }

    if (capture)
        {
        if (move.isEnPassant())
            {
            uint64_t piece;

            if (sideToMove == pieceColor::White)
                {
                piece = Masks::squareMask[enPassantSquare - 8];
                pieceSet[enPassantSquare - 8] = Null;
                updatePstScore
                    <Sub>(enemy, Eval::pieceSquareScore(pieceInfo(enemy, pieceType::Pawn), enPassantSquare - 8));
                zobrist ^= Zobrist::pieceInfo[enemy][pieceType::Pawn][enPassantSquare
                    - 8];        
                }
            else
                {
                piece = Masks::squareMask[enPassantSquare + 8];
                pieceSet[enPassantSquare + 8] = Null;
                updatePstScore
                    <Sub>(enemy, Eval::pieceSquareScore(pieceInfo(enemy, pieceType::Pawn), enPassantSquare + 8));
                zobrist ^= Zobrist::pieceInfo[enemy][pieceType::Pawn][enPassantSquare
                    + 8];        
                }

            pieces[enemy] ^= piece;
            bitBoardSet[enemy][pieceType::Pawn] ^= piece;
            occupiedSquares ^= FromTo ^ piece;
            emptySquares ^= FromTo ^ piece;

            pawnsOnFile[sideToMove][Square::getFileIndex(from)]--;
            pawnsOnFile[sideToMove][Square::getFileIndex(to)]++;
            pawnsOnFile[enemy][Square::getFileIndex(to)]--;
            }
        else
            {
            if (captured == pieceType::Rook)
                {
                if (enemy == pieceColor::White)
                    {
                    if (to == Square::H1)
                        castlingStatus &= ~Castle::whiteCastleOO;

                    else if (to == Square::A1)
                        castlingStatus &= ~Castle::whiteCastleOOO;
                    }
                else
                    {
                    if (to == Square::H8)
                        castlingStatus &= ~Castle::blackCastleOO;

                    else if (to == Square::A8)
                        castlingStatus &= ~Castle::blackCastleOOO;
                    }
                }
            else if (captured == pieceType::Pawn)
                {
                pawnsOnFile[enemy][Square::getFileIndex(to)]--;
                }

            if (pieceMoved == pieceType::Pawn)
                {
                pawnsOnFile[sideToMove][Square::getFileIndex(from)]--;
                pawnsOnFile[sideToMove][Square::getFileIndex(to)]++;
                }

            updatePstScore<Sub>(enemy, Eval::pieceSquareScore(pieceInfo(enemy, captured), to));
            bitBoardSet[enemy][captured] ^= To;
            pieces[enemy] ^= To;    
            occupiedSquares ^= From;
            emptySquares ^= From;
            zobrist ^= Zobrist::pieceInfo[enemy][captured][to];     
            }

        numPieces[enemy][captured]--;
        material[enemy] -= pieceValue[captured];
        incrementClock = false;             
        }
    else
        {
        occupiedSquares ^= FromTo;
        emptySquares ^= FromTo;
        }

    if (enPassantSquare != Square::noSquare)
        zobrist ^= Zobrist::Enpassant[Square::getFileIndex(enPassantSquare)];

    enPassantSquare = Square::noSquare;

    if (pieceMoved == pieceType::Pawn)
        {
        incrementClock = false;                
        int sq = to - from;           

        if (sq == 16 || sq == -16)     
            {
            enPassantSquare = to - sq / 2;
            zobrist ^= Zobrist::Enpassant[Square::getFileIndex(enPassantSquare)];
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

void Position::undoMove(Move move)
    {
    uint8_t from = move.fromSquare();
    uint8_t to = move.toSquare();
    uint8_t enemy = sideToMove;
    bool promotion = move.isPromotion();
    bool capture;
    uint8_t promoted;
    uint8_t captured;
    uint8_t pieceMoved;

    currentPly--;

    captured = capturedPieceHistory[currentPly];
    capture = captured != pieceType::noType;

    zobrist ^= Zobrist::Color;                             

    if (castlingStatusHistory[currentPly] != castlingStatus)
        zobrist ^= Zobrist::Castling[castlingStatus];      

    if (enPassantSquare != Square::noSquare)
        zobrist ^= Zobrist::Enpassant[Square::getFileIndex(enPassantSquare)];

    if (enpSquaresHistory[currentPly] != Square::noSquare)
        zobrist ^= Zobrist::Enpassant[Square::getFileIndex(enpSquaresHistory[currentPly])];

    halfMoveClock = halfMoveClockHistory[currentPly];

    if (promotion)
        pieceMoved = pieceType::Pawn;
    else
        pieceMoved = pieceSet[to].Type;

    sideToMove = Piece::getOpposite(sideToMove);

    pieceSet[from] = pieceSet[to];    

    if (!promotion)
        {
        updatePstScore<Sub>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, pieceMoved), to));
        updatePstScore<Add>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, pieceMoved), from));
        }

    uint64_t From = Masks::squareMask[from];
    uint64_t To = Masks::squareMask[to];
    uint64_t FromTo = From | To;

    bitBoardSet[sideToMove][pieceMoved] ^= FromTo;
    zobrist ^= Zobrist::pieceInfo[sideToMove][pieceMoved][from];        
    zobrist ^= Zobrist::pieceInfo[sideToMove][pieceMoved][to];          

    pieces[sideToMove] ^= FromTo;

    if (pieceMoved == pieceType::King)
        {
        kingSquare[sideToMove] = from;

        if (move.isCastle())
            {
            undoCastle(from, to);
            }

        castlingStatus = castlingStatusHistory[currentPly];         
        }
    else if (pieceMoved == pieceType::Rook)
        {
        castlingStatus = castlingStatusHistory[currentPly];
        }
    else if (promotion)
        {
        promoted = move.piecePromoted();
        numPieces[sideToMove][pieceType::Pawn]++;
        numPieces[sideToMove][promoted]--;

        material[sideToMove] += pieceValue[pieceType::Pawn];
        material[sideToMove] -= pieceValue[promoted];
        pieceSet[from] = pieceInfo(sideToMove, pieceType::Pawn);
        bitBoardSet[sideToMove][promoted] ^= To;
        bitBoardSet[sideToMove][pieceType::Pawn] ^= To;
        zobrist ^= Zobrist::pieceInfo[sideToMove][pieceType::Pawn][to];
        zobrist ^= Zobrist::pieceInfo[sideToMove][promoted][to];
        updatePstScore<Add>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, pieceType::Pawn), from));
        updatePstScore<Sub>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, promoted), to));

        if (!capture)
            pawnsOnFile[sideToMove][Square::getFileIndex(from)]++;
        else
            pawnsOnFile[sideToMove][Square::getFileIndex(to)]++;
        }

    enPassantSquare = enpSquaresHistory[currentPly];

    if (capture)
        {
        if (move.isEnPassant())
            {
            pieceSet[to] = Null;           
            uint64_t piece;
			const uint8_t offset = 8;

            if (sideToMove == pieceColor::White)
                {
                piece = Masks::squareMask[enPassantSquare - offset];
                pieceSet[enPassantSquare - offset] = pieceInfo(pieceColor::Black, pieceType::Pawn);
                updatePstScore <Add>(enemy, Eval::pieceSquareScore(pieceInfo(enemy, pieceType::Pawn), enPassantSquare - offset));
                zobrist ^= Zobrist::pieceInfo[enemy][pieceType::Pawn][enPassantSquare - offset];
                }
            else
                {
                piece = Masks::squareMask[enPassantSquare + offset];
                pieceSet[enPassantSquare + offset] = pieceInfo(pieceColor::White, pieceType::Pawn);
                updatePstScore <Add>(enemy, Eval::pieceSquareScore(pieceInfo(enemy, pieceType::Pawn), enPassantSquare + offset));
                zobrist ^= Zobrist::pieceInfo[enemy][pieceType::Pawn][enPassantSquare + offset];
                }

            pieces[enemy] ^= piece;
            bitBoardSet[enemy][pieceType::Pawn] ^= piece;
            occupiedSquares ^= FromTo ^ piece;
            emptySquares ^= FromTo ^ piece;

            pawnsOnFile[sideToMove][Square::getFileIndex(from)]++;
            pawnsOnFile[sideToMove][Square::getFileIndex(to)]--;
            pawnsOnFile[enemy][Square::getFileIndex(to)]++;
            }
        else
            {
            if (captured == pieceType::Rook)
                {
                castlingStatus = castlingStatusHistory[currentPly];
                }
            else if (captured == pieceType::Pawn)
                {
                pawnsOnFile[enemy][Square::getFileIndex(to)]++;
                }

            if (pieceMoved == pieceType::Pawn)
                {
                pawnsOnFile[sideToMove][Square::getFileIndex(from)]++;
                pawnsOnFile[sideToMove][Square::getFileIndex(to)]--;
                }

            updatePstScore<Add>(enemy, Eval::pieceSquareScore(pieceInfo(enemy, captured), to));

            pieceSet[to] = pieceInfo(enemy, captured);
            bitBoardSet[enemy][captured] ^= To;

            pieces[enemy] ^= To;    
            occupiedSquares ^= From;
            emptySquares ^= From;

            zobrist ^= Zobrist::pieceInfo[enemy][captured][to];     
            }

        numPieces[enemy][captured]++;
        material[enemy] += pieceValue[captured];
        }
    else
        {
        pieceSet[to] = Null;
        occupiedSquares ^= FromTo;
        emptySquares ^= FromTo;
        }
    }

void Position::makeCastle(uint8_t from, uint8_t to)
    {
    uint64_t rook;
    uint8_t fromR;
    uint8_t toR;

    if (from < to)   
        {
        if (sideToMove == pieceColor::White)
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
        if (sideToMove == pieceColor::White)
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

    rook = Masks::squareMask[fromR] | Masks::squareMask[toR];
    pieces[sideToMove] ^= rook;
    bitBoardSet[sideToMove][pieceType::Rook] ^= rook;
    occupiedSquares ^= rook;
    emptySquares ^= rook;
    pieceSet[fromR] = Null;                         
    pieceSet[toR] = pieceInfo(sideToMove, pieceType::Rook);    

    updatePstScore<Sub>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, pieceType::Rook), fromR));
    updatePstScore<Add>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, pieceType::Rook), toR));

    zobrist ^= Zobrist::pieceInfo[sideToMove][pieceType::Rook][fromR];
    zobrist ^= Zobrist::pieceInfo[sideToMove][pieceType::Rook][toR];
    castled[sideToMove] = true;
    }

void Position::undoCastle(uint8_t from, uint8_t to)
    {
    uint64_t rook;
    uint8_t fromR;
    uint8_t toR;

    if (from < to)   
        {
        if (sideToMove == pieceColor::White)
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
        if (sideToMove == pieceColor::White)
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

    rook = Masks::squareMask[fromR] | Masks::squareMask[toR];
    pieces[sideToMove] ^= rook;
    bitBoardSet[sideToMove][pieceType::Rook] ^= rook;
    occupiedSquares ^= rook;
    emptySquares ^= rook;
    pieceSet[fromR] = pieceInfo(sideToMove, pieceType::Rook);    
    pieceSet[toR] = Null;                             

    updatePstScore<Add>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, pieceType::Rook), fromR));
    updatePstScore<Sub>(sideToMove, Eval::pieceSquareScore(pieceInfo(sideToMove, pieceType::Rook), toR));

    castlingStatus = castlingStatusHistory[currentPly];         

    zobrist ^= Zobrist::pieceInfo[sideToMove][pieceType::Rook][fromR];
    zobrist ^= Zobrist::pieceInfo[sideToMove][pieceType::Rook][toR];
    castled[sideToMove] = false;
    }

bool Position::isAttacked(uint64_t target, uint8_t side) const
    {
    uint64_t slidingAttackers;
    uint64_t pawnAttacks;
    uint8_t enemyColor = Piece::getOpposite(side);
    uint8_t to;

    while (target != 0)
        {
        to = bitScanForwardReset(target);
        pawnAttacks = Moves::pawnAttacks[side][to];

        if ((Pieces(enemyColor, pieceType::Pawn) & pawnAttacks) != 0)
            return true;

        if ((Pieces(enemyColor, pieceType::Knight) & Moves::knightAttacks[to]) != 0)
            return true;

        if ((Pieces(enemyColor, pieceType::King) & Moves::kingAttacks[to]) != 0)
            return true;

        slidingAttackers = Pieces(enemyColor, pieceType::Queen) | Pieces(enemyColor, pieceType::Rook);

        if (slidingAttackers != 0)
            {
#ifdef PEXT
            if ((Moves::getRookAttacks(occupiedSquares, to) & slidingAttackers) != 0)
                return true;
#else
            if ((Moves::getRankAttacks(occupiedSquares, to) & slidingAttackers) != 0)
                return true;

            if ((Moves::getFileAttacks(occupiedSquares, to) & slidingAttackers) != 0)
                return true;
#endif
            }

        slidingAttackers = Pieces(enemyColor, pieceType::Queen) | Pieces(enemyColor, pieceType::Bishop);

        if (slidingAttackers != 0)
            {
            if ((Moves::getH1A8DiagonalAttacks(occupiedSquares, to) & slidingAttackers) != 0)
                return true;

            if ((Moves::getA1H8DiagonalAttacks(occupiedSquares, to) & slidingAttackers) != 0)
                return true;
            }
        }
    return false;
    }

std::string Position::getFen() const
    {
    std::string fen = "";

    for (int r = 7; r >= 0; r--)
        {
        int empty = 0;

        for (int c = 0; c < 8; c++)
            {
            if (pieceSet[Square::getSquareIndex(c, r)].Type == pieceType::noType)
                empty++;
            else
                {
                if (empty != 0)
                    {
                    fen += (char)empty + '0';
                    empty = 0;
                    }

                fen += Piece::getInitial(pieceSet[Square::getSquareIndex(c, r)]);
                }
            }

        if (empty != 0)
            fen += (char)empty + '0';

        if (r > 0)
            fen += '/';
        }

    fen += " ";

    if (sideToMove == pieceColor::White)
        fen += "w";
    else
        fen += "b";

    fen += " ";

    if (castlingStatus)
        {
        fen += (castlingStatus & Castle::whiteCastleOO ? "K" : "");
        fen += (castlingStatus & Castle::whiteCastleOOO ? "Q" : "");
        fen += (castlingStatus & Castle::blackCastleOO ? "k" : "");
        fen += (castlingStatus & Castle::blackCastleOOO ? "q" : "");
        }
    else
        fen += '-';

    fen += " ";

    if (enPassantSquare != Square::noSquare)
        fen += Square::toAlgebraic(enPassantSquare);
    else
        fen += '-';

    fen += " ";
    fen += "0 1";

    return fen;
    }

void Position::Display() const
	{
	pieceInfo piece;

	for (int r = 7; r >= 0; r--)
		{
		for (int c = 0; c <= 7; c++)
			{
			piece = pieceSet[Square::getSquareIndex(c, r)];
			if (piece.Type != pieceType::noType)
				std::cout << Piece::getInitial(piece);
			else
				std::cout << "-";
			}
		std::cout << std::endl;
		}
	}