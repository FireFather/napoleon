#pragma once
#include "movegen.h"

class MovePick
    {
    public:
    Position &position;
    Move moves[MoveGen::MaxMoves];
    Move hashMove;
    int count;
    MovePick(Position &, SearchInfo &);
    template <bool>
    void Sort( int = 0 );
    Move First();
    Move Next();
    void Reset();
    Move & operator []( int );
    private:
    int scores[MoveGen::MaxMoves];
    SearchInfo &info;
    int first;
    };

inline Move &MovePick::operator []( int index )
    {
    return moves[index];
    }

inline Move MovePick::First()
    {
    if (!hashMove.IsNull())
        return hashMove;
    else
        return Next();
    }

inline void MovePick::Reset()
    {
    first = 0;
    }

inline Move MovePick::Next()
    {
    if (first == -1)
        return First();

    int max = first;

    if (max >= count)
        return NullMove;

    for ( auto i = first+1; i < count; i++ )
        if (scores[i] > scores[max])
            max = i;

    if (max != first)
        {
        std::swap(moves[first], moves[max]);
        std::swap(scores[first], scores[max]);
        }

    Move move = moves[first++];

    if (move != hashMove)
        return move;
    else
        return Next();
    }

template <bool quiesce>
void MovePick::Sort( int ply )
    {
    int max = 0;
    int historyScore;

    for ( auto i = 0; i < count; i++ )
        {
        if (moves[i].IsPromotion())
            {
            scores[i] = PieceValue[moves[i].PiecePromoted()];
            }
        else if (position.IsCapture(moves[i]))
            {
            if (quiesce)
                {
                uint8_t captured = moves[i].IsEnPassant() ? uint8_t(PieceType::Pawn) : position.PieceOnSquare(moves[i].ToSquare()).Type;  
                scores[i] = PieceValue[captured] - PieceValue[position.PieceOnSquare(moves[i].FromSquare()).Type];
                }
            else
                {
                scores[i] = position.See(moves[i]);  
                }
            }

        else if (moves[i] == info.FirstKiller(ply))
            scores[i] = -1;

        else if (moves[i] == info.SecondKiller(ply))
            scores[i] = -2;

        else if ((historyScore = info.HistoryScore(moves[i], position.SideToMove())) > max)
            max = historyScore;
        }

    for ( auto i = 0; i < count; i++ )
        {
        if (!position.IsCapture(moves[i]) && moves[i] != info.FirstKiller(ply) && moves[i] != info.SecondKiller(ply))
            scores[i] = info.HistoryScore(moves[i], position.SideToMove()) - max - 3;
        }
    }