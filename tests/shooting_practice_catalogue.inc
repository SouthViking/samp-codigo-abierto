// Native TextDraw calls are captured by the harness. Real GTA rendering, mouse
// hit boxes and object collisions are covered by the manual client checklist.
stock CatalogueCardCount(playerid = 0)
{
    new count;
    for (new i = 0; i < PracticeCatalogueDrawCount[playerid]; i++)
    {
        new id = _:PracticeCatalogueDraws[playerid][i];
        if (TestTDFonts[id] == 5) count++;
    }
    return count;
}

stock TestModernCatalogue()
{
    ResetTest(); OpenTarget(1); MenuChoose(0); MenuInput("Entrada lateral");
    MenuChoose(1);
    new revision = PracticeRevision[0], Float:x = PracticeDraft[0][PracticePointX];
    new bool:stylesValid = true, bool:previewsValid = true, bool:labelsValid = true;
    for (new i = 0; i < PracticeCatalogueDrawCount[0]; i++)
    {
        new id = _:PracticeCatalogueDraws[0][i], action = PracticeCatalogueActions[0][i];
        stylesValid = stylesValid && TestTDAlive[id] && TestTDVisible[id]
            && (TestTDFonts[id] == 1 || TestTDFonts[id] == 5);
        if (TestTDFonts[id] == 5)
            previewsValid = previewsValid && action >= 0 && action < 6
                && TestTDModels[id] == PracticeModels[action][PracticeModelID]
                && TestTDTextSize[id][0] == 144.0 && TestTDTextSize[id][1] == 68.0
                && TestTDPosition[id][0] >= 72.0 && TestTDPosition[id][0] + 144.0 <= 566.0
                && TestTDPosition[id][1] >= 140.0 && TestTDPosition[id][1] + 68.0 <= 344.0;
        else if (action >= 0)
            labelsValid = labelsValid && !strcmp(TestTDText[id], PracticeModels[action][PracticeModelName])
                && TestTDTextSize[id][0] == 20.0 && TestTDTextSize[id][1] == 152.0;
    }
    Check(stylesValid && previewsValid && labelsValid && CatalogueCardCount() == 6 && !TestSelectionOrderWrong,
        "catalogue has six named cards, ordinary fonts and correctly sized native model previews");
    Check(!ClickCatalogueAction(SP_CATALOGUE_PREVIOUS) && !PracticeCataloguePage[0],
        "first-page Previous is disabled and not bound to an action");
    Check(!PracticeHandleCatalogueClick(0, PlayerText:INVALID_TEXT_DRAW)
        && !PracticeHandleCatalogueClick(0, PlayerText:4000)
        && !PracticeHandleCatalogueClick(1, PracticeCatalogueDraws[0][0]),
        "catalogue ignores invalid, foreign and other-player handles");
    ClickCatalogueAction(SP_CATALOGUE_NEXT);
    Check(PracticeCataloguePage[0] == 1 && CatalogueCardCount() == 3 && TestTDCount[0] == 29
        && TestCatalogue[0] && !TestCancelSelections[0] && !ClickCatalogueAction(SP_CATALOGUE_NEXT),
        "second page exposes remaining cards and keeps cursor without leaking first-page draws");
    ClickCatalogueAction(SP_CATALOGUE_PREVIOUS);
    Check(!PracticeCataloguePage[0] && CatalogueCardCount() == 6 && TestTDCount[0] == 38,
        "Previous recreates exactly one complete first page");
    for (new category = 1; category < sizeof(PracticeCategoryNames); category++)
    {
        ClickCatalogueAction(SP_CATALOGUE_CATEGORY - category);
        new models[sizeof(PracticeModels)], count = PracticeCatalogueModels(0, models);
        new bool:matches = true;
        for (new i = 0; i < PracticeCatalogueDrawCount[0]; i++)
        {
            new id = _:PracticeCatalogueDraws[0][i], action = PracticeCatalogueActions[0][i];
            if (TestTDFonts[id] == 5)
                matches = matches && action >= 0
                    && _:PracticeModels[action][PracticeModelCategory] == category;
        }
        Check(matches && count == CatalogueCardCount() && !PracticeCataloguePage[0] && TestCatalogue[0],
            "category filter displays only matching cards and resets the page");
    }
    Check(PracticeRevision[0] == revision && PracticeDraft[0][PracticePointX] == x
        && !strcmp(PracticeDraft[0][PracticePointName], "Entrada lateral") && !PracticePoints[0][1][PracticePointName][0],
        "filters and pages preserve pending edits without changing saved configuration");
    ClickCatalogueAction(SP_CATALOGUE_BACK);
    Check(!PracticeCatalogueOpen[0] && !TestCatalogue[0] && !TestTDCount[0]
        && PracticeMenuPage[0] == SP_MENU_DRAFT && PracticeHasDraft(0) && TestCancelSelections[0] == 1,
        "Back returns to draft, cleans all draws and handles native cancellation without re-entry");
    MenuChoose(1); EscapeCatalogue();
    Check(PracticeMenuPage[0] == SP_MENU_DRAFT && PracticeHasDraft(0) && !TestTDCount[0]
        && TestCancelSelections[0] == 1, "ESC retains existing draft without cancelling selection twice");
    MenuChoose(1); ClickCatalogueAction(SP_CATALOGUE_NEXT); ClickCatalogueAction(8);
    Check(PracticeDraft[0][PracticePointModel] == 1238 && !TestTDCount[0] && !TestCatalogue[0]
        && PracticeMenuPage[0] == SP_MENU_DRAFT && PracticePoints[0][1][PracticePointModel] == 1271,
        "selecting a second-page card only changes the pending model");
    MenuChoose(7);
    Check(PracticePoints[0][1][PracticePointModel] == 1238 && PracticeLoadConfiguration()
        && PracticePoints[0][1][PracticePointModel] == 1238, "Save persists the selected expanded model");

    ResetTest(); OpenTarget(0); ClickCatalogueAction(SP_CATALOGUE_BACK);
    Check(!PracticeHasDraft(0) && !PracticeCatalogueOpen[0] && !TestTDCount[0]
        && PracticeMenuPage[0] == SP_MENU_TARGETS && PracticeCountTargets(0) == 3,
        "Back from a new-target wizard releases the writer and creates nothing");
    OpenTarget(0); EscapeCatalogue();
    Check(!PracticeHasDraft(0) && !TestTDCount[0] && !TestCatalogue[0],
        "ESC also cancels an unplaced new target");
    OpenTarget(0); SelectModel(1543);
    Check(PracticeMenuPage[0] == SP_MENU_BEHAVIOUR && PracticeDraft[0][PracticePointModel] == 1543
        && !TestTDCount[0], "a bottle enters the same behavior and native placement wizard");
    MenuChoose(0); EditorResult(EDIT_RESPONSE_FINAL, 32.0);
    Check(PracticePoints[0][4][PracticePointModel] == 1543 && !PracticeHasDraft(0),
        "bottle placement commits the chosen model through the native editor");

    ResetTest(); OpenTarget(1, 0); OpenTarget(1, 1);
    Check(!PracticeCatalogueOpen[1], "another editor cannot open a catalogue in an owned zone");
    MenuChoose(1); TestAdmin[0] = false; ClickCatalogueAction(SP_CATALOGUE_NEXT);
    Check(!PracticeHasDraft(0) && !PracticeCatalogueOpen[0] && !TestCatalogue[0] && !TestTDCount[0],
        "a click revalidates RCON before allowing navigation or model changes");
    ResetTest(); OpenTarget(1); MenuChoose(1); PracticeRevision[0]++;
    ClickCatalogueAction(1);
    Check(!PracticeHasDraft(0) && !PracticeCatalogueOpen[0] && !TestTDCount[0]
        && PracticePoints[0][1][PracticePointModel] == 1271, "stale revision cannot apply a card selection");
    ResetTest(); OpenTarget(1); MenuChoose(1); TestPosition[0][0] = 101.0; EscapeCatalogue();
    Check(!PracticeHasDraft(0) && !TestTDCount[0], "leaving setup radius also invalidates the ESC return");
    ResetTest(); OpenTarget(1); MenuChoose(1); TestWorld[0] = 1; PracticeEditorTick();
    Check(!PracticeCatalogueOpen[0] && !TestTDCount[0] && !PracticeHasDraft(0),
        "dimension changes clean the catalogue in the maintenance cycle");
    ResetTest(); OpenTarget(1); MenuChoose(1); TestDisconnect(0); PracticeResetUI(0);
    Check(!PracticeCatalogueOpen[0] && !PracticeCatalogueSelecting[0] && !TestTDCount[0] && !PracticeHasDraft(0),
        "disconnect destroys catalogue resources without cancelling another client's cursor");

    ResetTest(); OpenTarget(1); TestTDFailAt = TestNextTD + 8; MenuChoose(1);
    Check(!PracticeCatalogueOpen[0] && !TestCatalogue[0] && !TestTDCount[0] && PracticeHasDraft(0)
        && PracticeMenuPage[0] == SP_MENU_DRAFT, "partial allocation failure rolls back draws and preserves existing draft");
    TestTDFailAt = 0; MenuChoose(1); TestTDFailAt = TestNextTD + 5; ClickCatalogueAction(SP_CATALOGUE_NEXT);
    Check(!PracticeCatalogueOpen[0] && !TestCatalogue[0] && !TestTDCount[0]
        && TestCancelSelections[0] == 1, "redraw allocation failure releases the retained selection cursor once");
    ResetTest(); TestTDFails = true; OpenTarget(0);
    Check(!PracticeHasDraft(0) && !PracticeCatalogueOpen[0] && !TestTDCount[0]
        && PracticeMenuPage[0] == SP_MENU_TARGETS, "new-target allocation failure releases the wizard writer");
    TestTDFails = false;
}

