// Copyright 2026 Simulated Flow. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * Editor module for the RTS Landscape Architect plugin. Registers a Details-panel
 * customization for ARTSLandscapeActor that adds the "Generate Editor Landscape" button.
 */
class FRTSLandscapeArchitectEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
