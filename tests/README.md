# Shooting practice regression tests

`shooting_practice.pwn` includes the production configuration, storage, session and command code.
Only player state, clock, messages and streamer object operations are simulated. The server's Pawn VM,
`format`, file natives and sscanf 2.13.1 perform the actual parsing/serialization/execution.

The tests create and overwrite `scriptfiles/shooting_practice.cfg` in their runtime. Always run them in
an isolated test server directory, never in the playable server's working directory. They request shutdown after completion
and print `[Practice tests] PASS` when every assertion succeeds. This does not test client rendering,
object collision, multiplayer transport, or the existing YSI callback dispatcher.

## Windows, using the prepared local dependencies

From the repository root after `scripts/prepare-local-server.ps1` has completed:

```powershell
$practiceTestRoot = Join-Path (Get-Location) '.local-server/practice-tests'
New-Item -ItemType Directory -Force -Path $practiceTestRoot | Out-Null
foreach ($folder in @('plugins', 'gamemodes', 'scriptfiles')) {
    New-Item -ItemType Directory -Force -Path (Join-Path $practiceTestRoot $folder) | Out-Null
}
Copy-Item .local-server/samp-server.exe $practiceTestRoot -Force
Copy-Item .local-server/plugins/sscanf.dll (Join-Path $practiceTestRoot 'plugins') -Force
@'
rcon_password local-practice-tests
maxplayers 2
port 7779
bind 127.0.0.1
hostname Practice Tests
gamemode0 practice_tests 1
plugins sscanf
announce 0
query 0
lagcompmode 1
'@ | Set-Content (Join-Path $practiceTestRoot 'server.cfg') -Encoding ASCII
Push-Location tests
try {
    & ../pawno/pawncc.exe shooting_practice.pwn '-i../pawno/include' '-;+' '-(+' '-d3' '-E' ('-o' + (Join-Path $practiceTestRoot 'gamemodes/practice_tests.amx'))
    if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed.' }
} finally { Pop-Location }
Remove-Item (Join-Path $practiceTestRoot 'server_log.txt') -ErrorAction SilentlyContinue
$practiceTestProcess = Start-Process -FilePath (Join-Path $practiceTestRoot 'samp-server.exe') -WorkingDirectory $practiceTestRoot -PassThru
if (!$practiceTestProcess.WaitForExit(15000)) { Stop-Process -Id $practiceTestProcess.Id }
$practiceTestLog = Get-Content (Join-Path $practiceTestRoot 'server_log.txt') -Raw
if ($practiceTestLog -notmatch '\[Practice tests\] PASS' -or $practiceTestLog -match '\[Practice tests\] FAIL:') {
    throw 'Practice regression tests failed; inspect the isolated server log.'
}
```

The runner clears its previous log and limits the isolated process to 15 seconds. Port 7779 must be free. This test server is independent of the normal local server on 7777.

## Unix validation

Compile with Pawn Community Compiler 3.10.10 and `-i../pawno/include -;+ -(+ -d3 -E` from `tests/`.
Use the checked-in includes. A compatible server with the sscanf 2.13.1 plugin can run the test AMX as its
only gamemode. On open.mp, set `pawn.main_scripts` to `["practice_tests"]` and `pawn.legacy_plugins` to
`["sscanf"]`, with all runtime data in an isolated directory. No clients need to connect.

Compiling the **full gamemode** on Unix also requires normalizing Windows backslashes in existing include
directives to slashes in a temporary source copy. Keep the committed upstream sources unchanged.

## Recorded verification (2026-10-02)

- Entire `SARP.pwn` and the test gamemode compiled with Pawn Community Compiler 3.10.10,
  the checked-in includes, `-;+ -(+ -d3 -E`, and no compiler diagnostics. Existing full-gamemode
  warning suppressions were retained; the isolated suite does not suppress type warnings.
- **97 assertions, 0 failures**, executing the compiled test AMX on open.mp 1.5.8.3079,
  with sscanf 2.13.1, in an isolated Linux x86 container. Player and object adapters were simulated.
- The compatibility runtime printed PASS and unloaded sscanf, but did not finish process shutdown
  under this container's emulation; the test containers were explicitly terminated afterward.
  This run does not verify Windows server shutdown or an actual SA-MP client session.
- Client collision/rendering and two-player gameplay remain manual checks in the testing guide.
