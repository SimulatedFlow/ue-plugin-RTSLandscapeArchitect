// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "RTSLandscapeActorDetails.h"
#include "RTSLandscapeActor.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "RTSLandscapeActorDetails"

TSharedRef<IDetailCustomization> FRTSLandscapeActorDetails::MakeInstance()
{
	return MakeShared<FRTSLandscapeActorDetails>();
}

void FRTSLandscapeActorDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	for (const TWeakObjectPtr<UObject>& Obj : Objects)
	{
		if (ARTSLandscapeActor* Actor = Cast<ARTSLandscapeActor>(Obj.Get()))
		{
			SelectedActors.Add(Actor);
		}
	}

	if (SelectedActors.Num() == 0)
	{
		return;
	}

	// Put the buttons + a one-line hint into the SAME category as the settings and pin it to
	// the top, so the designer sees the actual settings the moment they select the actor
	// (a long help block used to push them off-screen).
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(
		"RTS Landscape", LOCTEXT("ToolsCategory", "RTS Landscape"),
		ECategoryPriority::Important);

	Category.AddCustomRow(LOCTEXT("GenerateRowFilter", "Generate Editor Landscape"))
	.WholeRowContent()
	[
		SNew(SVerticalBox)
		// One concise hint — the settings above are the real UI; this actor is self-driving.
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Help",
				"Set the options above, then click Generate to build (it also runs PCG + the NavMesh). "
				"Randomize rolls a new seed; Clear removes the terrain. Tick \"Auto Generate In Editor\" "
				"to rebuild live as you edit instead."))
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 6.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SBox)
				.HeightOverride(32.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ToolTipText(LOCTEXT("GenerateTip", "Rebuild the terrain now and also run PCG decoration and the NavMesh rebuild (the full pipeline)."))
					.OnClicked(this, &FRTSLandscapeActorDetails::OnGenerateClicked)
					[
						SNew(STextBlock).Text(LOCTEXT("GenerateBtn", "Generate Editor Landscape"))
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SBox)
				.HeightOverride(32.0f)
				.MinDesiredWidth(110.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ToolTipText(LOCTEXT("RandomizeTip", "Pick a new random seed and regenerate — roll a fresh map layout."))
					.OnClicked(this, &FRTSLandscapeActorDetails::OnRandomizeClicked)
					[
						SNew(STextBlock).Text(LOCTEXT("RandomizeBtn", "Randomize"))
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.HeightOverride(32.0f)
				.MinDesiredWidth(90.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ToolTipText(LOCTEXT("ClearTip", "Remove the generated geometry."))
					.OnClicked(this, &FRTSLandscapeActorDetails::OnClearClicked)
					[
						SNew(STextBlock).Text(LOCTEXT("ClearBtn", "Clear"))
					]
				]
			]
		]
	];
}

FReply FRTSLandscapeActorDetails::OnGenerateClicked()
{
	const FScopedTransaction Transaction(LOCTEXT("GenerateTransaction", "Generate RTS Landscape"));
	for (const TWeakObjectPtr<ARTSLandscapeActor>& WeakActor : SelectedActors)
	{
		if (ARTSLandscapeActor* Actor = WeakActor.Get())
		{
			Actor->Modify();
			Actor->GenerateLandscape();
		}
	}
	return FReply::Handled();
}

FReply FRTSLandscapeActorDetails::OnClearClicked()
{
	const FScopedTransaction Transaction(LOCTEXT("ClearTransaction", "Clear RTS Landscape"));
	for (const TWeakObjectPtr<ARTSLandscapeActor>& WeakActor : SelectedActors)
	{
		if (ARTSLandscapeActor* Actor = WeakActor.Get())
		{
			Actor->Modify();
			Actor->ClearLandscape();
		}
	}
	return FReply::Handled();
}

FReply FRTSLandscapeActorDetails::OnRandomizeClicked()
{
	const FScopedTransaction Transaction(LOCTEXT("RandomizeTransaction", "Randomize RTS Landscape"));
	for (const TWeakObjectPtr<ARTSLandscapeActor>& WeakActor : SelectedActors)
	{
		if (ARTSLandscapeActor* Actor = WeakActor.Get())
		{
			Actor->Modify();
			Actor->RandomizeSeed();
		}
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
