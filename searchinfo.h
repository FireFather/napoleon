#pragma once
#include "square.h"
#include "clock.h"

const int MaxPly = 1024;
class Move;

class SearchInfo
	{
	public:
	enum class Time : int { Infinite = -1 };
	SearchInfo(int time = int(Time::Infinite), int depth = 1, int nodes = 0) noexcept;
	void NewSearch(int time = int(Time::Infinite));
	void StopSearch();
	int IncrementDepth();
	int MaxDepth();
	int Nodes();
	bool TimeOver();
	void ResetNodes();
	void VisitNode();
	void SetKillers(Move, int);
	void SetHistory(Move, uint8_t, int);
	void SetDepthLimit(int);
	void SetGameTime(int);
	Move FirstKiller(int);
	Move SecondKiller(int);
	int HistoryScore(Move, uint8_t);
	double ElapsedTime();
	int SelDepth;

	private:
	int depthLimit;
	int depth;
	int nodes;
	int history[2][64*64];
	int allocatedTime;
	Move killers[MaxPly][2];
	Clock timer;
	};

inline bool SearchInfo::TimeOver()
	{
	if (allocatedTime == int(Time::Infinite) && depth <= depthLimit)
	return false;

	return (timer.ElapsedMilliseconds() >= allocatedTime || timer.ElapsedMilliseconds() / allocatedTime >= 0.85);
	}

inline int SearchInfo::Nodes()
	{
	return nodes;
	}

inline void SearchInfo::VisitNode()
	{
	++nodes;
	}

inline Move SearchInfo::FirstKiller(int depth)
	{
	return killers[depth][0];
	}

inline Move SearchInfo::SecondKiller(int depth)
	{
	return killers[depth][1];
	}

inline int SearchInfo::HistoryScore(Move move, uint8_t color)
	{
	return history[color][move.ButterflyIndex()];
	}

inline double SearchInfo::ElapsedTime()
	{
	return timer.ElapsedMilliseconds();
	}

inline void SearchInfo::SetKillers(Move move, int depth)
	{
	if (move != killers[depth][0])
		killers[depth][1] = killers[depth][0];
	killers[depth][0] = move;
	}

inline void SearchInfo::SetHistory(Move move, uint8_t color, int depth)
	{
	history[color][move.ButterflyIndex()] += (1 << depth);
	}


