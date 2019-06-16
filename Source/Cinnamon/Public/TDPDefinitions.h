// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

using LayerIndexType = uint8;
using NodeIndexType = int32;
using SubnodeIndexType = uint8;
using MortonCodeType = uint_fast64_t;

#define INVALID_LAYER_INDEX 15

class CINNAMON_API NodeHelper final
{
public:
	static const FIntVector NeighborDirections[];
	static const NodeIndexType ChildOffsets[6][4];
	static const NodeIndexType LeafChildOffsets[6][16];
};

class CINNAMON_API DebugHelper final
{
public:
	static const FColor LayerColors[];
};
