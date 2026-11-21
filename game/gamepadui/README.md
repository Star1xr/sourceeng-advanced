# GamepadUI

Port of [Joshua-Ashton/HL2-GamepadUI](https://github.com/Joshua-Ashton/HL2-GamepadUI)
(upstream commit `866232f`, MIT, see `LICENSE`) to this tree and its waf build.

Supported games: **HL2**, **Episode 1 / Episode 2** (`episodic`) and **Portal**.

## What was changed from upstream

- `.vpc` projects replaced by `wscript` (builds the `gamepadui` module, selected by `--build-games`).
- `game/client/cdll_client_int.cpp`: GamepadUI hooks merged in (`#if defined( GAMEPADUI )`),
  next to the existing touch code. `GAMEPADUI` is defined for the hl2/episodic/portal client in `game/client/wscript`.
- `IsSteamDeck()` is not redefined in the client: it already lives in `tier1/KeyValues.cpp`.
- `PostMessageToAllSiblingsOfType` is not re-enabled in `Panel.h`. The single call in
  `gamepadui_button.cpp` is done with a local loop instead, so the shared header is untouched.
- Steam Input code stays behind `HL2_RETAIL` (not defined here), same as upstream's SDK 2013 setup.

## Using it

Start the game with `-gamepadui` (`-nogamepadui` forces it off). Without the flag nothing is loaded.

The module needs the upstream resources next to the game. Copy from the upstream repo's `game/` folder:

| Game | Copy from upstream | Into the game folder |
|---|---|---|
| HL2 | `mod_hl2/{gamepadui,materials,resource}` | `hl2/` |
| EP1, EP2 | `mod_hl2/{gamepadui,materials,resource}` (upstream only ships `mod_hl2` and `mod_portal`) | `episodic/`, `ep2/` |
| Portal | `mod_portal/{gamepadui,materials,resource}` | `portal/` |

Do not copy the prebuilt `bin/*.dll` files, they are Windows binaries.
