#pragma once
#include "position.h"
#include "move.h"
#include "piece.h"
#include "moves.h"
#include "hashtable.h"
#include "zobrist.h"
#include "uci.h"
#include "pawn.h"
#include "masks.h"
#include "ranks.h"
#include "evalterms.h"
#include "searchinfo.h"
#include "square.h"
#include "masks.h"

const int MaxPhase = 256;
const PieceInfo Null = PieceInfo(PieceColor::None, PieceType::None);
const std::string StartPosition = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const uint64_t Empty = 0x0000000000000000;

class MoveList;
class Fen;

enum GameStage
    {
    Opening = 0,
    MiddleGame = 1,
    EndGame = 2
    };

enum Operation
    {
    Add,
    Sub
    };

class Position
    {
    public:
    uint64_t OccupiedSquares;
    uint64_t EmptySquares;

    uint64_t zobrist;

    Position() noexcept;
	void Display() const;
    void LoadFen(std::string = StartPosition);
    void AddPiece(PieceInfo, uint8_t);
    uint64_t PlayerPieces()const;
    uint64_t EnemyPieces()const;
    uint64_t Pieces(uint8_t, uint8_t)const;
    uint64_t Pieces(uint8_t)const;
    uint64_t PinnedPieces()const;
    uint64_t KingAttackers(uint8_t, uint8_t)const;
    uint64_t AttacksTo(uint8_t, uint8_t, uint64_t)const;
    uint64_t MovesTo(uint8_t, uint8_t, uint64_t)const;
    std::pair<uint64_t, uint8_t> LeastValuableAttacker(uint8_t, uint64_t)const;

    int See(Move)const;

    PieceInfo PieceOnSquare(uint8_t)const;
    const PieceInfo *PieceList()const;

    void MakeMove(Move);
    void UndoMove(Move);
    void MakeNullMove();
    void UndoNullMove();

    void SetCheckState(bool);

    bool IsCapture(Move)const;
    bool IsMoveLegal(Move, uint64_t);
    bool IsAttacked(uint64_t, uint8_t)const;
    bool IsPromotingPawn()const;
    bool IsOnSquare(uint8_t, uint8_t, uint8_t)const;
    bool IsRepetition()const;

    uint8_t KingSquare(uint8_t)const;

    uint8_t SideToMove()const;
    uint8_t CastlingStatus()const;
    uint8_t EnPassantSquare()const;

    int HalfMoveClock()const;
    int CurrentPly()const;
    bool AllowNullMove()const;
    void ToggleNullMove();
    bool IsCheck()const;

    Score PstValue(uint8_t)const;
    int NumOfPieces(uint8_t, uint8_t)const;
    int NumOfPieces(uint8_t)const;
    int Material(uint8_t)const;
    int Material()const;
    int MaterialBalance(uint8_t)const;
    int PawnsOnFile(uint8_t, uint8_t)const;

    GameStage Stage()const;
    bool Opening()const;
    bool MiddleGame()const;
    bool EndGame()const;
    bool HasCastled(uint8_t);
    int Phase()const;

    std::string GetFen()const;
    Move ParseMove(std::string)const;
    private:
    uint8_t castlingStatusHistory[MaxPly];
    uint8_t capturedPieceHistory[MaxPly];
    uint8_t enpSquaresHistory[MaxPly];
    uint64_t hashHistory[MaxPly];          
    int halfMoveClockHistory[MaxPly];

    uint64_t bitBoardSet[2][7];     
    uint8_t kingSquare[2];         

    PieceInfo pieceSet[74];        
    uint64_t pieces[2];            

    uint8_t sideToMove;
    uint8_t castlingStatus;
    uint8_t enPassantSquare;

    int halfMoveClock;
    int currentPly;
    bool allowNullMove;
    bool isCheck;
    bool castled[2] =
        {
        false, false
        };                  

    int numOfPieces[2][6];   
    int pawnsOnFile[2][8];   

    Score pstValue[2];      
    int material[2];        

    template <Operation>
    void updatePstvalue( uint8_t, Score );

    void clearPieceSet();
    void updateGenericBitBoards();
    void initializeBitBoards(const Fen &);
    void initializesideToMove(const Fen &);
    void initializeCastlingStatus(const Fen &);
    void initializeEnPassantSquare(const Fen &);
    void initializeHalfMoveClock(const Fen &);
    void initializePieceSet(const Fen &);
    void makeCastle(uint8_t, uint8_t);
    void undoCastle(uint8_t, uint8_t);
    Score calculatePST(uint8_t)const;
    };

