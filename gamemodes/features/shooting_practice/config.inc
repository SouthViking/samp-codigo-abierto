#if defined SP_CONFIG_INCLUDED
    #endinput
#endif
#define SP_CONFIG_INCLUDED

#define PRACTICE_MAX_TARGETS (10)
#define PRACTICE_START_RADIUS (1.5)
#define PRACTICE_ACTIVE_RADIUS (3.0)
#define PRACTICE_MAX_DISTANCE (100.0)
// Temporary GTA gunbox. Replace the model/offset after the first client test.
#define PRACTICE_TARGET_MODEL (1271)
#define PRACTICE_TARGET_Z_OFFSET (0.0)
#define PRACTICE_CONFIG_FILE "shooting_practice.cfg"
enum E_PRACTICE_POINT
{
    bool:PracticePointSet,
    Float:PracticePointX,
    Float:PracticePointY,
    Float:PracticePointZ,
    Float:PracticePointAngle,
    PracticePointInterior,
    PracticePointWorld
}

enum E_PRACTICE_RULE
{
    PracticeWeapon,
    PracticePassPoints,
    PracticePreparation,
    PracticeExposure,
    PracticeInterval
}

// Slot 0 is the start; slots 1..10 are optional target positions.
// Configuration is separate from the single station's runtime session.
new PracticePoints[PRACTICE_MAX_TARGETS + 1][E_PRACTICE_POINT];
new PracticeRules[E_PRACTICE_RULE] = {24, 1, 3, 5, 1};

stock bool:PracticeIsSupportedWeapon(weaponid)
{
    return weaponid >= 22 && weaponid <= 34;
}

stock PracticeCountTargets()
{
    new count;
    for (new slot = 1; slot <= PRACTICE_MAX_TARGETS; slot++)
        if (PracticePoints[slot][PracticePointSet]) count++;
    return count;
}

stock bool:PracticeRulesValid(const rules[E_PRACTICE_RULE])
{
    return PracticeIsSupportedWeapon(rules[PracticeWeapon])
        && rules[PracticePassPoints] >= 1 && rules[PracticePassPoints] <= PRACTICE_MAX_TARGETS
        && rules[PracticePreparation] >= 1 && rules[PracticePreparation] <= 30
        && rules[PracticeExposure] >= 1 && rules[PracticeExposure] <= 60
        && rules[PracticeInterval] >= 0 && rules[PracticeInterval] <= 30;
}

stock bool:PracticePointValid(const point[E_PRACTICE_POINT])
{
    return (point[PracticePointSet] == false || point[PracticePointSet] == true)
        && point[PracticePointX] >= -20000.0 && point[PracticePointX] <= 20000.0
        && point[PracticePointY] >= -20000.0 && point[PracticePointY] <= 20000.0
        && point[PracticePointZ] >= -20000.0 && point[PracticePointZ] <= 20000.0
        && point[PracticePointAngle] >= 0.0 && point[PracticePointAngle] <= 360.0
        && point[PracticePointInterior] >= 0 && point[PracticePointInterior] <= 255
        && point[PracticePointWorld] >= 0;
}

stock bool:PracticeSameDimension(const first[E_PRACTICE_POINT], const second[E_PRACTICE_POINT])
{
    return first[PracticePointInterior] == second[PracticePointInterior]
        && first[PracticePointWorld] == second[PracticePointWorld];
}

stock bool:PracticeTargetDistanceValid(const start[E_PRACTICE_POINT], const target[E_PRACTICE_POINT])
{
    new Float:x = start[PracticePointX] - target[PracticePointX];
    new Float:y = start[PracticePointY] - target[PracticePointY];
    new Float:z = start[PracticePointZ] - target[PracticePointZ];
    new Float:distanceSquared = x*x + y*y + z*z;
    return distanceSquared >= PRACTICE_ACTIVE_RADIUS * PRACTICE_ACTIVE_RADIUS
        && distanceSquared <= PRACTICE_MAX_DISTANCE * PRACTICE_MAX_DISTANCE;
}

stock bool:PracticeConfigurationReady(reason[], size = sizeof reason)
{
    if (!PracticePoints[0][PracticePointSet])
        format(reason, size, "Falta marcar el inicio con /practicaconfig inicio.");
    else if (!PracticeRulesValid(PracticeRules))
        format(reason, size, "La configuracion de arma, puntos o tiempos no es valida.");
    else if (!PracticeCountTargets())
        format(reason, size, "Marca al menos un blanco con /practicaconfig blanco [1-10].");
    else
    {
        if (PracticeRules[PracticePassPoints] > PracticeCountTargets())
        {
            format(reason, size, "Los puntos para aprobar superan los blancos. Ajusta /practicaconfig aprobar.");
            return false;
        }
        for (new slot = 1; slot <= PRACTICE_MAX_TARGETS; slot++)
        {
            if (!PracticePoints[slot][PracticePointSet]) continue;
            if (!PracticePointValid(PracticePoints[slot])
                || !PracticeSameDimension(PracticePoints[0], PracticePoints[slot])
                || !PracticeTargetDistanceValid(PracticePoints[0], PracticePoints[slot]))
            {
                format(reason, size, "Revisa el blanco %d: mismo interior/mundo y entre 3 y 100 metros del inicio.", slot);
                return false;
            }
        }
        reason[0] = EOS;
        return true;
    }
    return false;
}
