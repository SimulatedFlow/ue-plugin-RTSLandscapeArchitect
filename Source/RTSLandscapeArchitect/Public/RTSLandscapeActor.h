// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RTSLandscapeTypes.h"
#include "RTSLandscapeActor.generated.h"

class UProceduralMeshComponent;
class UPCGComponent;

/** Fired once geometry, collision, PCG decoration and NavMesh rebuild are all complete. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRTSOnLandscapeGenerationComplete);

/**
 * The actor that is placed in a level to build an RTS / topdown landscape.
 *
 * At runtime it builds an optimized UProceduralMeshComponent from the height
 * field produced by URTSHeightmapGenerator, assigns the configured material,
 * cooks collision, optionally runs the configured PCG graph over the plateaus
 * and rebuilds the navigation mesh. The editor module adds a
 * "Generate Editor Landscape" button that drives the same generation so
 * designers get an editable starting point.
 */
UCLASS(Blueprintable, ClassGroup = (RTSLandscapeArchitect), meta = (BlueprintSpawnableComponent))
class RTSLANDSCAPEARCHITECT_API ARTSLandscapeActor : public AActor
{
	GENERATED_BODY()

public:
	ARTSLandscapeActor();

	/** Landscape configuration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape", meta = (ShowOnlyInnerProperties))
	FRTSLandscapeConfig Config;

	/** Generate the landscape automatically when the actor begins play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape")
	bool bGenerateOnBeginPlay = true;

	/**
	 * Rebuild the terrain LIVE while you edit the settings above. Off (the default) is the
	 * calmer, button-driven workflow: the terrain is still built once when you place the actor
	 * or open the map (so it is never empty), but changing a setting does NOT rebuild until you
	 * press "Generate Editor Landscape". Turn this on if you want every tweak to update instantly.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape")
	bool bAutoGenerateInEditor = false;

	/** Fired when the full generation pipeline has finished. */
	UPROPERTY(BlueprintAssignable, Category = "RTS Landscape")
	FRTSOnLandscapeGenerationComplete OnLandscapeGenerationComplete;

	/**
	 * Build (or rebuild) the entire landscape: heightmap -> procedural mesh ->
	 * collision -> material -> PCG decoration -> NavMesh rebuild -> completion event.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "RTS Landscape")
	void GenerateLandscape();

	/** Remove the currently generated geometry. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "RTS Landscape")
	void ClearLandscape();

	/** Pick a new random seed and rebuild — the quickest way to roll fresh map layouts. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "RTS Landscape")
	void RandomizeSeed();

	/** Run the configured PCG graph over the generated mesh (called by GenerateLandscape). */
	UFUNCTION(BlueprintCallable, Category = "RTS Landscape")
	void TriggerPCGGeneration();

	/** Rebuild the navigation mesh over the generated area (called by GenerateLandscape). */
	UFUNCTION(BlueprintCallable, Category = "RTS Landscape")
	void RebuildNavigation();

	/** The procedural mesh component that holds the generated terrain. */
	UFUNCTION(BlueprintPure, Category = "RTS Landscape")
	UProceduralMeshComponent* GetLandscapeMesh() const { return LandscapeMesh; }

protected:
	virtual void BeginPlay() override;
#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

private:
	/**
	 * Build only the terrain geometry: heightmap -> procedural mesh -> material.
	 * Cheap enough to run on every editor edit; used by both OnConstruction and
	 * GenerateLandscape. Returns false if the heightmap could not be generated.
	 */
	bool BuildGeometryOnly();

	/** Build the procedural mesh section from a generated height field. */
	void BuildMesh(const FRTSHeightmapData& Data);

	/** Resolve the material to use: the configured one, or the shipped default so the terrain is never unlit/black. */
	UMaterialInterface* ResolveLandscapeMaterial() const;

	UPROPERTY(VisibleAnywhere, Category = "RTS Landscape", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProceduralMeshComponent> LandscapeMesh;

	UPROPERTY(Transient)
	TObjectPtr<UPCGComponent> PCGComponent;
};
