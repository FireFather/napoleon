#include <sstream>
#include "fen.h"
#include "evalterms.h"
#include "square.h"
#include "position.h"

Fen::Fen( std::string str ) :
    FullString(str)
    {
    Parse();
    }

void Fen::Parse()
    {
    std::istringstream stream(FullString);

    std::string piecePlacement;
    std::string sideToMove;
    std::string castling;
    std::string enPassant;
    std::string halfMove;
    std::string fullMove;

    stream >> piecePlacement;
    stream >> sideToMove;
    stream >> castling;
    stream >> enPassant;

    std::string token;
    stream >> token;
	halfMove = token;
	stream >> fullMove;

    parsePiecePlacement(piecePlacement);
    parsesideToMove(sideToMove);
    parseCastling(castling);
    parseEnPassant(enPassant);

    if (!halfMove.empty())
        parseHalfMove(halfMove);
    }

void Fen::split(vstring & result, const string & source, const string & separator, bool skipEmpty)
{
	string copy = source;

	bool check(true);

	while (check)
	{
		string::size_type pos = copy.find(separator);

		if (string::npos != pos)
		{
			string substr = copy.substr(0, pos);
			if (!(skipEmpty && substr.empty()))
				result.push_back(substr);
			copy.erase(0, pos + separator.length());
			check = true;
		}
		else
		{
			result.push_back(copy);
			check = false;
		}
	}
}

PieceInfo chrToPiece(const string::value_type & chr)
{
	pieceHere = 1;
	switch (chr)
	{
	case 'P':
		return PieceInfo(PieceColor::White, PieceType::Pawn);
	case 'N':
		return PieceInfo(PieceColor::White, PieceType::Knight);
	case 'B':
		return PieceInfo(PieceColor::White, PieceType::Bishop);
	case 'R':
		return PieceInfo(PieceColor::White, PieceType::Rook);
	case 'Q':
		return PieceInfo(PieceColor::White, PieceType::Queen);
	case 'K':
		return PieceInfo(PieceColor::White, PieceType::King);
	case 'p':
		return PieceInfo(PieceColor::Black, PieceType::Pawn);
	case 'n':
		return PieceInfo(PieceColor::Black, PieceType::Knight);
	case 'b':
		return PieceInfo(PieceColor::Black, PieceType::Bishop);
	case 'r':
		return PieceInfo(PieceColor::Black, PieceType::Rook);
	case 'q':
		return PieceInfo(PieceColor::Black, PieceType::Queen);
	case 'k':
		return PieceInfo(PieceColor::Black, PieceType::King);
	}
	pieceHere = 0;
	return (Null);
}

enum BSquare
{
	A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7,
	A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15,
	A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23,
	A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31,
	A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39,
	A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47,
	A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55,
	A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63,
	noSquare = 64,
};

void Fen::parsePiecePlacement( std::string field )
    {
	vstring lines;
	Fen::split(lines, field, "/", true);

	uint32_t linepos(8);
	vstring::iterator itLine = lines.begin();

	for (int i = 0; i < 64; i++)
		{
		PiecePlacement[i] = Null;
		}

	while (itLine != lines.end())
	{
		vstring::value_type & line = *itLine++;

		int32_t startpos(0);

		switch (linepos--)
		{
		case 8:
			startpos = BSquare::A8;
			break;
		case 7:
			startpos = BSquare::A7;
			break;
		case 6:
			startpos = BSquare::A6;
			break;
		case 5:
			startpos = BSquare::A5;
			break;
		case 4:
			startpos = BSquare::A4;
			break;
		case 3:
			startpos = BSquare::A3;
			break;
		case 2:
			startpos = BSquare::A2;
			break;
		case 1:
			startpos = BSquare::A1;
			break;
		}

		string::iterator it = line.begin();

		while (it != line.end())
		{
			string::value_type & chr = *it++;

			PieceInfo piece= chrToPiece(chr);
			if (pieceHere == 1)
				PiecePlacement[startpos++] = piece;
			else
			{
				switch (chr)
				{
				case '1':
					startpos += 1;
					break;
				case '2':
					startpos += 2;
					break;
				case '3':
					startpos += 3;
					break;
				case '4':
					startpos += 4;
					break;
				case '5':
					startpos += 5;
					break;
				case '6':
					startpos += 6;
					break;
				case '7':
					startpos += 7;
					break;
				case '8':
					startpos += 8;
					break;
				}
			}
		}
	}
    }

void Fen::parsesideToMove( std::string field )
    {
    switch( field[0] )
        {
        case 'w':
            sideToMove = PieceColor::White;
            break;
        case 'b':
            sideToMove = PieceColor::Black;
            break;
        }
    }

void Fen::parseCastling( std::string field )
    {
    WhiteCanCastleShort = false;
    WhiteCanCastleLong = false;
    BlackCanCastleShort = false;
    BlackCanCastleLong = false;

    for ( unsigned i = 0; i < field.size(); i++ )
        {
        switch( field[i] )
            {
            case 'K':
                WhiteCanCastleShort = true;
                break;
            case 'k':
                BlackCanCastleShort = true;
                break;
            case 'Q':
                WhiteCanCastleLong = true;
                break;
            case 'q':
                BlackCanCastleLong = true;
                break;
            }
        }
    }

void Fen::parseEnPassant( std::string field )
    {
    if (field.size() == 1)
        {
        if (field[0] == '-')
            EnPassantSquare = Square::None;
        }
    else
        {
        EnPassantSquare = Square::Parse(field);
        }
    }

void Fen::parseHalfMove( std::string field )
    {
    HalfMove = std::stoi(field);
    }
