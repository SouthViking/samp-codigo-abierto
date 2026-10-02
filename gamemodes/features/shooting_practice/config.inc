#if defined _shooting_practice_config_included
    #endinput
#endif
#define _shooting_practice_config_included

#define PRACTICE_TARGET_POSITION_COUNT (3)

// One development layout. Coordinates are captured from the player's transform,
// without an assumed object pivot/height offset or a dependency on a target model.
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

new PracticeStartPoint[E_PRACTICE_POINT];
new PracticeTargetPoints[PRACTICE_TARGET_POSITION_COUNT][E_PRACTICE_POINT];

stock bool:PracticeIsLayoutReady()
{
    if (!PracticeStartPoint[PracticePointSet])
        return false;

    for (new index = 0; index < PRACTICE_TARGET_POSITION_COUNT; index++)
    {
        if (!PracticeTargetPoints[index][PracticePointSet]
            || PracticeTargetPoints[index][PracticePointInterior] != PracticeStartPoint[PracticePointInterior]
            || PracticeTargetPoints[index][PracticePointWorld] != PracticeStartPoint[PracticePointWorld])
            return false;
    }
    return true;
}
