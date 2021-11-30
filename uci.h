#pragma once
#include <sstream>
#include <iostream>
#include <thread>

enum class Command
    {
    Options,
    BestMove,
    Info
    };

class Position;

namespace Uci
    {
    void Start();
    template <Command>
    void Send( std::string, std::string = "" );
    void Go(std::istringstream &);
    extern Position position;
    extern std::thread search;
    }

template <Command cmdType>
void Uci::Send( std::string command, std::string ponder )
    {
    switch( cmdType )
        {
        case Command::Options:
            std::cout << command << std::endl;
            break;

        case Command::BestMove:
            std::cout << "bestmove " << command;

            if (!ponder.empty())
                std::cout << " ponder " << ponder;
            std::cout << std::endl;
            break;

        case Command::Info:
            std::cout << "info " << command << std::endl;
            break;

        default:
            std::cout << std::endl;
            break;
        }
    }