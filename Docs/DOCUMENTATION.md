# RTS & Topdown Landscape Architect — Documentation

Unreal Engine 5.8 · Full C++ source · Engine-only (no third-party libraries)

---

## 1. Installation

1. Copy the `RTSLandscapeArchitect` folder into your project's `Plugins/` directory.
2. Regenerate project files and build (the plugin ships full C++ source).
3. Enable **RTS & Topdown Landscape Architect** in *Edit → Plugins* if it is not
   already enabled, then restart the editor.

The plugin depends only on engine plugins that ship with Unreal Engine:
`ProceduralMeshComponent`, `GeometryProcessing`, `PCG`, and the engine
`NavigationSystem` module. These are enabled automatically.

---

## 2. Quick start (editor)

1. Drag an **RTS Landscape Actor** into your level — **the terrain appears
   immediately**. There is nothing else you have to do.
2. Change any value in the **RTS Landscape** section of the Details panel
   (number of levels, map size, border type, seed, …) and the terrain **rebuilds
   live** as you edit. This is the whole workflow.
3. (Optional) Click **Generate Editor Landscape** to rebuild the geometry *and*
   run PCG decoration + a NavMesh rebuild (the full pipeline). **Randomize** rolls a
   new random seed for a fresh layout; **Clear** removes the generated geometry.
   Uncheck **Auto Generate In Editor** if you have finished tweaking and want to
   freeze the current terrain.

The surface is shaded by the plugin's **built-in landscape material** by default —
plateaus, ramps, cliffs and the map border are coloured automatically from the mesh
vertex colours, so the terrain is **never unlit/black**. Assign your own material to
the **Landscape Material** slot to override it.

## 3. Quick start (runtime)

Leave **Generate On Begin Play** enabled (default) and press Play — the actor
builds its terrain procedurally, cooks collision, runs the optional PCG graph and
rebuilds the NavMesh. You can also call `GenerateLandscape()` from Blueprint or
C++ at any time, and bind to the `OnLandscapeGenerationComplete` event.

### Included example maps

The plugin ships two ready-to-open demo maps under
`Content/RTSLandscapeArchitect/Maps/`:

- **`L_RTSLandscape_Mountains`** — a four-level plateau map closed off by rising
  mountains (seed 1337).
- **`L_RTSLandscape_Water`** — a three-level map whose border falls away into water
  (seed 2024).

Each map contains a configured `ARTSLandscapeActor` plus a daylight
directional sun, sky/atmosphere/fog, an exposure-locked `PostProcessVolume`, a
`PlayerStart` and a `NavMeshBoundsVolume`. The terrain rebuilds itself automatically
when the map opens, so just open one and look — or press *Play*, or edit the
landscape actor's settings to reshape it live.

---

## 4. Configuration — `FRTSLandscapeConfig`

| Property | Meaning |
| --- | --- |
| `NumberOfLevels` | Number of vertical plateau levels (2–5). |
| `PlateauCount` | How many plateau regions to scatter (4–128, default 24). **The main dial for how varied/busy the map is** — more regions = more, smaller plateaus. |
| `LevelHeightDifference` | Cliff height between two adjacent levels (Unreal units). |
| `MapSize` | Total ground-plane size (X/Y, Unreal units). |
| `BorderType` | `Mountains` (impassable ridge) or `Water` (falls into an abyss). |
| `bGenerateIslands` | Sink the lowest plateaus below the water line so the map breaks into islands/lakes with sloped shores (best with the Water border). |
| `Symmetry` | Enforce symmetry for balanced competitive maps: `Mirror2`/`Mirror4` (reflect) or `Rotate2`/`Rotate4` (rotate — same handedness for every spawn, ideal for fair 1v1). |
| `RampWidth` | Width of each ramp/chokepoint opening (Unreal units). |
| `RampDensity` | How many one-step boundaries become ramps (0–1). Low = only 1–3 tight chokepoints per plateau (StarCraft-style); high = most boundaries are ramps. |
| `CliffSoftness` | 0 = crisp faceted cliffs; higher rounds off the cliff/ramp edges. Plateaus stay flat either way. |
| `GridResolution` | Quads per map edge (16–512). Higher = smoother, more triangles. |
| `TerrainVariation` | How quickly plateau heights change across the map (0.5–8). Higher = more level changes → more ramps and variety. |
| `LevelSpread` | How strongly levels separate into distinct low/mid/high ground (1–3). |
| `BorderThickness` | Thickness of the bounded map border, as a fraction of the map. |
| `Seed` | Deterministic seed — same seed + config gives the same map. |
| `LandscapeMaterial` | Material applied to the generated surface. **Leave empty to use the plugin's built-in material** (vertex-colour level/border + slope shading) so the terrain is never unlit/black. |
| `DefaultPCGGraph` | PCG graph run over the plateaus after generation. |
| `bRebuildNavMeshAtRuntime` | Rebuild the NavMesh automatically when done. |

