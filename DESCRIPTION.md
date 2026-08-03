# RTS & Topdown Landscape Architect — Plateaus, Ramps & Cliffs

Build tactics-ready terrain in one click - plateaus, ramps and cliffs, not noise.

Most runtime landscape tools generate endless wavy noise terrain - the wrong shape for an RTS, MOBA, city-builder or turn-based tactics game, which needs clearly defined flat plateaus, clean connecting ramps, deterministic cliffs and a bounded map edge. RTS & Topdown Landscape Architect generates exactly that - in the editor as an editable starting point, or fully procedurally at runtime - then decorates it with PCG and rebuilds the NavMesh for you.

KEY FEATURES

- Structured plateaus, not noise - a deterministic Voronoi partition builds 2-5 flat, walkable levels; same seed + config always produce the same map, in editor and at runtime.
- Walkable ramps, real cliffs - one-step level boundaries blend into ramps of your chosen width; larger drops stay impassable cliffs.
- Bounded play field - close the map edge with rising Perlin mountains or terrain falling into water / an abyss.
- Editor and runtime, zero setup - drag the actor in and the terrain appears instantly, rebuilding live as you edit (no button to hunt for); it also builds itself at BeginPlay, and ships with a built-in material so it is never unlit/black.
- Hands-free PCG decoration - assign a PCG graph and it runs over the finished plateaus (trees, rocks, resources, spawn points land on the flats, not the cliffs).
- Automatic navigation - the NavMesh is rebuilt over the generated area so units path across plateaus and ramps immediately.
