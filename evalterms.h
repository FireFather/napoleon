#pragma once
#include "piece.h"

static int pawnValue = 90; 
static int knightValue = 335;
static int bishopValue = 350;
static int rookValue = 540;
static int queenValue = 1100;
static int kingValue = 2000;

static int pieceValue[] =
	{
	pawnValue, knightValue, bishopValue, rookValue, queenValue, kingValue, 99999
	};

static int openingMaterial =				// 12540
	pieceValue[pieceType::Pawn] * 16		// 1440
	+ pieceValue[pieceType::Knight] * 4		// 1340
	+ pieceValue[pieceType::Bishop] * 4		// 1400
	+ pieceValue[pieceType::Rook] * 4		// 2160
	+ pieceValue[pieceType::Queen] * 2		// 2200
	+ pieceValue[pieceType::King] * 2;		// 4000

static int endGameMaterial = openingMaterial - 4200; // to test: 6270
static int maxMaterial = openingMaterial / 2;
static int openingNonPawnMaterial = openingMaterial - pieceValue[pieceType::Pawn] * 16;

static int tempoBonus[2] =
	{
	5, 5
	};

static int queenPenalty[2] =
	{
	15, 0
	};

static int bishopPair[2] =
	{
	33, 55
	};

static int isolatedPawn[2] =
	{
	16, 8
	};

static int doubledPawn[2] =      
	{
	5, 2
	};

static int passedPawn[2][8] =			  
	{
	{ 0, 0, 10, 15, 25, 30, 30, 0 },	 							   
	{ 0, 0, 10, 20, 40, 60, 125, 0 }	 
	};

static int pawnShelter[2][2] =
	{ 
	{ 5, 4 },
	{ 3, 2 }
	};

static int mobilityBonus[2][7][28] =
	{
		{
		{},
		{ -15, -10, -5, 0, 5, 10, 10, 15, 15 },
		{ -15, -10, -5, 0, 5, 10, 15, 20, 25, 30, 30, 35, 35, 35 },
		{ -5, -5, 0, 5, 10, 10, 15, 20, 30, 35, 35, 40, 40, 40, 40 },
		{ -5, -4, -3, -2, -1, 0, 5, 10, 13, 16, 18, 20, 22, 24, 26, 28, 29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
		{},
		{},
		},
		{
		{},
		{ -15, -10, -5, 0, 5, 10, 10, 15, 15 },
		{ -15, -10, -5, 0, 5, 10, 15, 20, 25, 30, 30, 35, 35, 35 },
		{ -5, -5, 0, 5, 10, 10, 15, 20, 30, 35, 35, 40, 40, 40, 40 },
		{ -5, -4, -3, -2, -1, 0, 5, 10, 13, 16, 18, 20, 22, 24, 26, 28, 29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30 },
		{},
		{},
		},
	};
