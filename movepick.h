#pragma once
#include "movegen.h"

class MovePick
    {
    public:
    Position &position;
    Move moves[moveGen::maxMoves];
    Move hashMove;
    int count;
    MovePick(Position &, SearchInfo &);

    template <bool>
    void Sort(int = 0);
    Move First();
    Move Next();
    void Reset();
    Move & operator [](int);
    private:
    int scores[moveGen::maxMoves];
    SearchInfo &info;
    int first;
    };

inline Move &MovePick::operator [](int index)
    {
    return moves[index];
    }

inline Move MovePick::First()
    {
    if (!hashMove.isNull())
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
        return nullMove;

    for (auto i = first+1; i < count; i++)
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
void MovePick::Sort(int ply)
    {
    int max = 0;
    int historyScore;

    for (auto i = 0; i < count; i++)
        {
        if (moves[i].isPromotion())
            {
            scores[i] = pieceValue[moves[i].piecePromoted()];
            }
        else if (position.isCapture(moves[i]))
            {
            if (quiesce)
                {
                uint8_t captured = moves[i].isEnPassant() ? uint8_t(pieceType::Pawn) : position.pieceOnSquare(moves[i].toSquare()).Type;  
                scores[i] = pieceValue[captured] - pieceValue[position.pieceOnSquare(moves[i].fromSquare()).Type];
                }
            else
                {
                scores[i] = position.See(moves[i]);  
                }
            }

        else if (moves[i] == info.firstKiller(ply))
            scores[i] = -1;

        else if (moves[i] == info.secondKiller(ply))
            scores[i] = -2;

        else if ((historyScore = info.historyScore(moves[i], position.getSideToMove())) > max)
            max = historyScore;
        }

    for (auto i = 0; i < count; i++)
        {
        if (!position.isCapture(moves[i]) && moves[i] != info.firstKiller(ply) && moves[i] != info.secondKiller(ply))
            scores[i] = info.historyScore(moves[i], position.getSideToMove()) - max - 3;
        }
    }