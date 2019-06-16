// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TDPDefinitions.h"

class ATDPVolume;
struct FNavigationPath;

struct CINNAMON_API TDPPathPoint
{
	FVector Position;
	LayerIndexType LayerIndex;
	bool IsLeafChild;

	TDPPathPoint(const FVector& position = FVector(), LayerIndexType layer = INVALID_LAYER_INDEX, bool leafChild = false);
};

/**
 * 
 */
class CINNAMON_API TDPNavigationPath
{
public:
	TDPNavigationPath() = default;
	~TDPNavigationPath() = default;

	void DrawDebugVisualization(UWorld* world, const ATDPVolume& volume);
	void CreateUENavigationPath(FNavigationPath& path);

	void Reset();

	bool IsReady() const;
	void SetIsReady(bool ready);
	TArray<TDPPathPoint>& GetPath();
	const TArray<TDPPathPoint>& GetPath() const;

private:
	bool mIsReady = false;
	TArray<TDPPathPoint> mPath;
};
