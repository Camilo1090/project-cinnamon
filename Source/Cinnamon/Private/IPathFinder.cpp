// Fill out your copyright notice in the Description page of Project Settings.


#include "IPathFinder.h"

IPathFinder::IPathFinder(const ATDPVolume& volume, const PathHelper::Heuristic& heuristic, const FTDPPathFinderSettings& settings) :
	mHeuristic(heuristic), mVolume(&volume), mSettings(&settings)
{
}

void IPathFinder::BuildPath(const TMap<TDPNodeLink, TDPNodeLink>& trail, TDPNodeLink currentLink, const FVector& startPosition, const FVector& endPosition, TDPNavigationPath& path) const
{
	auto& points = path.GetPath();
	while (const TDPNodeLink* link = trail.Find(currentLink))
	{
		FVector position;
		mVolume->GetNodePositionFromLink(currentLink, position);

		if (currentLink.LayerIndex == 0)
		{
			const auto& node = mVolume->GetNodeFromLink(currentLink);
			if (!node->HasChildren())
			{
				points.Emplace(position, static_cast<LayerIndexType>(currentLink.LayerIndex), false);
			}
			else
			{
				points.Emplace(position, static_cast<LayerIndexType>(currentLink.LayerIndex), true);
			}
		}
		else
		{
			points.Emplace(position, static_cast<LayerIndexType>(currentLink.LayerIndex), false);
		}

		currentLink = *link;
	}

	points.Emplace(startPosition, static_cast<LayerIndexType>(currentLink.LayerIndex), false);
}
