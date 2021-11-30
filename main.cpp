#include "uci.h"
#include "search.h"

int main()
    {
	Search::Hash.SetSize(32);
	Search::InitializeThreads();
    Uci::Start();
    return 0;
    }