#pragma once
#include "piece.h"

static int pawnValue = 100;
static int knightValue = 330;
static int bishopValue = 330;
static int rookValue = 500;
static int queenValue = 1000;
static int kingValue = 2000;

static int PieceValue[] =
	{
	pawnValue, knightValue, bishopValue, rookValue, queenValue, kingValue, 99999
	};

static int OpeningGameMat =
	PieceValue[PieceType::Pawn] * 16
	+ PieceValue[PieceType::Knight] * 4
	+ PieceValue[PieceType::Bishop] * 4
	+ PieceValue[PieceType::Rook] * 4
	+ PieceValue[PieceType::Queen] * 2
	+ PieceValue[PieceType::King] * 2;

static int OpeningMargin = 1200;
static int MiddleGameMargin = 3000;
static int MiddleGameMat = OpeningGameMat - OpeningMargin;
static int EndGameMat = MiddleGameMat - MiddleGameMargin;
static int MaxPlayerMat = OpeningGameMat / 2;
static int OpeningNonPawnMaterial = OpeningGameMat - PieceValue[PieceType::Pawn] * 16;

static int TempoBonus = 5;
static int KnightTropism = 5;
static int BishopTropism = 10;
static int RookTropism = 10;
static int QueenTropism = 3;
static int QueenPenaltyOpening = 15;
static int QueenPenaltyEndGame = 0;

static int BishopPair[3] =
	{
	30, 50, 50
	};

static int PawnBonus[3] =
	{
	0, 10, 30
	};

static int KnightBonus[3] =
	{
	3, -5, -12
	};

static int RookBonus[3] =
	{
	0, 5, 15
	};  

static int isolatedPawn[8] =
	{
	5, 7, 10, 18, 18, 10, 7, 5
	};

static int multiPawn[8] =      
	{
	0, 0, 10, 20, 35, 50, 75, 100
	};

static int passedPawn[3][8] =			  
	{
	{ 0, 0, 10, 15, 25, 30, 30, 0 },	 
	{ 0 },								   
	{ 0, 0, 10, 20, 40, 60, 125, 0 }	 
	};

static int mobilityBonus[][28] =
	{
	{},																 
	{ -15, -10, -5, 0, 5, 10, 10, 15, 15 },							 
	{ -15, -10, -5, 0, 5, 10, 15, 20, 25, 30, 30, 35, 35, 35 },		 
	{ -5, -5, 0, 5, 10, 10, 15, 20, 30, 35, 35, 40, 40, 40, 40 },	 
	{ -5, -4, -3, -2, -1, 0, 5, 10, 13, 16, 18, 20, 22, 24, 26, 28, 29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },				 
	{},																 
	};
