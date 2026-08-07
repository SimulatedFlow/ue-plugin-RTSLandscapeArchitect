// Copyright 2026 Silvan Teufel. All Rights Reserved.

#include "RTSHeightmapGenerator.h"
#include "RTSLandscapeArchitectLog.h"

namespace
{
	/** Fraction of the map, measured from each edge inward, reserved for the border band. */
	constexpr float BorderBandFraction = 0.12f;

	/**
	 * Median-filter the height field to clean up jagged (stair-stepped) plateau / cliff
	 * boundaries while keeping flat plateaus flat and cliff steps sharp. A median filter is
	 * ideal here: on a flat plateau the median is the plateau height (unchanged); at a jagged
	 * boundary it snaps stray cells to the local majority level, straightening the cliff edge.
	 */
	void SmoothCliffBoundaries(FRTSHeightmapData& Data, int32 Iterations)
	{
		if (Iterations <= 0 || !Data.IsValid())
		{
			return;
		}

		const int32 N = Data.VertsPerSide;
		TArray<float> OutH; OutH.SetNumUninitialized(N * N);
		TArray<int32> OutL; OutL.SetNumUninitialized(N * N);

		for (int32 It = 0; It < Iterations; ++It)
		{
			for (int32 Y = 0; Y < N; ++Y)
			{
				for (int32 X = 0; X < N; ++X)
				{
					float H[9];
					int32 L[9];
					int32 C = 0;
					for (int32 DY = -1; DY <= 1; ++DY)
					{
						for (int32 DX = -1; DX <= 1; ++DX)
						{
							const int32 NX = FMath::Clamp(X + DX, 0, N - 1);
							const int32 NY = FMath::Clamp(Y + DY, 0, N - 1);
							const int32 I = Data.Index(NX, NY);
							H[C] = Data.Heights[I];
							L[C] = Data.LevelIndices[I];
							++C;
						}
					}
					// Insertion-sort the 3x3 neighbourhood and take the median (index 4).
					for (int32 i = 1; i < 9; ++i)
					{
						const float KH = H[i]; int32 j = i - 1;
						while (j >= 0 && H[j] > KH) { H[j + 1] = H[j]; --j; }
						H[j + 1] = KH;
					}
					for (int32 i = 1; i < 9; ++i)
					{
						const int32 KL = L[i]; int32 j = i - 1;
						while (j >= 0 && L[j] > KL) { L[j + 1] = L[j]; --j; }
						L[j + 1] = KL;
					}
					const int32 Idx = Data.Index(X, Y);
					OutH[Idx] = H[4];
					OutL[Idx] = L[4];
				}
			}
			Data.Heights = OutH;
			Data.LevelIndices = OutL;
		}
	}

}

void URTSHeightmapGenerator::BuildRegions(const FRTSLandscapeConfig& Config, FRandomStream& Rng, TArray<FRegionSeed>& OutRegions)
{
	// How many plateau regions to scatter. Driven directly by the designer via PlateauCount
	// so the map's busy-ness is a single, obvious dial.
	const int32 DesiredRegions = FMath::Clamp(Config.PlateauCount, 4, 128);

	// Place the seeds on a jittered grid rather than fully at random: this keeps plateau
	// sizes even (no big empty gaps or dense clumps) so the whole map is used.
	const int32 GridDim = FMath::Max(1, FMath::CeilToInt(FMath::Sqrt(static_cast<float>(DesiredRegions))));

	// A per-seed random offset makes the smooth height field (and island layout) depend on Seed.
	const FVector2D NoiseOffset(Rng.FRand() * 1000.0f, Rng.FRand() * 1000.0f);
	const float LevelScale = FMath::Clamp(Config.TerrainVariation, 0.5f, 8.0f);
	const int32 MaxLevel = FMath::Max(0, Config.NumberOfLevels - 1);

	OutRegions.Reset(DesiredRegions);
	for (int32 Gy = 0; Gy < GridDim && OutRegions.Num() < DesiredRegions; ++Gy)
	{
		for (int32 Gx = 0; Gx < GridDim && OutRegions.Num() < DesiredRegions; ++Gx)
		{
			FRegionSeed Seed;
			// Jitter inside the grid cell (kept away from the very edges so neighbours don't overlap).
			const float Jx = 0.18f + 0.64f * Rng.FRand();
			const float Jy = 0.18f + 0.64f * Rng.FRand();
			Seed.Position = FVector2D((Gx + Jx) / GridDim, (Gy + Jy) / GridDim);

			// Assign the plateau level from a SMOOTH noise field sampled at the seed position.
			// Because neighbouring seeds sample nearby points, their levels usually differ by
			// only one step -> the generator produces walkable ramps all over the map instead
			// of the rare, random one-off ramps the old purely-random assignment gave.
			// Sample a single smooth elevation field at the seed position, then stretch its
			// contrast. Raw Perlin clusters around 0.5, which would drop almost every plateau on
			// the middle level; stretching spreads the levels into distinct large low / mid / high
			// areas -- the StarCraft-style layout of clearly separated high and low ground.
			float Elevation = 0.5f * (FMath::PerlinNoise2D((Seed.Position + NoiseOffset) * LevelScale) + 1.0f);
			Elevation = FMath::Clamp(0.5f + (Elevation - 0.5f) * FMath::Max(1.0f, Config.LevelSpread), 0.0f, 1.0f);

			if (Config.bGenerateIslands)
			{
				// Islands: below the water line is sea; land rises from the shore (level 0) up to
				// its peak in tiers, so every island is genuinely multi-level -- ramps between
				// adjacent tiers and cliffs where it steps up sharply.
				const float SeaLevel = 0.45f;
				if (Elevation < SeaLevel)
				{
					Seed.bWater = true;
					Seed.Level = 0;
				}
				else
				{
					const float LandT = (Elevation - SeaLevel) / FMath::Max(1.0f - SeaLevel, KINDA_SMALL_NUMBER);
					Seed.Level = FMath::Clamp(FMath::FloorToInt(LandT * Config.NumberOfLevels), 0, MaxLevel);
				}
			}
			else
			{
				Seed.Level = FMath::Clamp(FMath::FloorToInt(FMath::Clamp(Elevation, 0.0f, 1.0f) * Config.NumberOfLevels), 0, MaxLevel);
			}

			OutRegions.Add(Seed);
		}
	}
}

