# Shooting Practice MVP - Implementation Handoff

Status: temporary coordinate setup implemented; the practice engine, countdown, targets, HUD, and scoring have not been implemented or tested in-game.

- Prepared: 2026-10-01.
- Fork: <https://github.com/SouthViking/samp-codigo-abierto>.
- Working branch: `feat/shooting-practice-mvp`.
- Upstream: <https://github.com/San-Andreas-Roleplay-ES/samp-codigo-abierto>.
- Inspected upstream base: `e7f84d1294a523fe2aa7c0e5f1c6ae1e8d87f907` (`main`).
- Document purpose: internal development planning and agent handoff. This file is versioned in a **public fork**; "internal" does not mean private.

## 1. Objective

Build a small, functional shooting practice system that can eventually be used at LSSD, LSPD, and civilian/private shooting ranges. The engine must not depend on a particular faction, the police ODP terminal, a business system, or production character records.

The MVP validates one complete loop:

> A player reaches a configured shooting station, starts a practice with a command, shoots a configured sequence of targets, receives measured results, and leaves the station ready for another attempt.

For a future Field Training Program (FTP), these results can support an instructor's assessment of a new deputy. Completing or passing a shooting exercise is not automatic certification of the whole FTP.

## 2. Decisions and scope boundaries

### Confirmed product direction

- Separate **configuration/administration** from **execution of a practice**.
- Implement the execution with a small temporary coordinate setup command for testing. Exercise timings, model, and weapon can remain static data in Pawn for this MVP.
- Start from a predefined position using an in-game command.
- Present targets according to configuration and measure the player's performance.
- Assume the player already has the weapon and ammunition. Do not supply, remove, replenish, save, or restore equipment as part of the practice.
- Keep the module reusable across police and civilian facilities.
- Use English names for branches, commits, and implementation identifiers. Keep player-facing messages consistent with the existing Spanish gamemode.
- A custom target modeled in Blender is a possible later asset, not a prerequisite for validating the engine.

### Proposed initial defaults

The values below are suggested starting points, not official LSSD/LSPD standards or previously validated game balance. Keep them together in configuration so they can be tuned after testing.

| Setting | Suggested initial value |
| --- | --- |
| Configured stations in the first demo | 1 |
| Participants per station | 1 |
| Active targets per session | 1 |
| Available target positions | 3 fixed transforms |
| Target presentations per exercise | 10, in a fixed sequence |
| Countdown | 3 seconds |
| Maximum exposure per target | 3 seconds |
| Gap between targets | 1 second |
| Starting radius | 1.5 meters from the station |
| Allowed movement during the exercise | Up to 3 meters from the station |
| Weapon | One configurable, supported pistol for initial validation |
| Target model | An existing GTA/SA-MP model selected after a collision test |
| Optional pass thresholds | At least 80% accuracy and 90% targets completed |

Station coordinates, target transforms, the exact weapon, and the target model still need to be selected in-game. Use a clear test location that does not overlap existing sample objects or actors. The interval follows either a valid hit or expiry, so finishing faster by hitting targets is intentional.

## 3. What the MVP includes

- A temporary `/practicaconfig` command to capture a test station and three target positions from the character's position and facing angle; see the setup instructions below.
- A self-service command, proposed as `/practica`, to start at the nearest eligible configured station.
- `/practica cancelar` to end the participant's current attempt.
- Static station and exercise definitions with bounded capacities.
- A station occupancy check and at most one active session per player.
- Countdown, sequential target presentation, exposure timeout, and gap between targets.
- One-hit targets: any valid impact on the active target completes that presentation.
- Shot counting, valid-hit counting, expired-target counting, accuracy, and target completion.
- A compact final result in chat or an existing dialog mechanism.
- Optional exercise-level pass/fail derived from configured thresholds.
- An explicit incomplete/cancelled result for interrupted attempts.
- Cleanup of owned targets, timers, and occupancy on every exit path.
- A small access check that can later consult faction, instructor, or business permissions.

The first demonstration may allow any eligible player to use the station as an explicit development policy. Put that decision in the access integration function, not throughout the engine. A production deployment must replace it with the desired access policy.

### Temporary coordinate setup

