// Copyright 2026 Simulated Flow. All Rights Reserved.

#include "RTSLandscapeActor.h"
#include "RTSHeightmapGenerator.h"
#include "RTSLandscapeArchitectLog.h"

#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "Materials/MaterialInterface.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"

#include "PCGComponent.h"
#include "PCGGraph.h"

namespace
{
	/** Content path of the landscape material shipped with the plugin (used when none is configured). */
	const TCHAR* GDefaultLandscapeMaterialPath = TEXT("/RTSLandscapeArchitect/RTSLandscapeArchitect/Materials/M_RTSLandscape.M_RTSLandscape");
}

ARTSLandscapeActor::ARTSLandscapeActor()
{
	PrimaryActorTick.bCanEverTick = false;

	LandscapeMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("LandscapeMesh"));
	SetRootComponent(LandscapeMesh);
	LandscapeMesh->bUseAsyncCooking = true;
	LandscapeMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	LandscapeMesh->SetCanEverAffectNavigation(true);
}

void ARTSLandscapeActor::BeginPlay()
{
	Super::BeginPlay();
	if (bGenerateOnBeginPlay)
	{
		GenerateLandscape();
	}
}

#if WITH_EDITOR
void ARTSLandscapeActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// In the editor, build the terrain ONCE when the actor is placed or the map is opened (so it
	// is never empty), but do NOT rebuild on every settings change unless the designer opted into
	// live mode (bAutoGenerateInEditor) -- otherwise the Generate button drives updates. We build
	// geometry ONLY here; PCG decoration and the NavMesh rebuild are heavier and belong on the
	// Generate button / BeginPlay. Game worlds are skipped so BeginPlay owns runtime generation.
	const UWorld* World = GetWorld();
	if (World && !World->IsGameWorld())
	{
		const bool bNeedsFirstBuild = !LandscapeMesh || LandscapeMesh->GetNumSections() == 0;
		if (bAutoGenerateInEditor || bNeedsFirstBuild)
		{
			BuildGeometryOnly();
		}
	}
}
#endif

void ARTSLandscapeActor::GenerateLandscape()
{
	if (!BuildGeometryOnly())
	{
		UE_LOG(LogRTSLandscape, Warning, TEXT("GenerateLandscape aborted: geometry build failed."));
		return;
	}

	TriggerPCGGeneration();

	if (Config.bRebuildNavMeshAtRuntime)
	{
		RebuildNavigation();
	}

	UE_LOG(LogRTSLandscape, Log, TEXT("Landscape generation complete for '%s'."), *GetName());
	OnLandscapeGenerationComplete.Broadcast();
}

bool ARTSLandscapeActor::BuildGeometryOnly()
{
	FRTSHeightmapData Data;
	if (!URTSHeightmapGenerator::BuildHeightmap(Config, Data))
	{
		UE_LOG(LogRTSLandscape, Warning, TEXT("Heightmap generation failed for '%s'."), *GetName());
		return false;
	}

	BuildMesh(Data);
	return true;
}

void ARTSLandscapeActor::ClearLandscape()
{
	if (LandscapeMesh)
	{
		LandscapeMesh->ClearAllMeshSections();
	}
}

void ARTSLandscapeActor::RandomizeSeed()
{
	Config.Seed = FMath::Rand();
	GenerateLandscape();
}

