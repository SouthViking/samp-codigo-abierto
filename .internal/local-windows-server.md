# Local Windows development server

Current local state (2026-10-07): the server was stopped at the user's request. The latest verdict-word colors, literal accuracy percentage, final Save row, protected-target label and automatic departure reason compile successfully and pass 606 isolated assertions. They will load on the next requested start. Runtime files and saved station configuration remain local.

Prepared on 2026-10-02 and startup-checked again on 2026-10-03 for this checkout's `feat/shooting-practice-mvp` branch.

## Run

From the repository root in Windows PowerShell:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/local-server.ps1 -Action Start
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/local-server.ps1 -Action Status
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/local-server.ps1 -Action Stop
```

Add **127.0.0.1:7777** to a compatible SA-MP 0.3.DL-R1 client's favourites on the same computer. The mode reports `Servidor de Prueba`. Start builds the current sources, launches the server in the background, and checks both the startup log and the SA-MP UDP query response. Starting again reports the existing instance. To apply code changes, stop and start it again. Closing the launcher's window leaves the server running.

Stop sends an authenticated local RCON `exit` command and waits for shutdown. It only controls this checkout's executable and has a checked process fallback if graceful shutdown fails. Build can also be run independently with `-Action Build` while the server is stopped.

The local configuration is `.local-server/server.cfg`, the runtime log is `.local-server/server_log.txt`, and exports are written under `.local-server/scriptfiles/`. The local RCON password is generated randomly during preparation and is not printed by these scripts. Read the local configuration to log in using `/rcon login <your local password>`, then follow the [shooting practice testing guide](shooting-practice-testing.md).

## Reproduce the installation

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/prepare-local-server.ps1
```

Run this with the local server stopped. The script downloads the pinned packages in `scripts/local-server-packages.json`, checks their SHA256 hashes, and installs explicit files. Existing local configuration and scriptfiles data are preserved. The current directory is not significant because paths are resolved relative to the scripts.

The server runs in `.local-server/`, which Git ignores in full. The compiler executable and DLL are installed in `pawno/`, already covered by the repository's binary ignore rules. Existing includes and the committed `server.cfg` template are preserved.

| Component | Pinned source |
| --- | --- |
| SA-MP server | [0.3.DL-R1 Windows archive linked by the repository README](https://sampforum.blast.hk/files/03DL/samp03DL_svr_R1_win32.zip) |
| Pawn compiler | [3.10.10](https://github.com/pawn-lang/compiler/releases/tag/v3.10.10) |
| CrashDetect | [4.22](https://github.com/Y-Less/samp-plugin-crashdetect/releases/tag/v4.22) |
| Streamer | [2.9.6](https://github.com/samp-incognito/samp-streamer-plugin/releases/tag/v2.9.6) |
| sscanf | [2.13.1](https://github.com/Y-Less/sscanf/releases/tag/v2.13.1) |
| Pawn.CMD | [3.3.6](https://github.com/katursis/Pawn.CMD/releases/tag/3.3.6) |
| Pawn.RakNet | [1.6.0](https://github.com/katursis/Pawn.RakNet/releases/tag/1.6.0) |
| MySQL plugin and supporting DLLs | [R41-4](https://github.com/pBlueG/SA-MP-MySQL/releases/tag/R41-4) |
| FCNPC-DL, bundled ColAndreas and node data | [2.0.11](https://github.com/ziggi/FCNPC/releases/tag/v2.0.11) |
| ColAndreas collision database | [Community database pinned to commit 3a362e5](https://github.com/xorweak/rzmp-gamemode/blob/3a362e5ff50c8d051abcf14aa434f7d597b92033/scriptfiles/colandreas/ColAndreas.cadb) |

The collision database is an existing community copy, not an extraction from this machine's GTA installation. Its `cadf` v2 header declares 11,362 models and 44,759 placements. It loaded successfully. The official [ColAndreas instructions](https://github.com/Pottus/ColAndreas/wiki/Instructions) describe generating a replacement from a game's installation using its wizard if needed later.

FCNPC-DL bundles ColAndreas and registers the `CA_*` natives, so the local plugin line loads FCNPC-DL once and omits the redundant standalone ColAndreas plugin. Seven plugins load in total. MySQL loads as a library; the current test gamemode does not connect to a database server.

The local configuration binds to loopback, retains lag compensation for shot callbacks, and uses a one-second CrashDetect slow-call threshold to accommodate map/YSI initialization. YSI's `scriptfiles/YSI/fixes` and `scriptfiles/YSI/temp` directories are created during preparation. Firewall/router settings are not needed for same-PC access.

## Validation and limits

- Compiled the full gamemode with the committed includes and Pawn 3.10.10.
- Loaded all seven configured plugins and the ColAndreas database.
- Observed completion of `OnGameModeInit` and the server's `Number of vehicle models` startup marker.
- Queried SA-MP UDP information at 127.0.0.1:7777 and received the expected hostname, mode and player capacity.
- Confirmed local RCON shutdown and a subsequent launch through the scripts.
- Runtime files, credentials, caches and generated AMX files remain ignored by Git.

The 2026-10-03 revision includes dialog configuration, native object editing, per-target models/names/full rotations, private previews, multiple permitted weapons, numeric help, detailed private results, named departure and a participant countdown overlay. The existing layout loaded successfully and retained its exact SHA256 after restart; its configuration and previous executable were backed up locally. Versions 1-4 load automatically and upgrade to v5 only on a later save. The updated server is running at 127.0.0.1:7777.

On 2026-10-07 the root menu was simplified: Punto de inicio saves directly, Vista previa stays on the root, Resumen was removed, and reset confirmation uses Reiniciar. The updated build was verified and deployed locally.

Later feedback adds conditional start actions, early distance checks, stable visible targets during editing, simpler behavior names and no duplicated label numbering. The latest source also adds preview-hit repair, five result lines with white details and a green/red verdict, minimum one-second countdown
cue spacing and decimal exposure/interval in tenths. It compiled separately and passed 569 assertions.
The latest build was deployed on 2026-10-07 after an explicit user restart request. All seven plugins
and the practice configuration loaded. UDP responds at 127.0.0.1:7777, and the configuration SHA256
was unchanged. The previous executable and configuration are backed up under the ignored
.local-server/backups/verdict-colors-b94276aa573e475d8c8cbf9923e87037 directory.
New saves use format 6, converting older whole-second
timings on load without rewriting the saved file.

A subsequent wording correction changes the result to Veredicto: Aprobado / Veredicto: Reprobado,
keeping the green/red colors. This correction was loaded on 2026-10-07 after the user explicitly requested another restart.
All seven plugins and the configuration loaded, UDP status responds, and the configuration hash is unchanged.

The next start-capture correction restricts the 100 m draft radius to target drafts, so an initial
start or a relocation far from the previous station can be saved. It reproduces the reported failure
and passes 587 assertions after the fix, plus full compilation. This fix and the new /irpruebas
shortcut were deployed on explicit request on 2026-10-07. The full gamemode compiled, all seven
plugins and the saved configuration loaded, UDP status responds and the configuration hash is unchanged.

During in-game feedback, keep the playable server running. Restart it only when the user explicitly requests it.

The full gamemode compiled with warnings treated as errors. The original practice was confirmed in-game by the user. Countdown placement, rendered accents, model cards, private visibility, silhouette collision and sound require client validation. See the [configuration and testing guide](shooting-practice-testing.md).

Development shortcut: /irpruebas requires RCON and on-foot state. It moves only the caller to
802.8580, 1674.5205, 5.2813 at angle 181.2324, interior/world 0. The station remains independently configured.
