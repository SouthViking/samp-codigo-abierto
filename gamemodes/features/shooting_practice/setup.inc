#if defined _shooting_practice_setup_included
    #endinput
#endif
#define _shooting_practice_setup_included

#define PRACTICE_LAYOUT_EXPORT_FILE "shooting_practice_points.inc"

stock PracticeCapturePoint(playerid, point[E_PRACTICE_POINT])
{
    GetPlayerPos(playerid, point[PracticePointX], point[PracticePointY], point[PracticePointZ]);
    GetPlayerFacingAngle(playerid, point[PracticePointAngle]);
    point[PracticePointInterior] = GetPlayerInterior(playerid);
    point[PracticePointWorld] = GetPlayerVirtualWorld(playerid);
    point[PracticePointSet] = true;
    return 1;
}

stock PracticeShowPoint(playerid, const label[], const point[E_PRACTICE_POINT])
{
    if (!point[PracticePointSet])
    {
        va_SendClientMessage(playerid, COLOR_FADE3, "%s: pendiente de marcar.", label);
        return 1;
    }

    va_SendClientMessage(playerid, COLOR_GRAD1, "%s: X %.4f | Y %.4f | Z %.4f | Angulo %.4f",
        label, point[PracticePointX], point[PracticePointY], point[PracticePointZ], point[PracticePointAngle]);
    va_SendClientMessage(playerid, COLOR_GRAD1, "Interior: %d | Mundo virtual: %d",
        point[PracticePointInterior], point[PracticePointWorld]);
    return 1;
}

stock PracticeShowLayout(playerid)
{
    PracticeShowPoint(playerid, "Inicio", PracticeStartPoint);
    for (new index = 0; index < PRACTICE_TARGET_POSITION_COUNT; index++)
    {
        new label[24];
        format(label, sizeof(label), "Blanco %d", index + 1);
        PracticeShowPoint(playerid, label, PracticeTargetPoints[index]);
    }

    if (PracticeIsLayoutReady())
        SendClientMessage(playerid, COLOR_LIGHTGREEN, "Los puntos estan completos. Usa /practicaconfig exportar para conservarlos.");
    else
        SendClientMessage(playerid, COLOR_LIGHTORANGE, "Faltan puntos: marca el inicio y los 3 blancos.");

    SendClientMessage(playerid, COLOR_GRAD1, "Configuracion temporal: se pierde al reiniciar el servidor.");
    return 1;
}

stock PracticeShowSetupHelp(playerid)
{
    SendClientMessage(playerid, COLOR_HELP_1, "[ Configuracion temporal de practica ]");
    SendClientMessage(playerid, COLOR_GRAD1, "/practicaconfig inicio - marca la posicion del tirador.");
    SendClientMessage(playerid, COLOR_GRAD1, "/practicaconfig blanco [1-3] - marca o reemplaza un punto de blanco.");
    SendClientMessage(playerid, COLOR_GRAD1, "/practicaconfig ver - consulta los puntos y los que faltan.");
    SendClientMessage(playerid, COLOR_GRAD1, "/practicaconfig exportar - escribe los puntos en scriptfiles/shooting_practice_points.inc.");
    SendClientMessage(playerid, COLOR_GRAD1, "Ponte a pie y orienta al personaje como quieras guardar cada punto.");
    SendClientMessage(playerid, COLOR_LIGHTORANGE, "Los puntos duran hasta reiniciar. Exportar reemplaza el archivo anterior; no lo carga automaticamente.");
    return 1;
}

stock PracticeExportLayout(playerid)
{
    if (!PracticeIsLayoutReady())
        return SendClientMessage(playerid, COLOR_FADE3, "Marca el inicio y los 3 blancos antes de exportar. Consulta /practicaconfig ver.");

    new File:output = fopen(PRACTICE_LAYOUT_EXPORT_FILE, io_write);
    if (!output)
        return SendClientMessage(playerid, COLOR_FADE3, "No se pudo exportar. Comprueba que exista scriptfiles y tenga permiso de escritura.");

    new bool:success = true;
    new line[256];

    if (!fwrite(output, "// Captured with /practicaconfig exportar. Replace the declarations in config.inc.\r\n"))
        success = false;
    if (!fwrite(output, "// Fields: set, x, y, z, facing angle, interior, virtual world.\r\n"))
        success = false;
    if (!fwrite(output, "// Validate the target model pivot, orientation and collision in-game.\r\n\r\n"))
        success = false;

    format(line, sizeof(line), "new PracticeStartPoint[E_PRACTICE_POINT] = {true, %.4f, %.4f, %.4f, %.4f, %d, %d};\r\n\r\n",
        PracticeStartPoint[PracticePointX], PracticeStartPoint[PracticePointY], PracticeStartPoint[PracticePointZ],
        PracticeStartPoint[PracticePointAngle], PracticeStartPoint[PracticePointInterior], PracticeStartPoint[PracticePointWorld]);
    if (!fwrite(output, line))
        success = false;

    if (!fwrite(output, "new PracticeTargetPoints[PRACTICE_TARGET_POSITION_COUNT][E_PRACTICE_POINT] =\r\n{\r\n"))
        success = false;

    for (new index = 0; index < PRACTICE_TARGET_POSITION_COUNT; index++)
    {
        format(line, sizeof(line), "    {true, %.4f, %.4f, %.4f, %.4f, %d, %d}",
            PracticeTargetPoints[index][PracticePointX], PracticeTargetPoints[index][PracticePointY], PracticeTargetPoints[index][PracticePointZ],
            PracticeTargetPoints[index][PracticePointAngle], PracticeTargetPoints[index][PracticePointInterior], PracticeTargetPoints[index][PracticePointWorld]);
        if (!fwrite(output, line))
            success = false;
        if (index + 1 < PRACTICE_TARGET_POSITION_COUNT && !fwrite(output, ","))
            success = false;
        if (!fwrite(output, "\r\n"))
            success = false;
    }

    if (!fwrite(output, "};\r\n"))
        success = false;
    if (!fclose(output))
        success = false;

    if (!success)
        return SendClientMessage(playerid, COLOR_FADE3, "La exportacion fallo y el archivo puede estar incompleto. Los puntos siguen en memoria.");

    SendClientMessage(playerid, COLOR_LIGHTGREEN, "Puntos exportados a scriptfiles/shooting_practice_points.inc.");
    SendClientMessage(playerid, COLOR_GRAD1, "El archivo sirve para formalizar la configuracion; no se carga automaticamente.");
    return 1;
}