void ARTSLandscapeActor::BuildMesh(const FRTSHeightmapData& Data)
{
	if (!LandscapeMesh || !Data.IsValid())
	{
		return;
	}

	const int32 Verts = Data.VertsPerSide;
	const int32 Res = Verts - 1;
	const float CellX = Config.MapSize.X / Res;
	const float CellY = Config.MapSize.Y / Res;
	const FVector2D Origin(-0.5f * Config.MapSize.X, -0.5f * Config.MapSize.Y);

	// Per-corner data helpers for the height-field grid.
	auto CornerPos = [&](int32 X, int32 Y) -> FVector
	{
		return FVector(Origin.X + X * CellX, Origin.Y + Y * CellY, Data.Heights[Data.Index(X, Y)]);
	};
	auto CornerUV = [&](int32 X, int32 Y) -> FVector2D
	{
		return FVector2D(static_cast<float>(X) / Res, static_cast<float>(Y) / Res);
	};
	auto CornerColor = [&](int32 X, int32 Y) -> FLinearColor
	{
		// Vertex colour encodes the plateau level in R and flags border/water cells in B,
		// which the built-in material reads to shade plateaus, ramps, cliffs and the border.
		const int32 Level = Data.LevelIndices[Data.Index(X, Y)];
		const float LevelT = (Config.NumberOfLevels > 1 && Level >= 0)
			? static_cast<float>(Level) / (Config.NumberOfLevels - 1) : 0.0f;
		return FLinearColor(LevelT, 0.0f, Level == INDEX_NONE ? 1.0f : 0.0f, 1.0f);
	};

	// Pass 1: accumulate a smoothed normal at every grid vertex (average of the surrounding
	// face normals). CliffSoftness blends the crisp face normals towards these to round off
	// the cliff/ramp edges; on flat plateaus both are "up", so the plateaus stay flat.
	// Blend normals only partway towards smooth even at max softness: fully smoothed normals on
	// the lightly-blurred plateaus read as blotchy shading, so cap the normal contribution while
	// the geometry (median + box blur) does the heavy lifting for the soft look.
	const float Softness = FMath::Clamp(Config.CliffSoftness, 0.0f, 1.0f) * 0.55f;
	TArray<FVector> SmoothN;
	SmoothN.Init(FVector::ZeroVector, Verts * Verts);
	for (int32 Y = 0; Y < Res; ++Y)
	{
		for (int32 X = 0; X < Res; ++X)
		{
			const FVector P00 = CornerPos(X, Y),     P10 = CornerPos(X + 1, Y);
			const FVector P01 = CornerPos(X, Y + 1),  P11 = CornerPos(X + 1, Y + 1);
			const FVector N1 = FVector::CrossProduct(P10 - P00, P01 - P00).GetSafeNormal(); // tri (P00,P01,P10)
			const FVector N2 = FVector::CrossProduct(P11 - P10, P01 - P10).GetSafeNormal(); // tri (P10,P01,P11)
			SmoothN[Data.Index(X, Y)]         += N1;
			SmoothN[Data.Index(X, Y + 1)]     += N1 + N2;
			SmoothN[Data.Index(X + 1, Y)]     += N1 + N2;
			SmoothN[Data.Index(X + 1, Y + 1)] += N2;
		}
	}
	for (FVector& Nrm : SmoothN) { Nrm = Nrm.GetSafeNormal(); if (Nrm.IsNearlyZero()) { Nrm = FVector::UpVector; } }
	auto CornerSmoothN = [&](int32 X, int32 Y) -> FVector { return SmoothN[Data.Index(X, Y)]; };

	// Pass 2: build a faceted mesh (every triangle its own three vertices) so plateaus read as
	// crisp flat tiers, cliffs as clear faces and ramps as distinct slopes -- with the per-vertex
	// normal softened towards the smoothed normal by CliffSoftness.
	const int32 TriCount = Res * Res * 2;
	TArray<FVector> Vertices;          Vertices.Reserve(TriCount * 3);
	TArray<int32> Triangles;           Triangles.Reserve(TriCount * 3);
	TArray<FVector> Normals;           Normals.Reserve(TriCount * 3);
	TArray<FVector2D> UVs;             UVs.Reserve(TriCount * 3);
	TArray<FLinearColor> VertexColors; VertexColors.Reserve(TriCount * 3);
	TArray<FProcMeshTangent> Tangents; Tangents.Reserve(TriCount * 3);

	auto AddFace = [&](const FVector& P0, const FVector& P1, const FVector& P2,
		const FVector2D& U0, const FVector2D& U1, const FVector2D& U2,
		const FLinearColor& C0, const FLinearColor& C1, const FLinearColor& C2,
		const FVector& S0, const FVector& S1, const FVector& S2)
	{
		// Face normal chosen so flat top cells face up, matching the winding below.
		const FVector FaceN = FVector::CrossProduct(P2 - P0, P1 - P0).GetSafeNormal();
		const FVector TangentX = (P1 - P0).GetSafeNormal();
		const int32 Base = Vertices.Num();
		Vertices.Add(P0); Vertices.Add(P1); Vertices.Add(P2);
		Normals.Add(FMath::Lerp(FaceN, S0, Softness).GetSafeNormal());
		Normals.Add(FMath::Lerp(FaceN, S1, Softness).GetSafeNormal());
		Normals.Add(FMath::Lerp(FaceN, S2, Softness).GetSafeNormal());
		UVs.Add(U0); UVs.Add(U1); UVs.Add(U2);
		VertexColors.Add(C0); VertexColors.Add(C1); VertexColors.Add(C2);
		Tangents.Add(FProcMeshTangent(TangentX, false));
		Tangents.Add(FProcMeshTangent(TangentX, false));
		Tangents.Add(FProcMeshTangent(TangentX, false));
		Triangles.Add(Base); Triangles.Add(Base + 1); Triangles.Add(Base + 2);
	};

	for (int32 Y = 0; Y < Res; ++Y)
	{
		for (int32 X = 0; X < Res; ++X)
		{
			const FVector P00 = CornerPos(X, Y),     P10 = CornerPos(X + 1, Y);
			const FVector P01 = CornerPos(X, Y + 1),  P11 = CornerPos(X + 1, Y + 1);
			const FVector2D U00 = CornerUV(X, Y),     U10 = CornerUV(X + 1, Y);
			const FVector2D U01 = CornerUV(X, Y + 1), U11 = CornerUV(X + 1, Y + 1);
			const FLinearColor C00 = CornerColor(X, Y),     C10 = CornerColor(X + 1, Y);
			const FLinearColor C01 = CornerColor(X, Y + 1), C11 = CornerColor(X + 1, Y + 1);
			const FVector S00 = CornerSmoothN(X, Y),     S10 = CornerSmoothN(X + 1, Y);
			const FVector S01 = CornerSmoothN(X, Y + 1), S11 = CornerSmoothN(X + 1, Y + 1);

			// Two triangles per quad (winding matched to the original so faces point up).
			AddFace(P00, P01, P10, U00, U01, U10, C00, C01, C10, S00, S01, S10);
			AddFace(P10, P01, P11, U10, U01, U11, C10, C01, C11, S10, S01, S11);
		}
	}

	LandscapeMesh->ClearAllMeshSections();
	LandscapeMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, /*bCreateCollision*/ true);
	LandscapeMesh->ContainsPhysicsTriMeshData(true);

	// Always set slot 0 — passing null resets it to the engine default — so the surface
	// never keeps a stale override after a previously configured material is cleared.
	LandscapeMesh->SetMaterial(0, ResolveLandscapeMaterial());
}

