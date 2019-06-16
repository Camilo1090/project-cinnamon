// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PathHelper.h"
#include "TDPVolume.h"
#include "TDPNavigationPath.h"
#include "TDPNodeLink.h"

/**
 * 
 */
class CINNAMON_API IPathFinder
{
public:
	IPathFinder(const ATDPVolume& volume, const PathHelper::Heuristic& heuristic, const FTDPPathFinderSettings& settings);
	virtual ~IPathFinder() = default;

	virtual void FindPath(const TDPNodeLink startLink, const TDPNodeLink endLink, const FVector& startPosition, const FVector& endVector, TDPNavigationPath& path) const = 0;

	void BuildPath(const TMap<TDPNodeLink, TDPNodeLink>& trail, TDPNodeLink currentLink, const FVector& startPosition, const FVector& endPosition, TDPNavigationPath& path) const;

protected:
	PathHelper::Heuristic mHeuristic;
	const ATDPVolume* mVolume;
	const FTDPPathFinderSettings* mSettings;
};
