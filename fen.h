#pragma once
#include "piece.h"
#include "move.h"

typedef vector<string> vstring;

static int pieceHere = 0;

class Fen
    {
    public:
    std::string fullString;
    pieceInfo piecePlacement[64];
    uint8_t sideToMove;
    bool whiteCanCastleShort;
    bool whiteCanCastleLong;
    bool blackCanCastleShort;
    bool blackCanCastleLong;
    int getPassantSquare;
    int halfMove;
    Fen(std::string);
    void Parse();
	void split(vstring &, const string &, const string &, bool);

    private:
    void parsePiecePlacement(std::string);
    void parsesideToMove(std::string);
    void parseCastling(std::string);
    void parseEnPassant(std::string);
    void parseHalfMove(std::string);
    };