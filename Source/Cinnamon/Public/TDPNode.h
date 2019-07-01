// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TDPDefinitions.h"
#include "TDPNodeLink.h"
#include <array>

/**
 * 
 */
class CINNAMON_API TDPNode
{
public:
	TDPNode();
	~TDPNode() = default;

	MortonCodeType& GetMortonCode();
	const MortonCodeType& GetMortonCode() const;
	void SetMortonCode(MortonCodeType code);

	TDPNodeLink& GetParent();
	const TDPNodeLink& GetParent() const;
	void SetParent(TDPNodeLink node);
	
	TDPNodeLink& GetFirstChild();
	const TDPNodeLink& GetFirstChild() const;
	void SetFirstChild(TDPNodeLink node);

	TDPNodeLink* GetNeighbors();
	const TDPNodeLink* GetNeighbors() const;

	bool HasChildren() const;

private:
	MortonCodeType mMortonCode;
	TDPNodeLink mParent;
	TDPNodeLink mFirstChild;
	TDPNodeLink mNeighbors[6];
};

FORCEINLINE FArchive &operator <<(FArchive &Ar, TDPNode& node)
{
	Ar << node.GetMortonCode();
	Ar << node.GetParent();
	Ar << node.GetFirstChild();

	for (int i = 0; i < 6; i++)
	{
		Ar << node.GetNeighbors()[i];
	}

	return Ar;
}
