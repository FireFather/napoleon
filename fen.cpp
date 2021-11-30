#include <sstream>
#include "fen.h"
#include "evalterms.h"
#include "square.h"
#include "position.h"

Fen::Fen(std::string str) :
    fullString(str)
    {
    Parse();
    }

void Fen::Parse()
    {
    std::istringstream stream(fullString);

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

pieceInfo chrToPiece(const string::value_type & chr)
	{
	pieceHere = 1;
	switch (chr)
		{
		case 'P':
			return pieceInfo(pieceColor::White, pieceType::Pawn);
		case 'N':
			return pieceInfo(pieceColor::White, pieceType::Knight);
		case 'B':
			return pieceInfo(pieceColor::White, pieceType::Bishop);
		case 'R':
			return pieceInfo(pieceColor::White, pieceType::Rook);
		case 'Q':
			return pieceInfo(pieceColor::White, pieceType::Queen);
		case 'K':
			return pieceInfo(pieceColor::White, pieceType::King);
		case 'p':
			return pieceInfo(pieceColor::Black, pieceType::Pawn);
		case 'n':
			return pieceInfo(pieceColor::Black, pieceType::Knight);
		case 'b':
			return pieceInfo(pieceColor::Black, pieceType::Bishop);
		case 'r':
			return pieceInfo(pieceColor::Black, pieceType::Rook);
		case 'q':
			return pieceInfo(pieceColor::Black, pieceType::Queen);
		case 'k':
			return pieceInfo(pieceColor::Black, pieceType::King);
		}
	pieceHere = 0;
	return (Null);
}

void Fen::parsePiecePlacement(std::string field)
    {
	vstring lines;
	Fen::split(lines, field, "/", true);

	uint32_t linepos(8);
	vstring::iterator itLine = lines.begin();

	for (int i = 0; i < 64; i++)
		{
		piecePlacement[i] = Null;
		}

	while (itLine != lines.end())
		{
		vstring::value_type & line = *itLine++;

		int32_t startpos(0);

		switch (linepos--)
			{
			case 8:
				startpos = Square::A8;
				break;
			case 7:
				startpos = Square::A7;
				break;
			case 6:
				startpos = Square::A6;
				break;
			case 5:
				startpos = Square::A5;
				break;
			case 4:
				startpos = Square::A4;
				break;
			case 3:
				startpos = Square::A3;
				break;
			case 2:
				startpos = Square::A2;
				break;
			case 1:
				startpos = Square::A1;
				break;
			}

		string::iterator it = line.begin();

		while (it != line.end())
			{
			string::value_type & chr = *it++;

			pieceInfo piece= chrToPiece(chr);
			if (pieceHere == 1)
				piecePlacement[startpos++] = piece;
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

void Fen::parsesideToMove(std::string field)
    {
    switch(field[0])
        {
        case 'w':
            sideToMove = pieceColor::White;
            break;
        case 'b':
            sideToMove = pieceColor::Black;
            break;
        }
    }

void Fen::parseCastling(std::string field)
    {
    whiteCanCastleShort = false;
    whiteCanCastleLong = false;
    blackCanCastleShort = false;
    blackCanCastleLong = false;

    for (unsigned i = 0; i < field.size(); i++)
        {
        switch(field[i])
            {
            case 'K':
                whiteCanCastleShort = true;
                break;
            case 'k':
                blackCanCastleShort = true;
                break;
            case 'Q':
                whiteCanCastleLong = true;
                break;
            case 'q':
                blackCanCastleLong = true;
                break;
            }
        }
    }

void Fen::parseEnPassant(std::string field)
    {
    if (field.size() == 1)
        {
        if (field[0] == '-')
            getPassantSquare = Square::noSquare;
        }
    else
        {
        getPassantSquare = Square::Parse(field);
        }
    }

void Fen::parseHalfMove(std::string field)
    {
    halfMove = std::stoi(field);
    }
