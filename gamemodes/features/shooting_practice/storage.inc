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
    new bool:success = PracticeWriteLine(output, "SARP_PRACTICE 6\r\n");
    format(line, sizeof(line), "%d %d %d %d %d %d %d %d\r\n", PracticeRules[PracticeWeaponMask],
        PracticeRules[PracticePassPoints], PracticeRules[PracticePreparation],
        PracticeRules[PracticeExposure], PracticeRules[PracticeInterval],
        PracticeRules[PracticeMaxErrors], PracticeRules[PracticeRandomOrder], PracticeRules[PracticeRandomTypes]);
    if (!PracticeWriteLine(output, line)) success = false;

    for (new slot = 0; slot <= PRACTICE_MAX_TARGETS; slot++)
    {
        format(line, sizeof(line), "%d %.6f %.6f %.6f %.6f %d %d %d %d %.6f %.6f\r\n",
            PracticePoints[slot][PracticePointSet], PracticePoints[slot][PracticePointX],
            PracticePoints[slot][PracticePointY], PracticePoints[slot][PracticePointZ],
            PracticePoints[slot][PracticePointAngle], PracticePoints[slot][PracticePointInterior],
            PracticePoints[slot][PracticePointWorld], PracticePoints[slot][PracticePointNoShoot],
            PracticePoints[slot][PracticePointModel], PracticePoints[slot][PracticePointRotX], PracticePoints[slot][PracticePointRotY]);
        if (!PracticeWriteLine(output, line)) success = false;
        format(line, sizeof(line), "NAME %s\r\n", PracticePoints[slot][PracticePointName]);
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
    new rules[E_PRACTICE_RULE], line[256], version, legacyNoShootCount;
    new bool:valid = PracticeReadLine(input, line);
    if (valid)
    {
        if (!strcmp(line, "SARP_PRACTICE 1")) version = 1;
        else if (!strcmp(line, "SARP_PRACTICE 2")) version = 2;
        else if (!strcmp(line, "SARP_PRACTICE 3")) version = 3;
        else if (!strcmp(line, "SARP_PRACTICE 4")) version = 4;
        else if (!strcmp(line, "SARP_PRACTICE 5")) version = 5;
        else if (!strcmp(line, "SARP_PRACTICE 6")) version = 6;
        else valid = false;
    }
    if (valid)
    {
        valid = PracticeReadLine(input, line);
        // Legacy layouts keep their coordinates and rules. New fields default to
        // zero (all shoot targets, fixed order/distribution, no identification errors).
        if (valid && version == 1)
            valid = !sscanf(line, "iiiii", rules[PracticeWeaponMask], rules[PracticePassPoints],
                rules[PracticePreparation], rules[PracticeExposure], rules[PracticeInterval]);
        else if (valid && version == 2)
            valid = !sscanf(line, "iiiiiiiii", rules[PracticeWeaponMask], rules[PracticePassPoints],
                rules[PracticePreparation], rules[PracticeExposure], rules[PracticeInterval],
                legacyNoShootCount, rules[PracticeMaxErrors], rules[PracticeRandomOrder], rules[PracticeRandomTypes])
                && legacyNoShootCount >= 0 && legacyNoShootCount < PRACTICE_MAX_TARGETS
                && rules[PracticeMaxErrors] <= legacyNoShootCount;
        else if (valid)
            valid = !sscanf(line, "iiiiiiii", rules[PracticeWeaponMask], rules[PracticePassPoints],
                rules[PracticePreparation], rules[PracticeExposure], rules[PracticeInterval],
                rules[PracticeMaxErrors], rules[PracticeRandomOrder], rules[PracticeRandomTypes]);
        // v1-v4 store one weapon ID; v5 stores the set of permitted weapons.
        if (valid && version < 5)
        {
            valid = PracticeIsSupportedWeapon(rules[PracticeWeaponMask]);
            if (valid) rules[PracticeWeaponMask] = 1 << (rules[PracticeWeaponMask] - PRACTICE_WEAPON_FIRST);
        }
        // v1-v5 store exposure/interval in seconds. Validate before multiplication
        // so an overflowing legacy value cannot turn into a plausible duration.
        if (valid && version < 6)
        {
            valid = rules[PracticeExposure] >= 1 && rules[PracticeExposure] <= 60
                && rules[PracticeInterval] >= 0 && rules[PracticeInterval] <= 30;
            if (valid)
            {
                rules[PracticeExposure] *= 1000;
                rules[PracticeInterval] *= 1000;
            }
        }
        valid = valid && PracticeRulesValid(rules);
    }

    for (new slot = 0; valid && slot <= PRACTICE_MAX_TARGETS; slot++)
    {
        new marked, noShoot;
        valid = PracticeReadLine(input, line);
        if (valid && version >= 4)
        {
            valid = !sscanf(line, "iffffiiiiff", marked,
                points[slot][PracticePointX], points[slot][PracticePointY], points[slot][PracticePointZ],
                points[slot][PracticePointAngle], points[slot][PracticePointInterior], points[slot][PracticePointWorld], noShoot,
                points[slot][PracticePointModel], points[slot][PracticePointRotX], points[slot][PracticePointRotY]);
            if (valid)
            {
                valid = PracticeReadLine(input, line) && strfind(line, "NAME ") == 0
                    && strlen(line) < 5 + PRACTICE_NAME_SIZE;
                if (valid) strmid(points[slot][PracticePointName], line, 5, strlen(line), PRACTICE_NAME_SIZE);
            }
        }
        else if (valid && version == 3)
            valid = !sscanf(line, "iffffiii", marked,
                points[slot][PracticePointX], points[slot][PracticePointY], points[slot][PracticePointZ],
                points[slot][PracticePointAngle], points[slot][PracticePointInterior], points[slot][PracticePointWorld], noShoot);
        else if (valid)
            valid = !sscanf(line, "iffffii", marked,
                points[slot][PracticePointX], points[slot][PracticePointY], points[slot][PracticePointZ],
                points[slot][PracticePointAngle], points[slot][PracticePointInterior], points[slot][PracticePointWorld]);
        if (!valid || (marked != 0 && marked != 1) || (noShoot != 0 && noShoot != 1) || (!slot && noShoot))
        {
            valid = false;
            break;
        }
        points[slot][PracticePointSet] = bool:marked;
        points[slot][PracticePointNoShoot] = bool:noShoot;
        if (version < 4 && slot && marked) points[slot][PracticePointModel] = PRACTICE_TARGET_MODEL;
        valid = PracticePointValid(points[slot]);
        if (slot && points[slot][PracticePointSet])
            valid = valid && points[0][PracticePointSet] && PracticeSameDimension(points[0], points[slot])
                && PracticeModelIndex(points[slot][PracticePointModel]) >= 0;
        else valid = valid && !points[slot][PracticePointModel];
    }
    if (valid) valid = PracticeReadLine(input, line) && !strcmp(line, "END");
    fclose(input);

    if (!valid)
    {
        print("[Shooting Practice] Invalid/truncated shooting_practice.cfg. Configuration was not loaded.");
        return false;
    }
    // v2 assigned no-shoot to the highest marked slots. Materialize those types
    // once, retaining coordinates, order and random-distribution preferences.
    // Incomplete legacy layouts can have fewer positions than their old quota.
    if (version == 2)
        for (new slot = PRACTICE_MAX_TARGETS; slot > 0 && legacyNoShootCount; slot--)
            if (points[slot][PracticePointSet])
            {
                points[slot][PracticePointNoShoot] = true;
                legacyNoShootCount--;
            }
    for (new slot = 0; slot <= PRACTICE_MAX_TARGETS; slot++) PracticePoints[slot] = points[slot];
    PracticeRules = rules;
    PracticeSavePending = false;
    print("[Shooting Practice] Configuration loaded from shooting_practice.cfg.");
    return true;
}

stock PracticeSaveAndNotify(playerid)
{
    PracticeSavePending = !PracticeSaveConfiguration();
    if (!PracticeSavePending)
        SendClientMessage(playerid, COLOR_LIGHTGREEN, "Configuración guardada. Se recuperará al reiniciar.");
    else
        SendClientMessage(playerid, COLOR_LIGHTORANGE, "No se pudo guardar. Cambios en memoria: usa Reintentar guardado en /practicaconfig antes de reiniciar.");
    return 1;
}
