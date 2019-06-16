// Fill out your copyright notice in the Description page of Project Settings.


#include "PathHelper.h"
#include "TDPVolume.h"

const PathHelper::Heuristic PathHelper::ManhattanDistance = [](const TDPNodeLink& start, const TDPNodeLink& end, const ATDPVolume& volume) -> float
{
	FVector startPosition, endPosition;
	volume.GetNodePositionFromLink(start, startPosition);
	volume.GetNodePositionFromLink(end, endPosition);

	return FMath::Abs(endPosition.X - startPosition.X) + FMath::Abs(endPosition.Y - startPosition.Y) + FMath::Abs(endPosition.Z - startPosition.Z);
};

const PathHelper::Heuristic PathHelper::EuclideanDistance = [](const TDPNodeLink& start, const TDPNodeLink& end, const ATDPVolume& volume) -> float
{
	FVector startPosition, endPosition;
	volume.GetNodePositionFromLink(start, startPosition);
	volume.GetNodePositionFromLink(end, endPosition);

	return (endPosition - startPosition).Size();
};

const TMap<ETDPHeuristic, PathHelper::Heuristic> PathHelper::Heuristics = 
{
	{ ETDPHeuristic::ManhattanDistance, PathHelper::ManhattanDistance },
	{ ETDPHeuristic::EuclideanDistance, PathHelper::EuclideanDistance }
};
