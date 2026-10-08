#if defined SP_CONFIG_INCLUDED
    #endinput
#endif
#define SP_CONFIG_INCLUDED

#define PRACTICE_MAX_TARGETS (10)
#define PRACTICE_START_RADIUS (1.5)
#define PRACTICE_ACTIVE_RADIUS (3.0)
#define PRACTICE_MAX_DISTANCE (100.0)
// Legacy layouts keep the marked box; each target can now select its own model.
#define PRACTICE_TARGET_MODEL (1271)
#define PRACTICE_TARGET_Z_OFFSET (0.0)
#define PRACTICE_NAME_SIZE (33)
#define PRACTICE_WEAPON_FIRST (22)
#define PRACTICE_WEAPON_LAST (34)
#define PRACTICE_WEAPONS_ALL ((1 << (PRACTICE_WEAPON_LAST - PRACTICE_WEAPON_FIRST + 1)) - 1)
#define PRACTICE_WEAPON_DEFAULT (1 << (24 - PRACTICE_WEAPON_FIRST))
#define PRACTICE_CONFIG_FILE "shooting_practice.cfg"
enum E_PRACTICE_POINT
{
    bool:PracticePointSet,
    Float:PracticePointX,
    Float:PracticePointY,
    Float:PracticePointZ,
    Float:PracticePointAngle,
    PracticePointInterior,
    PracticePointWorld,
    bool:PracticePointNoShoot,
    PracticePointModel,
    Float:PracticePointRotX,
    Float:PracticePointRotY,
    PracticePointName[PRACTICE_NAME_SIZE]
}

enum E_PRACTICE_RULE
{
    PracticeWeaponMask,
    PracticePassPoints,
    PracticePreparation,
    PracticeExposure,
    PracticeInterval,
    PracticeMaxErrors,
    PracticeRandomOrder,
    PracticeRandomTypes
}

// Slot 0 is the start; slots 1..10 are optional target positions.
// Configuration is separate from the single station's runtime session.
new PracticePoints[PRACTICE_MAX_TARGETS + 1][E_PRACTICE_POINT];
// Preparation is whole seconds. Exposure and interval are milliseconds in 100 ms steps.
new PracticeRules[E_PRACTICE_RULE] = {PRACTICE_WEAPON_DEFAULT, 1, 3, 5000, 1000, 0, 0, 0};
new PracticeRevision;
new bool:PracticeSavePending;

enum E_PRACTICE_MODEL { PracticeModelID, PracticeModelName[24] }
new const PracticeModels[][E_PRACTICE_MODEL] = {
    {1271, "Caja marcada"},
    {1583, "Silueta armada A"},
    {1584, "Silueta armada B"},
    {1585, "Silueta civil A"},
    {1586, "Silueta civil B"}
};

new const PracticeWeaponNames[][32] = {
    "Pistola", "Pistola silenciada", "Desert Eagle", "Escopeta", "Escopeta recortada",
    "Escopeta de combate", "Micro Uzi", "MP5", "AK-47", "M4", "Tec-9", "Rifle", "Rifle de francotirador"
};

stock PracticeWeaponName(weaponid, name[], size = sizeof(name))
{
    if (weaponid >= 22 && weaponid <= 34) format(name, size, "%s", PracticeWeaponNames[weaponid - 22]);
    else format(name, size, "Sin seleccionar");
    return 1;
}

stock PracticeModelIndex(modelid)
{
    for (new index = 0; index < sizeof(PracticeModels); index++)
        if (PracticeModels[index][PracticeModelID] == modelid) return index;
    return -1;
}

stock bool:PracticeNameValid(const name[])
{
    if (strlen(name) >= PRACTICE_NAME_SIZE) return false;
    for (new i = 0; name[i]; i++)
        if (name[i] < ' ' || name[i] == '{' || name[i] == '}') return false;
    return true;
}

stock PracticePointDisplayName(const point[E_PRACTICE_POINT], slot, name[], size = sizeof(name))
{
    if (point[PracticePointName][0]) format(name, size, "%s", point[PracticePointName]);
    else if (!slot) format(name, size, "Inicio");
    else format(name, size, "Blanco %d", slot);
    return 1;
}

stock bool:PracticeIsSupportedWeapon(weaponid)
{
    return weaponid >= PRACTICE_WEAPON_FIRST && weaponid <= PRACTICE_WEAPON_LAST;
}

stock bool:PracticeWeaponAllowed(weaponid, mask)
{
    return PracticeIsSupportedWeapon(weaponid) && (mask & (1 << (weaponid - PRACTICE_WEAPON_FIRST))) != 0;
}

stock bool:PracticeWeaponsValid(mask)
{
    return mask > 0 && (mask & ~PRACTICE_WEAPONS_ALL) == 0;
}

stock PracticeWeaponCount(mask)
{
    new count;
    for (new weaponid = PRACTICE_WEAPON_FIRST; weaponid <= PRACTICE_WEAPON_LAST; weaponid++)
        if (PracticeWeaponAllowed(weaponid, mask)) count++;
    return count;
}

