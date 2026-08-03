// Copyright 2026 Simulated Flow. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class ARTSLandscapeActor;

/**
 * Details-panel customization for ARTSLandscapeActor. Adds a prominent
 * "Generate Editor Landscape" button so designers can build (and rebuild) the
 * terrain directly in the editor and then keep editing the result by hand.
 */
class FRTSLandscapeActorDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	//~ IDetailCustomization
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	FReply OnGenerateClicked();
	FReply OnClearClicked();
	FReply OnRandomizeClicked();

	/** The actors currently being customized. */
	TArray<TWeakObjectPtr<ARTSLandscapeActor>> SelectedActors;
};
