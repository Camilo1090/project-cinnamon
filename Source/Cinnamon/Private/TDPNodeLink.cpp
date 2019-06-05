// Fill out your copyright notice in the Description page of Project Settings.


#include "TDPNodeLink.h"

const TDPNodeLink TDPNodeLink::InvalidLink{};

TDPNodeLink::TDPNodeLink(LayerIndexType layer, NodeIndexType node, SubnodeIndexType subnode)
	: LayerIndex(layer), NodeIndex(node), SubnodeIndex(subnode)
{
}

void TDPNodeLink::SetLayerIndex(LayerIndexType layer)
{
	LayerIndex = layer;
}

void TDPNodeLink::SetNodeIndex(NodeIndexType node)
{
	NodeIndex = node;
}

void TDPNodeLink::SetSubnodeIndex(SubnodeIndexType subnode)
{
	SubnodeIndex = subnode;
}

void TDPNodeLink::Invalidate()
{
	LayerIndex = INVALID_LAYER_INDEX;
}

bool TDPNodeLink::IsValid() const
{
	return LayerIndex != INVALID_LAYER_INDEX;
}

FString TDPNodeLink::ToString() const
{
	return FString::Printf(TEXT("%i, %i, %i"), LayerIndex, NodeIndex, SubnodeIndex);
}
