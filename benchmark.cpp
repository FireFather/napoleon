#include "defines.h"
#include "benchmark.h"
#include "movegen.h"
#include "move.h"
#include "moves.h"
#include "string.h"
#include "uci.h"
#include "search.h"
#include "searchinfo.h"
#include <windows.h>

Benchmark::Benchmark(Position &position) :
    position(position)
    {
    }

uint64_t Benchmark::Perft(int depth)
    {
    int pos = 0;
    Move moves[moveGen::maxMoves];
    uint64_t nodes = 0;
    moveGen::getLegalMoves(moves, pos, position);

    if (depth == 1)
        return pos;
    if (depth == 0)
        return 1;

    for (int i = 0; i < pos; i++)
        {
        position.makeMove(moves[i]);
        nodes += Perft(depth - 1);
        position.undoMove(moves[i]);
        }
    return nodes;
    }

void Benchmark::runPerft(int depth)
    {
	position.Display();
    cout << "Depth: " << depth << endl;
    Clock timer = Clock::startNow();
    uint64_t nodes = Perft(depth);
    double time = timer.elapsedMilliseconds();
    double nps = nodes / time;
    cout << "Nodes: " << nodes << endl;
    cout << "Time : " << time << " ms" << endl;
    std::ostringstream ss;
    ss.precision(1);
    ss << "Speed: " << std::fixed << nps << " kNps" << endl;
    std::cout << ss.str();
    }

uint64_t Benchmark::Divide(int depth)
    {
    int pos = 0;
    Move moves[moveGen::maxMoves];
    uint64_t nodes = 0;
    int32_t moveNum = 0;
    moveGen::getLegalMoves(moves, pos, position);

    for (int i = 0; i < pos; i++)
        {
        position.makeMove(moves[i]);
        uint64_t delta = Loop(depth - 1);
        nodes += delta;
        position.undoMove(moves[i]);
        string num = int32ToStr(++moveNum);
        if (1 == num.size())
            num = " " + num;
        string line = "move " + num + ": " + moves[i].toAlgebraic() + " " + int64ToStr(delta) + " nodes";
		cout << line << endl;
        }
    return nodes;
    }

void Benchmark::runDivide(int depth)
    {
	position.Display();
    cout << "Depth: " << depth << endl;
    Clock timer = Clock::startNow();
    uint64_t nodes = Divide(depth);
    double time = timer.elapsedMilliseconds();
    double nps = nodes / time;
    cout << "Nodes: " << nodes << endl;
    cout << "Time : " << time << " ms" << endl;
    std::ostringstream ss;
    ss.precision(1);
    ss << "Speed: " << std::fixed << nps << " kNps" << endl;
    std::cout << ss.str();
    }

uint64_t Benchmark::Loop(int depth)
    {
    int pos = 0;
    if (depth == 0)
        return (1);

    Move moves[moveGen::maxMoves];
    moveGen::getLegalMoves(moves, pos, position);
    uint64_t count = 0;

    for (int i = 0; i < pos; ++i)
        {
        position.makeMove(moves[i]);
        count += Perft(depth - 1);
        position.undoMove(moves[i]);
        }
    return (count);
    }

struct PerftItem
    {
    string fen;
    uint32_t depth;
    uint64_t count;
    PerftItem(string f, uint32_t d, uint64_t c)
        {
        fen = f;
        depth = d;
        count = c;
        }
    };

struct BenchItem
    {
    string fen;
    BenchItem(string f)
        {
        fen = f;
        }
    };

