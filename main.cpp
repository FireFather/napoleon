#include "search.h"

int main()
    {
	Uci::engineInfo();
	Search::Hash.setSize(32);
	Search::initThreads();
    Uci::Start();
    return 0;
    }