INLINE uint64_t Position::PinnedPieces() const
    {
    uint8_t enemy = Piece::GetOpposite(sideToMove);
    int kingSq = kingSquare[sideToMove];

    uint64_t playerPieces = PlayerPieces();
    uint64_t b;
    uint64_t pinned = 0;
    uint64_t pinners = ((bitBoardSet[enemy][PieceType::Rook]
        | bitBoardSet[enemy][PieceType::Queen]) & Moves::PseudoRookAttacks[kingSq])
        | ((bitBoardSet[enemy][PieceType::Bishop]
            | bitBoardSet[enemy][PieceType::Queen]) & Moves::PseudoBishopAttacks[kingSq]);

    while (pinners)
        {
        int sq = BitScanForwardReset(pinners);
        b = Moves::ObstructedTable[sq][kingSq] & OccupiedSquares;

        if ((b != 0) && ((b &(b - 1)) == 0) && ((b & playerPieces) != 0))
            {
            pinned |= b;
            }
        }
    return pinned;
    }

INLINE bool Position::IsMoveLegal( Move move, uint64_t pinned )
    {
    if (pieceSet[move.FromSquare()].Type == PieceType::King)
        {
        return !IsAttacked(Masks::SquareMask[move.ToSquare()], sideToMove);
        }

    if (move.IsEnPassant())
        {
        MakeMove(move);
        bool islegal =
            !IsAttacked(bitBoardSet[Piece::GetOpposite(sideToMove)][PieceType::King], Piece::GetOpposite(sideToMove));
        UndoMove(move);
        return islegal;
        }

    return (pinned == 0) || ((pinned & Masks::SquareMask[move.FromSquare()]) == 0)
        || Moves::AreSquareAligned(move.FromSquare(), move.ToSquare(), kingSquare[sideToMove]);
    }

INLINE uint64_t Position::KingAttackers( uint8_t square, uint8_t color ) const
    {
    uint8_t opp = Piece::GetOpposite(color);
    uint64_t bishopAttacks =
        Moves::GetA1H8DiagonalAttacks(OccupiedSquares, square) | Moves::GetH1A8DiagonalAttacks(OccupiedSquares, square);

#ifdef PEXT
    uint64_t rookAttacks = Moves::GetRookAttacks(OccupiedSquares, square);
#else
    uint64_t rookAttacks =
        Moves::GetFileAttacks(OccupiedSquares, square) | Moves::GetRankAttacks(OccupiedSquares, square);
#endif

    return (Moves::PawnAttacks[color][square] & bitBoardSet[opp][PieceType::Pawn])
        | (Moves::KnightAttacks[square] & bitBoardSet[opp][PieceType::Knight])
            | (bishopAttacks &(bitBoardSet[opp][PieceType::Bishop] | bitBoardSet[opp][PieceType::Queen]))
            | (rookAttacks &(bitBoardSet[opp][PieceType::Rook] | bitBoardSet[opp][PieceType::Queen]));
    }

INLINE uint64_t Position::AttacksTo( uint8_t square, uint8_t color, uint64_t occ ) const
    {
    uint8_t opp = Piece::GetOpposite(color);
    uint64_t bishopAttacks = Moves::GetA1H8DiagonalAttacks(occ, square) | Moves::GetH1A8DiagonalAttacks(occ, square);

#ifdef PEXT
    uint64_t rookAttacks = Moves::GetRookAttacks(occ, square);
#else
    uint64_t rookAttacks = Moves::GetFileAttacks(occ, square) | Moves::GetRankAttacks(occ, square);
#endif

    return (Moves::KingAttacks[square] & bitBoardSet[color][PieceType::King])
        | (Moves::PawnAttacks[opp][square] & bitBoardSet[color][PieceType::Pawn])
            | (Moves::KnightAttacks[square] & bitBoardSet[color][PieceType::Knight])
            | (bishopAttacks &(bitBoardSet[color][PieceType::Bishop] | bitBoardSet[color][PieceType::Queen]))
            | (rookAttacks &(bitBoardSet[color][PieceType::Rook] | bitBoardSet[color][PieceType::Queen]));
    }

