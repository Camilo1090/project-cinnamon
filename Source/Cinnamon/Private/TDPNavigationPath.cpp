// Fill out your copyright notice in the Description page of Project Settings.


#include "TDPNavigationPath.h"
#include "TDPVolume.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem/Public/NavigationData.h"


TDPPathPoint::TDPPathPoint(const FVector& position, LayerIndexType layer, bool leafChild) :
	Position(position), LayerIndex(layer), IsLeafChild(leafChild)
{
}

void TDPNavigationPath::DrawDebugVisualization(UWorld* world, const ATDPVolume& volume)
{
	for (int32 i = 0; i < mPath.Num() - 1; ++i)
	{
		TDPPathPoint& point = mPath[i];

		FVector offSet(0.0f);

		if (point.IsLeafChild)
		{
			float size = volume.GetVoxelSizeInLayer(point.LayerIndex) / 8;
			DrawDebugBox(world, point.Position, FVector(size), FColor::Emerald, true);
		}
		else
		{
			float size = volume.GetVoxelSizeInLayer(point.LayerIndex) / 2;
			DrawDebugBox(world, point.Position, FVector(size), DebugHelper::LayerColors[point.LayerIndex], true);
		}

		DrawDebugString(world, point.Position, FString::FromInt(mPath.Num() - i - 1), nullptr, FColor::Black);
		//DrawDebugSphere(world, point.Position + offSet, 30.0f, 20, FColor::Emerald, true, -1.0f, 0, 100.0f);
	}
}

void TDPNavigationPath::CreateUENavigationPath(FNavigationPath& path)
{
	auto& pathPoints = path.GetPathPoints();

	pathPoints.Reserve(mPath.Num());
	for (const auto& point : mPath)
	{
		pathPoints.Add(point.Position);
	}
}

void TDPNavigationPath::Reset()
{
	mPath.Reset();
}

bool TDPNavigationPath::IsReady() const
{
	return mIsReady;
}

void TDPNavigationPath::SetIsReady(bool ready)
{
	mIsReady = ready;
}

TArray<TDPPathPoint>& TDPNavigationPath::GetPath()
{
	return mPath;
}

const TArray<TDPPathPoint>& TDPNavigationPath::GetPath() const
{
	return mPath;
}
