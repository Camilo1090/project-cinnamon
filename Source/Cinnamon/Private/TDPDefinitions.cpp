// Fill out your copyright notice in the Description page of Project Settings.


#include "TDPDefinitions.h"

const FIntVector NodeHelper::NeighborDirections[] = {
	{ 1, 0, 0 },
	{ -1, 0, 0 },
	{ 0, 1, 0 },
	{ 0, -1, 0 },
	{ 0, 0, 1 },
	{ 0, 0, -1 },
};

const NodeIndexType NodeHelper::ChildOffsets[6][4] = {
	{ 0, 4, 2, 6 },
	{ 1, 3, 5, 7 },
	{ 0, 1, 4, 5 },
	{ 2, 3, 6, 7 },
	{ 0, 1, 2, 3 },
	{ 4, 5, 6, 7 }
};

const NodeIndexType NodeHelper::LeafChildOffsets[6][16] = {
	{ 0, 2, 16, 18, 4, 6, 20, 22, 32, 34, 48, 50, 36, 38, 52, 54 },
	{ 9, 11, 25, 27, 13, 15, 29, 31, 41, 43, 57, 59, 45, 47, 61, 63 },
	{ 0, 1, 8, 9, 4, 5, 12, 13, 32, 33, 40, 41, 36, 37, 44, 45 },
	{ 18, 19, 26, 27, 22, 23, 30, 31, 50, 51, 58, 59, 54, 55, 62, 63 },
	{ 0, 1, 8, 9, 2, 3, 10, 11, 16, 17, 24, 25, 18, 19, 26, 27 },
	{ 36, 37, 44, 45, 38, 39, 46, 47, 52, 53, 60, 61, 54, 55, 62, 63 }
};

const FColor DebugHelper::LayerColors[] = {
	FColor::Yellow,
	FColor::Blue,
	FColor::Red,
	FColor::Green,
	FColor::Orange,
	FColor::Cyan,
	FColor::Magenta,
	FColor::Black,
	FColor::White,
	FColor::Emerald,
	FColor::Turquoise
};
