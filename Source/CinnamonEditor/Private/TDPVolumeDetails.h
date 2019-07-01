// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class ATDPVolume;
class IDetailLayoutBuilder;

/**
 * 
 */
class TDPVolumeDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	FReply OnGenerateSVOClicked();
	FReply OnClearSVOClicked();

private:
	TWeakObjectPtr<ATDPVolume> mVolume;
};
