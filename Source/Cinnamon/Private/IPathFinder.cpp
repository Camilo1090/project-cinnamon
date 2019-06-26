// Fill out your copyright notice in the Description page of Project Settings.


#include "IPathFinder.h"

IPathFinder::IPathFinder(const ATDPVolume& volume, const PathHelper::Heuristic& heuristic, const FTDPPathFinderSettings& settings) :
	mHeuristic(heuristic), mVolume(&volume), mSettings(&settings)
{
}

void IPathFinder::BuildPath(const TMap<TDPNodeLink, TDPNodeLink>& trail, TDPNodeLink currentLink, const FVector& startPosition, const FVector& endPosition, TDPNavigationPath& path) const
{
	// build path from end to start
	TArray<TDPPathPoint> points;

	FVector position;
	mVolume->GetNodePositionFromLink(currentLink, position);
	points.Emplace(position, static_cast<LayerIndexType>(currentLink.LayerIndex), false);

	while (const TDPNodeLink* link = trail.Find(currentLink))
	{
		mVolume->GetNodePositionFromLink(*link, position);

		if (link->LayerIndex == 0)
		{
			const auto& node = mVolume->GetNodeFromLink(*link);
			if (!node->HasChildren())
			{
				points.Emplace(position, static_cast<LayerIndexType>(link->LayerIndex), false);
			}
			else
			{
				points.Emplace(position, static_cast<LayerIndexType>(link->LayerIndex), true);
			}
		}
		else
		{
			points.Emplace(position, static_cast<LayerIndexType>(link->LayerIndex), false);
		}

		currentLink = *link;
	}

	// redundant but clearer
	//points.Emplace(startPosition, static_cast<LayerIndexType>(currentLink.LayerIndex), false);

	// build real path from start to end
	auto& outPoints = path.GetPath();
	outPoints.Reserve(points.Num());
	for (int32 i = points.Num() - 1; i >= 0; --i)
	{
		outPoints.Emplace(points[i]);
	}
}
