#if defined SP_SETUP_INCLUDED
    #endinput
#endif
#define SP_SETUP_INCLUDED

stock PracticeCapturePoint(playerid, point[E_PRACTICE_POINT])
{
    GetPlayerPos(playerid, point[PracticePointX], point[PracticePointY], point[PracticePointZ]);
    GetPlayerFacingAngle(playerid, point[PracticePointAngle]);
    point[PracticePointInterior] = GetPlayerInterior(playerid);
    point[PracticePointWorld] = GetPlayerVirtualWorld(playerid);
    point[PracticePointSet] = true;
    return 1;
}

stock PracticeShowPoint(playerid, slot)
{
    if (!PracticePoints[slot][PracticePointSet]) return 1;
    va_SendClientMessage(playerid, COLOR_GRAD1, "Punto %d: X %.2f | Y %.2f | Z %.2f | Angulo %.1f",
        slot, PracticePoints[slot][PracticePointX], PracticePoints[slot][PracticePointY],
        PracticePoints[slot][PracticePointZ], PracticePoints[slot][PracticePointAngle]);
    return 1;
}

stock PracticeShowLayout(playerid)
{
    SendClientMessage(playerid, COLOR_HELP_1, "[ Practica: punto 0 = inicio; puntos 1-10 = blancos ]");
    for (new slot = 0; slot <= PRACTICE_MAX_TARGETS; slot++) PracticeShowPoint(playerid, slot);
    va_SendClientMessage(playerid, COLOR_GRAD1, "Interior: %d | Mundo: %d | Blancos: %d/10 | Arma: %d | Aprobar: %d puntos.",
        PracticePoints[0][PracticePointInterior], PracticePoints[0][PracticePointWorld],
        PracticeCountTargets(), PracticeRules[PracticeWeapon], PracticeRules[PracticePassPoints]);
    va_SendClientMessage(playerid, COLOR_GRAD1, "Preparacion: %ds | Exposicion: %ds | Intervalo: %ds | Ocupado: %s.",
        PracticeRules[PracticePreparation], PracticeRules[PracticeExposure], PracticeRules[PracticeInterval],
        PracticeIsBusy() ? ("si") : ("no"));
    new reason[144];
    if (PracticeConfigurationReady(reason))
        SendClientMessage(playerid, COLOR_LIGHTGREEN, "Configuracion lista. Equipa el arma y usa /practica en el inicio.");
    else
        SendClientMessage(playerid, COLOR_LIGHTORANGE, reason);
    return 1;
}

stock PracticeShowSetupHelp(playerid)
{
    SendClientMessage(playerid, COLOR_HELP_1, "[ Configuracion de practica - cambios guardados automaticamente ]");
    SendClientMessage(playerid, COLOR_GRAD1, "/practicaconfig inicio - marca el puesto desde tu posicion actual.");
    SendClientMessage(playerid, COLOR_GRAD1, "/practicaconfig blanco [1-10] | eliminar [1-10] - marca/reemplaza o elimina un blanco.");
    SendClientMessage(playerid, COLOR_GRAD1, "/practicaconfig arma [22-34] | aprobar [puntos] - un impacto vale un punto.");
    SendClientMessage(playerid, COLOR_GRAD1, "/practicaconfig preparacion [1-30] | tiempo [1-60] | intervalo [0-30] - segundos.");
    SendClientMessage(playerid, COLOR_GRAD1, "/practicaconfig ver | guardar - consulta la configuracion o reintenta el guardado.");
    SendClientMessage(playerid, COLOR_GRAD1, "Marca a pie. Los blancos deben estar a 3-100 metros del inicio, en el mismo interior/mundo.");
    return 1;
}

stock PracticeConfigurationChanged(playerid)
{
    PracticeSaveAndNotify(playerid);
    new reason[144];
    if (!PracticeConfigurationReady(reason)) SendClientMessage(playerid, COLOR_LIGHTORANGE, reason);
    return 1;
}

