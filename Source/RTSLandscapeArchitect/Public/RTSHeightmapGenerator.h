// Copyright 2026 Silvan Teufel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "RTSLandscapeTypes.h"
#include "RTSHeightmapGenerator.generated.h"

/**
 * Math helper that turns an FRTSLandscapeConfig into a discrete height field.
 *
 * The generator scatters a set of deterministic region seeds (a Voronoi-style
 * partition), assigns each region a plateau level, then:
 *   - places flat connecting ramps between adjacent regions that differ by one
 *     level, so units can walk up/down where a ramp exists but not across cliffs,
 *   - closes the outer map edge according to the configured ERTSBorderType
 *     (rising mountains via Perlin noise, or falling water / abyss).
 *
 * The result is fully deterministic for a given seed + config, so editor and
 * runtime generation of the same actor produce identical terrain.
 */
UCLASS(BlueprintType)
class RTSLANDSCAPEARCHITECT_API URTSHeightmapGenerator : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Generate the height field for the supplied configuration.
	 * @param Config   The landscape configuration.
	 * @param OutData  Filled with the generated grid of heights and level indices.
	 * @return true if a valid height field was produced.
	 */
	UFUNCTION(BlueprintCallable, Category = "RTS Landscape")
	bool GenerateHeightmap(const FRTSLandscapeConfig& Config, FRTSHeightmapData& OutData) const;

	/** Static convenience wrapper for Blueprint / C++ callers that do not need an instance. */
	UFUNCTION(BlueprintCallable, Category = "RTS Landscape", meta = (DisplayName = "Generate RTS Heightmap"))
	static bool BuildHeightmap(const FRTSLandscapeConfig& Config, FRTSHeightmapData& OutData);

private:
	/** One Voronoi region seed. */
	struct FRegionSeed
	{
		FVector2D Position = FVector2D::ZeroVector;
		int32 Level = 0;
		/** When true (island mode), this region sits below the water line. */
		bool bWater = false;
	};

	/** Scatter deterministic region seeds (jittered grid) and assign each a plateau level from a smooth field. */
	static void BuildRegions(const FRTSLandscapeConfig& Config, FRandomStream& Rng, TArray<FRegionSeed>& OutRegions);

	/** Border falloff in [0,1]: 1 at the outer edge, 0 once inside the playable area. */
	static float BorderMask(const FVector2D& NormalizedPos, float BorderFraction);

	/** Enforce map symmetry (mirror/rotational) on a finished height field for balanced spawns. */
	static void ApplySymmetry(ERTSSymmetry Sym, FRTSHeightmapData& Data);
};
