// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <functional>
#include "CoreMinimal.h"
#include "TDPNodeLink.h"
#include "PathHelper.generated.h"


UENUM(BlueprintType)
enum class ETDPPathFinder : uint8
{
	AStar	UMETA(DisplayName="A*")
};

UENUM(BlueprintType)
enum class ETDPHeuristic : uint8
{
	ManhattanDistance	UMETA(DisplayName = "Manhattan Distance"),
	EuclideanDistance	UMETA(DisplayName = "Euclidean Distance")
};

USTRUCT(BlueprintType)
struct CINNAMON_API FTDPPathFinderSettings
{
	GENERATED_BODY()

	bool DebugOpenNodes = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Navigation | Heuristics")
	bool UseUnitCost = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Navigation | Heuristics")
	float UnitCost = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Navigation | Heuristics")
	float HeuristicWeight = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Navigation | Heuristics")
	float NodeSizeCompensation = 1.0f;

	//int SmoothingIterations;
	//TArray<FVector> DebugPoints;
};

class ATDPVolume;
class IPathFinder;

/**
 * 
 */
class CINNAMON_API PathHelper
{
public:
	using Heuristic = std::function<float(const TDPNodeLink&, const TDPNodeLink&, const ATDPVolume&)>;

	static const Heuristic ManhattanDistance;
	static const Heuristic EuclideanDistance;

	static const TMap<ETDPHeuristic, Heuristic> Heuristics;
};
