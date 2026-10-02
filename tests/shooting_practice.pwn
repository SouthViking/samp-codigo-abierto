// Run as a separate SA-MP gamemode with sscanf 2.13.1 (never on the live server).
// Player, clock and streamer adapters are simulated; Pawn logic, parsing and file I/O are real.
#include <a_samp>
#include <sscanf2>
#pragma warning disable 203

#define CMD:%0(%1) stock TestCommand_%0(%1)
#define isnull(%0) (!(%0)[0])
#define STREAMER_TAG_OBJECT _
#define INVALID_STREAMER_ID (0)
#define STREAMER_TYPE_OBJECT (0)
#define COLOR_FADE3 (-1)
#define COLOR_LIGHTGREEN (-1)
#define COLOR_LIGHTORANGE (-1)
#define COLOR_GRAD1 (-1)
#define COLOR_HELP_1 (-1)

new TestNow, TestFailures, TestAssertions;
new bool:TestConnected[2], bool:TestAdmin[2];
new TestState[2], TestWeapon[2], TestAmmo[2], TestWorld[2], TestInterior[2];
new Float:TestPosition[2][3], Float:TestHealth[2];
new bool:TestObjects[256], TestNextObject, TestLiveObjects;
new bool:TestCreateFails, bool:TestVisible = true;
new TestResult, TestResultShots, TestResultHits, TestResultExpired;

stock TestSendMessage(playerid, color, const message[])
{
    if (!strcmp(message, "[Practica] APROBADO.")) TestResult = 1;
    if (!strcmp(message, "[Practica] REPROBADO.")) TestResult = 2;
    return 1;
}
stock va_SendClientMessage(playerid, color, const message[], {Float,_}:...)
{
    if (strfind(message, "[Practica] INCOMPLETA:") == 0) TestResult = 3;
    if (strfind(message, "Disparos:") == 0)
    {
        TestResultShots = getarg(3);
        TestResultHits = getarg(4);
        TestResultExpired = getarg(6);
    }
    return 1;
}
stock TestIsConnected(playerid) { return playerid >= 0 && playerid < 2 && TestConnected[playerid]; }
stock TestIsNPC(playerid) { return 0; }
stock TestIsAdmin(playerid) { return TestAdmin[playerid]; }
stock TestGetState(playerid) { return TestState[playerid]; }
stock TestGetWeapon(playerid) { return TestWeapon[playerid]; }
stock TestGetWorld(playerid) { return TestWorld[playerid]; }
stock TestGetInterior(playerid) { return TestInterior[playerid]; }
stock TestGetHealth(playerid, &Float:health) { health = TestHealth[playerid]; return 1; }
stock TestGetPos(playerid, &Float:x, &Float:y, &Float:z)
{
    x = TestPosition[playerid][0]; y = TestPosition[playerid][1]; z = TestPosition[playerid][2]; return 1;
}
stock TestGetAngle(playerid, &Float:angle) { angle = 90.0; return 1; }
stock TestGetWeaponData(playerid, slot, &weaponid, &ammo)
{
    weaponid = 24; ammo = TestAmmo[playerid]; return 1;
}
stock TestInRange(playerid, Float:range, Float:x, Float:y, Float:z)
{
    x -= TestPosition[playerid][0]; y -= TestPosition[playerid][1]; z -= TestPosition[playerid][2];
    return x*x + y*y + z*z <= range*range;
}
stock TestGetTick() { return TestNow; }
stock TestCreateObject(modelid, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz, worldid, interiorid)
{
    if (TestCreateFails) return 0;
    TestObjects[++TestNextObject] = true;
    TestLiveObjects++;
    return TestNextObject;
}
stock TestObjectValid(objectid) { return objectid > 0 && objectid < sizeof(TestObjects) && TestObjects[objectid]; }
stock TestDestroyObject(objectid)
{
    if (TestObjectValid(objectid)) { TestObjects[objectid] = false; TestLiveObjects--; }
    return 1;
}
stock TestStreamerUpdate(playerid, type) { return 1; }
stock TestObjectVisible(playerid, type, objectid) { return TestVisible && TestObjectValid(objectid); }
stock TestStreamerID(playerid, type, internalid)
{
    new id = internalid - 1000 - playerid * 100;
    return TestObjectValid(id) ? id : 0;
}

