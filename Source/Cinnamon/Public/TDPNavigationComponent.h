// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Runtime/Core/Public/Async/AsyncWork.h"
#include "FindPathTask.h"
#include "TDPNavigationPath.h"
#include "PathHelper.h"
#include "IPathFinder.h"
#include "ThreadSafeBool.h"
#include "TDPNavigationComponent.generated.h"

class ATDPVolume;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CINNAMON_API UTDPNavigationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTDPNavigationComponent();
	~UTDPNavigationComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Navigation | Debug")
	bool DrawPath = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Navigation | Debug")
	bool PrintCurrentPosition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Navigation | Debug")
	bool PrintMortonCodes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Navigation | Debug")
	bool DrawOpenNodes = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Navigation | Path Finder")
	ETDPPathFinder PathFinder = ETDPPathFinder::AStar;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Navigation | Path Finder")
	ETDPHeuristic Heuristic = ETDPHeuristic::ManhattanDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "3D Navigation | Path Finder")
	FTDPPathFinderSettings PathFinderSettings;

	UFUNCTION(BlueprintCallable)
	bool CanNavigate() const;

	UFUNCTION(BlueprintCallable)
	bool CanNavigateToPosition(const FVector& position) const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	bool IsNavigationPossible() const;
	bool HasValidNavigationVolume() const;
	bool FindNavigationVolume();

protected:
	TSharedPtr<TDPNavigationPath> mNavigationPath = nullptr;
	TArray<TSharedPtr<TDPNavigationPath>> mNavigationPaths;
	const ATDPVolume* mNavigationVolume = nullptr;
	TSharedPtr<IPathFinder> mPathFinder = nullptr;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual bool GetPawnPosition(FVector& position) const;

	UFUNCTION(BlueprintCallable)
	bool FindPath(const FVector& targetPosition);

	bool FindPathAsync(const FVector& targetPosition, FThreadSafeBool& complete);

	bool CanFindPathAsync(const FVector& targetPosition) const;
	bool CanFindPathAsync(const TDPNodeLink& targetLink) const;

	TSharedPtr<TDPNavigationPath> GetPath();
	const ATDPVolume* GetVolume() const;

	bool GetMoveRequested() const;
	void SetMoveRequested(bool requested);

private:
	TSharedPtr<FAsyncTask<FindPathTask>> mCurrentAsyncTask = nullptr;
	TArray<TSharedPtr<FAsyncTask<FindPathTask>>> mTasks;
	TDPNodeLink mLastTargetLink;
	bool mMoveRequested = false;
};
