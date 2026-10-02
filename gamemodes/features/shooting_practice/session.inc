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
    PracticePresented,
    PracticeShots,
    PracticeHits,
    PracticeExpired,
    PracticePhaseStarted,
    PracticePhaseDuration,
    PracticeLastCountdown
}

new PracticeSession[E_PRACTICE_SESSION];

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
    PracticeRemoveTarget();
    new empty[E_PRACTICE_SESSION];
    PracticeSession = empty;
    PracticeSession[PracticePlayer] = INVALID_PLAYER_ID;
    return 1;
}

stock PracticeEnd(bool:completed, const reason[] = "", bool:notify = true)
{
    if (!PracticeIsBusy()) return 1;
    new result[E_PRACTICE_SESSION];
    result = PracticeSession;
    PracticeCleanup();

    new playerid = result[PracticePlayer];
    if (!notify || !IsPlayerConnected(playerid)) return 1;
    new Float:accuracy;
    if (result[PracticeShots])
        accuracy = 100.0 * float(result[PracticeHits]) / float(result[PracticeShots]);

    if (completed)
    {
        if (result[PracticeHits] >= PracticeRules[PracticePassPoints])
            SendClientMessage(playerid, COLOR_LIGHTGREEN, "[Practica] APROBADO.");
        else
            SendClientMessage(playerid, COLOR_LIGHTORANGE, "[Practica] REPROBADO.");
        va_SendClientMessage(playerid, COLOR_GRAD1, "Puntos: %d/%d | Minimo para aprobar: %d.",
            result[PracticeHits], result[PracticeTotal], PracticeRules[PracticePassPoints]);
    }
    else
        va_SendClientMessage(playerid, COLOR_LIGHTORANGE, "[Practica] INCOMPLETA: %s", reason);

    va_SendClientMessage(playerid, COLOR_GRAD1, "Disparos: %d | Aciertos: %d | Precision: %.1f%% | Omitidas: %d.",
        result[PracticeShots], result[PracticeHits], accuracy, result[PracticeExpired]);
    va_SendClientMessage(playerid, COLOR_GRAD1, "Blancos presentados: %d/%d. Puesto disponible para otro intento.",
        result[PracticePresented], result[PracticeTotal]);
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
        PracticeEnd(false, "desconexion", false);
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
        PracticeEnd(false, "saliste del puesto de tiro.");
        return false;
    }
    if (GetPlayerWeapon(playerid) != PracticeRules[PracticeWeapon])
    {
        new weaponid, ammo;
        GetPlayerWeaponData(playerid, PracticeWeaponSlot(PracticeRules[PracticeWeapon]), weaponid, ammo);
        // SA-MP may switch to fists on exhaustion. Empty ammunition does not end the attempt.
        if (GetPlayerWeapon(playerid) != 0 || ammo > 0)
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
    new slot = PracticeSession[PracticeSlot] + 1;
    while (slot <= PRACTICE_MAX_TARGETS && !PracticePoints[slot][PracticePointSet]) slot++;
    if (slot > PRACTICE_MAX_TARGETS)
        return PracticeEnd(false, "no se encontro la siguiente posicion.");

    new playerid = PracticeSession[PracticePlayer];
    new STREAMER_TAG_OBJECT:objectid = CreateDynamicObject(PRACTICE_TARGET_MODEL,
        PracticePoints[slot][PracticePointX], PracticePoints[slot][PracticePointY],
        PracticePoints[slot][PracticePointZ] + PRACTICE_TARGET_Z_OFFSET,
        0.0, 0.0, PracticePoints[slot][PracticePointAngle],
        PracticePoints[0][PracticePointWorld], PracticePoints[0][PracticePointInterior]);
    PracticeSession[PracticeObject] = objectid;
    if (objectid == INVALID_STREAMER_ID || !IsValidDynamicObject(objectid))
        return PracticeEnd(false, "no se pudo crear el blanco.");

    Streamer_Update(playerid, STREAMER_TYPE_OBJECT);
    if (!Streamer_IsItemVisible(playerid, STREAMER_TYPE_OBJECT, objectid))
        return PracticeEnd(false, "el blanco no pudo mostrarse. Revisa la posicion y el limite de objetos.");

    PracticeSession[PracticeSlot] = slot;
    PracticeSession[PracticePresented]++;
    PracticeSession[PracticePhase] = PRACTICE_TARGET;
    PracticeSession[PracticePhaseStarted] = now;
    PracticeSession[PracticePhaseDuration] = PracticeRules[PracticeExposure] * 1000;
    va_SendClientMessage(playerid, COLOR_LIGHTGREEN, "[Practica] Blanco %d/%d (posicion %d). Tienes %d segundos.",
        PracticeSession[PracticePresented], PracticeSession[PracticeTotal], slot, PracticeRules[PracticeExposure]);
    return 1;
}

stock PracticeResolveTarget(bool:hit, now)
{
    if (PracticeSession[PracticePhase] != PRACTICE_TARGET) return 1;
    // Retire the phase before destroying the object, so it can resolve only once.
    PracticeSession[PracticePhase] = PRACTICE_INTERVAL;
    PracticeRemoveTarget();
    if (hit)
    {
        PracticeSession[PracticeHits]++;
        va_SendClientMessage(PracticeSession[PracticePlayer], COLOR_LIGHTGREEN,
            "[Practica] Acierto. Puntos: %d.", PracticeSession[PracticeHits]);
    }
    else
    {
        PracticeSession[PracticeExpired]++;
        SendClientMessage(PracticeSession[PracticePlayer], COLOR_LIGHTORANGE, "[Practica] Tiempo agotado. Blanco omitido.");
    }
    if (PracticeSession[PracticePresented] == PracticeSession[PracticeTotal])
        return PracticeEnd(true);
    PracticeSession[PracticePhaseStarted] = now;
    PracticeSession[PracticePhaseDuration] = PracticeRules[PracticeInterval] * 1000;
    return 1;
}

