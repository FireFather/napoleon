#include "searchinfo.h"

SearchInfo::SearchInfo(int time, int depth, int nodes)  noexcept :
    depth(depth),
    nodes(nodes)
    {
    SelDepth = 0;
    allocatedTime = time;
    setDepthLimit(100);
    }

int SearchInfo::incrementDepth()
    {
    return depth++;
    }

int SearchInfo::maxDepth()
    {
    return depth;
    }

void SearchInfo::setDepthLimit(int depth)
    {
    depthLimit = depth;
    }

void SearchInfo::newSearch(int time)
    {
    resetNodes();
    allocatedTime = time;
    depth = 1;
    std::memset(history, 0, sizeof(history));
    std::memset(killers, 0, sizeof(killers));
    timer.Restart();
    }

void SearchInfo::stopSearch()
    {
    setDepthLimit(100);
    }

void SearchInfo::resetNodes()
    {
    nodes = 0;
    }

void SearchInfo::setGameTime(int time)
    {
    allocatedTime = time;
    timer.Restart();
    }