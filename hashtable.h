#pragma once
#include <mutex>
#include <atomic>
#include "hashentry.h"

class SpinLock
    {
    public:

    void lock()
        {
        while (lck.test_and_set(std::memory_order_acquire)) { }
        }

    void unlock()
        {
        lck.clear(std::memory_order_release);
        }
    private:
    std::atomic_flag lck = ATOMIC_FLAG_INIT;
    };

class HashTable
    {
    public:
    static const int Unknown = -999999;
    static const int BucketSize = 4;
    HashTable(int size = 32) noexcept;
    void SetSize(int);
    void Save(uint64_t, uint8_t, int, Move, ScoreType);
    void Clear();
    std::pair<int, Move> Probe(uint64_t, uint8_t, int, int);
    Move GetPv(uint64_t);
    private:
    uint64_t mask;
    uint32_t entries;
    uint32_t lock_entries;
    HashEntry *table;
    SpinLock *locks;
    HashEntry *at(uint64_t, int = 0)const;
    };

inline HashEntry *HashTable::at( uint64_t key, int index ) const
    {
    return table + (key &mask)+index;
    }