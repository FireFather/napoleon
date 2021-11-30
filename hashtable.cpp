#include "hashtable.h"
#include "math.h"
#include "searchinfo.h"
#include <valarray>

hashTable::hashTable(int mb) noexcept
    {
    setSize(mb);
    }

void hashTable::setSize(int mb)
    {
    mb = int(std::pow(2, int(Math::Log2(mb))));

    entries = uint32_t((mb * std::pow(2, 20)) / sizeof(HashEntry));
    lockEntries = (entries / bucketSize);

    free(table);
    free(locks);
    table = (HashEntry *)std::calloc(entries * sizeof(HashEntry), 1);
    locks = new SpinLock[lockEntries];
    mask = entries - bucketSize;
    }

void hashTable::Save(uint64_t key, uint8_t depth, int score, Move move, ScoreType bound)
    {
    auto mux = locks + (key & mask) / bucketSize;
    std::lock_guard<SpinLock> lock(*mux);
    int min = maxPly;
    int index = 0;
    auto hash = at(key);

    for (auto i = 0; i < bucketSize; i++, hash++)
        {
        if (hash->Depth < min)
            {
            min = hash->Depth;
            index = i;
            }
        }

    if (depth >= min)
        {
        auto hashToOverride = at(key, index);
        hashToOverride->Key = key;
        hashToOverride->Score = score;
        hashToOverride->Depth = depth;
        hashToOverride->Bound = bound;
        hashToOverride->BestMove = move;
        }
    }

std::pair<int, Move> hashTable::Probe(uint64_t key, uint8_t depth, int alpha, int beta)
    {
    auto mux = locks + (key & mask) / bucketSize;
    std::lock_guard<SpinLock> lock(*mux);
    auto hash = at(key);
    auto move = nullMove;

    for (auto i = 0; i < bucketSize; i++, hash++)
        {
        if (hash->Key == key)
            {
            if (hash->Depth >= depth)
                {
                if (hash->Bound == ScoreType::Exact)
                    return std::make_pair(hash->Score, move);
                if (hash->Bound == ScoreType::Alpha && hash->Score <= alpha)
                    return std::make_pair(alpha, move);
                if (hash->Bound == ScoreType::Beta && hash->Score >= beta)
                    return std::make_pair(beta, move);
                }
            move = hash->BestMove;       
            }
        }
    return std::make_pair(Unknown, move);
    }

void hashTable::Clear()
    {
    std::memset(table, 0, entries*sizeof(HashEntry));
    }

Move hashTable::getPV(uint64_t key)
    {
    auto hash = at(key);
    for (auto i = 0; i < bucketSize; i++, hash++)
        {
        if (hash->Key == key)
            {
            if (!hash->BestMove.isNull())
                return hash->BestMove;
            }
        }
    return nullMove;
    }