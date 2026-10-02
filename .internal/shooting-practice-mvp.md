# Shooting Practice MVP - Implementation Handoff

Status: configuration and the practice loop are implemented. Client validation is still required,
particularly the provisional object's visibility, pivot/collision and actual shot events.

- Updated: 2026-10-02.
- Fork: <https://github.com/SouthViking/samp-codigo-abierto>.
- Branch: `feat/shooting-practice-mvp`.
- Upstream: <https://github.com/San-Andreas-Roleplay-ES/samp-codigo-abierto>.
- Original upstream base: `e7f84d1294a523fe2aa7c0e5f1c6ae1e8d87f907`.
- This is a development handoff in a **public fork**; "internal" does not mean private.
- [Player/admin testing guide](shooting-practice-testing.md).
- [Local Windows server](local-windows-server.md).

## Product contract

A player reaches a configured station, starts a practice, shoots a sequence of temporary targets,
receives measured results and leaves the station available for another attempt. The module can later
serve LSSD, LSPD or a civilian/private range without depending on faction, business or ODP systems.
Results may support an instructor's FTP assessment; they do not certify an entire FTP.

The current conversation supersedes the earlier proposal of three positions with ten repeated
presentations and percentage-based pass thresholds:

- One configured station and one participant at a time.
- **1-10 optional target positions, each presented once**, in ascending slot order.
- One active target at a time. One valid impact anywhere on it gives one point and retires it.
- One configurable weapon, ID 22-34 (bullet-firing weapons). Default: 24, Desert Eagle.
- Passing means hitting at least the configured **number of targets**. Accuracy is reported,
  but is not an additional pass criterion. Default: 1 point, adjustable to the target count.
- Configurable preparation (1-30 s), exposure (1-60 s) and interval (0-30 s).
  Defaults: 3 s, 5 s and 1 s respectively.
- Start radius 1.5 m; permitted movement radius 3 m.
- Target positions must be in the station's world/interior, between 3 and 100 m from its start.
  Use short distances compatible with the chosen weapon during initial testing.
- Model 1271 (temporary gunbox), captured player Z plus a configurable code-level offset of 0.
  A custom Blender asset is a later refinement, not a dependency.
- Equipment belongs to the player. The practice never grants, removes, saves, restores or replenishes it.
- Messages/results use Spanish chat text; implementation identifiers, branch and commits use English.

## Two command groups

`/practicaconfig` provides `inicio`, `blanco <1-10>`, `eliminar <1-10>`, `arma <22-34>`,
`aprobar <points>`, `preparacion <seconds>`, `tiempo <seconds>`, `intervalo <seconds>`, `ver`, and `guardar`.
It requires RCON administrator access because the layout is shared. Every successful edit saves the
configuration; `guardar` retries a failed save, and the former `exportar` is now an alias for saving.

`/practica` starts the exercise; `/practica cancelar` cancels the caller's attempt.
The initial development access policy allows any non-NPC player. `PracticeCanUseStation` is the
integration point for a later permission policy.

Configuration changes are blocked during an attempt. Changing the start's world/interior invalidates
its target positions. Deleting targets or moving the start can make the layout incomplete; `ver` and
start validation explain what must be corrected. Partial layouts can be saved while being configured.

## Storage

`storage.inc` saves `scriptfiles/shooting_practice.cfg`, relative to the running server directory.
For the Windows launcher this is `.local-server/scriptfiles/shooting_practice.cfg`.
No new plugin, database or production account schema is required. The file includes a version header,
rules, the start and all ten optional slots, and a completion marker.

Loading parses temporary records and validates them before applying anything. Unsupported versions,
invalid rules/dimensions or truncated files are rejected and logged. Missing files leave an unconfigured
station with default rules. Write failures are reported to the administrator; memory remains usable and
`guardar` can retry. Writes are not transactional: retain a backup of a valuable layout before manual
file edits, and do not treat this development file as a production database.

The earlier `shooting_practice_points.inc` exports are not automatically migrated. Re-mark the points
with the current commands. Runtime configuration, credentials, AMX files, logs and binaries stay ignored.

## Runtime and scoring

`IDLE -> COUNTDOWN -> TARGET -> INTERVAL -> TARGET -> ... -> IDLE`

- Start validates the layout, access, alive/on-foot state, world/interior, proximity, held weapon,
  and station occupancy before reserving it.
- Countdown shots are excluded. Scoring begins when the first target becomes active.
- Each target is a dynamic object restricted to the station's world and interior. Other players can
  observe it; only the participant can score or advance the session.
