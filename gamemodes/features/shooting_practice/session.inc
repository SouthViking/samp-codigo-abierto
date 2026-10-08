#if defined SP_SESSION_INCLUDED
    #endinput
#endif
#define SP_SESSION_INCLUDED

enum E_PRACTICE_PHASE
{
    PRACTICE_IDLE,
    PRACTICE_COUNTDOWN,
    PRACTICE_TARGET,
    PRACTICE_INTERVAL
}

enum E_PRACTICE_SESSION
{
    E_PRACTICE_PHASE:PracticePhase,
    PracticePlayer,
    STREAMER_TAG_OBJECT:PracticeObject,
    PracticeSlot,
    PracticeTotal,
    PracticeShootTotal,
    PracticeNoShootTotal,
    PracticeRequiredPoints,
    PracticeAllowedErrors,
    PracticeAllowedWeapons,
    PracticeLastWeapon,
    bool:PracticeNoShoot,
    PracticePresented,
    PracticeShots,
    PracticeHits,
    PracticeExpired,
    PracticeCorrectIgnores,
    PracticeIdentificationErrors,
    PracticePhaseStarted,
    PracticePhaseDuration,
    PracticeLastCountdown,
    PracticeCountdownCueAt
}

new PracticeSession[E_PRACTICE_SESSION];
new PracticeSequenceSlots[PRACTICE_MAX_TARGETS];
new bool:PracticeSequenceNoShoot[PRACTICE_MAX_TARGETS];
new PracticeSequenceModels[PRACTICE_MAX_TARGETS];

stock bool:PracticeIsBusy()
{
    return PracticeSession[PracticePhase] != PRACTICE_IDLE;
}

// Integration point for future faction/business permissions.
stock bool:PracticeCanUseStation(playerid)
{
    return !IsPlayerNPC(playerid);
}

stock PracticeRemoveTarget()
{
    new STREAMER_TAG_OBJECT:objectid = PracticeSession[PracticeObject];
    PracticeSession[PracticeObject] = INVALID_STREAMER_ID;
    if (objectid != INVALID_STREAMER_ID && IsValidDynamicObject(objectid))
        DestroyDynamicObject(objectid);
    return 1;
}

stock PracticeCleanup()
{
    PracticeHideCountdown(PracticeSession[PracticePlayer]);
    PracticeRemoveTarget();
    new empty[E_PRACTICE_SESSION];
    PracticeSession = empty;
    PracticeSession[PracticePlayer] = INVALID_PLAYER_ID;
    for (new index = 0; index < PRACTICE_MAX_TARGETS; index++)
    {
        PracticeSequenceSlots[index] = 0;
        PracticeSequenceNoShoot[index] = false;
        PracticeSequenceModels[index] = 0;
    }
    return 1;
}

stock PracticeResultPoints(const result[E_PRACTICE_SESSION])
{
    return result[PracticeHits] - result[PracticeIdentificationErrors];
}

stock bool:PracticeResultPassed(const result[E_PRACTICE_SESSION])
{
    return PracticeResultPoints(result) >= result[PracticeRequiredPoints]
        && result[PracticeIdentificationErrors] <= result[PracticeAllowedErrors];
}

stock PracticeEnd(bool:completed, const reason[] = "", bool:notify = true, bool:abandoned = false)
{
    if (!PracticeIsBusy()) return 1;
    new result[E_PRACTICE_SESSION];
    result = PracticeSession;
    PracticeCleanup();
    new playerid = result[PracticePlayer];
    PracticePresentEnd(completed, playerid, abandoned);
    if (notify && IsPlayerConnected(playerid))
        PracticeReportResult(playerid, completed, result, reason);
    return 1;
}

stock PracticeWeaponSlot(weaponid)
{
    switch (weaponid)
    {
        case 22..24: return 2;
        case 25..27: return 3;
        case 28, 29, 32: return 4;
        case 30, 31: return 5;
        case 33, 34: return 6;
    }
    return 0;
}

