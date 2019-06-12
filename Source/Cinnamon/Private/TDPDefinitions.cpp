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
