// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TDPDefinitions.h"

/**
 * 
 */
class CINNAMON_API TDPLeafNode
{
public:
	void SetSubnode(SubnodeIndexType subnode);
	void ClearSubnode(SubnodeIndexType subnode);
	bool GetSubnode(MortonCodeType code) const;

	uint64& GetSubnodes();
	const uint64& GetSubnodes() const;

	bool IsFullyBlocked() const;
	bool IsEmpty() const;

private:
	uint64 mSubnodes = 0;
};

FORCEINLINE FArchive &operator <<(FArchive &Ar, TDPLeafNode& leaf)
{
	Ar << leaf.GetSubnodes();

	return Ar;
}
