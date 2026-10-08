# Shooting practice regression tests

Current local state (2026-10-07): the playable server is stopped at the user's request. The latest compiled source will load on the next requested start.

The suite includes production configuration, storage, sequence, session, presentation, editor and
dialog code. shooting_practice_ui.inc exercises configuration flows; shooting_practice_refinements.inc
covers multiweapon configuration, countdown ownership, departure and detailed feedback.
shooting_practice_feedback.inc covers conditional actions, distance gates and stable editing references.
shooting_practice_polish.inc covers preview-shot repair, cue spacing and fractional timing/migration.
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

The tests intentionally overwrite their own scriptfiles/shooting_practice.cfg.
Never run the test AMX in a live/playable server directory.

## Recorded verification: 2026-10-07

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

The full SARP.pwn also compiled with Pawn 3.10.10 and -;+ -(+ -d3 -E, retaining existing global
warning suppressions. Real server startup loaded seven plugins and the saved configuration,
responded on UDP 127.0.0.1:7777 and was left running. Loading did not change the existing layout hash.
The latest feedback build was deployed on 2026-10-07 after the user explicitly requested a restart.
All seven plugins and the configuration loaded. UDP status responds and the configuration hash is unchanged.
A subsequent start-capture fix exempts start drafts from target placement radius checks. It is
compiled and tested, then loaded after the user explicitly requested a restart. The /irpruebas
shortcut was compiled with the full gamemode and loaded in the same restart. Native teleport/camera
behavior remains a client check. The saved station configuration was preserved.
The subsequent verdict-word color, literal percent, Save ordering, protected wording and
cancellation wording changes are compiled and pass the isolated tests, pending a requested restart.
Do not restart the playable server automatically during the user's in-game feedback.

Adapters do not execute the actual easyDialog/mSelection/YSI dispatcher, render the client,
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