float URTSHeightmapGenerator::BorderMask(const FVector2D& NormalizedPos, float BorderFraction)
{
	// Distance to nearest edge, in [0, 0.5]; 0 at the edge.
	const float DistX = FMath::Min(NormalizedPos.X, 1.0f - NormalizedPos.X);
	const float DistY = FMath::Min(NormalizedPos.Y, 1.0f - NormalizedPos.Y);
	const float EdgeDist = FMath::Min(DistX, DistY);

	if (EdgeDist >= BorderFraction)
	{
		return 0.0f;
	}
	// 1 at the very edge, smoothly falling to 0 at the inner border line.
	const float T = 1.0f - (EdgeDist / FMath::Max(BorderFraction, KINDA_SMALL_NUMBER));
	return FMath::SmoothStep(0.0f, 1.0f, FMath::Clamp(T, 0.0f, 1.0f));
}

void URTSHeightmapGenerator::ApplySymmetry(ERTSSymmetry Sym, FRTSHeightmapData& Data)
{
	if (Sym == ERTSSymmetry::None || !Data.IsValid())
	{
		return;
	}

	const int32 N = Data.VertsPerSide;
	const int32 Last = N - 1;

	// Copy one grid vertex (height + level) from a source cell to a destination cell.
	auto CopyFrom = [&](int32 DX, int32 DY, int32 SX, int32 SY)
	{
		const int32 D = Data.Index(DX, DY);
		const int32 S = Data.Index(SX, SY);
		Data.Heights[D] = Data.Heights[S];
		Data.LevelIndices[D] = Data.LevelIndices[S];
	};

	// We iterate in ascending index order and only ever write a cell FROM a lower-index
	// "source" cell (which has therefore not been overwritten yet), so a single pass suffices.
	for (int32 Y = 0; Y < N; ++Y)
	{
		for (int32 X = 0; X < N; ++X)
		{
			switch (Sym)
			{
			case ERTSSymmetry::Mirror2:
				// Reflect the left half onto the right (across the vertical centre).
				if (X > Last - X) { CopyFrom(X, Y, Last - X, Y); }
				break;

			case ERTSSymmetry::Mirror4:
				// Reflect the top-left quadrant into all four quadrants.
				CopyFrom(X, Y, FMath::Min(X, Last - X), FMath::Min(Y, Last - Y));
				break;

			case ERTSSymmetry::Rotate2:
			{
				// 180° rotation: fill the later half from the rotation of the earlier half.
				const int32 RX = Last - X, RY = Last - Y;
				if (Y * N + X > RY * N + RX) { CopyFrom(X, Y, RX, RY); }
				break;
			}
			case ERTSSymmetry::Rotate4:
			{
				// 90° rotation: fill each cell from the lowest-index cell of its 4-cell orbit.
				int32 CX = X, CY = Y, CBest = Y * N + X;
				const int32 Rots[3][2] = { { Last - Y, X }, { Last - X, Last - Y }, { Y, Last - X } };
				for (int32 r = 0; r < 3; ++r)
				{
					const int32 RIdx = Rots[r][1] * N + Rots[r][0];
					if (RIdx < CBest) { CBest = RIdx; CX = Rots[r][0]; CY = Rots[r][1]; }
				}
				if (CX != X || CY != Y) { CopyFrom(X, Y, CX, CY); }
				break;
			}
			default:
				break;
			}
		}
	}
}

