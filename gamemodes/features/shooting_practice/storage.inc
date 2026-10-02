#if defined SP_STORAGE_INCLUDED
    #endinput
#endif
#define SP_STORAGE_INCLUDED

stock bool:PracticeWriteLine(File:output, const line[])
{
    return fwrite(output, line) == strlen(line);
}

stock bool:PracticeSaveConfiguration()
{
    new File:output = fopen(PRACTICE_CONFIG_FILE, io_write);
    if (!output) return false;

    new line[256];
    new bool:success = PracticeWriteLine(output, "SARP_PRACTICE 1\r\n");
    format(line, sizeof(line), "%d %d %d %d %d\r\n", PracticeRules[PracticeWeapon],
        PracticeRules[PracticePassPoints], PracticeRules[PracticePreparation],
        PracticeRules[PracticeExposure], PracticeRules[PracticeInterval]);
    if (!PracticeWriteLine(output, line)) success = false;

    for (new slot = 0; slot <= PRACTICE_MAX_TARGETS; slot++)
    {
        format(line, sizeof(line), "%d %.6f %.6f %.6f %.6f %d %d\r\n",
            PracticePoints[slot][PracticePointSet], PracticePoints[slot][PracticePointX],
            PracticePoints[slot][PracticePointY], PracticePoints[slot][PracticePointZ],
            PracticePoints[slot][PracticePointAngle], PracticePoints[slot][PracticePointInterior],
            PracticePoints[slot][PracticePointWorld]);
        if (!PracticeWriteLine(output, line)) success = false;
    }
    if (!PracticeWriteLine(output, "END\r\n")) success = false;
    if (!fclose(output)) success = false;
    return success;
}

stock bool:PracticeReadLine(File:input, line[], size = sizeof line)
{
    if (!fread(input, line, size)) return false;
    new length = strlen(line);
    while (length && (line[length - 1] == '\r' || line[length - 1] == '\n'))
        line[--length] = EOS;
    return true;
}

stock bool:PracticeLoadConfiguration()
{
    if (!fexist(PRACTICE_CONFIG_FILE)) return false;
    new File:input = fopen(PRACTICE_CONFIG_FILE, io_read);
    if (!input) return false;

    // Parse into temporary records; truncated/invalid files never load partially.
    new points[PRACTICE_MAX_TARGETS + 1][E_PRACTICE_POINT];
    new rules[E_PRACTICE_RULE], line[256];
    new bool:valid = PracticeReadLine(input, line) && !strcmp(line, "SARP_PRACTICE 1");
    if (valid)
        valid = PracticeReadLine(input, line) && !sscanf(line, "iiiii", rules[PracticeWeapon],
            rules[PracticePassPoints], rules[PracticePreparation], rules[PracticeExposure], rules[PracticeInterval])
            && PracticeRulesValid(rules);

    for (new slot = 0; valid && slot <= PRACTICE_MAX_TARGETS; slot++)
    {
        new marked;
        valid = PracticeReadLine(input, line) && !sscanf(line, "iffffii", marked,
            points[slot][PracticePointX], points[slot][PracticePointY], points[slot][PracticePointZ],
            points[slot][PracticePointAngle], points[slot][PracticePointInterior], points[slot][PracticePointWorld]);
        if (!valid || (marked != 0 && marked != 1))
        {
            valid = false;
            break;
        }
        points[slot][PracticePointSet] = bool:marked;
        valid = PracticePointValid(points[slot]);
        if (slot && points[slot][PracticePointSet])
            valid = valid && points[0][PracticePointSet] && PracticeSameDimension(points[0], points[slot]);
    }
    if (valid) valid = PracticeReadLine(input, line) && !strcmp(line, "END");
    fclose(input);

    if (!valid)
    {
        print("[Shooting Practice] Invalid/truncated shooting_practice.cfg; configuration was not loaded.");
        return false;
    }
    for (new slot = 0; slot <= PRACTICE_MAX_TARGETS; slot++) PracticePoints[slot] = points[slot];
    PracticeRules = rules;
    print("[Shooting Practice] Configuration loaded from shooting_practice.cfg.");
    return true;
}

stock PracticeSaveAndNotify(playerid)
{
    if (PracticeSaveConfiguration())
        SendClientMessage(playerid, COLOR_LIGHTGREEN, "Configuracion guardada; se recuperara al reiniciar.");
    else
        SendClientMessage(playerid, COLOR_LIGHTORANGE, "No se pudo guardar. Cambios solo en memoria; revisa scriptfiles y usa /practicaconfig guardar.");
    return 1;
}
