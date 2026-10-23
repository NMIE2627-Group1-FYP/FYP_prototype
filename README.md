# 聽日 Hear Today, Gone Tomorrow

Unreal Engine 5.8 installation. One person at a time becomes a mole.

## Artist statement

We are four students working across hardware, sound, real-time graphics and storytelling. Together, we have created an installation that invites one person at a time to become a mole.

We do not claim to know what a mole feels. Instead, we ask a larger question: if we slow down and listen, what else might we begin to notice?

## Play

The surface level is `Content/GameProject/Levels/Surface`. The burrow is `Content/GameProject/Levels/Underground`.

1. Install Unreal Engine 5.8.
2. Open `FYP_prototype.uproject`.
3. If the editor asks to rebuild the `FYP_prototype` module, allow it.
4. Press Play on Surface, choose Start, and follow the arrows to the manhole.
5. In the burrow, walk to continue the mole's lines. The opening is edited on `BP_Mole`, under Dialogue, Lines.

On the surface the walk speed is 500. Underground it is 180, with the camera close to the floor.

## What you can edit

- `Content/GameProject/Blueprints/BP_Mole` — dialogue lines, how far a step must be before the next line, and the mole meshes.
- `Content/GameProject/Blueprints/BP_GuideArrow` — the arrow that turns toward the manhole.
- `Content/GameProject/Levels/Surface` and `Underground` — lights, placed actors, and tiles. Each ground tile holds the plants as instanced groups, not as separate actors.
- `Content/GameProject/Mesh` — plant, insect, mole, and manhole meshes.
- Materials, effects, and input actions remain in `Content/L_test`.

## MCP quick access

1. Open the project and wait until Surface is on screen.
2. In Cursor, open Settings, then MCP, and turn on `unreal-mcp`.
3. The editor serves `http://127.0.0.1:8000/mcp`. Start a new agent chat in this folder while the editor stays open.

## Share the project on GitHub

`.gitignore` leaves out generated folders and local tool folders: `.cursor/`, `.gemini/`, `.codex/`, and `.mcp.json`.

1. From this folder, add the GitHub remote if it is not set.
2. Teammates clone the repo, open the `.uproject`, and let the editor compile the game module.

`Content` holds large `.uasset` files. If GitHub rejects a file over 100 MB, use Git LFS for `*.uasset` and `*.umap` before pushing again.
