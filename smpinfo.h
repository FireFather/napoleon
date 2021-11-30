#pragma once
#include "position.h"

class SMPInfo
    {
    public:

    void updateInfo(int depth, int alpha, int beta, const Position &position, bool ready)
        {
        this->beta = beta;
        this->depth = depth;
        this->alpha = alpha;
        this->position = position;
        readyToSearch = ready;
        }

    int Alpha()
        {
        return alpha;
        }

    int Beta()
        {
        return beta;
        }

    int Depth()
        {
        return depth;
        }

    Position Board()
        {
        return position;
        }

    bool Ready()
        {
        return readyToSearch;
        }

    void setReady(bool ready)
        {
        readyToSearch = ready;
        }

    private:
    int alpha;
    int beta;
    int depth;
    Position position;
    bool readyToSearch;
    };