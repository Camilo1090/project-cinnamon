// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPathFinder.h"

/**
 * 
 */
class CINNAMON_API TDPAStar : public IPathFinder
{
public:
	TDPAStar(const ATDPVolume& volume, const PathHelper::Heuristic& heuristic, const FTDPPathFinderSettings& settings);
	TDPAStar(const TDPAStar&) = default;
	virtual ~TDPAStar() = default;

	virtual void FindPath(const TDPNodeLink startLink, const TDPNodeLink endLink, const FVector& startPosition, const FVector& endVector, TDPNavigationPath& path) const override;

	float GetCost(const TDPNodeLink& start, const TDPNodeLink& end) const;
	float CalculateHeuristic(const TDPNodeLink& start, const TDPNodeLink& end) const;
};
