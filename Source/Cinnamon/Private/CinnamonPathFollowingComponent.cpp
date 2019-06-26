// Fill out your copyright notice in the Description page of Project Settings.


#include "CinnamonPathFollowingComponent.h"

void UCinnamonPathFollowingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bUseBlockDetection = false;
}