INLINE uint64_t Position::MovesTo( uint8_t square, uint8_t color, uint64_t occ ) const
    {
    uint64_t bishopAttacks = Moves::GetA1H8DiagonalAttacks(occ, square) | Moves::GetH1A8DiagonalAttacks(occ, square);

#ifdef PEXT
    uint64_t rookAttacks = Moves::GetRookAttacks(occ, square);
#else
    uint64_t rookAttacks = Moves::GetFileAttacks(occ, square) | Moves::GetRankAttacks(occ, square);
#endif

    uint8_t pawnSquare;
    uint64_t pawn = 0;
    uint64_t pawnMove = 0;
    uint8_t enemy = Piece::GetOpposite(color);

    if (color == PieceColor::White)
        {
        if (square >= 16)
            {
            pawnSquare = square - 16;
            pawn = Masks::SquareMask[pawnSquare] & bitBoardSet[color][PieceType::Pawn];
            pawnMove = Pawn::GetQuietTargets(color, pawn, EmptySquares) & Masks::SquareMask[square];
            }

        if (square >= 8)
            {
            pawnSquare = square - 8;
            pawn |= Masks::SquareMask[pawnSquare] & bitBoardSet[color][PieceType::Pawn];
            pawnMove |= Pawn::GetQuietTargets(color, pawn, EmptySquares) & Masks::SquareMask[square];
            }
        }
    else
        {
        if (square <= (63 - 16))
            {
            pawnSquare = square + 16;
            pawn = Masks::SquareMask[pawnSquare] & bitBoardSet[color][PieceType::Pawn];
            pawnMove = Pawn::GetQuietTargets(color, pawn, EmptySquares) & Masks::SquareMask[square];
            }

        if (square <= (63 - 8))
            {
            pawnSquare = square + 8;
            pawn |= Masks::SquareMask[pawnSquare] & bitBoardSet[color][PieceType::Pawn];
            pawnMove |= Pawn::GetQuietTargets(color, pawn, EmptySquares) & Masks::SquareMask[square];
            }
        }

    return (Moves::KingAttacks[square] & bitBoardSet[color][PieceType::King])
        | (Masks::SquareMask[square] & Pieces(enemy)
            ? (Moves::PawnAttacks[enemy][square] & bitBoardSet[color][PieceType::Pawn]) : 0)
            | (pawn) | (Moves::KnightAttacks[square] & bitBoardSet[color][PieceType::Knight])
            | (bishopAttacks &(bitBoardSet[color][PieceType::Bishop] | bitBoardSet[color][PieceType::Queen]))
            | (rookAttacks &(bitBoardSet[color][PieceType::Rook] | bitBoardSet[color][PieceType::Queen]));
    }

inline void Position::MakeNullMove()
    {
    hashHistory[currentPly] = zobrist;
    enpSquaresHistory[currentPly] = enPassantSquare;
    sideToMove = Piece::GetOpposite(sideToMove);
    enPassantSquare = Square::None;

    zobrist ^= Zobrist::Color;

    if (enPassantSquare != Square::None)
        zobrist ^= Zobrist::Enpassant[Square::GetFileIndex(enPassantSquare)];

    if (enpSquaresHistory[currentPly] != Square::None)
        zobrist ^= Zobrist::Enpassant[Square::GetFileIndex(enpSquaresHistory[currentPly])];

    allowNullMove = false;
    currentPly++;
    }

inline void Position::UndoNullMove()
    {
    currentPly--;
    sideToMove = Piece::GetOpposite(sideToMove);
    enPassantSquare = enpSquaresHistory[currentPly];

    zobrist ^= Zobrist::Color;

    if (enpSquaresHistory[currentPly] != Square::None)
        zobrist ^= Zobrist::Enpassant[Square::GetFileIndex(enpSquaresHistory[currentPly])];

    allowNullMove = true;
    }

inline uint8_t Position::CastlingStatus() const
    {
    return castlingStatus;
    }

INLINE uint8_t Position::SideToMove() const
    {
    return sideToMove;
    }

inline uint8_t Position::EnPassantSquare() const
    {
    return enPassantSquare;
    }

inline int Position::HalfMoveClock() const
    {
    return halfMoveClock;
    }

inline int Position::CurrentPly() const
    {
    return currentPly;
    }

inline bool Position::AllowNullMove() const
    {
    return allowNullMove;
    }

inline void Position::ToggleNullMove()
    {
    allowNullMove = !allowNullMove;
    }

inline bool Position::IsCheck() const
    {
    return isCheck;
    }

inline void Position::SetCheckState( bool isCheck )
    {
    this->isCheck = isCheck;
    }

inline uint64_t Position::PlayerPieces() const
    {
    return pieces[sideToMove];
    }

inline uint64_t Position::EnemyPieces() const
    {
    return pieces[Piece::GetOpposite(sideToMove)];
    }

inline uint64_t Position::Pieces( uint8_t color, uint8_t type ) const
    {
    return bitBoardSet[color][type];
    }

inline uint64_t Position::Pieces( uint8_t color ) const
    {
    return pieces[color];
    }

inline PieceInfo Position::PieceOnSquare( uint8_t square ) const
    {
    return pieceSet[square];
    }

inline const PieceInfo *Position::PieceList() const
    {
    return pieceSet;
    }

inline bool Position::IsPromotingPawn() const
    {
    const uint64_t rank = (sideToMove == PieceColor::White ? Ranks::Seven : Ranks::Two);
    return (bitBoardSet[sideToMove][PieceType::Pawn] & rank);
    }

