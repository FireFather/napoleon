#include <algorithm>
#include <string>
#include "square.h"

int Square::Parse(std::string square)
    {
    std::transform(square.begin(), square.end(), square.begin(), ::tolower);
    int x = static_cast<int>(square[0] - 'a');
    int y = static_cast<int>(square[1] - '1');
    return getSquareIndex(x, y);
    }

std::string Square::toAlgebraic(uint8_t square)
    {
    if (square == noSquare)
        return "none";
    std::string str = "";
    str += static_cast<char>(getFileIndex(square) + 'a');
    str += std::to_string(getRankIndex(square) + 1);
    return str;
    }