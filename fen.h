#pragma once
#include "piece.h"
#include "move.h"

typedef vector<string> vstring;

static int pieceHere = 0;

class Fen
    {
    public:
    std::string FullString;
    PieceInfo PiecePlacement[64];
    uint8_t sideToMove;
    bool WhiteCanCastleShort;
    bool WhiteCanCastleLong;
    bool BlackCanCastleShort;
    bool BlackCanCastleLong;
    int EnPassantSquare;
    int HalfMove;
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