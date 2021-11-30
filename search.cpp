#include <random>
#include "search.h"
#include "eval.h"
#include "movepick.h"
#include "searchterms.h"

HashTable Search::Hash;
bool Search::pondering = false;
std::atomic<bool> Search::PonderHit( false );
std::atomic<bool> Search::StopSignal( true );
std::atomic<bool> Search::quit( false );
int Search::GameTime[2];
int Search::MoveTime;
thread_local bool Search::sendOutput = false;
thread_local SearchInfo Search::searchInfo;
std::vector<std::thread> Search::threads;
SMPInfo Search::smpInfo;
std::condition_variable Search::smp;
std::mutex mux;
int Search::depth_limit = 100;
int Search::cores;
const int Search::default_cores = 1;

int Search::razorMargin(int depth)
	{
	return (rzMultiplier * (depth - 1) + rzMargin);
	}
int Search::futilityMargin(int depth)
	{
	return (fpMargin * depth);
	}

int Search::predictTime( uint8_t color )
    {
    int gameTime = GameTime[color];
    return gameTime / 30 - (gameTime / (60*1000));
    }

Move Search::StartThinking( SearchType type, Position &position, bool verbose)
    {
    Hash.Clear();

    sendOutput = verbose;
    StopSignal = false;
    pondering = false;
    PonderHit = false;
    searchInfo.SetDepthLimit(depth_limit);

    if (type == SearchType::Infinite || type == SearchType::Ponder)
        {
        if (type == SearchType::Ponder)
            pondering = true;
        searchInfo.NewSearch();
        }
    else
        {
        int time;

        if (type == SearchType::TimePerGame)
            {
            int gameTime = GameTime[position.SideToMove()];
            time = gameTime / 30 - (gameTime / (60 * 1000));
            }
        else
            {
            time = MoveTime;
            }

        searchInfo.NewSearch(time);
        }

    Move move = iterativeSearch(position);

	if (sendOutput)
		{
		Move ponder = getPonderMove(position, move);

		if (ponder.IsNull())
			Uci::Send<Command::BestMove>(move.ToAlgebraic());
		else
			Uci::Send<Command::BestMove>(move.ToAlgebraic(), ponder.ToAlgebraic());
		}
    searchInfo.StopSearch();
    return move;
    }

void Search::StopThinking()
    {
    StopSignal = true;
    smpInfo.SetReady(false);
    }

void Search::KillThreads()
    {
    quit = true;
    smp.notify_all();

    for ( auto& t: threads )
        t.join();
    threads.clear();
    quit = false;
    }

void Search::InitializeThreads(int num_threads)
    {
	KillThreads();
		{
		std::lock_guard<std::mutex> lock(mux);
		smpInfo.SetReady(false);
		}
        cores=num_threads;
        for (int i=1; i<cores; i++)
            threads.push_back(std::thread(smpSearch));
    }

void Search::signalThreads(int depth, int alpha, int beta, const Position& position, bool ready)
    {
	std::unique_lock<std::mutex> lock(mux);
	smpInfo.UpdateInfo(depth, alpha, beta, position, ready);
	lock.unlock();
	smp.notify_all();
    }

void Search::smpSearch()
    {
	std::default_random_engine eng;
	std::uniform_int_distribution<int> score_dist(0, 25); // to tune
	std::uniform_int_distribution<int> depth_dist(0, 0); // to tune

	/* thread local information */
	sendOutput = false;
	searchInfo.NewSearch();

	Move* move = new Move();
	Position* position = new Position();
	while(!quit)
		{
		std::unique_lock<std::mutex> lock(mux);
		smp.wait(lock, []{return quit || smpInfo.Ready();});
		auto info = smpInfo;
		lock.unlock();
		if (quit)
			break;
		int rand_window = score_dist(eng);
		auto fen = info.Board().GetFen();

		if(position->GetFen() != fen)
			{
			searchInfo.NewSearch();
			*position = info.Board();
            }
            searchRoot(info.Depth(), info.Alpha() - rand_window, info.Beta() + rand_window, std::ref(*move), std::ref(*position));
        }
    }

    // iterative deepening
