#include <algorithm>
#include <string>
#include "square.h"

int Square::Parse( std::string square )
    {
    std::transform(square.begin(), square.end(), square.begin(), ::tolower);
    int x = static_cast<int>(square[0] - 'a');
    int y = static_cast<int>(square[1] - '1');
    return GetSquareIndex(x, y);
    }

std::string Square::ToAlgebraic( uint8_t square )
    {
    if (square == None)
        return "None";
    std::string str = "";
    str += static_cast<char>(GetFileIndex(square) + 'a');
    str += std::to_string(GetRankIndex(square) + 1);
    return str;
    }