void Benchmark::perftTest()
    {
    typedef vector<PerftItem> vecPerftItems;
    vecPerftItems items;
    Position position;

    items.push_back(PerftItem("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 1, 20));
    items.push_back(PerftItem("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 2, 400));
    items.push_back(PerftItem("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 3, 8902));
    items.push_back(PerftItem("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4, 197281));
    items.push_back(PerftItem("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5, 4865609));
    items.push_back(PerftItem("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 6, 119060324));
    items.push_back(PerftItem("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 7, 3195901860));
    items.push_back(PerftItem("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 8, 84998978956));
	items.push_back(PerftItem("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 9, 2439530234167));
	items.push_back(PerftItem("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 10, 69352859712417));

    vecPerftItems::iterator it = items.begin();
    while (it != items.end())
        {
        vecPerftItems::value_type &item = *it++;
        position.loadFen(item.fen);
		cout << "Fen: " + item.fen << endl;
		cout << "Depth: " << item.depth << endl;
        uint64_t nodes = Perft(item.depth);
		cout << "Nodes counted: " + int64ToStr(nodes) << endl;
		cout << "Correct count: " + int64ToStr(item.count) << endl;
		cout << endl;
        }
    position.loadFen(startPosition);
    }

void Benchmark::ttdTest(int depth)
    {
	static FILE *benchLog;
	static char fileName[256];
	char buf[256];

    Search::depth_limit = depth;
    SearchType type = SearchType::Infinite;
    typedef vector<BenchItem> vecBenchItems;
    vecBenchItems items;

    items.push_back(BenchItem("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -"));
    items.push_back(BenchItem("r1bn1rk1/ppp1qppp/3pp3/3P4/2P1n3/2B2NP1/PP2PPBP/2RQK2R w K -"));
    items.push_back(BenchItem("r2q1rk1/1bppbppp/p4n2/n2Np3/Pp2P3/1B1P1N2/1PP2PPP/R1BQ1RK1 w - -"));
    items.push_back(BenchItem("rnb2rk1/1pq1bppp/p3pn2/3p4/3NPP2/2N1B3/PPP1B1PP/R3QRK1 w - -"));
    items.push_back(BenchItem("2rq1rk1/p3bppp/bpn1pn2/2pp4/3P4/1P2PNP1/PBPN1PBP/R2QR1K1 w - -"));
    items.push_back(BenchItem("rn3rk1/1p2ppbp/1pp3p1/3n4/3P1Bb1/2N1PN2/PP3PPP/2R1KB1R w K -"));
    items.push_back(BenchItem("r1bq1rk1/3nbppp/p1p1pn2/1p4B1/3P4/2NBPN2/PP3PPP/2RQK2R w K -"));
    items.push_back(BenchItem("r3kbnr/1bpq2pp/p2p1p2/1p2p3/3PP2N/1PN5/1PP2PPP/R1BQ1RK1 w kq -"));
    items.push_back(BenchItem("r1b1k2r/pp1nqp1p/2p3p1/3p3n/3P4/2NBP3/PPQ2PPP/2KR2NR w kq -"));
    items.push_back(BenchItem("r2q1rk1/1b2ppbp/ppnp1np1/2p5/P3P3/2PP1NP1/1P1N1PBP/R1BQR1K1 w - -"));
    items.push_back(BenchItem("r2q1rk1/pp2ppbp/2n1bnp1/3p4/4PPP1/1NN1B3/PPP1B2P/R2QK2R w KQ -"));
    items.push_back(BenchItem("r1bq1rk1/bpp2ppp/p2p1nn1/4p3/4P3/1BPP1NN1/PP3PPP/R1BQ1RK1 w - -"));
    items.push_back(BenchItem("rn3rk1/pbppqpp1/1p2p2p/8/2PP4/2Q1PN2/PP3PPP/R3KB1R w KQ -"));
    items.push_back(BenchItem("r2q1rk1/p1p2ppp/2p1pb2/3n1b2/3P4/P4N1P/1PP2PP1/RNBQ1RK1 w - -"));
    items.push_back(BenchItem("rnb2rk1/p4ppp/1p2pn2/q1p5/2BP4/P1P1PN2/1B3PPP/R2QK2R w KQ -"));
    items.push_back(BenchItem("r2q1rk1/1p1bbppp/p1nppn2/8/3NPP2/2N1B3/PPPQB1PP/R4RK1 w - -"));
    items.push_back(BenchItem("r2q1rk1/3nbppp/bpp1pn2/p1Pp4/1P1P1B2/P1N1PN1P/5PP1/R2QKB1R w KQ -"));
    items.push_back(BenchItem("r1b1r1k1/pp1nqppp/2pbpn2/8/2pP4/2N1PN1P/PPQ1BPP1/R1BR2K1 w - -"));
    items.push_back(BenchItem("r3kbnr/1bqp1ppp/p3p3/1p2P3/5P2/2N2B2/PPP3PP/R1BQK2R w KQkq -"));
    items.push_back(BenchItem("r2q1rk1/pb1n1ppp/1p1ppn2/2p3B1/2PP4/P1Q2P2/1P1NP1PP/R3KB1R w KQ -"));
    items.push_back(BenchItem("r1bq1rk1/pp1n1ppp/4p3/2bpP3/3n1P2/2N1B3/PPPQ2PP/2KR1B1R w - -"));
    items.push_back(BenchItem("r2q1rk1/ppp1bppp/2n1b3/3np3/8/P1NPBNP1/1P2PPBP/R2Q1RK1 w - -"));
    items.push_back(BenchItem("2q1r1k1/1ppb4/r2p1Pp1/p4n1p/2P1n3/5NPP/PP3Q1K/2BRRB2 w - -"));
    items.push_back(BenchItem("7r/1p2k3/2bpp3/p3np2/P1PR4/2N2PP1/1P4K1/3B4 b - -"));
    items.push_back(BenchItem("4k3/p1P3p1/2q1np1p/3N4/8/1Q3PP1/6KP/8 w - -"));
    items.push_back(BenchItem("2r1b1k1/R4pp1/4pb1p/1pBr4/1Pq2P2/3N4/2PQ2PP/5RK1 b - -"));
    items.push_back(BenchItem("6k1/p1qb1p1p/1p3np1/2b2p2/2B5/2P3N1/PP2QPPP/4N1K1 b - -"));
    items.push_back(BenchItem("3q4/pp3pkp/5npN/2bpr1B1/4r3/2P2Q2/PP3PPP/R4RK1 w - -"));
    items.push_back(BenchItem("3rr1k1/pb3pp1/1p1q1b1p/1P2NQ2/3P4/P1NB4/3K1P1P/2R3R1 w - -"));
    items.push_back(BenchItem("r1b1r1k1/p1p3pp/2p2n2/2bp4/5P2/3BBQPq/PPPK3P/R4N1R b - -"));
    items.push_back(BenchItem("3r4/1b2k3/1pq1pp2/p3n1pr/2P5/5PPN/PP1N1QP1/R2R2K1 b - -"));
    items.push_back(BenchItem("2r4k/pB4bp/6p1/6q1/1P1n4/2N5/P4PPP/2R1Q1K1 b - -"));
    items.push_back(BenchItem("1r5r/3b1pk1/3p1np1/p1qPp3/p1N1PbP1/2P2PN1/1PB1Q1K1/R3R3 b - -"));
    items.push_back(BenchItem("5rk1/7p/p1N5/3pNp2/2bPnqpQ/P7/1P3PPP/4R1K1 w - -"));
    items.push_back(BenchItem("rnb2rk1/pp2np1p/2p2q1b/8/2BPPN2/2P2Q2/PP4PP/R1B2RK1 w - -"));
    items.push_back(BenchItem("2k4r/1pp2ppp/p1p1bn2/4N3/1q1rP3/2N1Q3/PPP2PPP/R4RK1 w - -"));
    items.push_back(BenchItem("r3kb1r/pp2pppp/3q4/3Pn3/6b1/2N1BN2/PP3PPP/R2QKB1R w KQkq -"));
    items.push_back(BenchItem("2rr2k1/1b3p1p/p4qpb/2R1n3/3p4/BP2P3/P3QPPP/3R1BKN b - -"));
    items.push_back(BenchItem("r1b1k3/5p1p/p1p5/3np3/1b2N3/4B3/PPP1BPrP/2KR3R w q -"));
    items.push_back(BenchItem("r3rbk1/1pq2ppp/2ppbnn1/p3p3/P1PPN3/BP1BPN1P/2Q2PP1/R2R2K1 w - -"));
    items.push_back(BenchItem("b7/2q2kp1/p3pbr1/1pPpP2Q/1P1N3P/6P1/P7/5RK1 w - -"));
    items.push_back(BenchItem("1rr1nbk1/5ppp/3p4/1q1PpN2/np2P3/5Q1P/P1BB1PP1/2R1R1K1 w - -"));
    items.push_back(BenchItem("r7/5kp1/2p1p2p/1p1n3P/2rP4/2P3R1/PK2RPP1/2B5 b - -"));
    items.push_back(BenchItem("1N2k3/5p2/p2P2p1/3Pp3/pP3b2/5P1r/P7/1K4R1 b - -"));
    items.push_back(BenchItem("2k2R2/6r1/8/B2pp2p/1p6/3P4/PP2b3/2K5 b - -"));
    items.push_back(BenchItem("2k5/1pp5/2pb2p1/7p/6n1/P5N1/1PP3PP/2K1B3 b - -"));
    items.push_back(BenchItem("2n5/1k6/3pNn2/3ppp2/7p/4P2P/1P4P1/5NK1 w - -"));
    items.push_back(BenchItem("5nk1/B4p2/7p/6p1/3N3n/2r2PK1/5P1P/4R3 b - -"));
    items.push_back(BenchItem("8/1p3pkp/p1r3p1/3P3n/3p1P2/3P4/PP3KP1/R3N3 b - -"));
    items.push_back(BenchItem("8/2B2k2/p2p2pp/2pP1p2/2P2P2/2b1N1PP/P4K2/2n5 b - -"));
    items.push_back(BenchItem("8/4p1kp/1n1p2p1/nPp5/b5P1/P5KP/3N1P2/4NB2 w - -"));
    items.push_back(BenchItem("r1b3k1/2p4p/3p1p2/1p1P4/1P3P2/P5P1/5KNP/R7 b - -"));
    items.push_back(BenchItem("1k2b3/1pp5/4r3/R3N1pp/1P3P2/p5P1/2P4P/1K6 w - -"));
    items.push_back(BenchItem("1r2b3/p3p1kp/1p4p1/2pPP3/P1P1B3/R7/3K2PP/8 w - -"));
    items.push_back(BenchItem("1r6/5ppk/R6p/P3p3/1Pn5/6P1/2p2P1P/2B4K w - -"));
    items.push_back(BenchItem("2k5/3n1pb1/p2n2pp/2pP4/2P2PP1/1K3N1P/2B5/4B3 w - -"));
    items.push_back(BenchItem("2n5/7r/1p1k4/2nP1p2/4P3/P3KP1P/3R4/5B2 w - -"));
    items.push_back(BenchItem("3b3k/5p2/1n1P4/p1p1P2p/P1p5/2P4b/3N2N1/3B3K b - -"));
    items.push_back(BenchItem("3k4/2p3pp/3p1b2/3P3P/b7/P3BB2/1P3P2/2K5 w - -"));
    items.push_back(BenchItem("3R1bk1/7p/1p2P1p1/3P4/pP6/P2N2P1/4p1KP/5r2 b - -"));
    items.push_back(BenchItem("3rn3/p4p2/1p3k2/6pp/2PpB3/P2K2P1/1P4PP/4R3 b - -"));
    items.push_back(BenchItem("4n3/2k1b3/p6p/P5p1/2K2pP1/5B1P/5P2/4B3 w - -"));
    items.push_back(BenchItem("4n3/p5k1/2P3pp/2P5/P3pp2/2K3P1/5r1P/R4N2 w - -"));
    items.push_back(BenchItem("6k1/p7/6pp/1p1Pp3/2n1P1Pb/6NP/P4KP1/B7 w - -"));

    vecBenchItems::iterator it = items.begin();

	Clock timer = Clock::startNow();
    while (it != items.end())
        {
        if (Search::stopSignal)
            {
            vecBenchItems::value_type &item = *it++;
            position.loadFen(item.fen);
            position.Display();
            Uci::search = thread(Search::startThinking, type, ref(position), true);
            Uci::search.detach();
            Search::Hash.Clear();
            }
        Sleep(500);
        }
	double elapsedTime = timer.elapsedMilliseconds() / 1000;

    Sleep(2000);

	cout << "Depth: " << depth << endl;
	std::ostringstream t;
	t.precision(3);
    t << "Time: " << std::fixed << elapsedTime << " secs" << endl;
    std::cout << t.str();

	time_t now = time(0);
	strftime(buf, 32, "%b-%d_%H-%M", localtime(&now));
	sprintf(fileName, "bench_%s.txt", buf);
	printf("results written to:\n");
	printf("benchmark_%s.txt\n\n", buf);
	benchLog = fopen(fileName, "wt");
	fprintf(benchLog, "" ENGINE " " VERSION " " PLATFORM "\n");
	fprintf(benchLog, "TTD bench\n\n");
	fprintf(benchLog, "Depth: %d\n", depth);
	fprintf(benchLog, "Time: %.2f secs\n", elapsedTime);
	fclose(benchLog);
	
    position.loadFen(startPosition);
    }
