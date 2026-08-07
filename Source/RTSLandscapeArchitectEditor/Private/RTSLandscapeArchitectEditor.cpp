// Copyright 2026 Simulated Flow. All Rights Reserved.

#include "RTSLandscapeArchitectEditor.h"
#include "RTSLandscapeActorDetails.h"
#include "RTSLandscapeActor.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FRTSLandscapeArchitectEditorModule"

void FRTSLandscapeArchitectEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(
		ARTSLandscapeActor::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FRTSLandscapeActorDetails::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();
}

void FRTSLandscapeArchitectEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(ARTSLandscapeActor::StaticClass()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRTSLandscapeArchitectEditorModule, RTSLandscapeArchitectEditor)
