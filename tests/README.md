# Shooting practice regression tests

Current verification: 2026-10-10. The native object selector and expanded catalogue were built and tested
in isolated Windows SA-MP 0.3.DL-R1. The playable server retains the previous pickup/report build
and its saved layout. The new catalogue has not been deployed or restarted into that server.

The suite includes production configuration, registry, storage, sequence, session, presentation, editor and
dialog code. shooting_practice_ui.inc exercises configuration flows; shooting_practice_refinements.inc
covers multiweapon configuration, countdown ownership, departure and detailed feedback.
shooting_practice_feedback.inc covers conditional actions, distance gates and stable editing references.
shooting_practice_polish.inc covers preview-shot repair, cue spacing and fractional timing/migration.
shooting_practice_zones.inc covers administration, public selection, independent sessions/editors, registry storage and recovery.
shooting_practice_starts.inc covers point CRUD, shared geometry, exclusive public use, nearest/frozen origins,
multiple preview markers, legacy v7 migration and v8 storage. shooting_practice_pause.inc covers
closing/resuming sections, retained drafts/views, stale records, save ordering and cleanup.
shooting_practice_discovery.inc covers per-zone pickups, private entry hints, allocation rollback and v1-v8 indicator migration.
shooting_practice_catalogue.inc covers native cards, filters/pages, cursor ownership and allocation failure,
plus expanded model storage, randomized pairs, previews and scoring paths.
Player state, clock, audio,
dialog rendering, model cards and streamer operations use deterministic adapters.
Pawn execution, format, sscanf 2.13.1 and file I/O are real. Results are captured before the actual
player-summary formatter runs.

## Run on Windows

