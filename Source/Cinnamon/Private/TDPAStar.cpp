// Fill out your copyright notice in the Description page of Project Settings.


#include "TDPAStar.h"
#include <functional>

TDPAStar::TDPAStar(const ATDPVolume& volume, const PathHelper::Heuristic& heuristic, const FTDPPathFinderSettings& settings) : IPathFinder(volume, heuristic, settings)
{
}

void TDPAStar::FindPath(const TDPNodeLink startLink, const TDPNodeLink endLink, const FVector& startPosition, const FVector& endPosition, TDPNavigationPath& path) const
{
	TSet<TDPNodeLink> openSet;
	TArray<TDPNodeLink> priorityQueue;
	TSet<TDPNodeLink> closedSet;
	TMap<TDPNodeLink, TDPNodeLink> breadcrumbTrail;
	TMap<TDPNodeLink, float> hScores; // h(n)
	TMap<TDPNodeLink, float> gScores; // g(n)

	TDPNodeLink currentLink = startLink;
	closedSet.Add(startLink);

	gScores.Emplace(startLink, 0.0f);
	hScores.Emplace(startLink, CalculateHeuristic(startLink, endLink));

	TArray<TDPNodeLink> neighbors;

	auto comparator = [&gScores, &hScores](const TDPNodeLink& left, const TDPNodeLink& right) 
	{ 
		return (gScores[left] + hScores[left]) < (gScores[right] + hScores[right]);
	};

	int32 iterations = 0;

	do
	{
		const auto currentNode = mVolume->GetNodeFromLink(currentLink);
		if (currentNode)
		{
			if (currentLink.LayerIndex == 0 && currentNode->GetFirstChild().IsValid())
			{
				mVolume->GetLeafNeighborsFromLink(currentLink, neighbors);
			}
			else
			{
				mVolume->GetNodeNeighborsFromLink(currentLink, neighbors);
			}

			for (const auto& neighbor : neighbors)
			{
				if (!closedSet.Contains(neighbor))
				{
					float pathCost = gScores[currentLink] + GetCost(currentLink, neighbor);

					if (openSet.Contains(neighbor))
					{
						if (pathCost < gScores[neighbor])
						{
							breadcrumbTrail[neighbor] = currentLink;
							gScores[neighbor] = pathCost;
							priorityQueue.Heapify(comparator);
						}
					}
					else
					{
						breadcrumbTrail.Emplace(neighbor, currentLink);
						hScores.Emplace(neighbor, CalculateHeuristic(neighbor, endLink));
						gScores.Emplace(neighbor, pathCost);
						openSet.Add(neighbor);
						priorityQueue.HeapPush(neighbor, comparator);
					}
				}
			}

			neighbors.Reset();
		}

		++iterations;

		if (openSet.Num() == 0)
		{
			break;
		}

		priorityQueue.HeapPop(currentLink, comparator);
		openSet.Remove(currentLink);
		closedSet.Add(currentLink);

	} while (currentLink != endLink);

	if (currentLink == endLink)
	{
		BuildPath(breadcrumbTrail, currentLink, startPosition, endPosition, path);
#if WITH_EDITOR
		UE_LOG(CinnamonLog, Display, TEXT("Pathfinding complete, iterations: %i"), iterations);
		UE_LOG(CinnamonLog, Display, TEXT("Pathfinding complete, visited nodes: %i"), closedSet.Num());
		UE_LOG(CinnamonLog, Display, TEXT("Pathfinding complete, frontier: %i"), openSet.Num());
		UE_LOG(CinnamonLog, Display, TEXT("Pathfinding complete, path length : %i"), path.GetPath().Num());
#endif
		return;
	}

#if WITH_EDITOR
	UE_LOG(CinnamonLog, Display, TEXT("Pathfinding failed, iterations : %i"), iterations);
#endif
}

float TDPAStar::GetCost(const TDPNodeLink& start, const TDPNodeLink& end) const
{
	float cost = 0.0f;

	if (mSettings->UseUnitCost)
	{
		cost = mSettings->UnitCost;
	}
	else
	{
		FVector startPosition, endPosition;
		mVolume->GetNodePositionFromLink(start, startPosition);
		mVolume->GetNodePositionFromLink(end, endPosition);
		cost = (startPosition - endPosition).Size();
	}

	cost *= 1.0f - (static_cast<float>(end.LayerIndex) / static_cast<float>(mVolume->GetTotalLayers())) * mSettings->NodeSizeCompensation;

	return cost;
}

float TDPAStar::CalculateHeuristic(const TDPNodeLink& start, const TDPNodeLink& end) const
{
	float heuristic = mHeuristic(start, end, *mVolume) * mSettings->EstimateWeight;
	heuristic *= 1.0f - (static_cast<float>(end.LayerIndex) / static_cast<float>(mVolume->GetTotalLayers())) * mSettings->NodeSizeCompensation;

	return heuristic;
}