Two flags on the actor itself (outside `FRTSLandscapeConfig`):

| Property | Meaning |
| --- | --- |
| `bAutoGenerateInEditor` | Rebuild the geometry in the editor whenever the actor is placed or a setting changes (default **on**). This is what makes the actor "just work". Turn it off to freeze a terrain you no longer want rebuilt. |
| `bGenerateOnBeginPlay` | Build the terrain procedurally at `BeginPlay` (default **on**) for runtime-generated matches. |

---

## 5. Core classes

### `URTSHeightmapGenerator`
Math helper (UObject) that turns a config into an `FRTSHeightmapData` height field.
- `GenerateHeightmap(Config, OutData)` — instance method.
- `BuildHeightmap(Config, OutData)` — static Blueprint-callable convenience.

The generator scatters deterministic Voronoi region seeds, assigns each a plateau
level, blends one-step level boundaries into walkable ramps (keeping larger drops
as cliffs), and closes the map edge with Perlin-noise mountains or falling water.

### `ARTSLandscapeActor`
The placeable actor.
- `GenerateLandscape()` — full pipeline: heightmap → ProceduralMesh → collision →
  material → PCG → NavMesh → `OnLandscapeGenerationComplete`.
- `ClearLandscape()` — remove generated geometry.
- `TriggerPCGGeneration()` — run `DefaultPCGGraph` over the mesh.
- `RebuildNavigation()` — rebuild the navigation mesh.
- `GetLandscapeMesh()` — the underlying `UProceduralMeshComponent`.

### Editor module — `FRTSLandscapeArchitectEditorModule`
Registers a Details-panel customization (`FRTSLandscapeActorDetails`) that adds the
**Generate Editor Landscape** / **Clear** buttons to `ARTSLandscapeActor`.

---

## 6. Testing checklist

1. **Editor:** place the actor, click *Generate* → a terrain with the configured
   number of flat plateaus, ramps and border appears.
2. **Runtime:** press Play → the map is generated procedurally.
3. **Collision & navigation:** a character can walk up the ramps; the NavMesh
   covers plateaus and ramps.
4. **PCG:** the assigned PCG graph runs automatically and decorates the flat
   plateaus.

---

## 7. Troubleshooting

**The terrain doesn't appear when I place the actor.** Make sure **Auto Generate In
Editor** is checked on the actor. If it is off, place the actor and click **Generate
Editor Landscape** once.

**The terrain (or my whole level) looks black.** This is a lighting/exposure problem
in *your level*, not the terrain. A level with no working sun renders black and the
generated mesh has nothing to catch light. Add a **Directional Light** angled downward
(a daytime sun), a **Sky Light** (set to *Real Time Capture*), a **Sky Atmosphere**,
and — recommended — a **Post Process Volume** with a fixed exposure so auto-exposure
can't clip to black. The included demo maps are already set up this way; copy their
lighting if in doubt.

**The terrain is grey/untextured.** The built-in material could not be found. Assign
any material to the **Landscape Material** slot, or reinstall the plugin's `Content`
so `/RTSLandscapeArchitect/RTSLandscapeArchitect/Materials/M_RTSLandscape` is present.

**Editing a value doesn't update the terrain.** Auto-update only runs in the editor
(not during Play) and only when **Auto Generate In Editor** is on.

---

## 8. Support

Email: simulatedflow@gmail.com

*© 2026 Simulated Flow. All rights reserved.*