From the repository root, after the local server dependencies have been prepared:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/test-shooting-practice.ps1
```

Each run creates a fresh ignored .local-server/practice-tests directory, compiles with warnings
treated as errors, and starts an isolated hidden server on loopback UDP 7779 for at most 15 seconds.
It requires PASS and normal shutdown. Logs remain at the printed path. The playable server's
positions, configuration, AMX and logs are not overwritten.

The tests intentionally overwrite their own scriptfiles/shooting_practice.cfg, .tmp and .bak.
Never run the test AMX in a live/playable server directory.

## Recorded verification: 2026-10-10 — object catalogue

**1228 assertions, 0 failures** on isolated Windows SA-MP 0.3.DL-R1, normal process exit.
Test log: .local-server/practice-tests/14e3549889c94a79adf881d45670b776/server_log.txt.
Tests and full SARP.pwn compile with Pawn 3.10.10 and -;+ -(+ -d3 -E.
The full AMX was written outside the playable runtime. No playable server file/process was changed.

New coverage:

- Six named cards per page, font 1 text/font 5 previews, model bindings and preview dimensions.
- Category filters, disabled boundary navigation, redraw ownership and cursor setup before showing.
- Foreign/invalid handles, Back/ESC, native cancellation re-entry and pending edit preservation.
- New-target wizard cancellation/placement, RCON/revision/dimension/range loss and disconnect cleanup.
- Initial and partial redraw allocation failure without leaked textdraws or a stuck cursor/writer.
- All nine model IDs persist in v9 without migration and retain behavior under randomization.
- Added bottles/can/cone use exact model IDs in previews and frozen sessions, one-hit scoring
  and protected air-shot errors.
- All earlier regressions below.

The harness captures native TextDraw arguments and logical click dispatch. Actual GTA rendering,
mouse hit areas, GXT accents, collisions and breakable model/network behavior remain client checks
in the [manual guide](../.internal/shooting-practice-testing.md).

## Previous verification: 2026-10-10 — start indicators and reports

**955 assertions, 0 failures** on isolated Windows SA-MP 0.3.DL-R1, normal process exit.
Full SARP.pwn and tests compile with Pawn 3.10.10 and warnings treated as errors.
The isolated test run never modifies playable server files or processes. After verification, the separate local launcher
rebuilt and restarted the playable server on explicit request. Its v8 saved configuration was not rewritten on load.

New coverage:
- Multiple start-point add/name/relocate/delete, cap, sparse slots, stale responses and RCON revocation.
- Common dimension/distance gates, retained targets when deleting the final point, inactive partial reload.
- Nearest and tied point selection, frozen movement/cue origin and departure from the selected post.
- One session per zone across its posts, public target ownership and release for another participant.
- Private markers per viewer, stable neighbour handles, update/start/dimension cleanup.
- V7 start migration without rewriting on load, V8 multiple/sparse start round trips and corrupt records.
- Close/resume through bare or matching-ID commands, walking before creation, retained draft/view,
  refreshed stale lists, replaced-point safety, zone switching, save order, RCON/lifecycle/range loss.
- Optional public pistol pickup per start, stable/changed/deleted handles, dimension isolation, active/archived/reset lifecycle,
  partial allocation rollback and repair. Private entry hints work with or without pickups, with post hysteresis and eligibility filters.
- Per-zone v9 flags, v1-v8 migration with indicators off, rejected metadata, administrative/stale/occupied checks and save retry.
- Seven-line white result bodies with soft-blue headers/dividers/pipes, colored verdict word, literal percent and compact cancellation.
- All existing zone, scoring, editor, preview, report, weapon, timing and storage recovery regressions.

The tests simulate client/streamer behavior. Validate actual rendering, sounds and public visibility
using the multiple-booth steps in the manual guide.

## Previous verification: 2026-10-08

**745 assertions, 0 failures** on isolated Linux SA-MP 0.3.DL-R1, sscanf 2.13.1 and crashdetect 4.22;
normal process exit. Tests and full SARP.pwn compile with Pawn 3.10.10 and -;+ -(+ -d3 -E.
The test VM heap/stack is 8192 cells; production keeps its existing allowance. Simulated player arrays
and production MAX_PLAYERS loops share the same five-slot limit in the harness.

New coverage includes:

- RCON-only create/rename/activate/deactivate/archive; incomplete configuration and permission loss.
- Stable IDs, pagination, captured row IDs, stale lists, capacity (including tombstones) and overflow.
- Public near-zone choice, overlap, explicit ID, dimension/distance/weapon/occupancy revalidation,
  unprivileged participants and one active round per player.
- Two concurrent rounds with isolated shot counts, targets, scoring, timers, cancellation and countdowns.
- Per-zone writer locks, menus, previews and unrelated edits during an active round.
- Whole-registry v7 round trip, names/full target transforms/rules, archive/active state and next ID.
- Legacy v1-v6 migration, blocked occupied/editor reload, malformed metadata/point lines/IDs,
  validated stage/backup, failed writes, torn primary recovery and pending repair.
- All previous scoring, editor, preview repair, report, weapon and timing regressions below.

The run used ignored .local-server/multi-zone-development/runtime resources, loopback only,
inside a temporary Docker container with networking disabled. The macOS full-build validation
normalizes Windows include separators only in an ignored copy. No game client integration was simulated
as a successful real client run. Logs are development artifacts, not source or deployable history.

## Previous verification: 2026-10-07

**606 assertions, 0 failures** on Windows SA-MP 0.3.DL-R1 and sscanf 2.13.1; normal process exit.

Coverage includes:

- Shot/score accounting, air/scenery no-shoot penalties, negative points, error thresholds,
  deadline boundaries, sparse/ten-slot sequences, interruptions and player ID reuse.
- Countdown/cue timing, participant-only textdraw ownership, cancellation/disconnect/ID reuse,
  delayed ticks, allocation failure, named departure and observer filtering.
- Five result lines with a white base and only Aprobado/Reprobado colored green/red, missed targets
  versus restraint/errors, scoring thresholds, literal accuracy percentage, zero-shot precision
  and the per-message chat limit. Extra attempts and interval shots lower accuracy while leaving
  the net-points and identification-error verdict rules intact.
- At least one second between actual countdown cues and the start cue under delayed/nonaligned ticks.
- Owned preview-hit veto/repair, duplicate coalescing, unchanged neighbours/viewers/native editor,
  hidden/lost-access cleanup and allocation failure without changing saved layout.
- Decimal point/comma input, tenths, strict bounds, exact exposure/interval deadlines and v1-v6 timing migration.
- Fixed/random order, exact type counts, frozen plans, model/behavior pairs staying together.
- Root command and real menu handlers; independent model/name/behavior edits; native final/cancel;
  final coordinates overriding update; finite coordinates, normalized rotations and distance limits.
- Direct start capture from the root and immediate persistence, including an unset start at real city
  coordinates and relocation beyond the old target radius. Far-away dimension confirmation survives
  editor maintenance, saves/clears targets only on confirmation and preserves them on cancel.
- Save as the final draft row for new/existing targets, correctly dispatched duplicate/delete,
  consistent protected labels and the exact automatic departure message.
- confirmed reset/delete/dimension change,
  duplicate slots, concurrency, stale dialog rejection,
  generic editor exclusion, RCON checks and loss of permission.
- Private object/checkpoint/label ownership, independent viewers, no scoring in preview,
  partial allocation rollback and cleanup at hide/start/death/disconnect/dimension changes.
- Conditional root actions with captured row mapping, rechecked eligibility, reset/retry when start
  is absent and participant-only cancellation.
- Early 100 m editing radius, final 3-100 m 3D bounds, initial placement at boundaries and stale final rejection.
- Automatic setup view through catalogue/native editing, unchanged neighbour handles, restoration
  on save/cancel, updates without hiding other targets, explicit preview intent and simple name labels.
- Multiple permitted weapons, same/different-slot switching, frozen rules, rejected unlisted shots,
  ammo exhaustion of the last held weapon, at-least-one selection and explanatory numeric input.
- Save failure/retry, v1-v4 migration to weapon masks, v6 round trips, custom names/models/full rotations,
  invalid masks and malformed/truncated files.

Earlier Windows evidence: Pawn 3.10.10 full compilation and isolated 606-assertion test run.
The prior playable Windows build loaded seven plugins and the saved configuration without changing
its layout hash, following explicitly requested restarts. The user subsequently stopped that server.
The latest explicit restart validates loading of the current build, not actual client rendering. Do not restart a playable server
automatically during the user's in-game feedback.

Adapters do not execute the actual easyDialog/YSI dispatcher or native mouse selection, render the client,
move a real cursor, verify collision/network shot reporting or native model breakage, exercise the
production timer dispatcher, or hear sounds. Preview repair after reported bullet hits is tested,
not native object invulnerability against melee/explosions.
Use the [manual guide](../.internal/shooting-practice-testing.md), preferably with two players,
to validate those integration paths.

## Other hosts

Compile from tests with Pawn Community Compiler 3.10.10 and the checked-in includes.
Use an isolated compatible server with sscanf 2.13.1. The full upstream gamemode uses Windows
backslashes in includes; normalize those only in a temporary copy for Unix validation.
Spanish source text uses Windows-1252 for the SA-MP client; documents use UTF-8.