UMaterialInterface* ARTSLandscapeActor::ResolveLandscapeMaterial() const
{
	// Prefer the material the designer configured...
	if (Config.LandscapeMaterial)
	{
		return Config.LandscapeMaterial;
	}

	// ...otherwise fall back to the material shipped with the plugin so the generated
	// terrain is always shaded and never renders as an unlit black/grey surface.
	static const FSoftObjectPath DefaultMatPath(GDefaultLandscapeMaterialPath);
	UObject* Loaded = DefaultMatPath.ResolveObject();
	if (!Loaded)
	{
		// Quiet load: if the asset is absent we simply keep the engine default rather
		// than spamming the log while auto-generating during editor drags.
		Loaded = StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
			*DefaultMatPath.ToString(), nullptr, LOAD_NoWarn | LOAD_Quiet);
	}
	return Cast<UMaterialInterface>(Loaded);
}

void ARTSLandscapeActor::TriggerPCGGeneration()
{
	if (!Config.DefaultPCGGraph)
	{
		return;
	}

	if (!PCGComponent)
	{
		PCGComponent = NewObject<UPCGComponent>(this, TEXT("RTSLandscapePCG"));
		PCGComponent->RegisterComponent();
	}

	PCGComponent->SetGraph(Config.DefaultPCGGraph);
	PCGComponent->Generate(/*bForce*/ true);

	UE_LOG(LogRTSLandscape, Log, TEXT("Triggered PCG graph '%s' over generated landscape."),
		*Config.DefaultPCGGraph->GetName());
}

void ARTSLandscapeActor::RebuildNavigation()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
	{
		NavSys->Build();
		UE_LOG(LogRTSLandscape, Log, TEXT("Requested navigation rebuild after landscape generation."));
	}
}
