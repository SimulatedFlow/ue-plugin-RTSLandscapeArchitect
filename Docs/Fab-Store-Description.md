# RTS & Topdown Landscape Architect

## Build tactics-ready terrain in one click — plateaus, ramps and cliffs, not noise.

Most runtime landscape tools generate endless wavy noise terrain. That is the
wrong shape for an RTS, MOBA, city-builder or turn-based tactics game, which needs
clearly defined **flat plateaus**, clean **connecting ramps**, deterministic
**cliffs** and a **bounded map edge**. RTS & Topdown Landscape Architect generates
exactly that — in the editor as an editable starting point, or fully procedurally
at runtime — then decorates it with PCG and rebuilds the NavMesh for you.

## Why it is different

- **Structured plateaus, not noise.** A deterministic Voronoi partition builds
  2–5 flat, walkable levels. Deterministic seed: the same seed and config always
  produce the same map, in the editor and at runtime.
- **Walkable ramps, real cliffs.** Level boundaries that differ by one step are
  automatically blended into walkable ramps of the width you choose; larger drops
  stay impassable cliffs — the core logic tactics maps depend on.
- **Bounded play field.** Close the map edge with rising Perlin-noise **mountains**
  or with terrain that falls away into **water / an abyss**.
- **Editor and runtime, zero setup.** Drag the actor into your level and the terrain
  appears instantly; change any setting and it rebuilds live — no button to hunt for.
  A **Generate Editor Landscape** button additionally runs PCG + NavMesh, and the same
  actor builds itself at `BeginPlay` for runtime-generated matches. A built-in material
  ships with the plugin, so the terrain is **never unlit/black** out of the box.
- **PCG decoration, hands-free.** Assign a PCG graph and it runs over the finished
  plateaus automatically — trees, rocks, resources and spawn points land on the
  flats, not on the cliffs.
- **Automatic navigation.** The navigation mesh is rebuilt over the generated area
  so AI and player units path across plateaus and ramps immediately.

## Feature highlights

- Multi-level plateau generation (2–5 levels), configurable cliff height and map size
- Automatic ramp placement between adjacent levels; configurable ramp width
- Mountain or water border modes for a clearly bounded battlefield
- Vertex-colour level encoding for easy multi-layer / splat materials
- Cooked collision on the generated mesh — characters walk the ramps out of the box
- One-click editor generation plus runtime generation with a completion event
- Blueprint-exposed configuration, generator and events; full C++ source included

## Perfect for

RTS and topdown strategy, MOBA-style arenas, tower defense, city-builders,
turn-based tactics and any project that needs bounded, multi-level battle maps.

---

## TECHNICAL DETAILS

**Features:**
- Deterministic multi-level plateau landscape generator (`URTSHeightmapGenerator`)
- Placeable `ARTSLandscapeActor` with full generation pipeline and completion delegate
- Editor Details-panel button ("Generate Editor Landscape") for in-editor authoring
- Automatic PCG graph execution and NavMesh rebuild after generation

**Code Modules:**
- `RTSLandscapeArchitect` (Runtime)
- `RTSLandscapeArchitectEditor` (Editor)

**Number of C++ Classes:** 3 (`URTSHeightmapGenerator`, `ARTSLandscapeActor`, editor Details customization) plus config/data structs (`FRTSLandscapeConfig`, `FRTSHeightmapData`)

**Supported Development Platforms:** Win64, Mac, Linux

**Supported Target Build Platforms:** Win64, Mac, Linux

**Engine Version:** Unreal Engine 5.8

**Plugin Dependencies (engine plugins, ship with UE):** ProceduralMeshComponent, GeometryProcessing, PCG, NavigationSystem

**Third-party software:** None. Engine-only.

**Network Replicated:** No (deterministic seed allows clients to generate identical terrain locally)

**Documentation:** Included `Docs/DOCUMENTATION.md` (installation, quick start, full API, testing checklist)

**Example Maps:** Yes — two ready-to-open demo maps (`L_RTSLandscape_Mountains`, `L_RTSLandscape_Water`), each fully lit and set up so the terrain builds itself the moment you open the map.

**Important/Additional Notes:** Runtime terrain is built as an optimized `UProceduralMeshComponent` with cooked collision. Generation is deterministic per seed. Full C++ source is included and Blueprint-exposed.

**Support Email:** teufelsilvan@gmail.com