stock TestExpandedTargetModels()
{
    ResetTest(sizeof(PracticeModels));
    for (new model = 0; model < sizeof(PracticeModels); model++)
    {
        PracticePoints[0][model + 1][PracticePointModel] = PracticeModels[model][PracticeModelID];
        PracticePoints[0][model + 1][PracticePointNoShoot] = model % 2 != 0;
    }
    Check(PracticeSaveConfiguration(), "expanded catalogue saves with the existing v9 format");
    for (new slot = 1; slot <= sizeof(PracticeModels); slot++)
        PracticePoints[0][slot][PracticePointModel] = 1271;
    Check(PracticeLoadConfiguration(), "expanded model IDs reload without a storage migration");
    new bool:matches = true;
    for (new model = 0; model < sizeof(PracticeModels); model++)
        matches = matches && PracticePoints[0][model + 1][PracticePointModel] == PracticeModels[model][PracticeModelID]
            && PracticePoints[0][model + 1][PracticePointNoShoot] == (model % 2 != 0);
    Check(matches, "round trip retains every model and its independent shoot/protected behavior");
    PracticeRules[0][PracticeRandomOrder] = PracticeRules[0][PracticeRandomTypes] = 1;
    new slots[PRACTICE_MAX_TARGETS], models[PRACTICE_MAX_TARGETS], bool:protected[PRACTICE_MAX_TARGETS];
    matches = true;
    for (new seed = 1; seed <= 20; seed++)
    {
        TestRandomSeed = seed;
        new count = PracticeBuildSequence(0, slots, protected, models);
        for (new i = 0; i < count; i++)
        {
            new source = PracticeModelIndex(models[i]);
            matches = matches && source >= 0 && protected[i] == (source % 2 != 0);
        }
    }
    Check(matches, "randomized expanded catalogue keeps each object's configured behavior");
    PracticeShowPreview(0);
    for (new model = 0; model < sizeof(PracticeModels); model++)
        Check(TestObjectModels[PracticePreviewObjects[0][model]] == PracticeModels[model][PracticeModelID],
            "private preview uses the exact expanded model ID");
    PracticeHidePreview(0);
    for (new model = 5; model < sizeof(PracticeModels); model++)
    {
        ResetTest(1); PracticePoints[0][1][PracticePointModel] = PracticeModels[model][PracticeModelID];
        ShowFirstTarget();
        Check(TestObjectModels[PracticeSession[0][PracticeObject]] == PracticeModels[model][PracticeModelID]
            && !TestCueBeforeReady, "added object spawns from the frozen practice plan before its cue");
        Shoot(true);
        Check(TestResult == 1 && TestResultHits == 1 && TestResultShots == 1,
            "added model scores and cleans up through the existing one-hit path");
        ResetTest(1); PracticePoints[0][1][PracticePointModel] = PracticeModels[model][PracticeModelID];
        PracticePoints[0][1][PracticePointNoShoot] = true;
        // A shoot target keeps the layout startable while the added object is protected.
        PracticePoints[0][2] = PracticePoints[0][1];
        PracticePoints[0][2][PracticePointNoShoot] = false; PracticePoints[0][2][PracticePointX] = 16.0;
        ShowFirstTarget(); Shoot(false);
        Check(PracticeSession[0][PracticeIdentificationErrors] == 1 && !TestLiveObjects,
            "added protected model penalizes an air shot and retires once");
    }
}