This is an explicit extension requested after the initial handoff: configure the points in-game for testing, then formalize them later. The command is implemented under `gamemodes/features/shooting_practice/` and included by `SARP.pwn`. It captures coordinates only; `/practica` and the exercise engine are still pending.

Log in as an RCON administrator to edit this shared development layout. Stand on foot at each desired position and face the desired direction before marking it.

| Command | Effect |
| --- | --- |
| `/practicaconfig` or `/practicaconfig ayuda` | Show setup instructions. |
| `/practicaconfig inicio` | Capture or replace the shooter's starting position, facing angle, interior, and virtual world. |
| `/practicaconfig blanco 1` | Capture or replace target position 1. Repeat with `2` and `3` for the other positions. |
| `/practicaconfig ver` | List the current points, their transforms, and any missing points. |
| `/practicaconfig exportar` | Export a complete layout to `scriptfiles/shooting_practice_points.inc`, replacing an earlier export. |

- Points are shared server memory and last until a gamemode/server restart. Marking the same slot again replaces it.
- Targets must be in the start point's interior and virtual world. Moving the start within that same dimension keeps the target positions; moving it to a different dimension invalidates the old targets and requires marking them again.
- The export contains Pawn declarations that can replace `PracticeStartPoint` and `PracticeTargetPoints` in `config.inc` when formalizing the layout. It is not automatically loaded on restart. The `scriptfiles` directory must exist and be writable; it is ignored by Git.
- The captured Z and angle are the character's exact transform. No target model offset is assumed. Validate the eventual object's pivot, height, facing, and collision before using these transforms in an exercise.
- These are three reusable positions, not three presentations. The exercise may revisit them across its proposed ten presentations.
- Once the practice engine is added, prevent layout edits during an active attempt and consume these configuration records separately from session state.

Validation on 2026-10-01: both the untouched branch baseline and the full gamemode with the setup module compiled successfully using Pawn Community Compiler 3.10.10 with `-i../pawno/include`, `-;+`, `-(+`, and `-d3`, from the `gamemodes` directory. No compiler diagnostics were emitted with the repository's existing warning settings. The compiler, logs, and generated AMX files were kept outside the repository. The existing entry point's encoding was preserved, with only one feature include added.

In-game validation remains pending: mark and replace all four points; check out-of-range target slots, non-admin access, being in a vehicle, and mismatched dimensions; verify missing points block export; inspect the generated Pawn declarations; and confirm that a restart clears the temporary layout. This compilation check does not establish that the command, export, or future target placement has been exercised in a SA-MP server/client.

## 4. Outside the MVP

- Full facility administration, a map editor, creation/deletion of facilities, or a configuration UI. The temporary coordinate setup above is the only administration included for initial testing.
- SQL, schema migrations, permanent player records, saved attempts, leaderboards, or external services.
- Faction membership, rank management, FTP enrollment, promotions, certificates, or automatic FTP approval.
- Instructor invitations, supervised-session controls, manual evaluations, or instructor dashboards.
- Business ownership, admission payments, rewards, or economy integration.
- Weapon issuance, ammunition accounting, reload modifications, equipment restoration, or inventory integration.
- Custom Blender models, texture packs, downloads, or a full shooting-range map.
- Moving targets, simultaneous targets, protected/no-shoot targets, scoring rings, head/body scoring, or ballistic simulation.
- Randomized or adaptive exercises, difficulty progression, competitions, or tournaments.
- Teleporting players to private instances or taking over their normal player state.
- A new anti-cheat system or changes to unrelated existing features.
- Dependencies on the open ODP, camera, or rappel-fix PRs.

Protected/no-shoot targets and supervisor-led FTP assessments are useful future additions. Their earlier discussion should not be interpreted as a requirement to implement them in this first version.

## 5. Player flow and session lifecycle

1. The player enters the station's starting radius and uses `/practica`.
2. Validate connection/spawn state, being on foot, access, station world/interior, starting distance, the selected allowed weapon, free station capacity, and no existing player session.
3. Validate the exercise configuration and reserve the station before starting any countdown or target creation.
4. Display a countdown and explain that one hit completes a target. Countdown shots are outside the scored exercise.
5. Start scoring when the first target becomes active. Create it at the configured transform and begin its exposure deadline.
6. The participant's first valid impact completes and immediately retires that presentation. Otherwise, exposure expiry records one omitted target and retires it.
7. Wait the configured gap, then present the next target. Count participant shots during these gaps as well as during target exposure.
8. End scoring when the last presentation resolves. Produce results, release resources, and free the station.