inline bool Position::IsCapture( Move move ) const
    {
    return (pieceSet[move.ToSquare()].Type != PieceType::None || move.IsEnPassant());
    }

inline bool Position::IsOnSquare( uint8_t color, uint8_t type, uint8_t sq ) const
    {
    return (bitBoardSet[color][type] & Masks::SquareMask[sq]);
    }

inline int Position::PawnsOnFile( uint8_t color, uint8_t file ) const
    {
    return pawnsOnFile[color][file];
    }

inline uint8_t Position::KingSquare( uint8_t color ) const
    {
    return kingSquare[color];
    }

inline Score Position::PstValue( uint8_t color ) const
    {
    return pstValue[color];
    }

inline int Position::NumOfPieces( uint8_t color, uint8_t type ) const
    {
    return numOfPieces[color][type];
    }

inline int Position::NumOfPieces( uint8_t type ) const
    {
    return numOfPieces[PieceColor::White][type] + numOfPieces[PieceColor::Black][type];
    }

inline int Position::Material( uint8_t color ) const
    {
    return material[color];
    }

inline int Position::Material() const
    {
    return material[PieceColor::White] + material[PieceColor::Black];
    }

inline int Position::MaterialBalance( uint8_t color ) const
    {
    return material[color] - material[Piece::GetOpposite(color)];
    }

inline std::pair<uint64_t, uint8_t> Position::LeastValuableAttacker( uint8_t color, uint64_t attackers ) const
    {
    for ( uint8_t type = PieceType::Pawn; type < PieceType::None; type++ )
        {
        uint64_t set = Pieces(color, type) & attackers;

        if (set)
            return std::make_pair(set & -set, type);
        }

    return std::make_pair(Empty, PieceType::None);
    }

template <>
inline void Position::updatePstvalue<Operation::Add>( uint8_t color, Score values )
    {
    pstValue[color].first += values.first;
    pstValue[color].second += values.second;
    }

template <>
inline void Position::updatePstvalue<Operation::Sub>( uint8_t color, Score values )
    {
    pstValue[color].first -= values.first;
    pstValue[color].second -= values.second;
    }

inline int Position::See( Move move ) const
    {
    uint8_t to = move.ToSquare();
    uint8_t from = move.FromSquare();
    PieceInfo captured = PieceOnSquare(to);
    uint8_t attackingPiece = PieceOnSquare(from).Type;

    int gain[100];
    int depth = 0;

    uint64_t fromSet;
    uint64_t occ = OccupiedSquares;
    uint64_t attackers;

    gain[depth++] = PieceValue[captured.Type];

    uint8_t side = Piece::GetOpposite(sideToMove);
    occ ^= Masks::SquareMask[from];

    attackers = AttacksTo(to, side, occ) & occ;

    while (attackers)
        {
        gain[depth] = PieceValue[attackingPiece] - gain[depth - 1];

        std::pair<uint64_t, uint8_t> tuple = LeastValuableAttacker(side, attackers);
        fromSet = tuple.first;
        attackingPiece = tuple.second;

        occ ^= fromSet;
        side = Piece::GetOpposite(side);
        attackers = AttacksTo(to, side, occ) & occ;
        depth++;
        }

    while (--depth)
        {
        gain[depth - 1] = -std::max(-gain[depth - 1], gain[depth]);
        }

    return gain[0];
    }

inline bool Position::IsRepetition() const
    {
    if (halfMoveClock >= 4)
        {
        int start = SideToMove() == PieceColor::White ? 0 : 1;

        for ( int i = start; i < currentPly; i += 2 )
            {
            if (hashHistory[i] == zobrist)
                return true;
            }
        }

    return false;
    }

inline GameStage Position::Stage() const
    {
    if (Opening())
        return GameStage::Opening;

    if (MiddleGame())
        return GameStage::MiddleGame;

    return GameStage::EndGame;
    }

inline bool Position::Opening() const
    {
    return Material() > MiddleGameMat;
    }

inline bool Position::MiddleGame() const
    {
    return !Opening() && Material() > EndGameMat;
    }

inline bool Position::EndGame() const
    {
    return Material() <= EndGameMat;
    }

inline int Position::Phase() const
    {
    const static int kingMaterial = PieceValue[King] * 2;
    int nonPawnMaterial = (Material() - NumOfPieces(Pawn) * PieceValue[Pawn] - kingMaterial);
    int openingMaterial = (OpeningNonPawnMaterial - kingMaterial);

    int phase = (nonPawnMaterial * MaxPhase + openingMaterial / 2) / openingMaterial;

    return MaxPhase - phase;    
    }

inline bool Position::HasCastled( uint8_t color )
    {
    return castled[color];
    }