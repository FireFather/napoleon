#include "movepick.h"

MovePick::MovePick(Position &position, SearchInfo &info) :
    position(position),
    hashMove(nullMove),
    count(0),
    info(info),
    first(0)
    {
    }