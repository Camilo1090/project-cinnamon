// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
DECLARE_LOG_CATEGORY_EXTERN(CinnamonLog, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(VisualCinnamonLog, Log, All);
#endif

class FCinnamonModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
