#pragma once
#include "square.h"
#include "clock.h"

const int maxPly = 1024;
class Move;

class SearchInfo
	{
	public:
	enum class Time : int { Infinite = -1 };
	SearchInfo(int time = int(Time::Infinite), int depth = 1, int nodes = 0) noexcept;
	void newSearch(int time = int(Time::Infinite));
	void stopSearch();
	int incrementDepth();
	int maxDepth();
	int Nodes();
	bool timeOver();
	void resetNodes();
	void visitNode();
	void setKillers(Move, int);
	void setHistory(Move, uint8_t, int);
	void setDepthLimit(int);
	void setGameTime(int);
	Move firstKiller(int);
	Move secondKiller(int);
	int historyScore(Move, uint8_t);
	double elapsedTime();
	int SelDepth;

	private:
	int depthLimit;
	int depth;
	int nodes;
	int history[2][64*64];
	int allocatedTime;
	Move killers[maxPly][2];
	Clock timer;
	};

inline bool SearchInfo::timeOver()
	{
	if (allocatedTime == int(Time::Infinite) && depth <= depthLimit)
	return false;

	return (timer.elapsedMilliseconds() >= allocatedTime || timer.elapsedMilliseconds() / allocatedTime >= 0.85);
	}

inline int SearchInfo::Nodes()
	{
	return nodes;
	}

inline void SearchInfo::visitNode()
	{
	++nodes;
	}

inline Move SearchInfo::firstKiller(int depth)
	{
	return killers[depth][0];
	}

inline Move SearchInfo::secondKiller(int depth)
	{
	return killers[depth][1];
	}

inline int SearchInfo::historyScore(Move move, uint8_t color)
	{
	return history[color][move.butterflyIndex()];
	}

inline double SearchInfo::elapsedTime()
	{
	return timer.elapsedMilliseconds();
	}

inline void SearchInfo::setKillers(Move move, int depth)
	{
	if (move != killers[depth][0])
		killers[depth][1] = killers[depth][0];
	killers[depth][0] = move;
	}

inline void SearchInfo::setHistory(Move move, uint8_t color, int depth)
	{
	history[color][move.butterflyIndex()] += (1 << depth);
	}