stock bool:PracticeParticipantValid()
{
    new playerid = PracticeSession[PracticePlayer];
    if (!IsPlayerConnected(playerid))
    {
        PracticeEnd(false, "desconexión", false);
        return false;
    }
    new Float:health;
    GetPlayerHealth(playerid, health);
    if (health <= 0.0 || GetPlayerState(playerid) != PLAYER_STATE_ONFOOT)
    {
        PracticeEnd(false, "debes permanecer vivo y a pie.");
        return false;
    }
    if (GetPlayerInterior(playerid) != PracticePoints[0][PracticePointInterior]
        || GetPlayerVirtualWorld(playerid) != PracticePoints[0][PracticePointWorld]
        || !IsPlayerInRangeOfPoint(playerid, PRACTICE_ACTIVE_RADIUS, PracticePoints[0][PracticePointX],
            PracticePoints[0][PracticePointY], PracticePoints[0][PracticePointZ]))
    {
        PracticeEnd(false, "Cancelada automáticamente por salir del puesto de tiro.", true, true);
        return false;
    }
    new heldWeapon = GetPlayerWeapon(playerid);
    if (PracticeWeaponAllowed(heldWeapon, PracticeSession[PracticeAllowedWeapons]))
        PracticeSession[PracticeLastWeapon] = heldWeapon;
    else
    {
        new weaponid, ammo, lastWeapon = PracticeSession[PracticeLastWeapon];
        GetPlayerWeaponData(playerid, PracticeWeaponSlot(lastWeapon), weaponid, ammo);
        // SA-MP may switch to fists on exhaustion. Check the last permitted
        // weapon's slot, including when the player switched weapons mid-round.
        if (heldWeapon != 0 || ammo > 0 || (weaponid != 0 && weaponid != lastWeapon))
        {
            PracticeEnd(false, "cambiaste a un arma no permitida.");
            return false;
        }
    }
    if (PracticeSession[PracticePhase] == PRACTICE_TARGET
        && !IsValidDynamicObject(PracticeSession[PracticeObject]))
    {
        PracticeEnd(false, "el blanco activo ya no existe.");
        return false;
    }
    return true;
}

stock PracticeShowNextTarget(now)
{
    new index = PracticeSession[PracticePresented];
    if (index >= PracticeSession[PracticeTotal])
        return PracticeEnd(false, "no se encontró la siguiente posición.");
    new slot = PracticeSequenceSlots[index];

    new playerid = PracticeSession[PracticePlayer];
    new STREAMER_TAG_OBJECT:objectid = CreateDynamicObject(PracticeSequenceModels[index],
        PracticePoints[slot][PracticePointX], PracticePoints[slot][PracticePointY],
        PracticePoints[slot][PracticePointZ] + PRACTICE_TARGET_Z_OFFSET,
        PracticePoints[slot][PracticePointRotX], PracticePoints[slot][PracticePointRotY], PracticePoints[slot][PracticePointAngle],
        PracticePoints[0][PracticePointWorld], PracticePoints[0][PracticePointInterior]);
    PracticeSession[PracticeObject] = objectid;
    if (objectid == INVALID_STREAMER_ID || !IsValidDynamicObject(objectid))
        return PracticeEnd(false, "no se pudo crear el blanco.");
    if (!PracticeApplyTargetAppearance(objectid, PracticeSequenceNoShoot[index], PracticeSequenceModels[index]))
        return PracticeEnd(false, "no se pudo identificar visualmente el blanco.");

    Streamer_Update(playerid, STREAMER_TYPE_OBJECT);
    if (!Streamer_IsItemVisible(playerid, STREAMER_TYPE_OBJECT, objectid))
        return PracticeEnd(false, "el blanco no pudo mostrarse. Revisa la posición y el límite de objetos.");

    PracticeHideCountdown(playerid);
    PracticeSession[PracticeSlot] = slot;
    PracticeSession[PracticeNoShoot] = PracticeSequenceNoShoot[index];
    PracticeSession[PracticePresented]++;
    PracticeSession[PracticePhase] = PRACTICE_TARGET;
    PracticeSession[PracticePhaseStarted] = now;
    PracticeSession[PracticePhaseDuration] = PracticeRules[PracticeExposure];
    // Sound only after creation, markings and streaming have succeeded. The first
    // appearance is also the start signal, with no duplicate cue or type hint.
    PracticePresentTarget();
    return 1;
}

