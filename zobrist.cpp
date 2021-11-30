#include <random>
#include "zobrist.h"

uint64_t Zobrist::pieceInfo[2][7][74];
uint64_t Zobrist::Castling[16];
uint64_t Zobrist::Enpassant[8];
uint64_t Zobrist::Color;

class RandomGenerator
    {
    public:

    RandomGenerator()  noexcept :
        gen(std::mt19937_64::default_seed)
        {
        }

    uint64_t Next()
        {
        return dist(gen);
        }
    private:
    std::uniform_int_distribution<uint64_t> dist;
    std::mt19937_64 gen;
    };

void Zobrist::Init()
    {
    RandomGenerator gen;
    for (int i = 0; i < 2; i++)
        {
        for (int j = 0; j < 6; j++)
            {
            for (int k = 0; k < 64; k++)
                {
                pieceInfo[i][j][k] = gen.Next();
                }
            }
        }
    Color = gen.Next();
    for (int i = 0; i < 16; i++)
        {
        Castling[i] = gen.Next();
        }

    for (int i = 0; i < 8; i++)
        {
        Enpassant[i] = gen.Next();
        }
    }