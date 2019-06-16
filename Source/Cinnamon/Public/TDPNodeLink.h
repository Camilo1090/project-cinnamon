// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TDPDefinitions.h"

/**
 * 
 */
struct CINNAMON_API TDPNodeLink
{
	uint32 LayerIndex : 4;
	uint32 NodeIndex : 22;
	uint32 SubnodeIndex : 6;

	static const TDPNodeLink InvalidLink;

	explicit TDPNodeLink(LayerIndexType layer = INVALID_LAYER_INDEX, NodeIndexType node = 0, SubnodeIndexType subnode = 0);
	bool operator==(const TDPNodeLink& other) const;
	bool operator!=(const TDPNodeLink& other) const;
	bool operator<(const TDPNodeLink& other) const;

	void SetLayerIndex(LayerIndexType layer);
	void SetNodeIndex(NodeIndexType node);
	void SetSubnodeIndex(SubnodeIndexType subnode);

	void Invalidate();
	bool IsValid() const;
	FString ToString() const;
};

FORCEINLINE uint32 GetTypeHash(const TDPNodeLink& link)
{
	return *reinterpret_cast<const uint32*>(&link);
}
