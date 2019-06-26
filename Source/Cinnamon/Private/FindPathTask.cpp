// Fill out your copyright notice in the Description page of Project Settings.


#include "FindPathTask.h"
#include "TDPAStar.h"
#include "TDPVolume.h"


FindPathTask::FindPathTask(UWorld* world, const ATDPVolume& volume, const FTDPPathFinderSettings& settings, ETDPPathFinder pathFinder, ETDPHeuristic heuristic,
	const TDPNodeLink& startLink, const TDPNodeLink& endLink, const FVector& startPosition, const FVector& endPosition, 
	TDPNavigationPath& path, FThreadSafeBool& complete) :
	mWorld(world), mVolume(&volume), mSettings(&settings), mPathFinder(pathFinder), mHeuristic(heuristic),
	mStartLink(startLink), mEndLink(endLink), mStartPosition(startPosition), mEndPosition(endPosition), 
	mPath(path), mComplete(complete)
{
}

void FindPathTask::DoWork()
{
	TSharedPtr<IPathFinder> pathFinder = nullptr;

	switch (mPathFinder)
	{
	case ETDPPathFinder::AStar:
		pathFinder = MakeShared<TDPAStar>(*mVolume, *PathHelper::Heuristics.Find(mHeuristic), *mSettings);
		break;
	default:
		break;
	}

	if (pathFinder)
	{
		pathFinder->FindPath(mStartLink, mEndLink, mStartPosition, mEndPosition, mPath);
		mPath.SetIsReady(true);
	}

	mComplete = true;
}
