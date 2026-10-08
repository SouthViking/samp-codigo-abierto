// Behavioural checks for preview repair, fixed-point persistence and cue pacing.
stock TestPreviewProtection()
{
    ResetTest(3);
    PracticePoints[1][PracticePointModel] = 1583;
    PracticePoints[1][PracticePointRotX] = 12.5;
    PracticePoints[1][PracticePointRotY] = 35.0;
    PracticePoints[1][PracticePointAngle] = 90.0;
    PracticeShowPreview(0); PracticeShowPreview(1);
    new hit = PracticePreviewObjects[0][0], neighbour = PracticePreviewObjects[0][1],
        otherViewer = PracticePreviewObjects[1][0], checkpoint = PracticePreviewCheckpoint[0],
        label = PracticePreviewLabels[0][2], created = TestNextObject, revision = PracticeRevision;
    Check(PracticeProtectPreviewShot(0, hit) && PracticeProtectPreviewShot(0, hit)
        && TestNextObject == created && TestObjects[hit],
        "preview hit is vetoed and duplicate repair waits until after shot dispatch");
    Check(!PracticeProtectPreviewShot(1, hit) && !PracticeProtectPreviewShot(0, otherViewer)
        && !PracticeProtectPreviewShot(0, 0) && !PracticeProtectPreviewShot(0, 999),
        "protection recognizes only the shooter's own preview handles");
    PracticeRepairPreviews();
    new repaired = PracticePreviewObjects[0][0];
    Check(repaired != hit && !TestObjects[hit] && TestObjects[repaired] && TestNextObject == created + 1,
        "one hit replaces exactly one private model even while the old server handle was valid");
    Check(TestObjectModels[repaired] == 1583 && TestObjectOwner[repaired] == 0
        && TestObjectPosition[repaired][0] == 8.0 && TestObjectPosition[repaired][3] == 12.5
        && TestObjectPosition[repaired][4] == 35.0 && TestObjectPosition[repaired][5] == 90.0,
        "replacement retains saved model, position and complete orientation");
    Check(PracticePreviewObjects[0][1] == neighbour && PracticePreviewObjects[1][0] == otherViewer
        && PracticePreviewCheckpoint[0] == checkpoint && PracticePreviewLabels[0][2] == label
        && PracticeRevision == revision && !PracticeIsBusy(),
        "repair preserves neighbours, other viewers, start marker, configuration and scoring");
    PracticeProtectPreviewShot(0, repaired); PracticeHidePreview(0);
    created = TestNextObject; PracticeRepairPreviews();
    Check(!PracticePreviewActive[0] && TestNextObject == created && TestLiveObjects == 3,
        "queued repair cannot resurrect a hidden view");
    PracticeHidePreview(1); TestCommand_practicaconfig(0, ""); MenuChoose(1);
    hit = PracticePreviewObjects[0][0];
    Check(PracticeSetupView[0] && PracticeProtectPreviewShot(0, hit), "automatic setup view is also protected");
    PracticeRepairPreviews();
    Check(PracticeSetupView[0] && PracticePreviewObjects[0][0] != hit && TestLiveObjects == 3,
        "repair keeps automatic configuration context active");
    OpenTarget(1); MenuChoose(3);
    hit = PracticePreviewObjects[0][1]; neighbour = PracticeEditObject[0];
    PracticeProtectPreviewShot(0, hit); PracticeRepairPreviews();
    Check(PracticeEditObject[0] == neighbour && TestEditorObject[0] == neighbour
        && PracticePreviewObjects[0][1] != hit, "neighbour repair preserves native editing object and editor");
    EditorResult(EDIT_RESPONSE_CANCEL);
    hit = PracticePreviewObjects[0][0]; PracticeProtectPreviewShot(0, hit); TestAdmin[0] = false;
    PracticeRepairPreviews();
    Check(!PracticeSetupView[0] && !TestLiveObjects && !TestLiveCP && !TestLiveLabels,
        "lost administrator access cancels pending repair and removes the owned view");

    ResetTest(); PracticeShowPreview(0); hit = PracticePreviewObjects[0][0];
    PracticeProtectPreviewShot(0, hit); TestCreateFails = true; PracticeRepairPreviews();
    Check(!PracticePreviewActive[0] && !TestLiveObjects && !TestLiveCP && !TestLiveLabels
        && PracticeCountTargets() == 3, "repair allocation failure cleans view without changing saved points");
    ResetTest(1); ShowFirstTarget();
    Check(!PracticeProtectPreviewShot(0, PracticeSession[PracticeObject]), "active practice target is never protected as preview");
    Shoot(true); Check(TestResultHits == 1, "active target continues to score normally");
}