stock PracticeWeaponsSummary(mask, text[], size = sizeof(text))
{
    new count = PracticeWeaponCount(mask);
    if (count == 1)
    {
        for (new weaponid = PRACTICE_WEAPON_FIRST; weaponid <= PRACTICE_WEAPON_LAST; weaponid++)
            if (PracticeWeaponAllowed(weaponid, mask)) return PracticeWeaponName(weaponid, text, size);
    }
    format(text, size, "%d armas seleccionadas", count);
    return 1;
}

stock PracticeFormatWeapons(mask, text[], size = sizeof(text))
{
    text[0] = EOS;
    for (new weaponid = PRACTICE_WEAPON_FIRST; weaponid <= PRACTICE_WEAPON_LAST; weaponid++)
    {
        if (!PracticeWeaponAllowed(weaponid, mask)) continue;
        if (text[0]) strcat(text, ", ", size);
        strcat(text, PracticeWeaponNames[weaponid - PRACTICE_WEAPON_FIRST], size);
    }
    return 1;
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
    return PracticeWeaponsValid(rules[PracticeWeaponMask])
        && rules[PracticePassPoints] >= 1 && rules[PracticePassPoints] <= PRACTICE_MAX_TARGETS
        && rules[PracticePreparation] >= 1 && rules[PracticePreparation] <= 30
        && rules[PracticeExposure] >= 100 && rules[PracticeExposure] <= 60000 && rules[PracticeExposure] % 100 == 0
        && rules[PracticeInterval] >= 0 && rules[PracticeInterval] <= 30000 && rules[PracticeInterval] % 100 == 0
        && rules[PracticeMaxErrors] >= 0 && rules[PracticeMaxErrors] <= PRACTICE_MAX_TARGETS
        && (rules[PracticeRandomOrder] == 0 || rules[PracticeRandomOrder] == 1)
        && (rules[PracticeRandomTypes] == 0 || rules[PracticeRandomTypes] == 1);
}

stock PracticeCountNoShootTargets()
{
    new count;
    for (new slot = 1; slot <= PRACTICE_MAX_TARGETS; slot++)
        if (PracticePoints[slot][PracticePointSet] && PracticePoints[slot][PracticePointNoShoot]) count++;
    return count;
}

stock PracticeCountShootTargets()
{
    return PracticeCountTargets() - PracticeCountNoShootTargets();
}

stock bool:PracticePointValid(const point[E_PRACTICE_POINT])
{
    return (point[PracticePointSet] == false || point[PracticePointSet] == true)
        && (point[PracticePointNoShoot] == false || point[PracticePointNoShoot] == true)
        && (point[PracticePointSet] || !point[PracticePointNoShoot])
        && point[PracticePointX] >= -20000.0 && point[PracticePointX] <= 20000.0
        && point[PracticePointY] >= -20000.0 && point[PracticePointY] <= 20000.0
        && point[PracticePointZ] >= -20000.0 && point[PracticePointZ] <= 20000.0
        && point[PracticePointAngle] >= 0.0 && point[PracticePointAngle] <= 360.0
        && point[PracticePointRotX] >= 0.0 && point[PracticePointRotX] <= 360.0
        && point[PracticePointRotY] >= 0.0 && point[PracticePointRotY] <= 360.0
        && (point[PracticePointModel] == 0 || PracticeModelIndex(point[PracticePointModel]) >= 0)
        && PracticeNameValid(point[PracticePointName])
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
        format(reason, size, "Falta marcar el inicio. Abre Punto de inicio en /practicaconfig.");
    else if (!PracticePointValid(PracticePoints[0]) || PracticePoints[0][PracticePointNoShoot])
        format(reason, size, "La posición de inicio no es válida. Vuelve a marcarla.");
    else if (!PracticeRulesValid(PracticeRules))
        format(reason, size, "La configuración de armas, puntos o tiempos no es válida.");
    else if (!PracticeCountTargets())
        format(reason, size, "Falta un blanco. Abre Blancos en /practicaconfig y elige Añadir.");
    else
    {
        if (PracticeCountShootTargets() < 1)
        {
            format(reason, size, "Falta un blanco de tiro. Revisa Comportamiento en el menú de Blancos.");
            return false;
        }
        if (PracticeRules[PracticePassPoints] > PracticeCountShootTargets())
        {
            format(reason, size, "Los puntos requeridos superan los blancos de tiro. Revisa Reglas del ejercicio.");
            return false;
        }
        if (PracticeRules[PracticeMaxErrors] > PracticeCountNoShootTargets())
        {
            format(reason, size, "Los errores permitidos superan los blancos protegidos. Revisa Reglas del ejercicio.");
            return false;
        }
        for (new slot = 1; slot <= PRACTICE_MAX_TARGETS; slot++)
        {
            if (!PracticePoints[slot][PracticePointSet]) continue;
            if (!PracticePointValid(PracticePoints[slot]) || PracticeModelIndex(PracticePoints[slot][PracticePointModel]) < 0
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