- Creation/streaming failures mark the attempt incomplete and release resources.
- Shots and hits are processed once in `OnPlayerWeaponShot`. A player-object hit ID is mapped through
  `Streamer_GetItemStreamerID` and must equal the current session's object. The separate dynamic-object
  callback is not used for counting, avoiding duplicate counts and callback-order dependencies.
- Shots during intervals count. A late hit first expires the old target and cannot score a new one.
  A hit at/after the last target's deadline is outside the finished scoring window.
- Exposure expiry and hits share a single resolution path, which retires the phase before removing
  the object. One target cannot be both hit and omitted.
- Precision is `100 * hits / shots`, or 0 when no shots were observed. Non-scoring shots are `shots-hits`.
  Completed attempts obey `hits + omitted = configured targets` and `hits <= shots`.
- Pass/fail uses integer hit count, so presentation rounding has no effect on approval.
- Interruptions are incomplete, never a normal failed exam; unpresented targets are not counted as omitted.

Example: 10 configured positions, 12 shots, 8 hits, 2 expired targets gives 8/10 points and 66.7% precision.
It passes if the configured minimum is at most 8 points.

The shared `Task500` tick checks deadlines and participant validity. No per-player delayed timers can
survive a disconnect or act on a reused player ID. `GetTickCount` differences handle signed wraparound.
Deadlines are nominal: transitions happen on the next shared tick; interval 0 also waits for that tick.
Shot callbacks enforce exposure deadlines between ticks without spawning replacement objects.

Cancellation, death, disconnection, entering a vehicle, changing dimension, leaving the radius, switching
weapons, or losing the active object release all owned objects and the station. Running out of ammunition
continues the attempt, including SA-MP switching to fists on exhaustion. The module does not teleport,
freeze, modify health, change weapons, or manage unrelated objects.

## Files and extension boundaries

| File under `gamemodes/features/shooting_practice/` | Responsibility |
| --- | --- |
| `header.inc` | Module includes; `SARP.pwn` retains one feature include. |
| `config.inc` | Bounded station/target data, exercise rules and validation. |
| `storage.inc` | File serialization/loading and save feedback. |
| `setup.inc` | Administrator commands and coordinate capture. |
| `session.inc` | Player command, session lifecycle, target ownership and scoring. |
| `hooks.inc` | Existing YSI hooks, shared timer, shots and cleanup routes. |

Keep station data separate from rules and session state when adding more installations. Permission checks
belong in the access function; persistence or FTP consumers should use the finished result rather than
coupling their records to object callbacks. Change the provisional model and offset in configuration,
then validate its collision/orientation before changing the exercise rules.

## Out of scope

- Multiple managed facilities, a map editor, advanced configuration UI or simultaneous sessions.
- Player histories, SQL, leaderboards, external services or business/economy integration.
- Faction membership, ranks, instructor invitations, FTP enrollment/certification or promotions.
- Weapon/ammunition administration.
- Custom assets, full range mapping, a graphical HUD, moving targets or simultaneous targets.
- Protected/no-shoot targets, scoring rings/body regions, ballistics, random/adaptive rounds or competitions.
- Teleportation/instancing, a new anti-cheat system or unrelated feature changes.

## Validation

`tests/shooting_practice.pwn` exercises the real module with simulated player/clock/streamer adapters.
Its command parser and file I/O use server natives and the existing sscanf plugin. It covers score and
shot accounting, sparse/ten-slot sequences, late hits, interruptions, reuse, configuration permissions,
bounds, save/load and invalid files. The 2026-10-02 run passed 97 assertions on an isolated open.mp
compatibility runtime; player/object adapters were simulated. See `tests/README.md` for execution
instructions and the runtime shutdown limitation observed under container emulation.

Compile the entire gamemode with Pawn Community Compiler 3.10.10 and the checked-in includes. The repository
uses Windows-1252 for existing sources; the new module uses ASCII. On Unix, existing Windows backslashes in
include directives need normalization in a temporary validation copy, without modifying upstream files.

Automated validation does not establish client rendering, collision, network shot reporting or multiplayer
visual behavior. Follow the manual guide with a compatible SA-MP 0.3.DL-R1 client, preferably with two players.
`lagcompmode 1` is explicit in `server.cfg`; shot callbacks depend on lag compensation.

Primary API references:

- <https://open.mp/docs/scripting/callbacks/OnPlayerWeaponShot>
- <https://github.com/samp-incognito/samp-streamer-plugin/wiki/Natives-(Miscellaneous)>
- <https://github.com/samp-incognito/samp-streamer-plugin/blob/v2.9.6/src/callbacks.cpp>