stock PracticeTick(now)
{
    if (!PracticeIsBusy() || !PracticeParticipantValid()) return 1;
    // Subtraction remains valid across GetTickCount's signed wraparound.
    new elapsed = now - PracticeSession[PracticePhaseStarted];
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
            va_SendClientMessage(PracticeSession[PracticePlayer], COLOR_GRAD1, "[Practica] Comienza en %d...", remaining);
        }
    }
    return 1;
}

stock PracticeHandleShot(playerid, weaponid, hittype, hitid, now)
{
    if (!PracticeIsBusy() || playerid != PracticeSession[PracticePlayer]) return 1;
    if (!PracticeParticipantValid()) return 1;
    if (weaponid != PracticeRules[PracticeWeapon])
        return PracticeEnd(false, "disparaste con un arma no permitida.");
    if (PracticeSession[PracticePhase] == PRACTICE_COUNTDOWN) return 1;

    // Enforce the deadline even between Task500 ticks. Never create a new target
    // from a shot callback: a late hit must not be credited to a reused object ID.
    if (PracticeSession[PracticePhase] == PRACTICE_TARGET
        && now - PracticeSession[PracticePhaseStarted] >= PracticeSession[PracticePhaseDuration])
        PracticeResolveTarget(false, now);
    if (!PracticeIsBusy()) return 1;

    PracticeSession[PracticeShots]++;
    if (PracticeSession[PracticePhase] == PRACTICE_TARGET
        && hittype == BULLET_HIT_TYPE_PLAYER_OBJECT
        && Streamer_GetItemStreamerID(playerid, STREAMER_TYPE_OBJECT, hitid) == PracticeSession[PracticeObject])
        PracticeResolveTarget(true, now);
    return 1;
}

stock PracticeStart(playerid)
{
    if (PracticeIsBusy())
    {
        if (PracticeSession[PracticePlayer] == playerid)
            return SendClientMessage(playerid, COLOR_FADE3, "Ya tienes una practica activa. Usa /practica cancelar.");
        return SendClientMessage(playerid, COLOR_FADE3, "El puesto de tiro esta ocupado.");
    }
    new reason[144];
    if (!PracticeConfigurationReady(reason))
        return SendClientMessage(playerid, COLOR_FADE3, reason);
    if (!PracticeCanUseStation(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "No tienes acceso a este puesto de tiro.");
    new Float:health;
    GetPlayerHealth(playerid, health);
    if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT || health <= 0.0)
        return SendClientMessage(playerid, COLOR_FADE3, "Debes estar vivo y a pie para iniciar.");
    if (GetPlayerInterior(playerid) != PracticePoints[0][PracticePointInterior]
        || GetPlayerVirtualWorld(playerid) != PracticePoints[0][PracticePointWorld]
        || !IsPlayerInRangeOfPoint(playerid, PRACTICE_START_RADIUS, PracticePoints[0][PracticePointX],
            PracticePoints[0][PracticePointY], PracticePoints[0][PracticePointZ]))
        return SendClientMessage(playerid, COLOR_FADE3, "Acercate al punto de inicio del puesto de tiro.");
    if (GetPlayerWeapon(playerid) != PracticeRules[PracticeWeapon])
        return va_SendClientMessage(playerid, COLOR_FADE3, "Equipa el arma permitida (ID %d) antes de iniciar.", PracticeRules[PracticeWeapon]);

    PracticeCleanup();
    PracticeSession[PracticePlayer] = playerid;
    PracticeSession[PracticeTotal] = PracticeCountTargets();
    PracticeSession[PracticePhase] = PRACTICE_COUNTDOWN;
    PracticeSession[PracticePhaseStarted] = GetTickCount();
    PracticeSession[PracticePhaseDuration] = PracticeRules[PracticePreparation] * 1000;
    PracticeSession[PracticeLastCountdown] = PracticeRules[PracticePreparation];
    va_SendClientMessage(playerid, COLOR_GRAD1, "[Practica] %d blancos, %d puntos para aprobar. Un impacto vale un punto.",
        PracticeSession[PracticeTotal], PracticeRules[PracticePassPoints]);
    SendClientMessage(playerid, COLOR_GRAD1, "No te alejes mas de 3 metros del inicio. Los disparos entre blancos cuentan para la precision.");
    va_SendClientMessage(playerid, COLOR_GRAD1, "[Practica] Comienza en %d...", PracticeRules[PracticePreparation]);
    return 1;
}

CMD:practica(playerid, const params[])
{
    if (isnull(params)) return PracticeStart(playerid);
    if (!strcmp(params, "cancelar", true))
    {
        if (!PracticeIsBusy() || PracticeSession[PracticePlayer] != playerid)
            return SendClientMessage(playerid, COLOR_FADE3, "No tienes una practica activa.");
        return PracticeEnd(false, "cancelada por el participante.");
    }
    return SendClientMessage(playerid, COLOR_GRAD1, "Usa /practica para iniciar o /practica cancelar para terminar.");
}