Suggested phases:

`IDLE -> COUNTDOWN -> TARGET_ACTIVE -> INTERVAL -> TARGET_ACTIVE -> ... -> FINISHED -> IDLE`

An interruption can transition from an active phase to `CANCELLED`, followed by the same cleanup used by normal completion. Mark a target resolved before advancing, so an impact and its timeout cannot both resolve it.

### Interruption rules

- Explicit cancellation, death, disconnection, changing world/interior, entering a vehicle, or leaving the allowed station radius cancels the attempt.
- Switching to an unsupported weapon cancels the attempt with a clear message. For the first test, allow one configured pistol.
- Running out of ammunition does not grant a refill or automatically pause the exercise. Targets continue to expire normally.
- A target creation failure or invalid runtime state ends the attempt as incomplete and releases the station. Do not grade an internal failure as a normal failed exam.
- An instructor or bystander hitting a target must not score it, retire it, or advance another player's attempt.
- The player remains in their normal location and world. The module manages only its own session state, targets, timers, and result display.

## 6. Counting and scoring contract

Observe actual shot events and target impacts. Do not infer shots from ammunition differences: reloads, weapon switches, or other systems can change equipment state independently.

For a normally completed exercise, define:

- `S`: participant shot events recorded between the first target activation and the final target resolution, including interval shots.
- `H`: valid target hits. Each presentation contributes at most one, so `H <= S` and `H <= N`.
- `N`: configured number of target presentations.
- `E`: targets whose exposure expired without a valid hit. At normal completion, `H + E = N`.

| Metric | Definition |
| --- | --- |
| Shots | `S` |
| Valid hits | `H` |
| Non-scoring shots | `S - H` |
| Omitted targets | `E` |
| Accuracy | `100 * H / S`; display 0% if `S == 0` |
| Target completion | `100 * H / N`; reject configurations where `N == 0` |
| Reaction time, if shown | Server-observed time from activation to the first valid hit |

Example: 10 targets, 12 shots, 8 valid hits, and 2 expiries produce 66.7% accuracy and 80% target completion. A player who fires one successful shot and ignores the other nine targets has 100% accuracy but only 10% completion.

If assessment is enabled, passing requires **normal completion and both configured percentage thresholds**. Use the underlying counts for comparisons so rounded display percentages cannot turn a failing attempt into a pass. With the proposed thresholds, 18 hits out of 22 shots across 20 targets would pass: 81.8% accuracy and 90% completion.

An interrupted attempt is incomplete even if its partial statistics exceed the thresholds. Show partial counts as such; do not label unpresented targets as expired or award a pass. A completed attempt with no shots has zero accuracy/completion and fails an enabled assessment with positive thresholds.

Reaction time is optional diagnostic information in the MVP. If included, average only successfully hit targets, report omissions separately, and do not use it as an approval gate before latency and streaming behavior have been tested. It is a server-observed duration, not an exact measurement of when the client first displayed a target.

### Event ownership and duplicate prevention

- Count each shot once, even when the shot also triggers the dynamic-object impact callback.
- Verify the participant, active session, current target instance, phase, weapon, world, and interior before crediting a hit.
- Retired targets and delayed callbacks must not score again or resolve a later presentation.
- Use a session generation/token or equivalent guard for asynchronous work so an old timer cannot affect a new attempt or a reused player ID.
- Ignore unrelated players and objects. Preserve normal behavior of the gamemode's other event handlers.

## 7. Minimal architecture

Use the repository's Pawn/include conventions. The following boundaries are enough; do not introduce a generic framework or a plugin architecture to implement this MVP.

| Boundary | Responsibility |
| --- | --- |
| Static configuration | Station identities, transforms, world/interior, distances, exercise references, sequence, durations, model/weapon choices, and pass thresholds. |
| Practice engine | Session lifecycle, occupancy, target ownership, timers, scoring, cancellation, and cleanup. |
| Server integration | Commands, access policy, event wiring, and publishing the completed/cancelled result. |

