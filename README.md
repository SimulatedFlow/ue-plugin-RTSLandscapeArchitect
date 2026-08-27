# RTS & Topdown Landscape Architect

Automatic, multi-level plateau landscape generation with clean ramps, cliffs and
bounded map borders (mountains or water) for RTS, topdown and turn-based tactics
games — in the editor or fully procedurally at runtime.

Supports **Unreal Engine 5.8**. Full C++ source included. Engine-only, no
third-party libraries.

## What it does

- **Structured plateaus, not noise.** A deterministic Voronoi partition builds 2–5
  flat, walkable levels — exactly what tactics maps need, instead of endless wavy
  terrain.
- **Walkable ramps, real cliffs.** Adjacent levels that differ by one step are
  joined by clean ramps; larger drops stay impassable cliffs.
- **Bounded maps.** Close the play field with rising mountains or falling water.
- **Editor & runtime, zero setup.** Drag the actor in and the terrain appears
  instantly; change any setting and it rebuilds live in the editor — no button to
  hunt for. It also generates itself at `BeginPlay` for runtime matches, and ships
  with a built-in material so it is **never unlit/black** out of the box.
- **PCG + NavMesh.** After the mesh is built the plugin runs your PCG graph over
  the plateaus and rebuilds the navigation mesh automatically.

Documentation, free and without an account: <https://wiki.teufel-engineering.com/en/RTSLandscapeArchitect/documentation> — installation, quick start and the full API.

The same manual ships with the plugin as `Docs/DOCUMENTATION.md`.
**Source-available** (see before you buy): https://github.com/SimulatedFlow/ue-plugin-RTSLandscapeArchitect

- **Support:** teufelsilvan@gmail.com
- **Version:** 1.0.0

*© 2026 Silvan Teufel. All rights reserved.*

<!-- SF-STORE-BLOCK:BEGIN -->
## 🛒 Source-available — see before you buy

This repository contains the **full source** of a commercial Unreal Engine plugin. It is **source-available, not open source**: read it, evaluate it, then buy a license to use it. See **the Fab Content License Agreement / Unreal Engine EULA (purchase required)**.

**Get it / Buy:**
- **Buy on Fab** (this plugin): https://www.fab.com/listings/aa5de54c-f705-41aa-8a96-939ff3267b4f
- Fab store — all our UE5 plugins: https://www.fab.com/sellers/Silvan%20Teufel

### 📬 **Free UE5 Snippet-Pack**

10 ready-to-use C++/Blueprint building blocks (subsystems, versioned saves, async nodes, editor tooling) — MIT licensed. Get it by joining the newsletter — plus a heads-up when something new ships. Double opt-in, unsubscribe in one click, no address sharing.

👉 **[Get the free pack](https://silvan.teufel-engineering.com/newsletter/plugins/?q=gh)**

_© 2026 Silvan Teufel. All rights reserved._
<!-- SF-STORE-BLOCK:END -->