CMD:practicaconfig(playerid, const params[])
{
    if (!IsPlayerAdmin(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "Necesitas ser administrador RCON para configurar la practica.");
    if (isnull(params) || !strcmp(params, "ayuda", true)) return PracticeShowSetupHelp(playerid);
    if (!strcmp(params, "ver", true)) return PracticeShowLayout(playerid);
    if (!strcmp(params, "guardar", true) || !strcmp(params, "exportar", true))
        return PracticeSaveAndNotify(playerid);
    if (PracticeIsBusy())
        return SendClientMessage(playerid, COLOR_FADE3, "Espera a que termine la practica para modificar la configuracion.");

    if (!strcmp(params, "inicio", true))
    {
        if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT)
            return SendClientMessage(playerid, COLOR_FADE3, "Debes estar a pie para marcar un punto.");
        new point[E_PRACTICE_POINT];
        PracticeCapturePoint(playerid, point);
        if (!PracticePointValid(point))
            return SendClientMessage(playerid, COLOR_FADE3, "La posicion actual no es valida.");
        if (PracticePoints[0][PracticePointSet] && !PracticeSameDimension(PracticePoints[0], point))
        {
            for (new slot = 1; slot <= PRACTICE_MAX_TARGETS; slot++)
                PracticePoints[slot][PracticePointSet] = false;
            SendClientMessage(playerid, COLOR_LIGHTORANGE, "El inicio cambio de interior/mundo. Vuelve a marcar los blancos.");
        }
        PracticePoints[0] = point;
        SendClientMessage(playerid, COLOR_LIGHTGREEN, "Inicio marcado en tu posicion actual.");
        return PracticeConfigurationChanged(playerid);
    }

    new action[20], value;
    if (sscanf(params, "s[20]i", action, value)) return PracticeShowSetupHelp(playerid);
    if (!strcmp(action, "blanco", true) || !strcmp(action, "eliminar", true))
    {
        if (value < 1 || value > PRACTICE_MAX_TARGETS)
            return SendClientMessage(playerid, COLOR_FADE3, "El numero de blanco debe estar entre 1 y 10.");
        if (!strcmp(action, "eliminar", true))
        {
            PracticePoints[value][PracticePointSet] = false;
            va_SendClientMessage(playerid, COLOR_LIGHTGREEN, "Blanco %d eliminado.", value);
            return PracticeConfigurationChanged(playerid);
        }
        if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT)
            return SendClientMessage(playerid, COLOR_FADE3, "Debes estar a pie para marcar un punto.");
        if (!PracticePoints[0][PracticePointSet])
            return SendClientMessage(playerid, COLOR_FADE3, "Primero marca /practicaconfig inicio.");
        new point[E_PRACTICE_POINT];
        PracticeCapturePoint(playerid, point);
        if (!PracticePointValid(point) || !PracticeSameDimension(PracticePoints[0], point))
            return SendClientMessage(playerid, COLOR_FADE3, "El blanco debe tener una posicion valida en el mismo interior y mundo que el inicio.");
        if (!PracticeTargetDistanceValid(PracticePoints[0], point))
            return SendClientMessage(playerid, COLOR_FADE3, "El blanco debe estar entre 3 y 100 metros del inicio. Ten en cuenta el alcance del arma.");
        PracticePoints[value] = point;
        va_SendClientMessage(playerid, COLOR_LIGHTGREEN, "Blanco %d marcado. Durante la practica aparecera una caja en esa posicion.", value);
        return PracticeConfigurationChanged(playerid);
    }

    if (!strcmp(action, "arma", true))
    {
        if (!PracticeIsSupportedWeapon(value))
            return SendClientMessage(playerid, COLOR_FADE3, "Usa un arma de fuego compatible: ID 22-34 (por ejemplo, 24 = Desert Eagle).");
        PracticeRules[PracticeWeapon] = value;
    }
    else if (!strcmp(action, "aprobar", true))
    {
        if (value < 1 || value > PracticeCountTargets())
            return va_SendClientMessage(playerid, COLOR_FADE3, "El minimo debe estar entre 1 y la cantidad de blancos configurados (%d).", PracticeCountTargets());
        PracticeRules[PracticePassPoints] = value;
    }
    else if (!strcmp(action, "preparacion", true))
    {
        if (value < 1 || value > 30)
            return SendClientMessage(playerid, COLOR_FADE3, "La preparacion debe durar entre 1 y 30 segundos.");
        PracticeRules[PracticePreparation] = value;
    }
    else if (!strcmp(action, "tiempo", true))
    {
        if (value < 1 || value > 60)
            return SendClientMessage(playerid, COLOR_FADE3, "Cada blanco debe durar entre 1 y 60 segundos.");
        PracticeRules[PracticeExposure] = value;
    }
    else if (!strcmp(action, "intervalo", true))
    {
        if (value < 0 || value > 30)
            return SendClientMessage(playerid, COLOR_FADE3, "El intervalo debe durar entre 0 y 30 segundos.");
        PracticeRules[PracticeInterval] = value;
    }
    else return PracticeShowSetupHelp(playerid);

    va_SendClientMessage(playerid, COLOR_LIGHTGREEN, "Configuracion actualizada: %s = %d.", action, value);
    return PracticeConfigurationChanged(playerid);
}
