// Fill out your copyright notice in the Description page of Project Settings.


#include "TDPDynamicObstacleComponent.h"
#include "TDPVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Function.h"

// Sets default values for this component's properties
UTDPDynamicObstacleComponent::UTDPDynamicObstacleComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

bool UTDPDynamicObstacleComponent::GetRandomPointInVolume(FVector& point)
{
	if (IsValid(mVolume))
	{
		point = mVolume->GetRandomPointInVolume();

		return true;
	}

	return false;
}

void UTDPDynamicObstacleComponent::PauseDynamicUpdate()
{
	GetWorld()->GetTimerManager().PauseTimer(mUpdateTimerHandle);
}

void UTDPDynamicObstacleComponent::ResumeDynamicUpdate()
{
	GetWorld()->GetTimerManager().UnPauseTimer(mUpdateTimerHandle);
}

void UTDPDynamicObstacleComponent::UpdateTrackedNodes()
{
	mTrackedNodes = mVolume->GetAffectedNodes(GetOwner());
}

TArray<TDPNodeLink>& UTDPDynamicObstacleComponent::GetTrackedNodes()
{
	return mTrackedNodes;
}

const TArray<TDPNodeLink>& UTDPDynamicObstacleComponent::GetTrackedNodes() const
{
	return mTrackedNodes;
}

// Called when the game starts
void UTDPDynamicObstacleComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Enabled && FindNavigationVolume())
	{
		UpdateTrackedNodes();

		GetWorld()->GetTimerManager().SetTimer(mUpdateTimerHandle, [this]() 
		{
			mVolume->RequestOctreeUpdate(*this);
		}, UpdateFrecuency, true);
	}	
}

bool UTDPDynamicObstacleComponent::FindNavigationVolume()
{
	auto owner = GetOwner();

	if (owner == nullptr)
	{
		return false;
	}
	
	auto position = owner->GetActorLocation();

	TArray<AActor*> volumes;
	UGameplayStatics::GetAllActorsOfClass(Cast<UObject>(GetWorld()), ATDPVolume::StaticClass(), volumes);

	for (auto actor : volumes)
	{
		auto volume = Cast<ATDPVolume>(actor);
		if (volume && volume->IsPointInside(position))
		{
			mVolume = volume;
			return true;
		}
	}

	return false;
}


// Called every frame
void UTDPDynamicObstacleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

