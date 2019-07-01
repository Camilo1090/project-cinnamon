// Fill out your copyright notice in the Description page of Project Settings.


#include "TDPNavigationComponent.h"
#include "TDPVolume.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TDPAStar.h"

// Sets default values for this component's properties
UTDPNavigationComponent::UTDPNavigationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

UTDPNavigationComponent::~UTDPNavigationComponent()
{
	for (auto& task : mTasks)
	{
		if (task.IsValid())
		{
			task->EnsureCompletion(false);
		}
	}
}

bool UTDPNavigationComponent::CanNavigate() const
{
	FVector pawnPosition;
	TDPNodeLink link;

	return HasValidNavigationVolume() && GetPawnPosition(pawnPosition) && mNavigationVolume->IsPointInside(pawnPosition) && 
		mNavigationVolume->GetTotalLayers() && mNavigationVolume->GetLinkFromPosition(pawnPosition, link);
}

bool UTDPNavigationComponent::CanNavigateToPosition(const FVector & position) const
{
	TDPNodeLink link;

	return CanNavigate() && mNavigationVolume->GetLinkFromPosition(position, link);
}

// Called when the game starts
void UTDPNavigationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (FindNavigationVolume())
	{
		mNavigationPath = MakeShared<TDPNavigationPath>();

		switch (PathFinder)
		{
		case ETDPPathFinder::AStar:
			mPathFinder = MakeShared<TDPAStar>(*mNavigationVolume, *PathHelper::Heuristics.Find(Heuristic), PathFinderSettings);
			break;
		default:
			break;
		}
	}
}

bool UTDPNavigationComponent::IsNavigationPossible() const
{
	FVector pawnPosition;

	return HasValidNavigationVolume() && GetPawnPosition(pawnPosition) && mNavigationVolume->IsPointInside(pawnPosition) && mNavigationVolume->GetTotalLayers();
}

bool UTDPNavigationComponent::HasValidNavigationVolume() const
{
	return IsValid(mNavigationVolume);
}

bool UTDPNavigationComponent::FindNavigationVolume()
{
	TArray<AActor*> volumes;

	UGameplayStatics::GetAllActorsOfClass(Cast<UObject>(GetWorld()), ATDPVolume::StaticClass(), volumes);

	FVector position;
	
	if (GetPawnPosition(position))
	{
		for (auto actor : volumes)
		{
			auto volume = Cast<ATDPVolume>(actor);
			if (volume && volume->IsPointInside(position))
			{
				mNavigationVolume = volume;
				return true;
			}
		}
	}

	return false;
}


// Called every frame
void UTDPNavigationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UTDPNavigationComponent::GetPawnPosition(FVector& position) const
{
	AController* controller = Cast<AController>(GetOwner());

	if (controller != nullptr)
	{
		APawn* pawn = controller->GetPawn();
		if (pawn != nullptr)
		{
			position = pawn->GetActorLocation();
			return true;
		}
	}

	return false;
}

bool UTDPNavigationComponent::FindPath(const FVector& targetPosition)
{
	FVector startPosition;
	GetPawnPosition(startPosition);

#if WITH_EDITOR
	UE_LOG(CinnamonLog, Log, TEXT("Finding path from %s and %s"), *startPosition.ToString(), *targetPosition.ToString());
#endif

	TDPNodeLink startLink;
	TDPNodeLink targetLink;
	if (IsNavigationPossible())
	{
		// Get the nav link from our volume
		if (!mNavigationVolume->GetLinkFromPosition(startPosition, startLink))
		{
#if WITH_EDITOR
			UE_LOG(CinnamonLog, Error, TEXT("Path finder failed to find start navigation link"));
#endif
			return false;
		}

		if (!mNavigationVolume->GetLinkFromPosition(targetPosition, targetLink))
		{
#if WITH_EDITOR
			UE_LOG(CinnamonLog, Error, TEXT("Path finder failed to find target navigation link"));
#endif
			return false;
		}

		mNavigationPath->Reset();
		mPathFinder->FindPath(startLink, targetLink, startPosition, targetPosition, *mNavigationPath);
		mNavigationPath->SetIsReady(true);

		if (DrawPath)
		{
			mNavigationPath->DrawDebugVisualization(GetWorld(), *mNavigationVolume);
		}

		return true;
	}
	else
	{
#if WITH_EDITOR
		UE_LOG(CinnamonLog, Error, TEXT("Pawn is not inside an navigation volume, or navigation data has not been generated"));
#endif
	}

	return false;
}

