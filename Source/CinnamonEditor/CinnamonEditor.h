// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "UnrealEd.h"

#if WITH_EDITOR
DECLARE_LOG_CATEGORY_EXTERN(CinnamonEditorLog, Log, All);
#endif

class FCinnamonEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
