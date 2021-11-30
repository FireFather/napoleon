#include "searchinfo.h"

SearchInfo::SearchInfo( int time, int depth, int nodes )  noexcept :
    depth(depth),
    nodes(nodes)
    {
    SelDepth = 0;
    allocatedTime = time;
    SetDepthLimit(100);
    }

int SearchInfo::IncrementDepth()
    {
    return depth++;
    }

int SearchInfo::MaxDepth()
    {
    return depth;
    }

void SearchInfo::SetDepthLimit( int depth )
    {
    depthLimit = depth;
    }

void SearchInfo::NewSearch( int time )
    {
    ResetNodes();
    allocatedTime = time;
    depth = 1;
    std::memset(history, 0, sizeof(history));
    std::memset(killers, 0, sizeof(killers));
    timer.Restart();
    }

void SearchInfo::StopSearch()
    {
    SetDepthLimit(100);
    }

void SearchInfo::ResetNodes()
    {
    nodes = 0;
    }

void SearchInfo::SetGameTime( int time )
    {
    allocatedTime = time;
    timer.Restart();
    }