// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

using LayerIndexType = uint8;
using NodeIndexType = uint_fast32_t;
using SubnodeIndexType = uint8;
using MortonCodeType = uint_fast64_t;

#define INVALID_LAYER_INDEX 15

class CINNAMON_API NodeHelper final
{

};

class CINNAMON_API DebugHelper final
{
public:
	static const FColor LayerColors[];
};
