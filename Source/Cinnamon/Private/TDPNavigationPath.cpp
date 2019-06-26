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

		DrawDebugString(world, point.Position, FString::FromInt(i + 1), nullptr, FColor::Black);
#if WITH_EDITOR
		UE_LOG(CinnamonLog, Log, TEXT("Point %i: (%f, %f, %f)"), i + 1, point.Position.X, point.Position.Y, point.Position.Z);
#endif
	}

	if (mPath.Num() > 0)
	{
		TDPNodeLink link;
		volume.GetLinkFromPosition(mPath[mPath.Num() - 1].Position, link);
		FVector pos;
		volume.GetNodePositionFromLink(link, pos);

		if (mPath[mPath.Num() - 1].IsLeafChild)
		{
			float size = volume.GetVoxelSizeInLayer(link.LayerIndex) / 8;
			DrawDebugBox(world, pos, FVector(size), FColor::Emerald, true);
		}
		else
		{
			float size = volume.GetVoxelSizeInLayer(link.LayerIndex) / 2;
			DrawDebugBox(world, pos, FVector(size), DebugHelper::LayerColors[link.LayerIndex], true);
		}

		DrawDebugString(world, pos, FString::FromInt(mPath.Num()), nullptr, FColor::Black);
#if WITH_EDITOR
		UE_LOG(CinnamonLog, Log, TEXT("Point %i: (%f, %f, %f)"), mPath.Num(), mPath[mPath.Num() - 1].Position.X, mPath[mPath.Num() - 1].Position.Y, mPath[mPath.Num() - 1].Position.Z);
#endif
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

	// temporary workaround for invalid paths of size 1
	if (pathPoints.Num() == 1)
	{
		pathPoints.Emplace(pathPoints[0]);
	}
}

void TDPNavigationPath::Reset()
{
	mPath.Reset();
	mIsReady = false;
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
