// Copyright 2026 Simulated Flow. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "RTSLandscapeTypes.generated.h"

class UMaterialInterface;
class UPCGGraphInterface;

/**
 * How the outer edge of the generated map is closed off so that the play field
 * is clearly bounded (a core requirement for RTS / topdown / tactics maps).
 */
UENUM(BlueprintType)
enum class ERTSBorderType : uint8
{
	/** High, impassable mountains rise along the map edge. */
	Mountains	UMETA(DisplayName = "Mountains"),

	/** The terrain falls away into water / an abyss along the map edge. */
	Water		UMETA(DisplayName = "Water")
};

/**
 * Symmetry enforced on the finished map so competitive spawns are perfectly balanced.
 * Mirror types reflect one part of the map; rotational types rotate it (so every spawn
 * has the same handedness — usually preferred for fair 1v1 / team maps).
 */
UENUM(BlueprintType)
enum class ERTSSymmetry : uint8
{
	/** No symmetry — a free-form map. */
	None		UMETA(DisplayName = "None"),

	/** Mirror the left half onto the right (2 balanced sides). */
	Mirror2		UMETA(DisplayName = "Mirror (2-way)"),

	/** Mirror one quadrant into all four (4 balanced quarters). */
	Mirror4		UMETA(DisplayName = "Mirror (4-way)"),

	/** 180° rotational symmetry — ideal for fair 1v1 maps. */
	Rotate2		UMETA(DisplayName = "Rotational (2-way / 1v1)"),

	/** 90° rotational symmetry — for 4-player maps. */
	Rotate4		UMETA(DisplayName = "Rotational (4-way)")
};

/**
 * Full configuration for a single generated RTS landscape. Editable in the
 * editor Details panel and fully Blueprint-exposed.
 */
USTRUCT(BlueprintType)
struct RTSLANDSCAPEARCHITECT_API FRTSLandscapeConfig
{
	GENERATED_BODY()

	/** Number of vertical plateau levels (2..5). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape",
		meta = (ClampMin = "2", ClampMax = "5", UIMin = "2", UIMax = "5"))
	int32 NumberOfLevels = 3;

	/**
	 * How many plateau regions to scatter across the map. Higher = more, smaller
	 * plateaus and a busier, more interesting battlefield. This is the main dial for
	 * how varied the map looks.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape",
		meta = (ClampMin = "4", ClampMax = "128", UIMin = "6", UIMax = "80"))
	int32 PlateauCount = 16;

	/** Height difference (cliff height) between two adjacent levels, in Unreal units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape",
		meta = (ClampMin = "50.0", UIMin = "50.0"))
	float LevelHeightDifference = 300.0f;

	/** Total map size on the X/Y ground plane, in Unreal units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape",
		meta = (ClampMin = "1000.0"))
	FVector2D MapSize = FVector2D(20000.0f, 20000.0f);

	/** How the outer map edge is closed off. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape")
	ERTSBorderType BorderType = ERTSBorderType::Mountains;

	/**
	 * Sink the lowest plateaus below the water line so the map breaks up into
	 * islands and lakes connected by land bridges. Works best with the Water border.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape")
	bool bGenerateIslands = false;

	/** Enforce symmetry so competitive spawns are perfectly balanced (mirror or rotational, 2- or 4-way). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape")
	ERTSSymmetry Symmetry = ERTSSymmetry::None;

	/** Width of the generated connecting ramps between adjacent levels, in Unreal units (a chokepoint). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape",
		meta = (ClampMin = "100.0"))
	float RampWidth = 1000.0f;

	/**
	 * How many one-step level boundaries become walkable ramps. Low = only a couple of
	 * tight chokepoints per plateau (StarCraft-style); high = most boundaries are ramps.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RampDensity = 0.5f;

	/**
	 * 0 = crisp, faceted StarCraft-style cliffs; higher rounds off the cliff and ramp
	 * edges for a softer look. The flat plateaus stay flat either way.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CliffSoftness = 0.35f;

	/** Number of grid quads per map edge. Higher = smoother geometry, more triangles. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape|Advanced",
		meta = (ClampMin = "16", ClampMax = "512"))
	int32 GridResolution = 128;

	/**
	 * How quickly plateau heights change across the map. Higher values make
	 * neighbouring plateaus step up/down more often, giving more ramps and variety;
	 * lower values make broad, gently terraced areas.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape|Advanced",
		meta = (ClampMin = "0.5", ClampMax = "8.0", UIMin = "1.0", UIMax = "6.0"))
	float TerrainVariation = 2.5f;

	/**
	 * How strongly plateau heights separate into distinct low / mid / high ground.
	 * Higher = bolder, more clearly separated levels; lower = gentler, more mid-level.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape|Advanced",
		meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float LevelSpread = 1.9f;

	/** Thickness of the bounded map border (mountains/water), as a fraction of the map size. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape|Advanced",
		meta = (ClampMin = "0.03", ClampMax = "0.30"))
	float BorderThickness = 0.12f;

	/** Deterministic seed. The same seed + config always produces the same map. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape|Advanced")
	int32 Seed = 1337;

	/**
	 * Material applied to the generated landscape surface. Leave this empty to use the
	 * plugin's built-in RTS landscape material, which shades plateaus, ramps, cliffs and
	 * the map border from the mesh vertex colours — so the terrain is never unlit/black.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape")
	TObjectPtr<UMaterialInterface> LandscapeMaterial = nullptr;

	/** PCG graph run over the finished mesh to decorate the plateaus (trees, rocks, spawn points). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape")
	TObjectPtr<UPCGGraphInterface> DefaultPCGGraph = nullptr;

	/** Rebuild the navigation mesh automatically once generation has finished. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS Landscape")
	bool bRebuildNavMeshAtRuntime = true;
};

/**
 * Result of a heightmap generation pass: a regular grid of world-space heights
 * plus the plateau level index each grid vertex belongs to.
 */
USTRUCT(BlueprintType)
struct RTSLANDSCAPEARCHITECT_API FRTSHeightmapData
{
	GENERATED_BODY()

	/** Number of vertices along each axis (GridResolution + 1). */
	UPROPERTY(BlueprintReadOnly, Category = "RTS Landscape")
	int32 VertsPerSide = 0;

	/** World-space Z height for every grid vertex (row-major, VertsPerSide * VertsPerSide entries). */
	UPROPERTY(BlueprintReadOnly, Category = "RTS Landscape")
	TArray<float> Heights;

	/** Plateau level index (0..NumberOfLevels-1) for every grid vertex; -1 marks a border cell. */
	UPROPERTY(BlueprintReadOnly, Category = "RTS Landscape")
	TArray<int32> LevelIndices;

	bool IsValid() const
	{
		return VertsPerSide > 1 && Heights.Num() == VertsPerSide * VertsPerSide;
	}

	FORCEINLINE int32 Index(int32 X, int32 Y) const { return Y * VertsPerSide + X; }
};
