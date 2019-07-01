// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Core/Public/Async/AsyncWork.h"
#include "ThreadSafeBool.h"
#include "TDPNodeLink.h"
#include "TDPNavigationPath.h"
#include "PathHelper.h"

class ATDPVolume;

/**
 * 
 */
class CINNAMON_API FindPathTask// : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FindPathTask>;
	friend class FAsyncTask<FindPathTask>;

public:
	FindPathTask(UWorld* world, const ATDPVolume& volume, const FTDPPathFinderSettings& settings, ETDPPathFinder pathFinder, ETDPHeuristic heuristic,
		const TDPNodeLink& startLink, const TDPNodeLink& endLink, const FVector& startPosition, const FVector& endPosition, 
		TDPNavigationPath& path, FThreadSafeBool& complete);

protected:
	UWorld* mWorld;
	const ATDPVolume* mVolume;
	const FTDPPathFinderSettings* mSettings;
	ETDPPathFinder mPathFinder;
	ETDPHeuristic mHeuristic;
	TDPNodeLink mStartLink;
	TDPNodeLink mEndLink;
	FVector mStartPosition;
	FVector mEndPosition;
	TDPNavigationPath& mPath;
	FThreadSafeBool& mComplete;

	void DoWork();
	bool CanAbandon() const;
	void Abandon();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FindPathTask, STATGROUP_ThreadPoolAsyncTasks);
	}
};
