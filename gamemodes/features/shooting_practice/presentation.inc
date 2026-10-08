#if defined SP_PRESENTATION_INCLUDED
    #endinput
#endif
#define SP_PRESENTATION_INCLUDED

#define PRACTICE_CUE_RADIUS (10.0)
#define PRACTICE_SOUND_COUNTDOWN (1056)
#define PRACTICE_SOUND_TARGET (1057)
#define PRACTICE_SOUND_END (1139)
// Object materials use ARGB, unlike chat's RGBA colours.
#define PRACTICE_SHOOT_MATERIAL (0xFFB83D3D)
#define PRACTICE_NO_SHOOT_MATERIAL (0xFF2465A8)

// Result lines are already formatted. Send them directly so a literal percent
// sign is never interpreted again by the gamemode's variadic chat wrapper.
#if !defined PracticeSendResultLine
    native PracticeSendResultLine(playerid, colour, const message[]) = SendClientMessage;
#endif

new PlayerText:PracticeCountdownText[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...};

stock PracticeHideCountdown(playerid)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS) return 1;
    if (PracticeCountdownText[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        if (IsPlayerConnected(playerid)) PlayerTextDrawDestroy(playerid, PracticeCountdownText[playerid]);
        PracticeCountdownText[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }
    return 1;
}

stock bool:PracticeCanObserve(playerid)
{
    return IsPlayerConnected(playerid) && !IsPlayerNPC(playerid)
        && GetPlayerVirtualWorld(playerid) == PracticePoints[0][PracticePointWorld]
        && GetPlayerInterior(playerid) == PracticePoints[0][PracticePointInterior]
        && IsPlayerInRangeOfPoint(playerid, PRACTICE_CUE_RADIUS, PracticePoints[0][PracticePointX],
            PracticePoints[0][PracticePointY], PracticePoints[0][PracticePointZ]);
}

stock PracticePlayCue(soundid)
{
    // These are built-in frontend tones. Filter recipients explicitly; do not
    // assume 3D attenuation or that a virtual world filters sound automatically.
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
        if (PracticeCanObserve(playerid)) PlayerPlaySound(playerid, soundid, 0.0, 0.0, 0.0);
    return 1;
}

stock PracticeAmbientMessage(const message[])
{
    // Anchor environmental messages to the station even if the participant has
    // disconnected, walked away or changed dimension. No player is impersonated.
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
        if (PracticeCanObserve(playerid)) SendClientMessage(playerid, COLOR_PURPLE, message);
    return 1;
}

stock PracticePresentCountdown()
{
    new playerid = PracticeSession[PracticePlayer], text[32];
    if (IsPlayerConnected(playerid))
    {
        // Plain GXT-safe text; only the participant sees this small, owned overlay.
        format(text, sizeof(text), "Comienza en %d", PracticeSession[PracticeLastCountdown]);
        if (PracticeCountdownText[playerid] == PlayerText:INVALID_TEXT_DRAW)
        {
            PracticeCountdownText[playerid] = CreatePlayerTextDraw(playerid, 320.0, 360.0, text);
            if (PracticeCountdownText[playerid] != PlayerText:INVALID_TEXT_DRAW)
            {
                PlayerTextDrawAlignment(playerid, PracticeCountdownText[playerid], 2);
                PlayerTextDrawLetterSize(playerid, PracticeCountdownText[playerid], 0.25, 1.2);
                PlayerTextDrawFont(playerid, PracticeCountdownText[playerid], 1);
                PlayerTextDrawColor(playerid, PracticeCountdownText[playerid], 0xFFFFFFDD);
                PlayerTextDrawSetOutline(playerid, PracticeCountdownText[playerid], 1);
                PlayerTextDrawShow(playerid, PracticeCountdownText[playerid]);
            }
        }
        else PlayerTextDrawSetString(playerid, PracticeCountdownText[playerid], text);
    }
    return PracticePlayCue(PRACTICE_SOUND_COUNTDOWN);
}

stock PracticePresentStart(playerid)
{
    new name[MAX_PLAYER_NAME], message[144];
    GetPlayerRoleplayName(playerid, name, sizeof(name));
    format(message, sizeof(message), "* %s se prepara para comenzar la práctica de tiro.", name);
    PracticeAmbientMessage(message);
    return PracticePresentCountdown();
}

stock PracticePresentTarget()
{
    return PracticePlayCue(PRACTICE_SOUND_TARGET);
}

stock PracticePresentEnd(bool:completed, playerid, bool:abandoned)
{
    PracticePlayCue(PRACTICE_SOUND_END);
    if (abandoned)
    {
        new name[MAX_PLAYER_NAME], message[144];
        GetPlayerRoleplayName(playerid, name, sizeof(name));
        format(message, sizeof(message), "* %s abandona el puesto de tiro.", name);
        PracticeAmbientMessage(message);
    }
    else if (completed)
        PracticeAmbientMessage("* La señal del puesto indica el final del ejercicio. (( Puesto de tiro ))");
    else
        PracticeAmbientMessage("* La señal del puesto indica que el ejercicio se ha interrumpido. (( Puesto de tiro ))");
    return 1;
}

stock PracticeApplyTargetAppearance(STREAMER_TAG_OBJECT:objectid, bool:noShoot, modelid = PRACTICE_TARGET_MODEL)
{
    // Preserve the native figures. Only the legacy marked box gets a text material.
    if (modelid != PRACTICE_TARGET_MODEL) return 1;
    return SetDynamicObjectMaterialText(objectid, 0, noShoot ? ("NO DISPARAR") : ("DISPARAR"),
        OBJECT_MATERIAL_SIZE_512x256, "Arial", 32, 1, 0xFFFFFFFF,
        noShoot ? PRACTICE_NO_SHOOT_MATERIAL : PRACTICE_SHOOT_MATERIAL, OBJECT_MATERIAL_TEXT_ALIGN_CENTER);
}

stock PracticeReportResult(playerid, bool:completed, const result[E_PRACTICE_SESSION], const reason[])
{
    new message[144];
    if (!completed)
    {
        format(message, sizeof(message), "[Práctica] Incompleta: %s", reason);
        return PracticeSendResultLine(playerid, COLOR_WHITE, message);
    }
    format(message, sizeof(message), "[Práctica] Reglas. Puntos netos mínimos: %d. Errores máximos: %d. Acierto +1, error -1.",
        result[PracticeRequiredPoints], result[PracticeAllowedErrors]);
    PracticeSendResultLine(playerid, COLOR_WHITE, message);
    format(message, sizeof(message), "[Práctica] Resultados. Aciertos: %d/%d. Sin impactar a tiempo: %d.",
        result[PracticeHits], result[PracticeShootTotal], result[PracticeExpired]);
    PracticeSendResultLine(playerid, COLOR_WHITE, message);
    format(message, sizeof(message), "[Práctica] Protegidos respetados: %d/%d. Errores de identificación: %d.",
        result[PracticeCorrectIgnores], result[PracticeNoShootTotal], result[PracticeIdentificationErrors]);
    PracticeSendResultLine(playerid, COLOR_WHITE, message);
    if (result[PracticeShots])
        format(message, sizeof(message), "[Práctica] Disparos: %d. Precisión: %.1f%c.",
            result[PracticeShots], 100.0 * float(result[PracticeHits]) / float(result[PracticeShots]), '%');
    else
        format(message, sizeof(message), "[Práctica] Disparos: 0. Precisión: no disponible.");
    PracticeSendResultLine(playerid, COLOR_WHITE, message);
    new bool:passed = PracticeResultPassed(result);
    format(message, sizeof(message), "[Práctica] Veredicto: {%06x}%s{FFFFFF}. Puntuación neta: %d.",
        (passed ? COLOR_LIGHTGREEN : COLOR_RED) >>> 8,
        passed ? ("Aprobado") : ("Reprobado"), PracticeResultPoints(result));
    return PracticeSendResultLine(playerid, COLOR_WHITE, message);
}

stock PracticeShowAllowedWeapons(playerid)
{
    new message[144] = "Armas permitidas: ", name[32], count;
    for (new weaponid = PRACTICE_WEAPON_FIRST; weaponid <= PRACTICE_WEAPON_LAST; weaponid++)
    {
        if (!PracticeWeaponAllowed(weaponid, PracticeRules[PracticeWeaponMask])) continue;
        PracticeWeaponName(weaponid, name);
        // Leave room for punctuation, even if every weapon is enabled.
        if (strlen(message) + strlen(name) + 3 >= sizeof(message))
        {
            SendClientMessage(playerid, COLOR_GRAD1, message);
            format(message, sizeof(message), "Armas permitidas: ");
            count = 0;
        }
        if (count++) strcat(message, ", ", sizeof(message));
        strcat(message, name, sizeof(message));
    }
    return SendClientMessage(playerid, COLOR_GRAD1, message);
}

stock PracticeShowPlayerHelp(playerid)
{
    SendClientMessage(playerid, COLOR_GRAD1, "En el puesto, equipa un arma permitida y usa /practica. Puedes alternar entre las permitidas. Para salir: /practica cancelar.");
    SendClientMessage(playerid, COLOR_GRAD1, "Blanco de tiro: un impacto suma 1. Blanco protegido: cualquier disparo, incluso al aire, resta 1 y cuenta un error de identificación.");
    SendClientMessage(playerid, COLOR_GRAD1, "Procura un tiro por blanco de tiro. Fallar o disparar de más reduce la precisión: aciertos divididos por todos los disparos.");
    SendClientMessage(playerid, COLOR_GRAD1, "Cajas: roja DISPARAR, azul NO DISPARAR. El instructor define el comportamiento de cada silueta.");
    PracticeShowAllowedWeapons(playerid);
    va_SendClientMessage(playerid, COLOR_GRAD1, "Preparación: %ds. Tiempo por blanco: %.1fs. Intervalo: %.1fs. La cuenta regresiva se muestra en pantalla.",
        PracticeRules[PracticePreparation], float(PracticeRules[PracticeExposure]) / 1000.0, float(PracticeRules[PracticeInterval]) / 1000.0);
    va_SendClientMessage(playerid, COLOR_GRAD1, "Para aprobar: mínimo %d puntos netos y máximo %d errores de identificación. Permanece a menos de 3 metros del inicio.",
        PracticeRules[PracticePassPoints], PracticeRules[PracticeMaxErrors]);
    return 1;
}
