# Shooting Practice MVP - Implementation Handoff

Current local state (2026-10-07): the server was stopped at the user's request. The latest compiled and tested source will load on the next requested start.

Updated: 2026-10-07. Dialog configuration, native editing, private preview, per-target models,
persistence, multiple permitted weapons, numeric help, detailed results, named departure,
on-screen countdown, conditional start actions, placement-radius checks, persistent editing references,
roleplay/audio and mixed shoot/no-shoot exercises are implemented. Latest feedback adds preview-shot repair,
compact results with white details and only the verdict word colored, minimum one-second cue spacing and fractional exposure/interval timing.

- Fork: https://github.com/SouthViking/samp-codigo-abierto
- Branch: feat/shooting-practice-mvp
- Upstream: https://github.com/San-Andreas-Roleplay-ES/samp-codigo-abierto
- Original upstream base: e7f84d1294a523fe2aa7c0e5f1c6ae1e8d87f907.
- Public handoff; internal does not mean private.
- [Configuration/testing guide](shooting-practice-testing.md) and [Windows server](local-windows-server.md).

## Contract

One station, one participant, 1-10 individually configured targets. Each position appears once,
one at a time. Future faction/civilian/private ranges can reuse it without dependencies on those systems.
Results can inform an instructor's assessment; they do not certify an entire FTP.

Targets store optional name (32 characters), model, independent behavior, XYZ position, XYZ rotation,
interior/world. Catalog: box 1271, armed silhouettes 1583/1584, civilian silhouettes 1585/1586.
No custom downloads or new plugins. The box gets red DISPARAR / blue NO DISPARAR material;
silhouettes retain native textures. Instructors should use distinguishable figures and explain
their criteria. Model and behavior are independent.

Shoot-target impact: +1; expiry: omission. Any participant discharge during no-shoot, including
air/scenery, gives -1 and one identification error, then retires that target. No-shoot expiry is correct.
Net points = hits - errors, without clamping. Both minimum points and maximum errors determine passing.
Precision separately uses all counted shots, including repeated attempts and interval shots.
Extra shots lower accuracy without changing net points, identification errors or passing thresholds.
The recommended clean exercise uses one shot per shoot target and none per protected target.

One or more permitted weapon IDs 22-34, selectable independently. Players may switch between them;
unlisted weapons interrupt the round. Defaults: only 24, 1 point, 0 errors, preparation/exposure/interval 3/5/1 seconds,
fixed order/distribution. Preparation accepts whole seconds from 1-30. Exposure accepts 0.1-60.0
seconds and interval 0.0-30.0 seconds, in tenths, using a dot or comma decimal separator. Start radius 1.5 m, movement radius
3 m, targets 3-100 m away in the same interior/world. Equipment is never granted, removed or replenished.
Spanish user-facing text; English implementation identifiers.

## Configuration, ownership and editing

/practicaconfig is the sole setup command, requiring RCON. Old arguments only open its menu.
Root sections: direct start capture, targets, rules, preview toggle, test/cancel-own-round and confirmed reset;
failed saves add a retry row. Existing easyDialog dispatch and mSelection cards are reused.

Iniciar práctica appears only when PracticeStartReady succeeds for this player: completed layout,
alive/on-foot access, start proximity, same dimension, permitted held weapon, no participant or editor.
The command uses the same gate. Cancelar práctica appears only for the participant. Main-row actions
are captured per rendered menu, so optional rows cannot shift reset/retry into another action.
Start eligibility is rechecked on response, including movement after opening the dialog.

Rules > Armas permitidas toggles each weapon and saves immediately; at least one must remain enabled.
Volver returns to rules. Each numeric input explains its effect, units, bounds, zero where applicable,
and current value. Minimum net points and maximum identification errors are independent passing
requirements. Without shoot targets, the points input explains that an objective must be added first.

Selecting Punto de inicio on the root immediately saves the character position/facing and returns to
the root. It has no submenu; preview is available only on the root. Reset confirmation uses the short
Reiniciar button. The former summary dialog has been removed. Changing dimension requires confirming deletion
of old targets; same-dimension moves retain them and can require distance corrections.
Start capture can create or relocate the station anywhere within valid coordinates. It is exempt
from the target-only 100 m draft radius, including while confirming a dimension change.
Partial configurations can be saved; readiness is checked before practicing.