Move Search::iterativeSearch( Position &position )
    {
    Move move;
    Move toMake = NullMove;
    int score;
    int temp;

    score = searchRoot(searchInfo.MaxDepth(), -Search::Infinity, Search::Infinity, move, position);
    searchInfo.IncrementDepth();

    while ((searchInfo.MaxDepth() < 100 && !searchInfo.TimeOver()) || pondering)
        {
        if (StopSignal)
            break;

        if (PonderHit)
            {
            searchInfo.SetGameTime(predictTime(position.SideToMove()));
            PonderHit = false;
            pondering = false;
            }

        searchInfo.SelDepth = 0;
        searchInfo.ResetNodes();

        if (searchInfo.MaxDepth() > 5 && cores > 1)
            signalThreads(searchInfo.MaxDepth(), -Search::Infinity, Search::Infinity, position, true);

        // aspiration search
        temp = searchRoot(searchInfo.MaxDepth(), score - aspirationValue, score + aspirationValue, move, position);

        if (temp <= score - aspirationValue)
            temp = searchRoot(searchInfo.MaxDepth(), -Search::Infinity, score + aspirationValue, move, position);

        if (temp >= score + aspirationValue)
            temp = searchRoot(searchInfo.MaxDepth(), score - aspirationValue, Search::Infinity, move, position);

        score = temp;

        if (score != Search::Unknown)
            toMake = move;

        searchInfo.IncrementDepth();
        }

    StopThinking();

    return toMake;
    }

int Search::searchRoot( int depth, int alpha, int beta, Move &moveToMake, Position &position )
    {
    int score;
    int startTime = int(searchInfo.ElapsedTime());

    MovePick moves(position, searchInfo);
    MoveGen::GetLegalMoves(moves.moves, moves.count, position);

    // chopper pruning
    if (moves.count == 1)
        {
        moveToMake = moves.Next();
        return alpha;
        }

    moves.Sort<false>();

    int i = 0;

    for ( auto move = moves.First(); !move.IsNull(); move = moves.Next(), i++ )
        {
        if ((searchInfo.TimeOver() || StopSignal))
            return Search::Unknown;

        position.MakeMove(move);

        if (i == 0)
            score = -Search::search<NodeType::PV>(depth - 1, -beta, -alpha, 1, position, false);
        else
            {
            score = -search<NodeType::NONPV>(depth - 1, -alpha - 1, -alpha, 1, position, true);

            if (score > alpha)
                score = -search<NodeType::PV>(depth - 1, -beta, -alpha, 1, position, false);
            }
        position.UndoMove(move);

        if (score > alpha)
            {
            moveToMake = move;

            if (score >= beta)
                {
                if (sendOutput)
                    Uci::Send <Command::Info>(GetInfo(position, moveToMake, beta, depth, startTime));
                return beta;
                }

            alpha = score;
            }
        }

    if (sendOutput)
        Uci::Send<Command::Info>(GetInfo(position, moveToMake, alpha, depth, startTime));

    return alpha;
    }

