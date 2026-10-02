#if defined SP_HOOKS_INCLUDED
    #endinput
#endif
#define SP_HOOKS_INCLUDED

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    PracticeCleanup();
    PracticeLoadConfiguration();
    return 1;
}

hook OnGameModeExit()
{
    PracticeCleanup();
    return 1;
}

// The shared tick owns all transitions. There are no per-player timers to
// outlive a session, disconnect, or player ID reuse.
hook Task500()
{
    PracticeTick(GetTickCount());
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
    if (PracticeIsBusy() && PracticeSession[PracticePlayer] == playerid)
        PracticeEnd(false, "el participante murio.");
    return 1;
}

hook OnPlayerDisconnect(playerid, reason)
{
    if (PracticeIsBusy() && PracticeSession[PracticePlayer] == playerid)
        PracticeEnd(false, "desconexion", false);
    return 1;
}

hook OnPlayerStateChange(playerid, newstate, oldstate)
{
    if (PracticeIsBusy() && PracticeSession[PracticePlayer] == playerid && newstate != PLAYER_STATE_ONFOOT)
        PracticeEnd(false, "debes permanecer a pie.");
    return 1;
}