A possible module location is `gamemodes/features/shooting_practice/`, with a small header, configuration include, and implementation include. File names and function names are suggestions; keep the resulting structure proportionate to the code.

### Data separation

- **Station definition:** stable station ID, start position, interior/world, start and movement radii, exercise ID, and allowed target transforms.
- **Exercise definition:** ordered presentation sequence, exposure/gap/countdown durations, allowed weapon, model choice, and optional grading rules. The sequence can reference the same target position multiple times.
- **Runtime session:** participant and station IDs, generation, phase, presentation index, owned target ID, timing state, counters, and completion/cancellation reason.

Keep immutable definitions separate from mutable attempts. Use bounded arrays and validate capacity/indexes rather than dynamically growing state. Although the first demo has one station, do not couple the whole implementation to one global participant or hardcode a faction into the engine.

The future administration layer should create/update these same definitions. Access policy should be replaceable through a small function such as `PracticeCanUseStation(playerid, stationid)`. A completion function/callback should expose the result for a future instructor view or persistence adapter without requiring either now.

The target model is configuration, not scoring logic. A later custom object still needs compatible registration, dimensions, orientation, and collision behavior; replacing an ID alone does not prove a new asset works correctly.

## 8. Repository integration facts

The public repository is a development/test gamemode, not the complete production server.

| Existing location | Relevant capability |
| --- | --- |
| `gamemodes/SARP.pwn` | Main entry point and feature includes; loads SA-MP, Streamer, Pawn.CMD, sscanf, easyDialog, YSI-related libraries, and other dependencies. |
| `gamemodes/features/breach_shotgun.inc` | Existing dynamic-object creation and object-shot handling. |
| `gamemodes/features/actors/core.inc` | Existing shot-related hooks that the new module must coexist with. |
| `gamemodes/features/admin/objects.inc` | Generic object creation/editing commands useful for inspecting candidate models and transforms. |
| `gamemodes/features/player/minigame-lumberjack/lumberjack.inc` | Example of per-player activity state and lifecycle handling; not a dependency of the practice module. |
| `gamemodes/features/player/chat.inc` | Existing proximity chat available for roleplay around a practice. |
| `gamemodes/config/timers.inc` | Existing shared timer hooks; choose a small compatible scheduling approach. |
| `pawno/include/streamer.inc` | Dynamic objects, movement, destruction, and `OnPlayerShootDynamicObject`. |

There is no existing shooting-practice module, target layout, or custom `.dff`/`.txd` asset pack in the inspected base. Select and validate the first target model before building scoring assumptions around it.

Technical notes:

- `OnPlayerWeaponShot` supplies the firing player, weapon, hit type, hit ID, and hit coordinates. It requires lag compensation to be enabled and has documented limitations. Keep the first validated exercise on foot with a supported firearm.
- Streamer exposes `OnPlayerShootDynamicObject`. Existing gamemode code uses `hook OnPlayerShootDynObject`; check the installed YSI hook conventions and actual event delivery rather than assuming a newly written handler is connected correctly.
- Confirm how shot and dynamic-object callbacks relate in this exact dependency stack; do not increment the shot counter independently in both.
- For the MVP, credit the target as a whole. Do not introduce hit-zone calculations or assume that reported hit coordinates are always absolute world coordinates.
- Preserve existing file encodings. `.vscode/settings.json` specifies Windows-1252 for the existing code. This handoff uses ASCII-compatible Markdown; avoid unrelated encoding rewrites.
- `docs/` is ignored by this repository. This handoff lives under `.internal/` so it is included in the fork and in a clone of this branch.

References:

