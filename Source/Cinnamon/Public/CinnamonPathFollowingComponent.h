// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/PathFollowingComponent.h"
#include "CinnamonPathFollowingComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class CINNAMON_API UCinnamonPathFollowingComponent : public UPathFollowingComponent
{
	GENERATED_BODY()
	
	//~ Begin UActorComponent Interface
	//virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	//~ End UActorComponent Interface
};