bool URTSHeightmapGenerator::GenerateHeightmap(const FRTSLandscapeConfig& Config, FRTSHeightmapData& OutData) const
{
	const int32 Res = FMath::Clamp(Config.GridResolution, 16, 512);
	const int32 Verts = Res + 1;

	OutData = FRTSHeightmapData();
	OutData.VertsPerSide = Verts;
	OutData.Heights.SetNumZeroed(Verts * Verts);
	OutData.LevelIndices.SetNumZeroed(Verts * Verts);

	FRandomStream Rng(Config.Seed);
	TArray<FRegionSeed> Regions;
	BuildRegions(Config, Rng, Regions);
	if (Regions.Num() == 0)
	{
		UE_LOG(LogRTSLandscape, Warning, TEXT("Heightmap generation produced no regions."));
		return false;
	}

	// Ramp width expressed as a fraction of the map so we can compare it against the
	// normalized signed distance to a region border.
	const float RampFrac = FMath::Clamp(Config.RampWidth / FMath::Max(Config.MapSize.GetMax(), 1.0f), 0.005f, 0.30f);
	const float BorderPeak = Config.LevelHeightDifference * (Config.NumberOfLevels + 1.5f);
	const float IslandWaterDepth = -Config.LevelHeightDifference * 0.8f;   // basin for island/lake regions
	const float EdgeWaterDepth = -Config.LevelHeightDifference * 2.0f;     // deeper abyss at a water border edge


	int32 RampCount = 0;
	int32 WaterRegions = 0;
	for (const FRegionSeed& R : Regions)
	{
		WaterRegions += R.bWater ? 1 : 0;
	}

	for (int32 Y = 0; Y < Verts; ++Y)
	{
		for (int32 X = 0; X < Verts; ++X)
		{
			const FVector2D P(static_cast<float>(X) / Res, static_cast<float>(Y) / Res);

			// Find the nearest and second-nearest region seed (Voronoi).
			int32 NearestIdx = 0, SecondIdx = INDEX_NONE;
			float NearestD2 = TNumericLimits<float>::Max();
			float SecondD2 = TNumericLimits<float>::Max();
			for (int32 r = 0; r < Regions.Num(); ++r)
			{
				const float D2 = FVector2D::DistSquared(P, Regions[r].Position);
				if (D2 < NearestD2)
				{
					SecondD2 = NearestD2; SecondIdx = NearestIdx;
					NearestD2 = D2; NearestIdx = r;
				}
				else if (D2 < SecondD2)
				{
					SecondD2 = D2; SecondIdx = r;
				}
			}

			const FRegionSeed& RegA = Regions[NearestIdx];
			int32 CellLevel = RegA.Level;
			float Height = RegA.Level * Config.LevelHeightDifference;

			if (RegA.bWater)
			{
				// This cell belongs to a submerged region.
				Height = IslandWaterDepth;
				CellLevel = INDEX_NONE;
			}
			else if (SecondIdx != INDEX_NONE)
			{
				const FRegionSeed& RegB = Regions[SecondIdx];
				const float DA = FMath::Sqrt(NearestD2);
				const float DB = FMath::Sqrt(SecondD2);
				const float BorderDist = 0.5f * (DB - DA); // 0 exactly on the border

				if (RegB.bWater)
				{
					// Shoreline: slope the land down to the water line for a beach instead of a wall.
					if (BorderDist < RampFrac)
					{
						const float T = FMath::Clamp(BorderDist / RampFrac, 0.0f, 1.0f);
						Height = FMath::Lerp(0.0f, RegA.Level * Config.LevelHeightDifference, T);
					}
				}
				else if (FMath::Abs(RegA.Level - RegB.Level) == 1 && BorderDist < RampFrac)
				{
					// Adjacent plateaus one step apart are joined by at most ONE narrow ramp at a
					// fixed spot on their shared border -- a StarCraft-style chokepoint. Only a
					// subset of boundaries get a ramp at all, so each plateau/island has just a few
					// (1-3) access points; everywhere else the one-step boundary stays a sharp cliff.
					// (Bigger level gaps are always cliffs.)

					// Canonical pair order (independent of which region is nearest) so the ramp lands
					// at the same place when evaluated from either side of the border.
					const bool bAFirst = (RegA.Position.X < RegB.Position.X) ||
						(RegA.Position.X == RegB.Position.X && RegA.Position.Y <= RegB.Position.Y);
					const FVector2D PA = bAFirst ? RegA.Position : RegB.Position;
					const FVector2D PB = bAFirst ? RegB.Position : RegA.Position;
					const FVector2D AB = PB - PA;
					const float ABLen = AB.Size();
					if (ABLen > KINDA_SMALL_NUMBER)
					{
						const FVector2D AlongDir(-AB.Y / ABLen, AB.X / ABLen); // runs along the shared border
						const FVector2D MidPos = 0.5f * (PA + PB);
						const float AlongCoord = FVector2D::DotProduct(P - MidPos, AlongDir);

						// Two deterministic per-pair values (order-independent hash of the seed positions).
						const FVector2D Sum = PA + PB;
						const float PairHash = FMath::Frac(FMath::Sin(FVector2D::DotProduct(Sum, FVector2D(12.9898f, 78.233f))) * 43758.5453f);
						const float AnchorHash = FMath::Frac(FMath::Sin(FVector2D::DotProduct(Sum, FVector2D(39.346f, 11.135f))) * 24634.6345f);

						const float RampAnchor = (AnchorHash - 0.5f) * (RampFrac * 1.4f); // where along the border the ramp sits
						const float RampHalfWidth = RampFrac * 0.5f;                       // NARROW opening

						if (PairHash < Config.RampDensity && FMath::Abs(AlongCoord - RampAnchor) < RampHalfWidth)
						{
							const float T = FMath::Clamp(BorderDist / RampFrac, 0.0f, 1.0f);
							const float HA = RegA.Level * Config.LevelHeightDifference;
							const float HB = RegB.Level * Config.LevelHeightDifference;
							const float MidH = 0.5f * (HA + HB);
							Height = FMath::Lerp(MidH, HA, T);
							++RampCount;
						}
					}
				}
			}

			// Border band: mountains rise, water falls away.
			const float Mask = BorderMask(P, FMath::Clamp(Config.BorderThickness, 0.03f, 0.30f));
			if (Mask > 0.0f)
			{
				if (Config.BorderType == ERTSBorderType::Mountains)
				{
					const float Noise = 0.5f * (FMath::PerlinNoise2D(P * 9.0f) + 1.0f);
					Height += Mask * BorderPeak * (0.65f + 0.35f * Noise);
				}
				else // Water
				{
					Height = FMath::Lerp(Height, EdgeWaterDepth, Mask);
				}
				CellLevel = INDEX_NONE;
			}

			const int32 Idx = OutData.Index(X, Y);
			OutData.Heights[Idx] = Height;
			OutData.LevelIndices[Idx] = CellLevel;
		}
	}

	// Clean the jagged, stair-stepped plateau/cliff boundaries with a MEDIAN filter. Crucially
	// this keeps the cliffs STEEP (a one-cell vertical step) so they break the NavMesh and stay
	// impassable -- only the wide, gently-sloped ramps are walkable, which is what gives the map
	// its StarCraft-style high/low ground gated by ramp chokepoints. CliffSoftness controls how
	// much the boundary corners are rounded here (and how soft the shading is), but it never
	// slopes the cliff face, so cliffs are always impassable regardless of the softness value.
	const int32 MedianPasses = FMath::Clamp(FMath::RoundToInt(FMath::Clamp(Config.CliffSoftness, 0.0f, 1.0f) * 2.0f), 0, 2);
	SmoothCliffBoundaries(OutData, MedianPasses);

	// Enforce symmetry last, so both/all sides of a competitive map are perfectly balanced.
	ApplySymmetry(Config.Symmetry, OutData);

	UE_LOG(LogRTSLandscape, Log, TEXT("Generated %dx%d heightmap from %d regions (%d water) across %d levels, %s border, %d ramp verts."),
		Verts, Verts, Regions.Num(), WaterRegions, Config.NumberOfLevels,
		Config.BorderType == ERTSBorderType::Mountains ? TEXT("mountain") : TEXT("water"), RampCount);
	return OutData.IsValid();
}

bool URTSHeightmapGenerator::BuildHeightmap(const FRTSLandscapeConfig& Config, FRTSHeightmapData& OutData)
{
	URTSHeightmapGenerator* Gen = NewObject<URTSHeightmapGenerator>();
	return Gen->GenerateHeightmap(Config, OutData);
}
