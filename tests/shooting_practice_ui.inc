// Exercise production dialog dispatch and editor callbacks with deterministic
// player/streamer adapters. Rendering and mouse controls remain a client check.
stock MenuChoose(row, playerid = 0) { return PracticeHandleMenu(playerid, true, row, ""); }
stock MenuBack(playerid = 0) { return PracticeHandleMenu(playerid, false, -1, ""); }
stock bool:MainHasAction(action, playerid = 0)
{
    for (new row = 0; row < PracticeMainRowCount[playerid]; row++)
        if (PracticeMainRows[playerid][row] == action) return true;
    return false;
}
stock MainChoose(action, playerid = 0)
{
    for (new row = 0; row < PracticeMainRowCount[playerid]; row++)
        if (PracticeMainRows[playerid][row] == action) return MenuChoose(row, playerid);
    return 0;
}
stock MenuInput(const text[], playerid = 0) { return PracticeHandleMenu(playerid, true, -1, text); }
stock OpenTarget(row, playerid = 0)
{
    TestCommand_practicaconfig(playerid, ""); MenuChoose(1, playerid); MenuChoose(row, playerid);
    return 1;
}
stock SelectModel(modelid, response = 1, playerid = 0)
{
    TestCatalogue[playerid] = false; // mSelection removes its own widgets before dispatch.
    return PracticeHandleModelSelection(playerid, response, modelid);
}
stock EditorResult(response, Float:x = 12.0, Float:y = 0.0, Float:z = 0.0,
    Float:rx = 0.0, Float:ry = 0.0, Float:rz = 0.0, playerid = 0)
{
    return PracticeHandleObjectEdit(playerid, PracticeEditObject[playerid], response, x, y, z, rx, ry, rz);
}