CMD:practicaconfig(playerid, params[])
{
    // The development layout is shared by the server, so editing requires RCON.
    if (!IsPlayerAdmin(playerid))
        return SendClientMessage(playerid, COLOR_FADE3, "Necesitas ser administrador RCON para configurar los puntos de practica.");

    if (isnull(params) || !strcmp(params, "ayuda", true))
        return PracticeShowSetupHelp(playerid);
    if (!strcmp(params, "ver", true))
        return PracticeShowLayout(playerid);
    if (!strcmp(params, "exportar", true))
        return PracticeExportLayout(playerid);

    if (!strcmp(params, "inicio", true))
    {
        if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT)
            return SendClientMessage(playerid, COLOR_FADE3, "Debes estar a pie para marcar un punto.");

        // A layout has one world/interior. Moving the start to another dimension
        // invalidates the old targets rather than leaving an unusable mixture.
        if (PracticeStartPoint[PracticePointSet]
            && (PracticeStartPoint[PracticePointInterior] != GetPlayerInterior(playerid)
                || PracticeStartPoint[PracticePointWorld] != GetPlayerVirtualWorld(playerid)))
        {
            for (new index = 0; index < PRACTICE_TARGET_POSITION_COUNT; index++)
                PracticeTargetPoints[index][PracticePointSet] = false;
            SendClientMessage(playerid, COLOR_LIGHTORANGE, "El inicio cambio de interior o mundo: vuelve a marcar los 3 blancos.");
        }

        PracticeCapturePoint(playerid, PracticeStartPoint);
        SendClientMessage(playerid, COLOR_LIGHTGREEN, "Inicio marcado en tu posicion y orientacion actuales.");
        return PracticeShowPoint(playerid, "Inicio", PracticeStartPoint);
    }

    new action[16], targetSlot;
    if (sscanf(params, "s[16]i", action, targetSlot) || strcmp(action, "blanco", true))
        return PracticeShowSetupHelp(playerid);
    if (targetSlot < 1 || targetSlot > PRACTICE_TARGET_POSITION_COUNT)
        return SendClientMessage(playerid, COLOR_FADE3, "El numero de blanco debe estar entre 1 y 3.");
    if (GetPlayerState(playerid) != PLAYER_STATE_ONFOOT)
        return SendClientMessage(playerid, COLOR_FADE3, "Debes estar a pie para marcar un punto.");
    if (!PracticeStartPoint[PracticePointSet])
        return SendClientMessage(playerid, COLOR_FADE3, "Primero marca el puesto del tirador con /practicaconfig inicio.");
    if (GetPlayerInterior(playerid) != PracticeStartPoint[PracticePointInterior]
        || GetPlayerVirtualWorld(playerid) != PracticeStartPoint[PracticePointWorld])
        return SendClientMessage(playerid, COLOR_FADE3, "El blanco debe estar en el mismo interior y mundo que el inicio.");

    new index = targetSlot - 1;
    PracticeCapturePoint(playerid, PracticeTargetPoints[index]);
    va_SendClientMessage(playerid, COLOR_LIGHTGREEN, "Blanco %d marcado en tu posicion y orientacion actuales.", targetSlot);
    new label[24];
    format(label, sizeof(label), "Blanco %d", targetSlot);
    PracticeShowPoint(playerid, label, PracticeTargetPoints[index]);
    if (PracticeIsLayoutReady())
        SendClientMessage(playerid, COLOR_LIGHTGREEN, "Inicio y 3 blancos marcados. Puedes exportarlos con /practicaconfig exportar.");
    return 1;
}
