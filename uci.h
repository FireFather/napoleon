#pragma once
#include <sstream>
#include <iostream>
#include <thread>

class Position;

namespace Uci
    {
    void Start();
    void Go(std::istringstream &);
	void engineInfo();
    extern Position position;
    extern std::thread search;
    }