stock TestMenuConfiguration()
{
    ResetTest(); TestAdmin[0] = false; TestCommand_practicaconfig(0, "");
    Check(PracticeMenuPage[0] == SP_MENU_NONE, "RCON is required to open configuration");
    TestAdmin[0] = true; TestCommand_practicaconfig(0, "reiniciar");
    Check(PracticeMenuPage[0] == SP_MENU_MAIN && PracticeCountTargets() == 3,
        "legacy direct arguments only open menu and cannot mutate configuration");
    Check(strfind(TestDialogBody[0], "Vista previa") != -1 && !strcmp(TestDialogTitle[0], "Campo de tiro") && MainHasAction(SP_MAIN_START_PRACTICE),
        "main menu displays preview and an available start action");
    TestPosition[0][0] = 1.0; MenuChoose(0);
    Check(PracticeMenuPage[0] == SP_MENU_MAIN && !PracticeConfigurationEditing()
        && PracticePoints[0][PracticePointX] == 1.0 && PracticeCountTargets() == 3
        && PracticeLoadConfiguration() && PracticePoints[0][PracticePointX] == 1.0,
        "start row directly captures and persists current position while preserving same-dimension targets");
    MenuBack(); Check(PracticeMenuPage[0] == SP_MENU_NONE, "closing main releases dialog");

    ResetTest(); OpenTarget(1);
    Check(strfind(TestDialogBody[0], "Guardar cambios") > strfind(TestDialogBody[0], "Eliminar blanco")
        && strfind(TestDialogBody[0], "Duplicar blanco") < strfind(TestDialogBody[0], "Eliminar blanco"),
        "saved-target actions end with Save after Duplicate and Delete");
    MenuChoose(2);
    Check(!strcmp(TestDialogBody[0], "Blanco de tiro (disparar)\nBlanco protegido (no disparar)"),
        "behavior menu uses the protected label consistently");
    MenuBack(); MenuChoose(0); MenuInput("Pasillo norte");
    Check(!strcmp(PracticeDraft[0][PracticePointName], "Pasillo norte") && !PracticePoints[1][PracticePointName][0],
        "name remains draft until explicit save");
    MenuChoose(1);
    Check(PracticeCatalogueOpen[0] && TestTDCount[0] == sizeof(PracticeModels), "catalogue shows named model previews");
    SelectModel(1585);
    Check(PracticeDraft[0][PracticePointModel] == 1585 && !PracticeDraft[0][PracticePointNoShoot]
        && !TestTDCount[0], "civilian model can have shoot behavior and catalogue labels are cleaned");
    MenuChoose(2); MenuChoose(1); MenuBack();
    Check(!PracticeConfigurationEditing() && PracticePoints[1][PracticePointModel] == 1271
        && !PracticePoints[1][PracticePointNoShoot] && !PracticePoints[1][PracticePointName][0],
        "discard cancels all pending name/model/behavior changes");

    OpenTarget(1); MenuChoose(0); MenuInput("{FF0000}bad");
    Check(!PracticeDraft[0][PracticePointName][0], "names reject injected color codes");
    MenuChoose(0); MenuInput("123456789012345678901234567890123");
    Check(!PracticeDraft[0][PracticePointName][0], "names reject more than 32 characters");
    MenuChoose(0); MenuInput("Puerta principal"); MenuChoose(1); SelectModel(1583);
    MenuChoose(2); MenuChoose(1); PracticeRules[PracticeRandomTypes] = 1; MenuChoose(6);
    Check(PracticePoints[1][PracticePointNoShoot] && PracticePoints[1][PracticePointModel] == 1583
        && !strcmp(PracticePoints[1][PracticePointName], "Puerta principal") && !PracticeRules[PracticeRandomTypes]
        && !PracticeConfigurationEditing(), "save commits independent model/role/name and fixed assignment together");
    Check(PracticeLoadConfiguration() && PracticePoints[1][PracticePointNoShoot]
        && !strcmp(PracticePoints[1][PracticePointName], "Puerta principal"), "dialog saves survive reload");
    OpenTarget(1); MenuChoose(0); MenuInput("-"); MenuChoose(6);
    Check(!PracticePoints[1][PracticePointName][0], "dash restores automatic target name");

    ResetTest(0); PracticeBeginDraft(0, 1, true); PracticeDraftWizard[0] = false; PracticeShowDraftMenu(0);
    Check(strfind(TestDialogBody[0], "Guardar cambios") > strfind(TestDialogBody[0], "Mover y orientar")
        && strfind(TestDialogBody[0], "Duplicar blanco") == -1, "new-target actions also end with Save");
    MenuChoose(4);
    Check(PracticePoints[1][PracticePointSet] && !PracticeConfigurationEditing(), "new-target final Save row commits its draft");
    ResetTest(0); OpenTarget(0);
    Check(PracticeDraftNew[0] && PracticeCatalogueOpen[0] && !PracticeCountTargets(), "add begins an unsaved model wizard");
    SelectModel(0, 0);
    Check(!PracticeConfigurationEditing() && !PracticeCountTargets() && !TestTDCount[0], "cancel add wizard allocates no saved point");
    OpenTarget(0); SelectModel(1586); MenuChoose(1);
    Check(TestLiveObjects == 1 && TestEditorLock[0] && TestObjectOwner[PracticeEditObject[0]] == 0,
        "new model selection and behavior open a private native editor");
    EditorResult(EDIT_RESPONSE_FINAL, 14.0, 1.0, 2.0, -15.0, 370.0, 725.0);
    Check(PracticeCountTargets() == 1 && PracticePoints[1][PracticePointModel] == 1586
        && PracticePoints[1][PracticePointNoShoot] && PracticePoints[1][PracticePointRotX] == 345.0
        && PracticePoints[1][PracticePointRotY] == 10.0 && PracticePoints[1][PracticePointAngle] == 5.0,
        "native confirm saves all axes and normalizes rotation");
    PracticeStart(0);
    Check(!PracticeIsBusy() && !TestSoundCount[0], "incomplete all-no-shoot layout cannot start");
    OpenTarget(0); SelectModel(1583); MenuChoose(0); EditorResult(EDIT_RESPONSE_FINAL, 20.0);
    ShowFirstTarget(); Shoot(false); AdvanceInterval(); Shoot(true);
    Check(TestResultErrors == 1 && TestResultHits == 1 && TestResultPoints == 0,
        "custom models and first-position no-shoot keep air-shot penalty and score totals");

    ResetTest(3); TestSetNoShoot(1); TestCommand_practicaconfig(0, ""); MenuChoose(2);
    MenuChoose(4); MenuInput("9");
    Check(PracticeRules[PracticeExposure] == 9000, "rule input saves exposure");
    MenuChoose(4); MenuInput("61"); MenuInput("-1"); MenuInput("2x"); MenuInput("");
    Check(PracticeRules[PracticeExposure] == 9000 && PracticeMenuPage[0] == SP_MENU_NUMBER, "invalid rule input stays editable without changes");
    MenuBack(); MenuChoose(3); MenuInput("2"); MenuChoose(5); MenuInput("0");
    MenuChoose(2); MenuInput("1"); MenuChoose(1); MenuInput("2");
    Check(PracticeRules[PracticePreparation] == 2 && !PracticeRules[PracticeInterval]
        && PracticeRules[PracticeMaxErrors] == 1 && PracticeRules[PracticePassPoints] == 2,
        "rule ranges accept allowed zero interval and valid counts");
    MenuChoose(1); MenuInput("3"); MenuBack(); MenuChoose(2); MenuInput("2"); MenuBack();
    Check(PracticeRules[PracticePassPoints] == 2 && PracticeRules[PracticeMaxErrors] == 1, "rules cannot exceed available target categories");
    MenuChoose(0); MenuChoose(1); MenuChoose(2); MenuBack(); MenuChoose(6); MenuChoose(7);
    Check(PracticeRules[PracticeWeaponMask] == (1 << (23 - PRACTICE_WEAPON_FIRST)) && PracticeRules[PracticeRandomOrder] && PracticeRules[PracticeRandomTypes],
        "weapon selection and both randomization toggles save");
    PracticeRules[PracticeWeaponMask] = PRACTICE_WEAPON_DEFAULT; ShowFirstTarget(); TestCommand_practicaconfig(0, ""); MenuChoose(2); MenuChoose(6);
    Check(PracticeRules[PracticeRandomOrder] != 0, "active practice blocks configuration changes");
    TestCommand_practicaconfig(0, ""); MenuChoose(4);
    Check(!PracticeIsBusy(), "main menu can cancel own active practice");

    ResetTest(); TestWorld[0] = 7; TestCommand_practicaconfig(0, ""); MenuChoose(0);
    Check(PracticeMenuPage[0] == SP_MENU_START_CONFIRM && PracticeCountTargets() == 3
        && PracticePoints[0][PracticePointWorld] == 0, "moving start to another world requires confirmation");
    MenuBack(); Check(PracticeCountTargets() == 3 && !PracticeConfigurationEditing(), "cancel start change retains station");
    MenuChoose(0); MenuChoose(0);
    Check(!PracticeCountTargets() && PracticePoints[0][PracticePointWorld] == 7, "confirmed dimension change clears old targets");
    TestPosition[0][0] = 25.0; MenuChoose(0);
    Check(PracticePoints[0][PracticePointX] == 25.0 && PracticePoints[0][PracticePointAngle] == 90.0,
        "start uses character position and facing");

    ResetTest(); TestCommand_practicaconfig(0, ""); MainChoose(SP_MAIN_RESET); MenuBack();
    Check(PracticeCountTargets() == 3, "cancel reset preserves layout");
    MainChoose(SP_MAIN_RESET); PracticeShowRules(1); MenuChoose(6, 1); MenuChoose(0);
    Check(PracticeCountTargets() == 3 && PracticeMenuPage[0] == SP_MENU_MAIN, "stale reset dialog cannot delete newer configuration");
    MainChoose(SP_MAIN_RESET); MenuChoose(0);
    Check(!PracticePoints[0][PracticePointSet] && !PracticeCountTargets() && !PracticeRules[PracticeRandomOrder]
        && PracticeLoadConfiguration() && !PracticePoints[0][PracticePointSet], "confirmed reset is complete and persistent");
    OpenTarget(0); Check(!PracticeConfigurationEditing(), "cannot add target before marking start");

    ResetTest(); OpenTarget(2); MenuChoose(5); MenuBack();
    Check(PracticePoints[2][PracticePointSet], "delete is canceled safely");
    MenuChoose(5); MenuChoose(0);
    Check(!PracticePoints[2][PracticePointSet] && PracticePoints[1][PracticePointSet] && PracticePoints[3][PracticePointSet],
        "delete removes only selected point");
    OpenTarget(2);
    Check(PracticeDraftSlot[0] == 3, "sparse target rows resolve to actual slot IDs");
    MenuChoose(4); EditorResult(EDIT_RESPONSE_FINAL, 33.0);
    Check(PracticePoints[2][PracticePointX] == 33.0 && PracticePoints[3][PracticePointX] == 24.0,
        "duplicate fills a free slot without modifying source");
    ResetTest(10); OpenTarget(0);
    Check(PracticeCountTargets() == 10 && !PracticeConfigurationEditing(), "add respects ten-position cap");
    OpenTarget(1); MenuChoose(4);
    Check(PracticeDraftSlot[0] == 1 && !PracticeDraftNew[0] && !PracticeEditObject[0] && TestLiveObjects == 10, "duplicate respects cap without losing original draft or layout context");
    PracticeResetUI(0);
}

