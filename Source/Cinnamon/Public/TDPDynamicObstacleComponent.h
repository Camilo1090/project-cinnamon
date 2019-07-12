// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TDPNodeLink.h"
#include "TDPDynamicObstacleComponent.generated.h"

class ATDPVolume;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CINNAMON_API UTDPDynamicObstacleComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTDPDynamicObstacleComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	bool GetRandomPointInVolume(FVector& point);

	UFUNCTION(BlueprintCallable)
	void PauseDynamicUpdate();

	UFUNCTION(BlueprintCallable)
	void ResumeDynamicUpdate();

public:
	void UpdateTrackedNodes();

	TArray<TDPNodeLink>& GetTrackedNodes();
	const TArray<TDPNodeLink>& GetTrackedNodes() const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Obstacle")
	bool Enabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Obstacle")
	float UpdateFrecuency = 0.5f;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	bool FindNavigationVolume();

protected:
	ATDPVolume* mVolume = nullptr;
	FTimerHandle mUpdateTimerHandle;

	TArray<TDPNodeLink> mTrackedNodes;

		
};
