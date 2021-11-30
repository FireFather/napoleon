#include "search.h"
#include "benchmark.h"
#include "searchterms.h"

using namespace std;
Position Uci::position;
thread Uci::search;

void Uci::Start()
    {
    cout.setf(ios::unitbuf);
    string line;
    string cmd;
    bool exit = false;
    position.loadFen();

    while (!exit && getline(cin, line))
        {
        istringstream stream(line);
        stream >> cmd;
        if (cmd == "uci")
            {
			cout << "id name " << ENGINE << " " << VERSION << " " << PLATFORM << endl;
			cout << "id author " << AUTHOR << endl;
			cout << "option name Hash type spin default 32 min 1 max 1024" << endl;
			cout << "option name Threads type spin default 1 min 1 max 64" << endl;
			cout << "uciok" << endl;
            }
		else if (cmd == "isready")
			{
			cout << "readyok" << endl;
			}
		else if (cmd == "ucinewgame")
			{
			Search::Hash.Clear();
			}
        else if (cmd == "setoption")
            {
            string token;
            stream >> token;
            stream >> token;
            if (token == "Hash")
                {
                int size;
                stream >> token;
                stream >> size;
                Search::Hash.setSize(size);
                }
            else if (token == "Threads")
                {
                int threads;
                stream >> token;
                stream >> threads;
                Search::initThreads(threads);
                }
            }
        else if (cmd == "position")
            {
            Move move;
            string token;
            stream >> token;
            if (token == "startpos")
                {
                position.loadFen();
                stream >> token;
                }
            else if (token == "fen")
                {
                string fen;
                while (stream >> token && token != "moves")
                    fen += token + " ";
                position.loadFen(fen);
                }
            while (stream >> token && !(move = position.parseMove(token)).isNull())
                {
                position.makeMove(move);
                }
            }
        else if (cmd == "go")
            {
            if (Search::stopSignal)
                Go(stream);
            }
		else if (cmd == "stop")
			{
			Search::stopThinking();
			}
		else if (cmd == "quit")
			{
			Search::killThreads();
			exit = true;
			}
        else if (cmd == "ponderhit")
            {
            Search::ponderHit = true;
            }
		else if (cmd == "perft")
			{
			Benchmark bench(position);
			int depth = 6;
			stream >> depth;
			bench.runPerft(depth);
			}
		else if (cmd == "divide")
			{
			Benchmark bench(position);
			int depth = 6;
			stream >> depth;
			bench.runDivide(depth);
			}
		else if (cmd == "bench")
			{
			Benchmark bench(position);
			int depth = 8;
			stream >> depth;
			bench.ttdTest(depth);
			}
		else if (cmd == "perfttest")
			{
			Benchmark bench(position);
			bench.perftTest();
			}
        else if (cmd == "disp")
            {
            position.Display();
            }
        }
    }

void Uci::Go(istringstream &stream)
    {
    string token;
    SearchType type = SearchType::Infinite;

    while (stream >> token)
        {
        if (token == "depth")
            {
            stream >> Search::depth_limit;
            type = SearchType::Infinite;
            }
        else if (token == "movetime")
            {
            stream >> Search::moveTime;
            type = SearchType::TimePerMove;
            }
        else if (token == "wtime")
            {
            stream >> Search::gameTime[pieceColor::White];
            type = SearchType::TimePerGame;
            }
        else if (token == "btime")
            {
            stream >> Search::gameTime[pieceColor::Black];
            type = SearchType::TimePerGame;
            }
        else if (token == "infinite")
            {
            type = SearchType::Infinite;
            }
        else if (token == "ponder")
            {
            type = SearchType::Ponder;
            }
        }
    search = thread(Search::startThinking, type, ref(position), true);
    search.detach();
    }

void Uci::engineInfo()
	{
	char* startup_banner = "" ENGINE " " VERSION " " PLATFORM "\n";
	cout << startup_banner;
	}