bool UTDPNavigationComponent::FindPathAsync(const FVector& targetPosition, FThreadSafeBool& complete)
{
	FVector startPosition;
	GetPawnPosition(startPosition);

#if WITH_EDITOR
	UE_LOG(CinnamonLog, Log, TEXT("Finding path from %s and %s"), *startPosition.ToString(), *targetPosition.ToString());
#endif

	TDPNodeLink startLink;
	TDPNodeLink targetLink;
	if (IsNavigationPossible())
	{
		// Get the nav link from our volume
		if (!mNavigationVolume->GetLinkFromPosition(startPosition, startLink))
		{
#if WITH_EDITOR
			UE_LOG(CinnamonLog, Error, TEXT("Path finder failed to find start navigation link"));
#endif
			return false;
		}

		if (!mNavigationVolume->GetLinkFromPosition(targetPosition, targetLink))
		{
#if WITH_EDITOR
			UE_LOG(CinnamonLog, Error, TEXT("Path finder failed to find target navigation link"));
#endif
			return false;
		}

		bool executed = false;
		if (CanFindPathAsync(targetLink))
		{
			/*if (mCurrentAsyncTask.IsValid() && !mCurrentAsyncTask->Cancel())
			{
				mCurrentAsyncTask->EnsureCompletion(false);
			}*/

			mNavigationPath->Reset();
			mNavigationPaths.Add(mNavigationPath);
			mNavigationPath = MakeShared<TDPNavigationPath>();
			mCurrentAsyncTask = MakeShared<FAsyncTask<FindPathTask>>(GetWorld(), *mNavigationVolume, PathFinderSettings, PathFinder, Heuristic, startLink, targetLink, startPosition, targetPosition, *mNavigationPath, complete);
			mCurrentAsyncTask->StartBackgroundTask();
			mTasks.Add(mCurrentAsyncTask);
			mLastTargetLink = targetLink;
			mMoveRequested = false;

			executed = true;
		}

		//mNavigationPath->Reset();
		//(new FAutoDeleteAsyncTask<FindPathTask>(GetWorld(), *mNavigationVolume, PathFinderSettings, PathFinder, Heuristic, startLink, targetLink, startPosition, targetPosition, *mNavigationPath, complete))->StartBackgroundTask();

		return executed;
	}
	else
	{
#if WITH_EDITOR
		UE_LOG(CinnamonLog, Error, TEXT("Pawn is not inside an navigation volume, or navigation data has not been generated"));
#endif
	}

	return false;
}

bool UTDPNavigationComponent::CanFindPathAsync(const FVector& targetPosition) const
{
	TDPNodeLink targetLink;
	mNavigationVolume->GetLinkFromPosition(targetPosition, targetLink);

	return targetLink != mLastTargetLink;
}

bool UTDPNavigationComponent::CanFindPathAsync(const TDPNodeLink& targetLink) const
{
	return targetLink != mLastTargetLink;
}

TSharedPtr<TDPNavigationPath> UTDPNavigationComponent::GetPath()
{
	return mNavigationPath;
}

const ATDPVolume* UTDPNavigationComponent::GetVolume() const
{
	return mNavigationVolume;
}

bool UTDPNavigationComponent::GetMoveRequested() const
{
	return mMoveRequested;
}

void UTDPNavigationComponent::SetMoveRequested(bool requested)
{
	mMoveRequested = requested;
}

