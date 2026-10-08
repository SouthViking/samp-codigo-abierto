#if defined SP_HOOKS_INCLUDED
    #endinput
#endif
#define SP_HOOKS_INCLUDED

#include <YSI_Coding\y_hooks>

new PracticePulseTimer;
forward PracticePulse();
public PracticePulse()
{
    PracticeRepairPreviews();
    PracticeTick(GetTickCount());
    return 1;
}

hook OnGameModeInit()
{
    PracticeCleanup();
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++) PracticeResetUI(playerid);
    PracticeLoadConfiguration();
    // One gamemode-owned pulse polls real deadlines. SA-MP timer delays never
    // shorten countdown pauses, exposure or decimal intervals.
    PracticePulseTimer = SetTimer("PracticePulse", 50, true);
    return 1;
}

hook OnGameModeExit()
{
    if (PracticePulseTimer) KillTimer(PracticePulseTimer);
    PracticePulseTimer = 0;
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++) PracticeResetUI(playerid);
    PracticeCleanup();
    return 1;
}

// Editor validity checks retain the shared 500 ms maintenance cycle.
hook Task500()
{
    PracticeEditorTick();
    return 1;
}

// YSI expands Dyn to Dynamic. Preview hits never enter the scoring callback.
hook OnPlayerShootDynObject(playerid, weaponid, objectid, Float:x, Float:y, Float:z)
{
    if (PracticeProtectPreviewShot(playerid, objectid)) return ~0;
    return 1;
}

hook OnPlayerWeaponShot(playerid, weaponid, hittype, hitid, Float:x, Float:y, Float:z)
{
    // Count and score once in the same callback. Streamer's dynamic-object
    // callback is deliberately not also counted (it fires for the same shot).
    PracticeHandleShot(playerid, weaponid, hittype, hitid, GetTickCount());
    return 1;
}

hook OnPlayerUpdate(playerid)
{
    if (PracticeIsBusy() && PracticeSession[PracticePlayer] == playerid)
        PracticeParticipantValid();
    return 1;
}

hook OnPlayerDeath(playerid, killerid, reason)
{
    PracticeResetUI(playerid);
    if (PracticeIsBusy() && PracticeSession[PracticePlayer] == playerid)
        PracticeEnd(false, "el participante murió.");
    return 1;
}

hook OnPlayerDisconnect(playerid, reason)
{
    PracticeResetUI(playerid);
    if (PracticeIsBusy() && PracticeSession[PracticePlayer] == playerid)
        PracticeEnd(false, "desconexión", false);
    return 1;
}

hook OnPlayerStateChange(playerid, newstate, oldstate)
{
    if (newstate != PLAYER_STATE_ONFOOT) PracticeResetUI(playerid);
    if (PracticeIsBusy() && PracticeSession[PracticePlayer] == playerid && newstate != PLAYER_STATE_ONFOOT)
        PracticeEnd(false, "debes permanecer a pie.");
    return 1;
}

hook OnPlayerConnect(playerid)
{
    PracticeHideCountdown(playerid);
    PracticeResetUI(playerid);
    return 1;
}

// YSI expands Dyn to Dynamic; the shorter name leaves room for its unique suffix.
hook OnPlayerEditDynObject(playerid, objectid, response, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz)
{
    PracticeHandleObjectEdit(playerid, objectid, response, x, y, z, rx, ry, rz);
    return 1;
}

hook OnPlayerModelSelectionEx(playerid, response, extraid, modelid)
{
    if (extraid == PRACTICE_CATALOGUE_ID) PracticeHandleModelSelection(playerid, response, modelid);
    return 1;
}
