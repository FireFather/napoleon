#pragma once
#include <vector>
#include "searchinfo.h"
#include "smpinfo.h"

enum class SearchType
    {
    Infinite,
    TimePerGame,
    TimePerMove,
    Ponder
    };

enum class NodeType
    {
    PV,
    NONPV,
    CUT,
    ALL
    };

class Position;
class hashTable;

namespace Search
    {
    extern bool pondering;
    extern std::atomic<bool> ponderHit;
    extern std::atomic<bool> stopSignal;
    extern int moveTime;
    extern int gameTime[2];
    extern thread_local SearchInfo searchInfo;
    extern thread_local bool sendOutput;
    extern hashTable Hash;
    extern std::condition_variable smp;
    extern SMPInfo smpInfo;
    extern std::vector<std::thread> threads;
    extern int depth_limit;
    extern int cores;
    extern std::atomic<bool> quit;
    extern const int defaultCores;

    void initThreads(int = defaultCores);
    void killThreads();
    void signalThreads(int, int, int, const Position &, bool);
    void smpSearch();
	int razorMargin(int);
	int futilityMargin(int);
    int predictTime(uint8_t);

    std::string getInfo(Position &, Move, int, int, int);
    std::string getPV(Position &, Move, int);
    Move getPonderMove(Position &, const Move);
    Move startThinking(SearchType, Position &, bool = true);
    void stopThinking();
    Move iterativeSearch(Position &);
    int searchRoot(int, int, int, Move &, Position &);
    template <NodeType>
    int search(int, int, int, int, Position &, bool);
    int quiescence(int, int, Position &);

    const int Infinity = 200000;
    const int Unknown = 2 * Infinity;
    const int Mate = std::numeric_limits<short>::max();
    }