Creation chooses model and behavior, then opens native object editing. Existing targets offer
name/model/behavior, move/rotate, duplicate, confirmed delete, then save as the last row.
New drafts end with save after move/rotate. The behavior menu consistently uses Blanco protegido. Draft changes are applied only
by Save or EDIT_RESPONSE_FINAL; cancel discards all pending fields. Duplicates fill a free slot without
changing their source. Choosing behavior sets fixed distribution on commit; administrators may then
enable random distribution explicitly. Model/name/position-only edits preserve distribution.

One writer owns the station draft. Other admins can inspect but cannot mutate. Practice starts wait
for commit/discard. Mutation requires alive/on-foot RCON state and is blocked during rounds.
Configuration revisions reject stale row, deletion and reset responses.

The native editor owns a private temporary object, separate from saved records and session objects.
FINAL coordinates are authoritative. UPDATE only previews on the client. Finite values, distance and
dimension are checked and XYZ rotation normalized before committing. Invalid placement offers
retry/discard while retaining the previous saved record. Ownership clears before native cancellation.
Target creation/editing must begin within 100 m of the start in the same dimension, before allocating a draft
or opening the wizard. The initial new-object position is kept inside the same 3-100 m final bounds.
Every native FINAL rechecks finite coordinates, distance and dimension. Leaving the 100 m editing
radius also releases the draft and references.
Death, disconnect, vehicles, dimension changes, lost RCON or lost temporary objects release resources.
The base command resumes pending menu edits.

General and practice editors mutually exclude via existing playerEditingObjectID and SP_ObjectEditor
PVar. No general object records are reused. Vendored ShowModelSelectionMenuEx now declares read-only
array parameters const. YSI OnPlayerEditDynObject uses its supported Dyn alias to avoid symbol truncation.

## Private preview

Vista previa closes the dialog and shows a player-restricted dynamic checkpoint at the saved start,
all objects with their exact rotations, and named behavior labels without a duplicated numeric prefix. The administrator can walk
around and reopen the root to hide it. No scoring, countdown or sounds.

Each viewer owns independent handles. Hide/death/disconnect/vehicles/dimension changes/lost RCON clean
them. Partial allocation failures roll back everything. Confirmed changes synchronize affected slots.
Unchanged objects keep their handles and remain visible. Successful practice starts remove all views
before countdown. Only owned dynamic checkpoints are used,
never SetPlayerCheckpoint or DisablePlayerCheckpoint.

Opening Blancos also enables a private automatic setup view through catalog, draft and native editing.
Only the actively edited slot's saved ghost is removed, replaced by the owned native editing object.
Other saved references stay visible. Save/cancel restore the saved slot, retaining neighbours.
A per-view point cache compares complete records and updates only changed/invalid slots.
Returning to the root clears the automatic view, while an explicitly enabled root preview remains
active. ResetUI and the shared tick clean both modes on lifecycle/access/distance changes.

Hits on owned preview objects return the YSI break-with-zero value from OnPlayerShootDynObject,
vetoing the bullet before the scoring callback. The client can already have broken a native model
locally, so the next 50 ms pulse recreates just the struck private slot, preserving its saved model,
position, rotation and material. Repeated hits coalesce. Neighbours, other viewers and the native
editing object retain their handles. Hide/access loss cannot resurrect a queued slot. Allocation
failure removes the owned view and leaves configuration intact. This is bullet repair, not a GTA
model invulnerability flag. Melee, explosions and rendered replacement timing need client checks.

## Runtime, randomization and scoring

/practica starts; /practica cancelar interrupts the caller's attempt; /practica ayuda explains rules.
Any non-NPC can currently practice; PracticeCanUseStation is the future permission integration point.

IDLE -> COUNTDOWN -> TARGET -> INTERVAL -> TARGET -> ... -> IDLE.

Start validates readiness, access, alive/on-foot state, dimension, proximity, held weapon and occupancy.
The sequence, model/behavior pairs, permitted weapon set and thresholds freeze once. Fixed order uses ascending marked slots.
Fisher-Yates provides independent randomization:

- Random order moves position/model/behavior together.
- Random distribution moves model and behavior together between positions, preserving exact counts and
  associations. Rotations belong to positions, so use compatible orientations.
- Saved assignments stay intact. A sequence can legitimately repeat.

Session targets are dimension-restricted but visible to observers; only the participant scores.
Creation/material/streaming failures interrupt and clean up.
OnPlayerWeaponShot processes each discharge once, without also counting the dynamic-object callback.
Shoot impacts resolve through Streamer_GetItemStreamerID to the active target; for no-shoot, any
configured-weapon discharge is an error regardless of hit type/ID.