template <NodeType node_type>
int Search::search( int depth, int alpha, int beta, int ply, Position &position, bool cut_node )
    {
    searchInfo.VisitNode();

    ScoreType bound = ScoreType::Alpha;
    const bool pv = node_type == NodeType::PV;
    bool futility = false;
    bool extension = false;
    int score;
    int legal = 0;
    Move best = NullMove;

    if (ply > searchInfo.SelDepth)
        searchInfo.SelDepth = ply;

    if (searchInfo.Nodes() % 10000 == 0 && sendOutput)
        if (searchInfo.TimeOver())
            StopSignal = true;

    if (StopSignal)
        return alpha;

    // mate distance pruning
    alpha = std::max(alpha, -Search::Mate + ply);
    beta = std::min(beta, Search::Mate - ply - 1);

    if (alpha >= beta)
        return alpha;

    // Hash table lookup
    auto hashHit = Hash.Probe(position.zobrist, depth, alpha, beta);

    if ((score = hashHit.first) != HashTable::Unknown)
        return score;
    best = hashHit.second;

    uint64_t attackers = position.KingAttackers(position.KingSquare(position.SideToMove()), position.SideToMove());

    if (attackers)
        {
        extension = true;
        ++depth;
        }

    if (depth == 0)
        return quiescence(alpha, beta, position);

    if (position.IsRepetition())
        return 0;

    int eval = Eval::Evaluate(position);

	// cutoff
    if (depth <= coDepth
		&& !pv
		&& !attackers
		&& std::abs(alpha) < Search::Mate - MaxPly
        && std::abs(beta) < Search::Mate - MaxPly
		&& eval - coMultiplier * depth >= beta)
        {
        return beta;
        }

    // null move pruning
    if (position.AllowNullMove()
    	&& !pv
    	&& depth >= nmpDepth
		&& !attackers
		&& !position.EndGame())
        {
        int R = depth >= nmpReductionDepth ? nmpReduction1 : nmpReduction2;

        // cut node
        position.MakeNullMove();

        // make a null-window search
        score = -search<NodeType::NONPV>(depth - R - 1, -beta, -beta + 1, ply, position, !cut_node);
        position.UndoNullMove();

        if (score >= beta)
            return beta;
        }

    // internal iterative deepening (IID)
    if (depth >= iidDepth && best.IsNull() && pv)
        {
        int R = iidReduction;

        if (position.AllowNullMove())
            position.ToggleNullMove();

        search<node_type>(depth - R - 1, alpha, beta, ply, position, cut_node);

        if (!position.AllowNullMove())
            position.ToggleNullMove();

        //hash table lookup
        hashHit = Hash.Probe(position.zobrist, depth, alpha, beta);

        best = hashHit.second;
        }

    // razoring
    if (!pv
		&& depth <= rzDepth
		&& eval + razorMargin(depth) <= alpha)
        {
        int res = quiescence(alpha - razorMargin(depth), beta - razorMargin(depth), position);

        if (res + razorMargin(depth) <= alpha)
            depth--;

        if (depth <= 0)
            return alpha;
        }

	// futility pruning
    if (!pv
		&& depth <= fpDepth
		&& eval + futilityMargin(depth) <= alpha)
        futility = true;

    MovePick moves(position, searchInfo);

    MoveGen::GetPseudoLegalMoves<false>(moves.moves, moves.count, attackers, position); // get captures and non-captures

    moves.Sort<false>(ply);
    moves.hashMove = best;

    // principal variation search
    bool capture;
    bool pruned = false;

    int moveNumber = 0;
    int newDepth = depth;
    uint64_t pinned = position.PinnedPieces();

    for ( auto move = moves.First(); !move.IsNull(); move = moves.Next() )
        {
        if (position.IsMoveLegal(move, pinned))
            {
            legal++;
            int E = 0;
            newDepth = depth + E;
            capture = position.IsCapture(move);
            position.MakeMove(move);

            // futility pruning
            if (futility
				&& moveNumber > 0
				&& !capture
				&& !move.IsPromotion()
                && !position.KingAttackers(position.KingSquare(position.SideToMove()), position.SideToMove()))
                {
                pruned = true;
                position.UndoMove(move);
                continue;
                }

            if (moveNumber == 0)
                {
                score = -search<node_type>(newDepth - 1, -beta, -alpha, ply + 1, position, !cut_node);
                }
            else
                {
                register int R = 0;
                register int lmrN = newDepth >= lmrNewDepth ? lmrN1 : lmrN2;

                // late move reduction
				if (moveNumber >= lmrN
					&& newDepth >= lmrDepth
					&& !extension
					&& !capture
					&& !move.IsPromotion()
                	&& !attackers
                    && move != searchInfo.FirstKiller(ply)
                    && move != searchInfo.SecondKiller(ply)
                    && !position.KingAttackers(position.KingSquare(position.SideToMove()), position.SideToMove()))
                    {
                    R = lmr1;
                    if (moveNumber > lmrMoveNumber)
                        R = lmr2;
                    }

                newDepth = std::max(1, depth - R);

                score = -search<NodeType::NONPV>(newDepth - 1, -alpha - 1, -alpha, ply + 1, position, !cut_node);

                if (score > alpha)
                    {
                    newDepth = depth;
                    score = -search<NodeType::PV>(newDepth - 1, -beta, -alpha, ply + 1, position, !cut_node);
                    }
                }

            position.UndoMove(move);

            if (score >= beta)
                {
                if (move == best) // we don't want to save our hash move also as a killer move
                    return beta;

                //killer moves and history heuristic
                if (!position.IsCapture(move))
                    {
                    searchInfo.SetKillers(move, ply);
                    searchInfo.SetHistory(move, position.SideToMove(), newDepth);
                    }

                // for safety, we don't save forward pruned nodes inside transposition table
                if (!pruned)
                    Hash.Save(position.zobrist, newDepth, beta, best, ScoreType::Beta);

                return beta; //  fail hard beta-cutoff
                }

            if (score > alpha)
                {
                bound = ScoreType::Exact;
                alpha = score; // alpha acts like max in MiniMax
                best = move;
                }

            moveNumber++;
            }
        }

    // check for stalemate and checkmate
    if (legal == 0)
        {
        if (position.IsCheck())
            alpha = -Search::Mate + ply; // return best score for the deepest mate
        else
            alpha = 0;  // return draw score
        }

    // check for fifty moves rule
    if (position.HalfMoveClock() >= 100)
        alpha = 0;

    // for safety, we don't save forward pruned nodes inside transposition table
    if (!pruned)
        Hash.Save(position.zobrist, newDepth, alpha, best, bound);

    return alpha;
    }