stock PracticeResolveTarget(bool:byShot, now)
{
    if (PracticeSession[PracticePhase] != PRACTICE_TARGET) return 1;
    // Retire the phase before destroying the object, so it can resolve only once.
    PracticeSession[PracticePhase] = PRACTICE_INTERVAL;
    PracticeRemoveTarget();
    if (PracticeSession[PracticeNoShoot])
    {
        if (byShot) PracticeSession[PracticeIdentificationErrors]++;
        else PracticeSession[PracticeCorrectIgnores]++;
    }
    else if (byShot)
    {
        PracticeSession[PracticeHits]++;
    }
    else
    {
        PracticeSession[PracticeExpired]++;
    }
    if (PracticeSession[PracticePresented] == PracticeSession[PracticeTotal])
        return PracticeEnd(true);
    PracticeSession[PracticePhaseStarted] = now;
    PracticeSession[PracticePhaseDuration] = PracticeRules[PracticeInterval];
    return 1;
}

stock PracticeTick(now)
{
    if (!PracticeIsBusy() || !PracticeParticipantValid()) return 1;
    // Subtraction remains valid across GetTickCount's signed wraparound.
    new elapsed = now - PracticeSession[PracticePhaseStarted];
    if (PracticeSession[PracticePhase] == PRACTICE_COUNTDOWN
        && now - PracticeSession[PracticeCountdownCueAt] < 1000) return 1;
    if (elapsed >= PracticeSession[PracticePhaseDuration])
    {
        switch (PracticeSession[PracticePhase])
        {
            case PRACTICE_COUNTDOWN, PRACTICE_INTERVAL: PracticeShowNextTarget(now);
            case PRACTICE_TARGET: PracticeResolveTarget(false, now);
        }
    }
    else if (PracticeSession[PracticePhase] == PRACTICE_COUNTDOWN)
    {
        new remaining = (PracticeSession[PracticePhaseDuration] - elapsed + 999) / 1000;
        if (remaining != PracticeSession[PracticeLastCountdown])
        {
            PracticeSession[PracticeLastCountdown] = remaining;
            // Keep a full second between actual cues, including the start signal.
            // Delayed ticks show the current value without replaying missed beeps.
            PracticeSession[PracticeCountdownCueAt] = now;
            PracticePresentCountdown();
        }
    }
    return 1;
}

stock PracticeHandleShot(playerid, weaponid, hittype, hitid, now)
{
    if (!PracticeIsBusy() || playerid != PracticeSession[PracticePlayer]) return 1;
    if (!PracticeParticipantValid()) return 1;
    if (!PracticeWeaponAllowed(weaponid, PracticeSession[PracticeAllowedWeapons]))
        return PracticeEnd(false, "disparaste con un arma no permitida.");
    if (PracticeSession[PracticePhase] == PRACTICE_COUNTDOWN) return 1;

    // Enforce the deadline even between timer ticks. Never create a new target
    // from a shot callback: a late hit must not be credited to a reused object ID.
    if (PracticeSession[PracticePhase] == PRACTICE_TARGET
        && now - PracticeSession[PracticePhaseStarted] >= PracticeSession[PracticePhaseDuration])
        PracticeResolveTarget(false, now);
    if (!PracticeIsBusy()) return 1;

    PracticeSession[PracticeShots]++;
    if (PracticeSession[PracticePhase] == PRACTICE_TARGET)
    {
        // A no-shoot presentation tests trigger restraint: the first discharge
        // resolves it as an error even if it hits air, scenery or another object.
        if (PracticeSession[PracticeNoShoot]) PracticeResolveTarget(true, now);
        else if (hittype == BULLET_HIT_TYPE_PLAYER_OBJECT
            && Streamer_GetItemStreamerID(playerid, STREAMER_TYPE_OBJECT, hitid) == PracticeSession[PracticeObject])
            PracticeResolveTarget(true, now);
    }
    return 1;
}