Countdown shots are excluded; interval shots count for precision. Expiry resolves before a late shot
can affect anything. Final-deadline shots lie outside the completed scoring window. A phase retires
before its object is destroyed, preventing duplicate scoring/omission. Complete rounds satisfy
hits + omissions = shoot count; correct ignores + errors = no-shoot count; hits + errors <= shots.
Interrupted rounds do not classify unpresented targets.

One gamemode-owned 50 ms timer polls session deadlines and pending preview repair. It is killed on
mode exit and holds no player-specific delayed callback. The shared 500 ms task checks editor/view
validity. Tick differences handle signed wraparound. Transitions can wait for the next pulse,
including interval 0. Exposure and interval are integer milliseconds, in 100 ms steps. Leaving the radius, changing dimension, equipping/firing an unlisted weapon,
death, disconnect, vehicle entry or lost objects interrupt. Ammo exhaustion, including switching to
fists, continues if the last permitted weapon's slot is empty; other slots may still contain ammo.
No teleport/freeze.

## Roleplay/audio

One neutral preparation action uses the existing roleplay name. One environmental /do-style line
announces closure. Leaving the radius/dimension replaces the generic closure with
"* Nombre Apellido abandona el puesto de tiro." using the participant's roleplay name, once.
Only the participant receives the final result/reason. A completed result uses five chat lines with a white base:
rules (minimum net points, maximum errors, +1/-1), results (hits and timed-out shoot targets),
protected targets respected and identification errors, shots and plain precision, then verdict and
net points. Only Aprobado is green or Reprobado red. Veredicto: and the trailing net points remain white.
Precision includes the literal % sign and has no trailing parenthesis. Preformatted result lines go
straight to the SA-MP SendClientMessage native without another variadic formatting pass. Zero shots display unavailable precision.
Every line fits the client's 144-cell buffer. Interrupted attempts keep a short private reason.
Radius/dimension departures report Incompleta: Cancelada automáticamente por salir del puesto de tiro.
No chat per countdown, appearance, hit or omission.

Built-in sounds: 1056 each countdown second, 1057 each successfully created/streamed target,
1139 at closure. First appearance is the start signal; both behaviors sound identical.
A real-clock guard keeps at least 1000 ms between delivered countdown cues and between the last
countdown cue and first appearance. Delayed ticks skip missed values without a burst, and a late
final cue extends preparation instead of shortening its pause. SA-MP timer/network delays can still
make cues arrive late. Audio/actions explicitly reach non-NPC observers within 10 m
in the station's dimension; they do not assume 3D attenuation.

A small centered player textdraw reads Comienza en 3, 2, 1 (or the configured preparation time).
Only the participant sees it; it follows the same tick/beeps and disappears at first target or any
interruption. It owns its handle, cleans on disconnect/ID reuse, and does not replace unrelated UI.
Textdraw allocation failure preserves the audible countdown. The plain text avoids GTA font accents.

## Storage and migration

File: scriptfiles/shooting_practice.cfg relative to server working directory; the local launcher uses
.local-server/scriptfiles/shooting_practice.cfg. New saves use SARP_PRACTICE 6:

- Rules line: weaponMask points preparation exposure interval maxErrors randomOrder randomDistribution.
- Preparation stays in whole seconds. Exposure and interval are integer milliseconds, in 100 ms steps.
- weaponMask uses bit (weapon ID - 22); default Desert Eagle is 4, all thirteen weapons is 8191.
  Zero, negative masks and unsupported bits are rejected.
- Eleven point records, start then ten slots: marked x y z rotZ interior world noShoot model rotX rotY.
- Each point is followed by a NAME line containing the optional name.
- END terminates the file.

Flags are 0/1. Start/unmarked points use model 0 and noShoot 0; marked models must be in the catalog.
Names reject controls, line breaks and color braces. Counts are derived from marked slots.

V1 retains original rules/positions and defaults to all shoot. V2 materializes its quota on highest
marked slots while preserving preferences/incomplete layouts. V3 retains explicit behavior.
V1-V3 targets get model 1271, zero X/Y rotation and automatic names; angle remains Z rotation.
V4 retains full transforms, models, names and behaviors. V1-V4 convert their single weapon ID into
exactly one permitted bit. V5 retains the weapon mask. V1-V5 exposure/interval seconds are validated
before multiplication and converted to milliseconds exactly once. Next save upgrades to v6.
Loading never rewrites the original.

Temporary records are validated before load applies anything. Invalid/unsupported/truncated files never
partially replace the layout. Failed writes keep memory usable and pending-save/retry visible.
Writes remain nontransactional: retain backups of valuable layouts. Older shooting_practice_points.inc
exports are not imported. Runtime data, credentials, logs and binaries remain ignored.

