# FYP prototype

Unreal Engine 5.8 project. The surface level is `Content/GameProject/Levels/Surface`. The burrow is `Content/GameProject/Levels/Underground`.

## MCP quick access

1. Open `FYP_prototype.uproject` and wait until Surface is on screen.
2. In Cursor, open Settings, then MCP, and turn on `unreal-mcp`.
3. The editor serves `http://127.0.0.1:8000/mcp`. Start a new agent chat in this folder while the editor stays open.

## Open the project

1. Install Unreal Engine 5.8.
2. Open `FYP_prototype.uproject`.
3. If the editor asks to rebuild the `FYP_prototype` module, allow it.

Play starts on the game mode set in `Config/DefaultEngine.ini`. Press Play on Surface, choose Start, and follow the arrows to the manhole.

## Start the Unreal MCP connection

The editor hosts the tool connection at `http://127.0.0.1:8000/mcp`. Cursor is already pointed at it in `.cursor/mcp.json`.

1. Open this project in the Unreal Editor and leave it open. The MCP server starts with the editor.
2. Open the same folder in Cursor.
3. In Cursor, open Settings, then MCP. The server named `unreal-mcp` should be listed. If it is off, turn it on.
4. Start a new agent chat. The editor has to be running before the agent can change levels or assets.

If the connection fails, close and reopen the Unreal Editor, then toggle `unreal-mcp` off and on in Cursor. Do not change the URL unless the plugin is using a different port.

## Share the project on GitHub

The `.gitignore` keeps generated folders out of the repository: `Binaries`, `DerivedDataCache`, `Intermediate`, `Saved`, and local build files. Source, `Content`, and `Config` are what collaborators need.

1. Create an empty GitHub repository.
2. From this folder:

```
git remote add origin https://github.com/<your-account>/<repo>.git
git push -u origin master
```

3. Teammates clone the repo, open the `.uproject`, and let the editor compile the game module.

`Content` holds large `.uasset` files. If GitHub rejects a push because a file is over 100 MB, use Git LFS for `*.uasset` and `*.umap` before pushing again.

Do not commit `Saved/`, `Intermediate/`, or `DerivedDataCache/`. Those are rebuilt on each machine.