#define SendClientMessage TestSendMessage
#define IsPlayerConnected TestIsConnected
#define IsPlayerNPC TestIsNPC
#define IsPlayerAdmin TestIsAdmin
#define GetPlayerState TestGetState
#define GetPlayerWeapon TestGetWeapon
#define GetPlayerVirtualWorld TestGetWorld
#define GetPlayerInterior TestGetInterior
#define GetPlayerHealth TestGetHealth
#define GetPlayerPos TestGetPos
#define GetPlayerFacingAngle TestGetAngle
#define GetPlayerWeaponData TestGetWeaponData
#define IsPlayerInRangeOfPoint TestInRange
#define GetTickCount TestGetTick
#define CreateDynamicObject TestCreateObject
#define IsValidDynamicObject TestObjectValid
#define DestroyDynamicObject TestDestroyObject
#define Streamer_Update TestStreamerUpdate
#define Streamer_IsItemVisible TestObjectVisible
#define Streamer_GetItemStreamerID TestStreamerID

#include "../gamemodes/features/shooting_practice/config.inc"
#include "../gamemodes/features/shooting_practice/storage.inc"
#include "../gamemodes/features/shooting_practice/session.inc"
#include "../gamemodes/features/shooting_practice/setup.inc"

stock Check(bool:condition, const label[])
{
    TestAssertions++;
    if (!condition) { TestFailures++; printf("[Practice tests] FAIL: %s", label); }
}
stock ResetTest(targets = 3)
{
    PracticeCleanup();
    Check(TestLiveObjects == 0, "cleanup leaves no objects");
    new empty[E_PRACTICE_POINT];
    for (new slot = 0; slot <= PRACTICE_MAX_TARGETS; slot++)
    {
        PracticePoints[slot] = empty;
        if (slot <= targets)
        {
            PracticePoints[slot][PracticePointSet] = true;
            PracticePoints[slot][PracticePointX] = float(slot * 8);
        }
    }
    new defaults[E_PRACTICE_RULE] = {24, 1, 3, 5, 1};
    PracticeRules = defaults;
    for (new playerid = 0; playerid < 2; playerid++)
    {
        TestConnected[playerid] = true; TestAdmin[playerid] = true;
        TestState[playerid] = PLAYER_STATE_ONFOOT; TestWeapon[playerid] = 24; TestAmmo[playerid] = 30;
        TestWorld[playerid] = 0; TestInterior[playerid] = 0; TestHealth[playerid] = 100.0;
        TestPosition[playerid][0] = TestPosition[playerid][1] = TestPosition[playerid][2] = 0.0;
    }
    TestNow = 1000; TestNextObject = 0; TestCreateFails = false; TestVisible = true;
    TestResult = 0; TestResultShots = 0; TestResultHits = 0; TestResultExpired = 0;
}
stock ShowFirstTarget()
{
    PracticeStart(0);
    TestNow += PracticeRules[PracticePreparation] * 1000;
    PracticeTick(TestNow);
    Check(PracticeSession[PracticePhase] == PRACTICE_TARGET, "first target appears after countdown");
}
stock Shoot(bool:hit, playerid = 0)
{
    PracticeHandleShot(playerid, 24, hit ? BULLET_HIT_TYPE_PLAYER_OBJECT : BULLET_HIT_TYPE_NONE,
        1000 + playerid * 100 + PracticeSession[PracticeObject], TestNow);
}
stock AdvanceInterval()
{
    TestNow += PracticeRules[PracticeInterval] * 1000;
    PracticeTick(TestNow);
}