## File boundaries

| File under gamemodes/features/shooting_practice/ | Responsibility |
| --- | --- |
| config.inc | Records, model catalog, rules, readiness. |
| storage.inc | Serialization, migration, save status. |
| sequence.inc | Frozen positions, pairs and randomization. |
| editor.inc | Draft ownership, private preview, native editor lifecycle. |
| setup.inc | Single command, dialogs, catalog, confirmations/navigation. |
| session.inc | Player command, target lifecycle, scoring. |
| presentation.inc | Materials, audio, countdown overlay, roleplay, help/results. |
| hooks.inc | YSI callbacks, 50 ms pulse, 500 ms editor maintenance, shots, cleanup. |
| header.inc | Module include order. |

## Validation and scope

- 606 assertions, 0 failures on isolated Windows SA-MP 0.3.DL-R1 with sscanf 2.13.1; normal shutdown.
  Production logic, VM and file I/O are real; player/UI/streamer/audio adapters are simulated.
  Coverage: dialog flows, drafts, native confirm/cancel, stale menus, concurrent admins, private preview,
  allocation rollback, cleanup, retry, v1-v6 migration, multiweapon selection/switching, frozen weapon
  rules, slot-specific exhaustion, countdown resource ownership, named departure and detailed results.
  New coverage checks conditional main actions, captured row dispatch, early/final 3D distance bounds,
  stable neighbour handles through move/save/cancel/model selection, label names and automatic view cleanup.
  Latest coverage checks isolated preview repairs and failures, compact reports with white base text and only the outcome word colored, cue timestamps,
  decimal parsing/bounds/deadlines and compatible migration without overflow. Added coverage checks
  save as the final draft row, duplicate/delete dispatch, the exact automatic departure reason,
  literal percent signs and reduced accuracy from repeat attempts/interval shots without changing verdict rules.
- Full SARP.pwn and tests compile with Pawn 3.10.10, warnings treated as errors. Existing global warning
  suppressions remain. Spanish UI sources use Windows-1252, matching the legacy client; documents use UTF-8.
- The earlier deployed build loaded seven plugins and saved layout and answered UDP at 127.0.0.1:7777.
  The latest feedback build was deployed on 2026-10-07 after an explicit restart request. All seven
  plugins and the practice configuration loaded, and UDP status responds at 127.0.0.1:7777.
  The configuration SHA256 stayed unchanged. The start-capture radius fix and /irpruebas shortcut were also loaded on explicit request.
  /irpruebas is an on-foot RCON development shortcut to XYZ 802.8580, 1674.5205, 5.2813,
  angle 181.2324, interior/world 0. It does not modify the station configuration.
  The subsequent result formatting, final Save row, behavior wording and cancellation wording changes
  are compiled and tested but pending a requested restart. Restart only on explicit user request.
- Original practice was confirmed by the user in-game. New dialogs/cursor, cards, model collision,
  private visibility, countdown placement and sounds still need client checks. Tests do not execute the YSI dispatcher,
  actual rendering or network shot reporting, native model breakage or the live timer dispatcher.

Out of scope: multiple stations/concurrent rounds, faction/rank policies, history/SQL/leaderboards,
FTP certification, weapon administration, custom assets, moving/simultaneous targets, scoring rings,
live scoring HUD, teleportation and unrelated changes.

## Engine references

- [Streamer 2.9.6 shot dispatch](https://github.com/samp-incognito/samp-streamer-plugin/blob/v2.9.6/src/callbacks.cpp):
  the dynamic-object callback can veto a reported shot. Owned repairs run after dispatch to avoid
  mutating the streamer's iterated object registry.
- [SA-MP timer limitations](https://open.mp/docs/scripting/functions/SetTimer): polling is approximate,
  so GetTickCount guards actual deadlines and cue spacing.
- [Built-in sounds](https://open.mp/docs/scripting/resources/sound-ids): 1056 is RACE_321 and 1057 is RACE_GO.

## Spanish interface wording

The root title is Campo de tiro; readiness and pending-save state live in the content. Dialogs use
short titles without pipe separators or semicolons in user-facing text. Target behavior is
Blanco de tiro (disparar) or Blanco protegido (no disparar).
Weapon fields show names. Dialog/chat/3D-label strings use Windows-1252; the catalog back button is
Atrás, with GTA font 1's GXT glyph 0x98 for á. The custom-list selector accepts an optional back_text
label, preserving defaults for other callers. Glyph reference: https://github.com/gta-reversed/gta-reversed/blob/master/source/game_sa/GxtChar.cpp.
