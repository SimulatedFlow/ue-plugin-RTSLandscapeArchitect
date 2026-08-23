# RTS & Topdown Landscape Architect

> Build tactics-ready terrain in one step — flat plateaus, walkable ramps, real
> cliffs and a bounded map edge (mountains or water) for RTS, MOBA, topdown and
> turn-based tactics games. Editable in the editor, or fully procedural at runtime.

Unreal Engine **5.8** · full C++ source · engine-only (no third-party libraries).

---

## 60-second quick start

1. Enable the plugin (*Edit → Plugins → RTS & Topdown Landscape Architect*) and, if
   prompted, let the editor build it.
2. In the **Place Actors** panel search for **RTS Landscape Actor** and drag it into
   your level.
3. **That's it — the terrain appears immediately.** Change any value in the
   *RTS Landscape* section of the Details panel (levels, map size, border type,
   ramp width, seed …) and the terrain **rebuilds live** as you edit.

There is no button to hunt for and nothing to read first. The surface is shaded by a
**built-in material** by default, so the terrain is *never* unlit or black.

> **Tip:** open one of the included demo maps —
> `L_RTSLandscape_Mountains` or `L_RTSLandscape_Water` (under the plugin's
> `Content/…/Maps/`) — to see a finished, fully-lit example you can copy.

---

## How it works

A deterministic **Voronoi partition** carves the map into 2–5 flat, walkable
**plateau levels**. Where two neighbouring regions differ by exactly one level, a
clean **ramp** of the width you choose is blended in so units can walk up and down;
bigger drops stay **impassable cliffs** — the core rule tactics maps depend on. The
outer edge is closed off as rising **mountains** (Perlin-noise ridge) or terrain that
falls away into **water / an abyss**, giving a clearly **bounded battlefield**.

Because it is seeded, the *same seed + settings always produce the same map* — in the
editor and at runtime, on every machine — so clients can generate identical terrain
without replication.

The terrain is a `UProceduralMeshComponent` with cooked collision. Its vertex colours
encode the plateau **level** (red) and **border** flag (blue), which the built-in
material uses to colour plateaus, ramps, cliffs and the border, blended by slope.

---

## Editor vs. runtime

|  | What happens |
| --- | --- |
| **Placing / editing in the editor** | `Auto Generate In Editor` (on by default) rebuilds the geometry the instant you place the actor or change a setting. |
| **Generate Editor Landscape button** | Rebuilds the geometry *and* runs the PCG graph + a NavMesh rebuild (the full pipeline). Use it when you want decoration/navigation refreshed too. |
| **Randomize button** | Rolls a new random seed and rebuilds — the fastest way to try fresh map layouts. |
| **Clear button** | Removes the generated geometry. |
| **At runtime (`BeginPlay`)** | `Generate On Begin Play` (on by default) builds the terrain, cooks collision, runs the optional PCG graph, rebuilds the NavMesh and fires `On Landscape Generation Complete`. |

Turn **Auto Generate In Editor** off once you are happy with a map so it is never
rebuilt behind your back.

---

## Settings (`RTS Landscape`)

| Property | Meaning |
| --- | --- |
| **Number Of Levels** | Vertical plateau levels (2–5). |
| **Plateau Count** | How many plateau regions to scatter (4–128, default 24). **The main dial for how varied the map is** — more = smaller, more numerous plateaus. |
| **Level Height Difference** | Cliff height between adjacent levels (Unreal units). |
| **Map Size** | Total ground-plane size (X/Y, Unreal units). |
| **Border Type** | `Mountains` (impassable ridge) or `Water` (falls into an abyss). |
| **Generate Islands** | Sink the lowest plateaus below the water line so the map breaks into islands/lakes with sloped shores (best with the Water border). |
| **Symmetry** | Balance competitive maps: Mirror (2/4-way) or Rotational (2/4-way — same handedness for every spawn, ideal for fair 1v1). |
| **Ramp Width** | Width of each ramp / chokepoint opening. |
| **Ramp Density** | How many one-step boundaries become ramps (0–1). Low = a few tight StarCraft-style chokepoints; high = most boundaries are ramps. |
| **Cliff Softness** | 0 = crisp faceted cliffs; higher rounds off the cliff/ramp edges (plateaus stay flat). |
| **Grid Resolution** | Quads per map edge (16–512). Higher = smoother, more triangles. |
| **Terrain Variation** | How quickly plateau heights change across the map. Higher = more ramps and variety. |
| **Level Spread** | How strongly levels separate into distinct low/mid/high ground. |
| **Border Thickness** | Thickness of the bounded map border, as a fraction of the map. |
| **Seed** | Deterministic seed — same seed + settings gives the same map. |
| **Landscape Material** | Surface material. **Leave empty to use the built-in material** so the terrain is never unlit/black. |
| **Default PCG Graph** | PCG graph run over the plateaus after generation. |
| **Rebuild Nav Mesh At Runtime** | Rebuild the NavMesh automatically after generation. |
| **Auto Generate In Editor** | Rebuild geometry live when placed / edited (default on). |
| **Generate On Begin Play** | Build procedurally at `BeginPlay` (default on). |

---

## PCG decoration

Assign a **PCG graph** to *Default PCG Graph*. When the full pipeline runs (the
*Generate* button or `BeginPlay`) the graph is executed over the finished mesh, so
trees, rocks, resources and spawn points land on the flat plateaus, not on the
cliffs. Sample the mesh's vertex colour (red = level, blue = border) in your graph to
place things per level or keep them off the border.

## Navigation

With *Rebuild Nav Mesh At Runtime* on, the navigation mesh is rebuilt over the
generated area after generation, so AI and player units path across plateaus and
ramps immediately. Add a `NavMeshBoundsVolume` covering the map (the demo maps
include one).

---

## Blueprint / C++ API

**`ARTSLandscapeActor`**
- `GenerateLandscape()` — full pipeline: heightmap → mesh → collision → material → PCG → NavMesh → `OnLandscapeGenerationComplete`.
- `ClearLandscape()` — remove the generated geometry.
- `TriggerPCGGeneration()` / `RebuildNavigation()` — the individual pipeline steps.
- `GetLandscapeMesh()` — the underlying `UProceduralMeshComponent`.
- `OnLandscapeGenerationComplete` — multicast delegate fired when generation finishes.

**`URTSHeightmapGenerator`**
- `BuildHeightmap(Config, OutData)` — static, Blueprint-callable; produces the height field for a config without needing an actor.

---

## Troubleshooting

**Terrain doesn't appear when I place the actor.** Make sure *Auto Generate In
Editor* is checked; otherwise place it and click *Generate Editor Landscape* once.

**The terrain or the whole level looks black.** This is a lighting/exposure problem
in *your level*, not the terrain — a level with no working sun renders black. Add a
**Directional Light** angled downward (a daytime sun, above the horizon), a **Sky
Light** set to *Real Time Capture*, a **Sky Atmosphere**, and a **Post Process
Volume** with a fixed exposure so auto-exposure can't clip to black. The demo maps
are already set up this way — copy their lighting if in doubt.

**Terrain is grey/untextured.** The built-in material couldn't be found — assign any
material to *Landscape Material*, or make sure the plugin's `Content` (the
`M_RTSLandscape` material) is installed.

**Editing a value doesn't update it.** Live update runs only in the editor (not
during Play) and only while *Auto Generate In Editor* is on.

---

*RTS & Topdown Landscape Architect — © 2026 Silvan Teufel. Support:
teufelsilvan@gmail.com*