main() {}
public OnGameModeInit()
{
    // Valid starts, occupancy, exact shot accounting and one point per presentation.
    ResetTest();
    PracticeStart(0);
    Shoot(false);
    Check(PracticeSession[PracticeShots] == 0, "countdown shots excluded");
    PracticeStart(1);
    Check(PracticeSession[PracticePlayer] == 0, "second player cannot steal station");
    PracticeStart(0);
    Check(PracticeSession[PracticePhaseStarted] == 1000, "duplicate start does not restart countdown");
    TestNow = 4000; PracticeTick(TestNow);
    Shoot(true, 1);
    Check(PracticeSession[PracticeHits] == 0 && TestLiveObjects == 1, "bystander cannot score or retire target");
    Shoot(false); Shoot(true); Shoot(true);
    Check(PracticeSession[PracticeHits] == 1 && PracticeSession[PracticeShots] == 3, "repeat hit during gap adds no point");
    Check(TestLiveObjects == 0, "hit removes object");
    AdvanceInterval();
    TestNow += 5000; PracticeTick(TestNow);
    Check(PracticeSession[PracticeExpired] == 1, "timeout counts one omission");
    AdvanceInterval(); Shoot(true);
    Check(!PracticeIsBusy() && TestLiveObjects == 0, "completion releases station and object");
    Check(TestResult == 1 && TestResultShots == 4 && TestResultHits == 2 && TestResultExpired == 1, "final score and counters");
    PracticeStart(1);
    Check(PracticeSession[PracticePlayer] == 1, "station immediately reusable");

    ResetTest(1); ShowFirstTarget();
    TestNow += 5000; Shoot(true);
    Check(TestResult == 2 && TestResultShots == 0 && TestResultHits == 0 && TestResultExpired == 1, "late final hit expires without division by zero");
    ResetTest(2); ShowFirstTarget();
    TestNow += 5000; Shoot(true);
    Check(PracticeSession[PracticeHits] == 0 && PracticeSession[PracticeExpired] == 1
        && PracticeSession[PracticeShots] == 1, "late hit before next target is a gap miss");
    AdvanceInterval();
    Check(PracticeSession[PracticePresented] == 2, "timeout and shot cannot skip a presentation");

    ResetTest(10); ShowFirstTarget();
    for (new i = 0; i < 10; i++) { Shoot(true); if (i < 9) AdvanceInterval(); }
    Check(TestResultHits == 10 && TestResultShots == 10 && TestResultExpired == 0, "ten target sequence");
    ResetTest(); PracticePoints[2][PracticePointSet] = false;
    ShowFirstTarget(); Shoot(true); AdvanceInterval();
    Check(PracticeSession[PracticeSlot] == 3 && PracticeSession[PracticeTotal] == 2, "sparse slots keep numeric order");
    ResetTest(1); PracticeRules[PracticeInterval] = 0; ShowFirstTarget(); Shoot(true);
    Check(!PracticeIsBusy(), "zero interval still finishes last target");
    ResetTest(2); PracticeRules[PracticeInterval] = 0; ShowFirstTarget(); Shoot(true);
    Check(TestLiveObjects == 0, "zero interval does not spawn inside the shot callback");
    PracticeTick(TestNow); Shoot(true);
    Check(TestResult == 1 && TestResultHits == 2, "zero interval advances on the next tick");
    ResetTest(3); PracticeRules[PracticePassPoints] = 3; ShowFirstTarget(); Shoot(true);
    AdvanceInterval(); Shoot(true); AdvanceInterval(); TestNow += 5000; PracticeTick(TestNow);
    Check(TestResult == 2 && TestResultHits == 2, "passing threshold is enforced");

    // Invalid entry and every cleanup route used by the lifecycle hooks.
    ResetTest(); TestPosition[0][0] = 2.0; PracticeStart(0);
    Check(!PracticeIsBusy(), "reject start outside radius");
    TestPosition[0][0] = 0.0; TestWorld[0] = 1; PracticeStart(0);
    Check(!PracticeIsBusy(), "reject wrong world");
    TestWorld[0] = 0; TestWeapon[0] = 23; PracticeStart(0);
    Check(!PracticeIsBusy(), "reject wrong weapon");
    TestWeapon[0] = 24; TestState[0] = PLAYER_STATE_DRIVER; PracticeStart(0);
    Check(!PracticeIsBusy(), "reject vehicle");
    ResetTest(); ShowFirstTarget(); TestPosition[0][0] = 4.0; PracticeTick(TestNow);
    Check(!PracticeIsBusy() && TestResult == 3 && TestLiveObjects == 0, "leaving cancels and cleans");
    ResetTest(); ShowFirstTarget(); TestInterior[0] = 1; PracticeTick(TestNow);
    Check(!PracticeIsBusy() && TestResult == 3, "changing interior cancels");
    ResetTest(); ShowFirstTarget(); TestWorld[0] = 1; PracticeTick(TestNow);
    Check(!PracticeIsBusy(), "changing world cancels");
    ResetTest(); ShowFirstTarget(); TestWeapon[0] = 23; PracticeTick(TestNow);
    Check(!PracticeIsBusy(), "changing weapon cancels");
    ResetTest(); ShowFirstTarget(); TestHealth[0] = 0.0; PracticeTick(TestNow);
    Check(!PracticeIsBusy(), "death cancels");
    ResetTest(); ShowFirstTarget(); TestConnected[0] = false; PracticeTick(TestNow);
    Check(!PracticeIsBusy() && TestLiveObjects == 0, "disconnect cleans");
    TestConnected[0] = true; PracticeStart(0);
    Check(PracticeSession[PracticeShots] == 0 && PracticeSession[PracticePresented] == 0, "reused player ID has fresh session");
    ResetTest(); ShowFirstTarget(); TestCommand_practica(1, "cancelar");
    Check(PracticeIsBusy(), "bystander cannot cancel");
    TestCommand_practica(0, "cancelar");
    Check(!PracticeIsBusy() && TestResult == 3, "explicit cancellation is incomplete");
    ResetTest(1); ShowFirstTarget(); TestAmmo[0] = 0; TestWeapon[0] = 0;
    PracticeTick(TestNow); Check(PracticeIsBusy(), "empty ammo does not cancel");
    TestNow += 5000; PracticeTick(TestNow);
    Check(TestResult == 2 && TestResultExpired == 1, "empty ammo attempt expires normally");
    ResetTest(); TestCreateFails = true; PracticeStart(0); TestNow += 3000; PracticeTick(TestNow);
    Check(!PracticeIsBusy() && TestResult == 3 && TestLiveObjects == 0, "creation failure releases station");
    ResetTest(); TestVisible = false; PracticeStart(0); TestNow += 3000; PracticeTick(TestNow);
    Check(!PracticeIsBusy() && TestResult == 3 && TestLiveObjects == 0, "streaming failure cleans object");
    ResetTest(); ShowFirstTarget(); TestDestroyObject(PracticeSession[PracticeObject]); PracticeTick(TestNow);
    Check(!PracticeIsBusy() && TestResult == 3, "lost object marks attempt incomplete");
    ResetTest(1); TestNow = 2147483000; ShowFirstTarget(); TestNow += 5000; PracticeTick(TestNow);
    Check(TestResult == 2, "clock wraparound preserves deadlines");

    // Real sscanf command parsing, admin checks, bounded configuration and persistence.
    ResetTest(); TestAdmin[0] = false; TestCommand_practicaconfig(0, "tiempo 9");
    Check(PracticeRules[PracticeExposure] == 5, "non admin cannot configure");
    TestAdmin[0] = true; TestCommand_practicaconfig(0, "tiempo 9");
    Check(PracticeRules[PracticeExposure] == 9, "time command updates exposure");
    TestCommand_practicaconfig(0, "tiempo 0"); TestCommand_practicaconfig(0, "tiempo 61");
    Check(PracticeRules[PracticeExposure] == 9, "invalid exposure rejected");
    TestCommand_practicaconfig(0, "preparacion 2"); TestCommand_practicaconfig(0, "intervalo 0");
    TestCommand_practicaconfig(0, "arma 23"); TestCommand_practicaconfig(0, "aprobar 2");
    Check(PracticeRules[PracticePreparation] == 2 && PracticeRules[PracticeInterval] == 0
        && PracticeRules[PracticeWeapon] == 23 && PracticeRules[PracticePassPoints] == 2, "rule commands");
    TestCommand_practicaconfig(0, "arma 35"); TestCommand_practicaconfig(0, "aprobar 4");
    Check(PracticeRules[PracticeWeapon] == 23 && PracticeRules[PracticePassPoints] == 2, "unsupported weapon and impossible pass rejected");
    TestPosition[0][0] = 15.0;
    TestCommand_practicaconfig(0, "blanco 10");
    Check(PracticePoints[10][PracticePointSet] && PracticePoints[10][PracticePointX] == 15.0, "tenth position captured");
    TestCommand_practicaconfig(0, "blanco 11"); TestCommand_practicaconfig(0, "blanco 0");
    Check(PracticeCountTargets() == 4, "out of bounds slots rejected");
    TestCommand_practicaconfig(0, "eliminar 2");
    Check(!PracticePoints[2][PracticePointSet], "slot deletion");
    Check(PracticeSaveConfiguration(), "configuration writes successfully");
    PracticeRules[PracticeExposure] = 1; PracticePoints[10][PracticePointSet] = false;
    Check(PracticeLoadConfiguration() && PracticeRules[PracticeExposure] == 9
        && PracticePoints[10][PracticePointSet] && !PracticePoints[2][PracticePointSet], "configuration round trip preserves sparse layout and rules");
    new File:bad = fopen(PRACTICE_CONFIG_FILE, io_write);
    fwrite(bad, "SARP_PRACTICE 1\r\n24 1 3 5 1\r\n"); fclose(bad);
    Check(!PracticeLoadConfiguration() && PracticeRules[PracticeExposure] == 9, "truncated file never partially applied");
    Check(PracticeSaveConfiguration(), "save repairs truncated file");
    bad = fopen(PRACTICE_CONFIG_FILE, io_readwrite);
    fwrite(bad, "SARP_PRACTICE 9\r\n"); fclose(bad);
    Check(!PracticeLoadConfiguration(), "unsupported storage version rejected");
    ResetTest(); ShowFirstTarget(); TestCommand_practicaconfig(0, "tiempo 9");
    TestCommand_practicaconfig(0, "eliminar 1"); TestCommand_practicaconfig(0, "inicio");
    Check(PracticeRules[PracticeExposure] == 5 && PracticePoints[1][PracticePointSet], "busy layout is immutable");
    ResetTest(); TestWorld[0] = 1; TestCommand_practicaconfig(0, "inicio");
    Check(PracticeCountTargets() == 0 && PracticePoints[0][PracticePointWorld] == 1, "changing dimension invalidates targets");
    TestCommand_practicaconfig(0, "blanco 1");
    Check(PracticeCountTargets() == 0, "target at shooter rejected");
    TestPosition[0][0] = 8.0; TestWorld[0] = 2; TestCommand_practicaconfig(0, "blanco 1");
    Check(PracticeCountTargets() == 0, "target in wrong dimension rejected");
    PracticeCleanup();
    printf("[Practice tests] %d assertions, %d failures", TestAssertions, TestFailures);
    if (!TestFailures) print("[Practice tests] PASS");
    new callback[] = "PracticeTestExit";
    SetTimer(callback, 100, false);
    return 1;
}
forward PracticeTestExit();
public PracticeTestExit()
{
    new command[] = "exit";
    SendRconCommand(command);
    return 1;
}
