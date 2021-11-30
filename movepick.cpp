#include "movepick.h"

MovePick::MovePick( Position &position, SearchInfo &info ) :
    position(position),
    hashMove(NullMove),
    count(0),
    info(info),
    first(0)
    {
    }