stock TestVisualEditor()
{
    ResetTest(); OpenTarget(1); MenuChoose(3); new edited = PracticeEditObject[0];
    EditorResult(EDIT_RESPONSE_UPDATE, 18.0, 1.0, 2.0, 10.0, 20.0, 30.0);
    Check(PracticePoints[1][PracticePointX] == 8.0, "intermediate object updates do not save");
    PracticeHandleObjectEdit(1, edited, EDIT_RESPONSE_FINAL, 80.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    Check(PracticeEditObject[0] == edited && PracticePoints[1][PracticePointX] == 8.0, "another player cannot confirm owned editor object");
    EditorResult(EDIT_RESPONSE_FINAL, 19.0, 2.0, 3.0, 15.0, 25.0, 35.0);
    Check(PracticePoints[1][PracticePointX] == 19.0 && PracticePoints[1][PracticePointY] == 2.0
        && PracticePoints[1][PracticePointZ] == 3.0 && PracticePoints[1][PracticePointRotX] == 15.0
        && PracticePoints[1][PracticePointRotY] == 25.0 && PracticePoints[1][PracticePointAngle] == 35.0,
        "FINAL coordinates are authoritative even when UPDATE lags");
    Check(TestLiveObjects == 3 && !PracticeEditObject[0] && !TestEditorLock[0] && !PracticeConfigurationEditing(), "successful final releases editor and lock while keeping saved targets visible");
    OpenTarget(1); MenuChoose(3); EditorResult(EDIT_RESPONSE_CANCEL, 90.0);
    Check(PracticePoints[1][PracticePointX] == 19.0 && TestLiveObjects == 3 && !PracticeEditObject[0] && !PracticeConfigurationEditing(), "native cancel preserves previous coordinates and restores saved target");
    OpenTarget(1); MenuChoose(3); EditorResult(EDIT_RESPONSE_FINAL, 1.0);
    Check(PracticeMenuPage[0] == SP_MENU_EDIT_ERROR && PracticePoints[1][PracticePointX] == 19.0
        && TestLiveObjects == 3 && !PracticeEditObject[0] && PracticeConfigurationEditing(), "too-close placement stays unsaved and restores context while offering repair");
    MenuChoose(0); EditorResult(EDIT_RESPONSE_FINAL, 101.0); MenuChoose(0);
    EditorResult(EDIT_RESPONSE_FINAL, 20.0, 0.0, 0.0, 36001.0);
    Check(PracticeMenuPage[0] == SP_MENU_EDIT_ERROR && PracticePoints[1][PracticePointX] == 19.0,
        "distance and nonfinite rotation guard reject invalid final input");
    MenuChoose(0); EditorResult(EDIT_RESPONSE_FINAL, Float:0x7FC00000);
    Check(PracticeMenuPage[0] == SP_MENU_EDIT_ERROR, "NaN coordinates cannot be saved");
    MenuBack(); Check(!PracticeConfigurationEditing(), "discard failed edit releases station");

    ResetTest(); OpenTarget(1); OpenTarget(2, 1); PracticeStart(2);
    Check(PracticeEditorOwner == 0 && !PracticeHasDraft(1) && !PracticeIsBusy(), "station writer excludes competing editors and practice starts");
    PracticeDiscardDraft(0); playerEditingObjectID[0] = 4; OpenTarget(1);
    Check(!PracticeConfigurationEditing(), "general object editor blocks practice editor");
    playerEditingObjectID[0] = -1; OpenTarget(1); MenuChoose(3);
    Check(TestEditorLock[0] == 1, "native edit publishes reciprocal general-editor lock");
    TestCommand_practicaconfig(0, "");
    Check(PracticeEditObject[0] != 0, "config command cannot replace active native editing");
    TestWorld[0] = 1; PracticeEditorTick();
    Check(!TestLiveObjects && !PracticeConfigurationEditing() && !TestEditorLock[0] && TestCancelEdits[0] == 1,
        "world change cancels native editor and releases lock");
    TestWorld[0] = 0; OpenTarget(1); MenuChoose(1); TestAdmin[0] = false; PracticeEditorTick();
    Check(!TestCatalogue[0] && !TestTDCount[0] && !PracticeConfigurationEditing(), "lost RCON cleans catalogue and pending draft");
    TestAdmin[0] = true; OpenTarget(1); MenuChoose(3); TestHealth[0] = 0.0; PracticeEditorTick();
    Check(!TestLiveObjects && !PracticeConfigurationEditing(), "death cancels edit state");
    TestHealth[0] = 100.0; OpenTarget(1); MenuChoose(3); TestConnected[0] = false; PracticeResetUI(0);
    Check(!TestLiveObjects && !TestEditorLock[0] && !PracticeConfigurationEditing(), "disconnect removes private editing objects");
    TestConnected[0] = true; PracticeResetUI(0); OpenTarget(2);
    Check(PracticeDraftSlot[0] == 2, "reused player ID starts a fresh draft");
    PracticeResetUI(0);

    ResetTest(); TestVisible = false; OpenTarget(1); MenuChoose(3);
    Check(TestLiveObjects == 3 && !PracticeEditObject[0] && PracticeMenuPage[0] == SP_MENU_DRAFT, "invisible temporary object leaves no active editor and preserves context");
    TestVisible = true; TestEditFails = true; MenuChoose(3);
    Check(TestLiveObjects == 3 && !PracticeEditObject[0] && !TestEditorLock[0], "native edit failure cleans temporary object and PVar while preserving context");
    TestEditFails = false; MenuChoose(3); TestDestroyObject(PracticeEditObject[0]); PracticeEditorTick();
    Check(!PracticeConfigurationEditing() && !TestEditorLock[0], "lost edit object releases pending draft");
}

stock TestPreviews()
{
    ResetTest(3); PracticePoints[2][PracticePointModel] = 1585; PracticePoints[2][PracticePointNoShoot] = true;
    PracticePoints[2][PracticePointRotX] = 11.0; PracticePoints[2][PracticePointRotY] = 22.0;
    PracticePoints[2][PracticePointAngle] = 33.0;
    format(PracticePoints[2][PracticePointName], PRACTICE_NAME_SIZE, "Civil junto a puerta");
    TestCommand_practicaconfig(0, ""); MenuChoose(3);
    new objectid = PracticePreviewObjects[0][1], cp = PracticePreviewCheckpoint[0], label = PracticePreviewLabels[0][2];
    Check(PracticePreviewActive[0] && PracticeMenuPage[0] == SP_MENU_NONE && TestLiveObjects == 3 && TestLiveLabels == 4 && TestLiveCP == 1,
        "preview closes dialog and shows all targets plus start checkpoint");
    Check(TestObjectOwner[objectid] == 0 && TestCPOwner[cp] == 0 && TestLabelOwner[label] == 0,
        "objects, checkpoint and labels are restricted to requesting admin");
    Check(TestCPPosition[cp][0] == PracticePoints[0][PracticePointX] && TestObjectPosition[objectid][0] == 16.0
        && TestObjectPosition[objectid][3] == 11.0 && TestObjectPosition[objectid][4] == 22.0
        && TestObjectPosition[objectid][5] == 33.0 && TestObjectModels[objectid] == 1585,
        "preview uses saved start and exact target model/position/rotation");
    Check(!TestMaterialText[objectid][0] && strfind(TestLabelText[label], "Civil junto a puerta") != -1
        && strfind(TestLabelText[label], "NO DISPARAR") != -1, "native textures remain intact and preview labels identify behavior");
    Shoot(true); TestNow += 10000; PracticeTick(TestNow);
    Check(!TestSoundCount[0] && !PracticeIsBusy() && !PracticeSession[PracticeHits], "preview has no countdown, audio or scoring");
    PracticeShowPreview(1); PracticeHidePreview(0);
    Check(TestLiveObjects == 3 && TestLiveCP == 1 && TestLiveLabels == 4 && PracticePreviewActive[1],
        "hiding one private preview does not remove another admin's preview");
    PracticeHidePreview(1); PracticeHidePreview(1);
    Check(!TestLiveObjects && !TestLiveCP && !TestLiveLabels, "repeated hide has no leaked resources");

    PracticeShowPreview(0); TestCommand_practicaconfig(0, ""); MenuChoose(3);
    Check(!PracticePreviewActive[0] && !TestLiveObjects, "root preview entry toggles off");
    PracticeShowPreview(0); PracticeShowPreview(1); PracticeStart(2);
    Check(PracticeIsBusy() && !TestLiveObjects && !TestLiveCP && !TestLiveLabels
        && !PracticePreviewActive[0] && !PracticePreviewActive[1], "practice start clears every private preview before countdown");
    PracticeCleanup(); PracticeShowPreview(0); PracticeShowRules(1); MenuChoose(6, 1);
    Check(TestLiveObjects == 3 && PracticePreviewActive[0], "saved rule changes preserve current private preview");
    PracticeShowPreview(0); TestInterior[0] = 1; PracticeEditorTick();
    Check(!TestLiveObjects && !TestLiveCP && !TestLiveLabels, "interior change removes private preview");
    TestInterior[0] = 0; PracticeShowPreview(0); TestConnected[0] = false; PracticeResetUI(0);
    Check(!TestLiveObjects && !TestLiveCP && !TestLiveLabels, "disconnect removes preview even after player leaves");

    ResetTest(); TestLabelFailAt = 3; PracticeShowPreview(0);
    Check(!PracticePreviewActive[0] && !TestLiveObjects && !TestLiveCP && !TestLiveLabels,
        "partial preview failure rolls back all previously created items");
    TestLabelFailAt = 0; TestCPFails = true; PracticeShowPreview(0);
    Check(!PracticePreviewActive[0] && !TestLiveObjects && !TestLiveCP, "checkpoint allocation failure aborts preview");
    TestCPFails = false; TestCreateFails = true; PracticeShowPreview(0);
    Check(!TestLiveLabels && !TestLiveCP && !PracticePreviewActive[0], "target allocation failure cleans start preview");
    TestCreateFails = false; TestWorld[0] = 3; PracticeShowPreview(0);
    Check(!PracticePreviewActive[0], "preview requires station dimension");
}

stock TestStorageV4()
{
    ResetTest(); TestSaveFails = true; TestCommand_practicaconfig(0, ""); MenuChoose(2); MenuChoose(4); MenuInput("9");
    MenuBack();
    Check(PracticeSavePending && PracticeRules[PracticeExposure] == 9000 && strfind(TestDialogBody[0], "Reintentar guardado") != -1
        && !strcmp(TestDialogTitle[0], "Campo de tiro") && strfind(TestDialogBody[0], "Cambios sin guardar") != -1, "save failure retains edits and exposes retry and pending status");
    TestSaveFails = false; MainChoose(SP_MAIN_RETRY_SAVE);
    Check(!PracticeSavePending && PracticeLoadConfiguration() && PracticeRules[PracticeExposure] == 9000,
        "save retry persists retained edits and clears pending status");
    ResetTest(); PracticePoints[1][PracticePointModel] = 1584;
    PracticePoints[1][PracticePointRotX] = 15.25; PracticePoints[1][PracticePointRotY] = 36.5;
    PracticePoints[1][PracticePointAngle] = 72.75; PracticePoints[2][PracticePointNoShoot] = true;
    format(PracticePoints[1][PracticePointName], PRACTICE_NAME_SIZE, "Entrada lateral 100%%");
    PracticePoints[3][PracticePointSet] = false; PracticePoints[3][PracticePointModel] = 0;
    Check(PracticeSaveConfiguration(), "v5 writes complete configuration");
    PracticePoints[1][PracticePointModel] = 1271; PracticePoints[1][PracticePointRotX] = 0.0;
    PracticePoints[1][PracticePointName][0] = EOS;
    Check(PracticeLoadConfiguration() && PracticePoints[1][PracticePointModel] == 1584
        && PracticePoints[1][PracticePointRotX] == 15.25 && PracticePoints[1][PracticePointRotY] == 36.5
        && PracticePoints[1][PracticePointAngle] == 72.75 && !strcmp(PracticePoints[1][PracticePointName], "Entrada lateral 100%")
        && PracticePoints[2][PracticePointNoShoot] && !PracticePoints[3][PracticePointSet],
        "v5 round trip retains names, all rotations, individual models, sparse positions and behavior");
    new File:bad = fopen(PRACTICE_CONFIG_FILE, io_write);
    fwrite(bad, "SARP_PRACTICE 4\r\n24 1 3 5 1 0 0 0\r\n"); fclose(bad);
    Check(!PracticeLoadConfiguration() && PracticePoints[1][PracticePointModel] == 1584, "truncated v4 never partially replaces data");
    PracticePoints[1][PracticePointModel] = 9999; PracticeSaveConfiguration(); PracticePoints[1][PracticePointModel] = 1584;
    Check(!PracticeLoadConfiguration() && PracticePoints[1][PracticePointModel] == 1584, "unknown stored model rejects entire load");
    PracticePoints[1][PracticePointRotY] = -1.0; PracticeSaveConfiguration(); PracticePoints[1][PracticePointRotY] = 36.5;
    Check(!PracticeLoadConfiguration() && PracticePoints[1][PracticePointRotY] == 36.5, "invalid stored rotation cannot overwrite configuration");
    format(PracticePoints[1][PracticePointName], PRACTICE_NAME_SIZE, "Injected\nrow"); PracticeSaveConfiguration();
    PracticePoints[1][PracticePointName][0] = EOS;
    Check(!PracticeLoadConfiguration(), "malformed multiline name rejects load");
    PracticeSaveConfiguration(); bad = fopen(PRACTICE_CONFIG_FILE, io_readwrite);
    fwrite(bad, "SARP_PRACTICE 9\r\n"); fclose(bad);
    Check(!PracticeLoadConfiguration(), "unsupported storage version rejected");

    ResetTest(5); TestSetNoShoot(2);
    for (new slot = 1; slot <= 5; slot++) PracticePoints[slot][PracticePointModel] = PracticeModels[slot - 1][PracticeModelID];
    new slots[PRACTICE_MAX_TARGETS], models[PRACTICE_MAX_TARGETS], bool:noShoot[PRACTICE_MAX_TARGETS], bool:valid = true;
    PracticeRules[PracticeRandomOrder] = PracticeRules[PracticeRandomTypes] = 1;
    for (new seed = 1; seed <= 50; seed++)
    {
        TestRandomSeed = seed; new total = PracticeBuildSequence(slots, noShoot, models);
        for (new i = 0; i < total; i++)
        {
            new source = PracticeModelIndex(models[i]) + 1;
            valid = valid && source >= 1 && source <= 5 && noShoot[i] == PracticePoints[source][PracticePointNoShoot];
        }
    }
    Check(valid, "random distribution always moves model and behavior together");
    ShowFirstTarget(); new objectid = PracticeSession[PracticeObject];
    Check(TestObjectModels[objectid] == PracticeSequenceModels[0], "practice spawns the frozen plan's model");
    PracticeCleanup(); Check(!PracticeSequenceModels[0], "session cleanup clears frozen models");
}
