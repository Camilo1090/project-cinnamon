// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CinnamonEditor.h"
#include "TDPVolumeDetails.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"

IMPLEMENT_GAME_MODULE(FCinnamonEditorModule, CinnamonEditor)

#if WITH_EDITOR
DEFINE_LOG_CATEGORY(CinnamonEditorLog);
#endif

#define LOCTEXT_NAMESPACE "FCinnamonEditorModule"

void FCinnamonEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("TDPVolume", FOnGetDetailCustomizationInstance::CreateStatic(&TDPVolumeDetails::MakeInstance));
}

void FCinnamonEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
}

#undef LOCTEXT_NAMESPACE
