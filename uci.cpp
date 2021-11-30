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
    position.LoadFen();

    while (!exit && getline(cin, line))
        {
        istringstream stream(line);
        stream >> cmd;

        if (cmd == "uci")
            {
            Send<Command::Options>("id name Napoleon");
            Send<Command::Options>("id author Marco Pampaloni");
            Send<Command::Options>("option name Hash type spin default 32 min 1 max 1024");
            Send<Command::Options>("option name Threads type spin default 1 min 1 max 64");
            Send<Command::Options>("uciok");
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
                Search::Hash.SetSize(size);
                }
            else if (token == "Threads")
                {
                int threads;
                stream >> token;
                stream >> threads;
                Search::InitializeThreads(threads);
                }
            }
        else if (cmd == "quit")
            {
            Search::KillThreads();
            exit = true;
            }
        else if (cmd == "isready")
            {
            Send<Command::Options>("readyok");
            }
        else if (cmd == "ucinewgame")
            {
            Search::Hash.Clear();
            }
        else if (cmd == "stop")
            {
            Search::StopThinking();
            }
        else if (cmd == "perft")
            {
            Benchmark bench(position);
            int depth = 6;
            stream >> depth;

            Clock timer = Clock::StartNew();
            uint64_t nodes = bench.Perft(depth);
            double time = timer.ElapsedMilliseconds();
            double nps = nodes / time;

            cout << "Perft: (" << depth << ")" << endl;
            cout << "Nodes: " << nodes << endl;
            cout << "Time : " << time << " ms" << endl;

            std::ostringstream ss;
            ss.precision(1);
            ss << "Speed: " << std::fixed << nps << " kNps" << endl;
            std::cout << ss.str();
            }
        else if (cmd == "position")
            {
            Move move;
            string token;
            stream >> token;

            if (token == "startpos")
                {
                position.LoadFen();
                stream >> token;
                }
            else if (token == "fen")
                {
                string fen;

                while (stream >> token && token != "moves")
                    fen += token + " ";
                position.LoadFen(fen);
                }
            while (stream >> token && !(move = position.ParseMove(token)).IsNull())
                {
                position.MakeMove(move);
                }
            }
        else if (cmd == "go")
            {
            if (Search::StopSignal)
                Go(stream);
            }
        else if (cmd == "ponderhit")
            {
            Search::PonderHit = true;
            }
		else if (cmd == "disp")
            {
			position.Display();
            }			
        }
    }

void Uci::Go( istringstream &stream )
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
            stream >> Search::MoveTime;
            type = SearchType::TimePerMove;
            }
        else if (token == "wtime")
            {
            stream >> Search::GameTime[PieceColor::White];
            type = SearchType::TimePerGame;
            }
        else if (token == "btime")
            {
            stream >> Search::GameTime[PieceColor::Black];
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
    search = thread(Search::StartThinking, type, ref(position), true);
    search.detach();
    }