stock TestDecimalTimings()
{
    ResetTest(2); PracticeShowRuleInput(0, PracticeExposure); MenuInput("0.5");
    Check(PracticeRules[PracticeExposure] == 500 && strfind(TestDialogBody[0], "0.5 segundos") != -1,
        "exposure input and rule list display a fractional second");
    PracticeShowRuleInput(0, PracticeInterval); MenuInput("0,2");
    Check(PracticeRules[PracticeInterval] == 200, "interval accepts Spanish comma decimal");
    PracticeShowRuleInput(0, PracticeExposure);
    new invalid[][16] = {"0", "0.0", "0.05", ".5", "0.", "1,2.3", "-1", "NaN", "1e2", "61", "60.1", "999999999"};
    for (new index = 0; index < sizeof(invalid); index++)
    {
        MenuInput(invalid[index]);
        Check(PracticeRules[PracticeExposure] == 500 && PracticeMenuPage[0] == SP_MENU_NUMBER,
            "invalid or out-of-range exposure never changes the saved duration");
    }
    MenuInput("0,1"); Check(PracticeRules[PracticeExposure] == 100, "minimum exposure is one tenth of a second");
    PracticeShowRuleInput(0, PracticeInterval); MenuInput("0");
    Check(!PracticeRules[PracticeInterval], "zero interval remains permitted");
    PracticeShowRuleInput(0, PracticeExposure); MenuInput("60.0");
    PracticeShowRuleInput(0, PracticeInterval); MenuInput("30,0");
    Check(PracticeRules[PracticeExposure] == 60000 && PracticeRules[PracticeInterval] == 30000,
        "decimal timing upper limits retain previous maximum whole seconds");
    PracticeShowRuleInput(0, PracticePreparation); MenuInput("0.5");
    Check(PracticeRules[PracticePreparation] == 3 && PracticeMenuPage[0] == SP_MENU_NUMBER,
        "preparation retains whole-second countdown input");

    PracticeRules[PracticeExposure] = 500; PracticeRules[PracticeInterval] = 200;
    Check(PracticeSaveConfiguration(), "fractional timing configuration can be saved");
    PracticeRules[PracticeExposure] = 5000; PracticeRules[PracticeInterval] = 1000;
    Check(PracticeLoadConfiguration() && PracticeRules[PracticeExposure] == 500 && PracticeRules[PracticeInterval] == 200,
        "v6 round trip retains exact milliseconds");
    new File:input = fopen(PRACTICE_CONFIG_FILE, io_read), header[32];
    PracticeReadLine(input, header, sizeof(header)); fclose(input);
    Check(!strcmp(header, "SARP_PRACTICE 6"), "fractional timing uses version 6 storage");
    for (new version = 1; version <= 5; version++)
    {
        new rules[64]; format(header, sizeof(header), "SARP_PRACTICE %d", version);
        if (version == 1) format(rules, sizeof(rules), "24 1 3 5 1");
        else if (version == 2) format(rules, sizeof(rules), "24 1 3 5 1 0 0 0 0");
        else format(rules, sizeof(rules), "%d 1 3 5 1 0 0 0", version == 5 ? PRACTICE_WEAPON_DEFAULT : 24);
        TestWriteConfigFixture(header, rules);
        Check(PracticeLoadConfiguration() && PracticeRules[PracticePreparation] == 3
            && PracticeRules[PracticeExposure] == 5000 && PracticeRules[PracticeInterval] == 1000,
            "each older version converts seconds once and keeps preparation in seconds");
    }
    TestWriteConfigFixture("SARP_PRACTICE 5", "4 1 3 2147483647 1 0 0 0");
    Check(!PracticeLoadConfiguration() && PracticeRules[PracticeExposure] == 5000,
        "legacy duration overflow is rejected before conversion");
    TestWriteConfigFixture("SARP_PRACTICE 6", "4 1 3 99 200 0 0 0");
    Check(!PracticeLoadConfiguration() && PracticeRules[PracticeExposure] == 5000,
        "v6 duration outside configured resolution cannot replace active rules");

    ResetTest(2); PracticeRules[PracticeExposure] = 500; PracticeRules[PracticeInterval] = 200;
    ShowFirstTarget(); TestNow += 499; PracticeTick(TestNow);
    Check(PracticeSession[PracticePhase] == PRACTICE_TARGET && !PracticeSession[PracticeExpired],
        "fractional exposure never expires before its deadline");
    TestNow++; Shoot(true);
    Check(PracticeSession[PracticeExpired] == 1 && !PracticeSession[PracticeHits] && PracticeSession[PracticePresented] == 1,
        "hit at fractional deadline is a miss without appearing or scoring a subsequent target");
    TestNow += 199; PracticeTick(TestNow);
    Check(PracticeSession[PracticePhase] == PRACTICE_INTERVAL && !TestLiveObjects,
        "decimal pause never presents a target early");
    TestNow++; PracticeTick(TestNow); Shoot(true);
    Check(TestResultHits == 1 && TestResultExpired == 1 && TestResultShots == 2,
        "decimal interval presents next target at deadline and retains scoring");
}

stock TestCountdownPacing()
{
    ResetTest(1); TestNow = 1317; PracticeStart(0);
    TestNow += 1010; PracticeTick(TestNow);
    TestNow += 990; PracticeTick(TestNow);
    Check(TestSoundCount[0] == 2 && PracticeSession[PracticeLastCountdown] == 2,
        "wall-clock second boundary cannot bunch cues after a delayed beep");
    TestNow += 10; PracticeTick(TestNow);
    Check(TestSoundCount[0] == 3 && PracticeSession[PracticeLastCountdown] == 1,
        "number one and its cue wait a full second after number two");
    TestNow += 990; PracticeTick(TestNow);
    Check(PracticeSession[PracticePhase] == PRACTICE_COUNTDOWN && TestSoundCount[0] == 3,
        "first target cannot cut short the last countdown pause");
    TestNow += 10; PracticeTick(TestNow);
    Check(PracticeSession[PracticePhase] == PRACTICE_TARGET && TestSoundCount[0] == 4,
        "first target appears once the final pause has elapsed");
    for (new cue = 1; cue < 4; cue++)
        Check(TestSoundTimes[0][cue] - TestSoundTimes[0][cue - 1] >= 1000,
            "every audible countdown step and start signal are at least one second apart");
}