- [SA-MP shot callback documentation](https://open.mp/docs/scripting/callbacks/OnPlayerWeaponShot).
- [Streamer callbacks](https://github.com/samp-incognito/samp-streamer-plugin/wiki/Callbacks).
- [Repository README](https://github.com/San-Andreas-Roleplay-ES/samp-codigo-abierto#readme).

## 9. Environment and validation

Local checkout update, 2026-10-02: the Windows runtime has been prepared, started, queried over UDP, stopped through RCON, and restarted successfully. See [local Windows server instructions](local-windows-server.md) for the launch commands, pinned downloads, and validation limits. The point-capture command and shooting exercise remain untested with a player in-game.

The clone contains source and includes, but not the complete executable runtime. The README names SA-MP `0.3.DL-R1` and Pawn Community Compiler `3.10.10`. The checked-in VS Code build task invokes `pawno/pawncc.exe`. Server binaries, compiler executables, plugin binaries, and any required runtime data must be prepared separately in a compatible environment.

Do not commit those dependencies, generated `.amx` files, logs, credentials, or machine-specific configuration. Keep setup details reproducible and state clearly which checks were actually run. A successful compilation alone is not evidence that targets appear, collide, or score correctly in a SA-MP client.

### Implementation order

1. Prepare a reproducible build/runtime environment and establish the baseline compile result.
2. Validate one candidate target in-game: it appears correctly, has usable collision, and delivers the expected impact event.
3. Verify one physical shot produces one recorded shot and at most one credited target hit.
4. Implement the configured lifecycle, ownership, cancellation, and resource cleanup.
5. Add the result calculation and optional grading thresholds.
6. Run the acceptance scenarios below and document the observed results and remaining limitations.

### Acceptance scenarios

| Scenario | Expected result |
| --- | --- |
| Start outside the station, in the wrong world/interior, or with an unsupported weapon | Clear rejection; no objects, timers, or reserved station. |
| Start twice or attempt to use an occupied station | Rejection without disturbing the active attempt. |
| Complete a known sequence | Configured order, transforms, target exposure, and gaps are respected. |
| Hit a target more than once or hit around its timeout | At most one completion for that presentation; no duplicate score. |
| Shoot and miss, shoot between targets, or never shoot | Counts and percentages match the scoring contract; no division by zero. |
| Another player shoots the participant's target | The participant's score and progression are unchanged. |
| Cancel, die, disconnect, leave the allowed area, change world/interior, enter a vehicle, or switch to an unsupported weapon | Incomplete attempt; all owned resources and station occupancy released. |
| Target creation/configuration fails | No graded completion; station remains usable after cleanup. |
| Finish, then immediately start another attempt | Counters reset and no stale target/timer affects the new session. |
| A player ID is reused after disconnect | The new player inherits no prior session or delayed callbacks. |
| Scores exactly at and just below pass thresholds | Correct outcome using unrounded values; cancellation never passes. |
| Configuration changes to another valid target position/timing | Engine behavior follows the configuration without code changes to scoring. |
| Existing unrelated commands and shot-driven features are used | The practice module does not swallow their events or manipulate their resources. |

Meaningful automated checks for scoring and lifecycle transitions are useful if supported by the chosen test setup. Client-based checks remain necessary for object appearance, collision, event delivery, and multi-player interference. Capture a short manual checklist or video showing a completed attempt, an interruption, and a clean restart for eventual review.

## 10. Working conventions and handoff

Clone this branch explicitly; the planning document is not on the fork's default `main` branch:

```sh
git clone --branch feat/shooting-practice-mvp https://github.com/SouthViking/samp-codigo-abierto.git
cd samp-codigo-abierto
git remote add upstream https://github.com/San-Andreas-Roleplay-ES/samp-codigo-abierto.git
```

Use `origin` for the SouthViking fork and `upstream` for the original project. Inspect the current working tree and any applicable repository instructions before implementing.

Continue with English Conventional Commit subjects, for example:

- `feat(shooting-practice): add configured practice sessions`
- `fix(shooting-practice): prevent duplicate target scoring`
- `test(shooting-practice): cover scoring boundaries`
- `docs(shooting-practice): document setup and validation`

Keep commits focused on the shooting practice feature. The intended future contribution is a PR from this fork's feature branch to upstream `main`, supported by build and in-game validation evidence. This handoff does not request opening that PR yet.

The definition of done for this MVP is a repeatable, correctly measured solo practice that cleans up on completion and interruption, with configuration separated from runtime behavior. Future administration, FTP supervision, private-range business rules, and richer targets should be able to build on those boundaries without being implemented now.