// quiescence is called at horizon nodes (depth = 0)
int Search::quiescence( int alpha, int beta, Position &position )
    {
    searchInfo.VisitNode();

    const uint64_t attackers = position.KingAttackers(position.KingSquare(position.SideToMove()), position.SideToMove());
    const bool inCheck = attackers;
    int stand_pat = 0; // to suppress warning
    int score;

    if (!inCheck)
        {
        stand_pat = Eval::Evaluate(position);

        if (stand_pat >= beta)
            return beta;

        int Delta = PieceValue[PieceType::Queen];

        if (position.IsPromotingPawn())
            Delta += PieceValue[PieceType::Queen] - PieceValue[PieceType::Pawn];

        // big delta futility pruning
        if (stand_pat < alpha - Delta)
            return alpha;

        if (alpha < stand_pat)
            alpha = stand_pat;
        }

    // TO TEST
    if (position.IsRepetition())
        return 0;

    const uint64_t pinned = position.PinnedPieces();

    MovePick moves(position, searchInfo);

    if (!inCheck)
        MoveGen::GetPseudoLegalMoves<true>(moves.moves, moves.count, attackers, position);  // get all capture moves
    else
        MoveGen::GetPseudoLegalMoves<false>(moves.moves, moves.count, attackers, position); // get all evading moves

    moves.Sort<true>();

    for ( auto move = moves.First(); !move.IsNull(); move = moves.Next() )
        {
        // delta futility pruning
        if (!inCheck)
            {
            if (!move.IsPromotion() && (PieceValue[position.PieceOnSquare(move.ToSquare()).Type]
				+ stand_pat + dfpMargin <= alpha || position.See(move) < 0))
                continue;
            }

        if (position.IsMoveLegal(move, pinned))
            {
            position.MakeMove(move);
            score = -quiescence(-beta, -alpha, position);
            position.UndoMove(move);

            if (score >= beta)
                return beta;

            if (score > alpha)
                alpha = score;
            }
        }

    return alpha;
    }

std::string Search::GetPv( Position &position, Move toMake, int depth )
    {
    std::string pv;

    if (toMake.IsNull() || depth == 0)
        return pv;
    else
        {
        pv = toMake.ToAlgebraic() + " ";

        position.MakeMove(toMake);
        pv += GetPv(position, Hash.GetPv(position.zobrist), depth - 1);
        position.UndoMove(toMake);

        return pv;
        }
    }

std::string Search::GetInfo( Position &position, Move toMake, int score, int depth, int startTime )
    {
    std::ostringstream info;
    double delta = searchInfo.ElapsedTime() - startTime;
    double nps = (delta > 0 ? searchInfo.Nodes() * cores / delta : searchInfo.Nodes() * cores / 1) * 1000;

    info << "depth " << depth << " seldepth " << searchInfo.SelDepth;

    if (std::abs(score) >= Search::Mate - MaxPly)
        {
        int plies = Search::Mate - std::abs(score) + 1;

        if (score < 0) // mated
            plies *= -1;

        info << " score mate " << plies / 2;
        }
    else
        info << " score cp " << score;

    info << " time " << searchInfo.ElapsedTime()
		 << " nodes " << searchInfo.Nodes() * cores
		 << " nps " << static_cast<int>(nps)
         << " pv " << GetPv(position, toMake, depth);

    return info.str();
    }

Move Search::getPonderMove( Position &position, const Move toMake )
    {
    Move move = NullMove;
    position.MakeMove(toMake);
    move = Hash.GetPv(position.zobrist);
    position.UndoMove(toMake);

    return move;
    }