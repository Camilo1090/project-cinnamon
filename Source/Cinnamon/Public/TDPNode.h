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

	MortonCodeType GetMortonCode() const;
	void SetMortonCode(MortonCodeType code);

	TDPNodeLink& GetParent();
	const TDPNodeLink& GetParent() const;
	void SetParent(TDPNodeLink node);
	
	TDPNodeLink& GetFirstChild();
	const TDPNodeLink& GetFirstChild() const;
	void SetFirstChild(TDPNodeLink node);

	TDPNodeLink* GetNeighbors();

private:
	MortonCodeType mMortonCode;
	TDPNodeLink mParent;
	TDPNodeLink mFirstChild;
	TDPNodeLink mNeighbors[6];
};
