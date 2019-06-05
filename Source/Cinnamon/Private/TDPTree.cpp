// Fill out your copyright notice in the Description page of Project Settings.


#include "TDPTree.h"


int32 TDPTree::GetTotalLayerNodes() const
{
	int32 total = 0;

	for (const auto& layer : Layers)
	{
		total += layer.Num();
	}

	return total;
}

const TArray<TDPNode>& TDPTree::GetLayer(LayerIndexType layer) const
{
	return Layers[layer];
}

TArray<TDPNode>& TDPTree::GetLayer(LayerIndexType layer)
{
	return Layers[layer];
}

int32 TDPTree::MemoryUsage() const
{
	int32 result = 0;

	for (int32 i = 0; i < Layers.Num(); ++i)
	{
		result += Layers[i].Num() * sizeof(TDPNode);
	}

	result += LeafNodes.Num() * sizeof(TDPLeafNode);

	return result;
}

void TDPTree::Clear()
{
	Layers.Empty();
	LeafNodes.Empty();
}