stock bool:PracticeStartReady(playerid, reason[], size = sizeof(reason))
{
    if (!IsPlayerConnected(playerid)) format(reason, size, "El participante no está conectado.");
    else if (PracticeConfigurationEditing()) format(reason, size, "Espera a que el administrador termine de editar el puesto.");
    else if (PracticeIsBusy())
    {
        if (PracticeSession[PracticePlayer] == playerid)
            format(reason, size, "Ya tienes una práctica activa. Usa /practica cancelar.");
        else format(reason, size, "El puesto de tiro está ocupado.");
    }
    else if (!PracticeConfigurationReady(reason, size)) return false;
    else if (!PracticeCanUseStation(playerid)) format(reason, size, "No tienes acceso a este puesto de tiro.");
    else
    {
        new Float:health;
        GetPlayerHealth(playerid, health);
        if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT || health <= 0.0)
            format(reason, size, "Debes estar vivo y a pie para iniciar.");
        else if (GetPlayerInterior(playerid) != PracticePoints[0][PracticePointInterior]
            || GetPlayerVirtualWorld(playerid) != PracticePoints[0][PracticePointWorld]
            || !IsPlayerInRangeOfPoint(playerid, PRACTICE_START_RADIUS, PracticePoints[0][PracticePointX],
                PracticePoints[0][PracticePointY], PracticePoints[0][PracticePointZ]))
            format(reason, size, "Acércate al punto de inicio del puesto de tiro.");
        else if (!PracticeWeaponAllowed(GetPlayerWeapon(playerid), PracticeRules[PracticeWeaponMask]))
            format(reason, size, "Equipa una de las armas permitidas antes de iniciar.");
        else { reason[0] = EOS; return true; }
    }
    return false;
}

stock bool:PracticeCanStart(playerid)
{
    new reason[144];
    return PracticeStartReady(playerid, reason);
}

stock PracticeStart(playerid)
{
    new reason[144];
    if (!PracticeStartReady(playerid, reason))
        return SendClientMessage(playerid, COLOR_FADE3, reason);

    PracticeCleanup();
    PracticeHideAllPreviews();
    PracticeSession[PracticePlayer] = playerid;
    PracticeSession[PracticeTotal] = PracticeBuildSequence(PracticeSequenceSlots, PracticeSequenceNoShoot, PracticeSequenceModels);
    PracticeSession[PracticeNoShootTotal] = PracticeCountNoShootTargets();
    PracticeSession[PracticeShootTotal] = PracticeCountShootTargets();
    PracticeSession[PracticeRequiredPoints] = PracticeRules[PracticePassPoints];
    PracticeSession[PracticeAllowedErrors] = PracticeRules[PracticeMaxErrors];
    PracticeSession[PracticeAllowedWeapons] = PracticeRules[PracticeWeaponMask];
    PracticeSession[PracticeLastWeapon] = GetPlayerWeapon(playerid);
    PracticeSession[PracticePhase] = PRACTICE_COUNTDOWN;
    PracticeSession[PracticePhaseStarted] = GetTickCount();
    PracticeSession[PracticePhaseDuration] = PracticeRules[PracticePreparation] * 1000;
    PracticeSession[PracticeLastCountdown] = PracticeRules[PracticePreparation];
    PracticeSession[PracticeCountdownCueAt] = PracticeSession[PracticePhaseStarted];
    PracticePresentStart(playerid);
    return 1;
}

CMD:practica(playerid, const params[])
{
    if (isnull(params)) return PracticeStart(playerid);
    if (!strcmp(params, "ayuda", true)) return PracticeShowPlayerHelp(playerid);
    if (!strcmp(params, "cancelar", true))
    {
        if (!PracticeIsBusy() || PracticeSession[PracticePlayer] != playerid)
            return SendClientMessage(playerid, COLOR_FADE3, "No tienes una práctica activa.");
        return PracticeEnd(false, "cancelada por el participante.");
    }
    return SendClientMessage(playerid, COLOR_GRAD1, "Usa /practica, /practica cancelar o /practica ayuda.");
}
