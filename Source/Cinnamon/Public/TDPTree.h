// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TDPNode.h"
#include "TDPLeafNode.h"

/**
 * 
 */
struct CINNAMON_API TDPTree
{
	TArray<TArray<TDPNode>> Layers;
	TArray<TDPLeafNode> LeafNodes;

	int32 GetTotalLayerNodes() const;

	const TArray<TDPNode>& GetLayer(LayerIndexType layer) const;
	TArray<TDPNode>& GetLayer(LayerIndexType layer);

	int32 MemoryUsage() const;
	void Clear();
};

FORCEINLINE FArchive &operator <<(FArchive &Ar, TDPTree& octree)
{
	Ar << octree.Layers;
	Ar << octree.LeafNodes;

	return Ar;
}
