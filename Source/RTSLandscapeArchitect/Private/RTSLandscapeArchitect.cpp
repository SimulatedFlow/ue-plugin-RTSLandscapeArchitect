// Copyright 2026 Simulated Flow. All Rights Reserved.

#include "RTSLandscapeArchitect.h"
#include "RTSLandscapeArchitectLog.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogRTSLandscape);

#define LOCTEXT_NAMESPACE "FRTSLandscapeArchitectModule"

void FRTSLandscapeArchitectModule::StartupModule()
{
	UE_LOG(LogRTSLandscape, Log, TEXT("RTSLandscapeArchitect runtime module started."));
}

void FRTSLandscapeArchitectModule::ShutdownModule()
{
	UE_LOG(LogRTSLandscape, Log, TEXT("RTSLandscapeArchitect runtime module shut down."));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRTSLandscapeArchitectModule, RTSLandscapeArchitect)
