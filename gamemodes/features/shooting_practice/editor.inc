#if defined SP_EDITOR_INCLUDED
    #endinput
#endif
#define SP_EDITOR_INCLUDED

// The station has one writer. Previews are private to each viewer and never use
// the session's object handle or the general administrator object's registry.
new PracticeEditorOwner = INVALID_PLAYER_ID;
new PracticeDraftSlot[MAX_PLAYERS] = {-1, ...};
new PracticeDraftRevision[MAX_PLAYERS];
new PracticeDraft[MAX_PLAYERS][E_PRACTICE_POINT];
new bool:PracticeDraftNew[MAX_PLAYERS], bool:PracticeDraftWizard[MAX_PLAYERS], bool:PracticeDraftFixTypes[MAX_PLAYERS];
new STREAMER_TAG_OBJECT:PracticeEditObject[MAX_PLAYERS];
new bool:PracticePreviewActive[MAX_PLAYERS], bool:PracticeSetupView[MAX_PLAYERS];
new PracticeViewPoints[MAX_PLAYERS][PRACTICE_MAX_TARGETS + 1][E_PRACTICE_POINT];
new STREAMER_TAG_OBJECT:PracticePreviewObjects[MAX_PLAYERS][PRACTICE_MAX_TARGETS];
new STREAMER_TAG_3D_TEXT_LABEL:PracticePreviewLabels[MAX_PLAYERS][PRACTICE_MAX_TARGETS + 1];
new STREAMER_TAG_CP:PracticePreviewCheckpoint[MAX_PLAYERS];
new bool:PracticePreviewRepair[MAX_PLAYERS][PRACTICE_MAX_TARGETS], bool:PracticePreviewRepairPending;

stock PracticeConfigurationEditing()
{
    return PracticeEditorOwner != INVALID_PLAYER_ID;
}

stock PracticeHasDraft(playerid)
{
    return PracticeEditorOwner == playerid && PracticeDraftSlot[playerid] >= 0;
}

stock bool:PracticeAdminAvailable(playerid)
{
    new Float:health;
    return IsPlayerConnected(playerid) && !IsPlayerNPC(playerid) && IsPlayerAdmin(playerid)
        && GetPlayerState(playerid) == PLAYER_STATE_ONFOOT && GetPlayerHealth(playerid, health) && health > 0.0;
}

stock bool:PracticeMayChange(playerid)
{
    if (!PracticeAdminAvailable(playerid))
    {
        SendClientMessage(playerid, COLOR_FADE3, "Necesitas ser administrador RCON, estar vivo y a pie.");
        return false;
    }
    if (PracticeIsBusy())
    {
        SendClientMessage(playerid, COLOR_FADE3, "Espera a que termine la práctica para modificar el puesto.");
        return false;
    }
    if (PracticeConfigurationEditing() && PracticeEditorOwner != playerid)
    {
        SendClientMessage(playerid, COLOR_FADE3, "Otro administrador está editando el puesto.");
        return false;
    }
    return true;
}

stock PracticeCapturePoint(playerid, point[E_PRACTICE_POINT])
{
    GetPlayerPos(playerid, point[PracticePointX], point[PracticePointY], point[PracticePointZ]);
    GetPlayerFacingAngle(playerid, point[PracticePointAngle]);
    point[PracticePointInterior] = GetPlayerInterior(playerid);
    point[PracticePointWorld] = GetPlayerVirtualWorld(playerid);
    point[PracticePointSet] = true;
    return 1;
}

stock PracticeRemoveViewSlot(playerid, slot)
{
    if (slot)
    {
        PracticePreviewRepair[playerid][slot - 1] = false;
        if (IsValidDynamicObject(PracticePreviewObjects[playerid][slot - 1]))
            DestroyDynamicObject(PracticePreviewObjects[playerid][slot - 1]);
        PracticePreviewObjects[playerid][slot - 1] = INVALID_STREAMER_ID;
    }
    else
    {
        if (IsValidDynamicCP(PracticePreviewCheckpoint[playerid])) DestroyDynamicCP(PracticePreviewCheckpoint[playerid]);
        PracticePreviewCheckpoint[playerid] = STREAMER_TAG_CP:INVALID_STREAMER_ID;
    }
    if (IsValidDynamic3DTextLabel(PracticePreviewLabels[playerid][slot]))
        DestroyDynamic3DTextLabel(PracticePreviewLabels[playerid][slot]);
    PracticePreviewLabels[playerid][slot] = STREAMER_TAG_3D_TEXT_LABEL:INVALID_STREAMER_ID;
    return 1;
}

stock PracticeHidePreview(playerid)
{
    PracticePreviewActive[playerid] = PracticeSetupView[playerid] = false;
    for (new slot = 0; slot <= PRACTICE_MAX_TARGETS; slot++) PracticeRemoveViewSlot(playerid, slot);
    return 1;
}

stock PracticeHideAllPreviews()
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
        if (PracticePreviewActive[playerid] || PracticeSetupView[playerid]) PracticeHidePreview(playerid);
    return 1;
}

stock STREAMER_TAG_OBJECT:PracticeCreatePrivateObject(playerid, const point[E_PRACTICE_POINT])
{
    new STREAMER_TAG_OBJECT:objectid = CreateDynamicObject(point[PracticePointModel],
        point[PracticePointX], point[PracticePointY], point[PracticePointZ],
        point[PracticePointRotX], point[PracticePointRotY], point[PracticePointAngle],
        point[PracticePointWorld], point[PracticePointInterior], playerid);
    if (!IsValidDynamicObject(objectid)) return STREAMER_TAG_OBJECT:INVALID_STREAMER_ID;
    if (!PracticeApplyTargetAppearance(objectid, point[PracticePointNoShoot], point[PracticePointModel]))
    {
        DestroyDynamicObject(objectid);
        return STREAMER_TAG_OBJECT:INVALID_STREAMER_ID;
    }
    return objectid;
}

stock bool:PracticeViewSlotCurrent(playerid, slot)
{
    if (!IsValidDynamic3DTextLabel(PracticePreviewLabels[playerid][slot])) return false;
    if (slot)
    {
        if (!IsValidDynamicObject(PracticePreviewObjects[playerid][slot - 1])) return false;
    }
    else if (!IsValidDynamicCP(PracticePreviewCheckpoint[playerid])) return false;
    for (new field = 0; field < _:E_PRACTICE_POINT; field++)
        if (PracticeViewPoints[playerid][slot][E_PRACTICE_POINT:field] != PracticePoints[slot][E_PRACTICE_POINT:field]) return false;
    return true;
}

stock bool:PracticeSynchronizeView(playerid)
{
    if (!PracticeAdminAvailable(playerid) || PracticeIsBusy() || !PracticePoints[0][PracticePointSet]
        || GetPlayerInterior(playerid) != PracticePoints[0][PracticePointInterior]
        || GetPlayerVirtualWorld(playerid) != PracticePoints[0][PracticePointWorld])
    { PracticeHidePreview(playerid); return false; }

    for (new slot = 0; slot <= PRACTICE_MAX_TARGETS; slot++)
    {
        new bool:editingSlot = slot > 0 && PracticeHasDraft(playerid)
            && PracticeEditObject[playerid] != INVALID_STREAMER_ID && PracticeDraftSlot[playerid] == slot;
        if (!PracticePoints[slot][PracticePointSet] || editingSlot)
        { PracticeRemoveViewSlot(playerid, slot); continue; }
        // Keep unchanged neighbours alive, including while the selected object moves.
        if (PracticeViewSlotCurrent(playerid, slot)) continue;
        PracticeRemoveViewSlot(playerid, slot);
        new label[100], name[PRACTICE_NAME_SIZE], bool:success;
        PracticePointDisplayName(PracticePoints[slot], slot, name);
        if (slot)
        {
            PracticePreviewObjects[playerid][slot - 1] = PracticeCreatePrivateObject(playerid, PracticePoints[slot]);
            success = IsValidDynamicObject(PracticePreviewObjects[playerid][slot - 1]) != 0;
            format(label, sizeof(label), "%s\n%s", name,
                PracticePoints[slot][PracticePointNoShoot] ? ("NO DISPARAR") : ("DISPARAR"));
        }
        else
        {
            PracticePreviewCheckpoint[playerid] = CreateDynamicCP(PracticePoints[0][PracticePointX],
                PracticePoints[0][PracticePointY], PracticePoints[0][PracticePointZ], PRACTICE_START_RADIUS,
                PracticePoints[0][PracticePointWorld], PracticePoints[0][PracticePointInterior], playerid, 200.0);
            success = IsValidDynamicCP(PracticePreviewCheckpoint[playerid]) != 0;
            format(label, sizeof(label), "INICIO DE PRÁCTICA");
        }
        if (success)
        {
            PracticePreviewLabels[playerid][slot] = CreateDynamic3DTextLabel(label,
                !slot ? 0xFFFF66FF : (PracticePoints[slot][PracticePointNoShoot] ? 0x66BFFFFF : 0xFF7777FF),
                PracticePoints[slot][PracticePointX], PracticePoints[slot][PracticePointY], PracticePoints[slot][PracticePointZ] + 1.4,
                60.0, INVALID_PLAYER_ID, INVALID_VEHICLE_ID, 0,
                PracticePoints[slot][PracticePointWorld], PracticePoints[slot][PracticePointInterior], playerid);
            success = IsValidDynamic3DTextLabel(PracticePreviewLabels[playerid][slot]) != 0;
        }
        if (!success)
        {
            PracticeHidePreview(playerid);
            SendClientMessage(playerid, COLOR_FADE3, "No se pudieron mostrar los blancos. Vuelve a intentarlo.");
            return false;
        }
        PracticeViewPoints[playerid][slot] = PracticePoints[slot];
    }
    Streamer_Update(playerid);
    return true;
}

stock bool:PracticeProtectPreviewShot(playerid, STREAMER_TAG_OBJECT:objectid)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS || objectid == INVALID_STREAMER_ID
        || !(PracticePreviewActive[playerid] || PracticeSetupView[playerid])) return false;
    for (new index = 0; index < PRACTICE_MAX_TARGETS; index++)
        if (PracticePreviewObjects[playerid][index] == objectid)
        {
            // The GTA client can locally break a model before the server veto.
            // Defer replacement until after the streamer has finished dispatching
            // this shot. Repeated hits coalesce and only this private copy changes.
            PracticePreviewRepair[playerid][index] = true;
            PracticePreviewRepairPending = true;
            return true;
        }
    return false;
}

stock PracticeRepairPreviews()
{
    if (!PracticePreviewRepairPending) return 1;
    PracticePreviewRepairPending = false;
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        new bool:changed;
        for (new index = 0; index < PRACTICE_MAX_TARGETS; index++)
        {
            if (!PracticePreviewRepair[playerid][index]) continue;
            PracticePreviewRepair[playerid][index] = false;
            if (!(PracticePreviewActive[playerid] || PracticeSetupView[playerid])) continue;
            PracticeRemoveViewSlot(playerid, index + 1);
            changed = true;
        }
        // Revalidates permissions, dimension, session and the current saved record.
        // Hidden views, cancelled drafts and reused player IDs cannot resurrect.
        if (changed) PracticeSynchronizeView(playerid);
    }
    return 1;
}

stock bool:PracticeInSetupRange(playerid)
{
    return IsPlayerInRangeOfPoint(playerid, PRACTICE_MAX_DISTANCE, PracticePoints[0][PracticePointX],
        PracticePoints[0][PracticePointY], PracticePoints[0][PracticePointZ]) != 0;
}

stock PracticeEnsureSetupView(playerid)
{
    if (!PracticePoints[0][PracticePointSet] || !PracticeAdminAvailable(playerid) || PracticeIsBusy()
        || GetPlayerInterior(playerid) != PracticePoints[0][PracticePointInterior]
        || GetPlayerVirtualWorld(playerid) != PracticePoints[0][PracticePointWorld] || !PracticeInSetupRange(playerid))
        return 0;
    PracticeSetupView[playerid] = true;
    return PracticeSynchronizeView(playerid);
}

stock PracticeLeaveSetupView(playerid)
{
    if (!PracticeSetupView[playerid]) return 1;
    PracticeSetupView[playerid] = false;
    if (PracticePreviewActive[playerid]) PracticeSynchronizeView(playerid);
    else PracticeHidePreview(playerid);
    return 1;
}

stock PracticeShowPreview(playerid)
{
    if (!PracticeMayChange(playerid) || PracticeHasDraft(playerid)) return 0;
    if (!PracticePoints[0][PracticePointSet])
        return SendClientMessage(playerid, COLOR_FADE3, "Marca primero el punto de inicio desde el menú.");
    if (GetPlayerInterior(playerid) != PracticePoints[0][PracticePointInterior]
        || GetPlayerVirtualWorld(playerid) != PracticePoints[0][PracticePointWorld])
        return SendClientMessage(playerid, COLOR_FADE3, "Ve al mismo interior y mundo del puesto para ver sus posiciones.");
    PracticePreviewActive[playerid] = true;
    if (!PracticeSynchronizeView(playerid)) return 0;
    SendClientMessage(playerid, COLOR_GRAD1, "Vista previa privada activa. Puedes caminar. Abre /practicaconfig para ocultarla.");
    return 1;
}

stock PracticeRemoveEditObject(playerid, bool:cancelNative = true)
{
    new STREAMER_TAG_OBJECT:objectid = PracticeEditObject[playerid];
    PracticeEditObject[playerid] = INVALID_STREAMER_ID;
    DeletePVar(playerid, "SP_ObjectEditor");
    if (objectid != INVALID_STREAMER_ID)
    {
        if (cancelNative && IsPlayerConnected(playerid)) CancelEdit(playerid);
        if (IsValidDynamicObject(objectid)) DestroyDynamicObject(objectid);
    }
    return 1;
}

stock PracticeDiscardDraft(playerid)
{
    // Clear state before native cancellation can trigger another callback.
    PracticeDraftSlot[playerid] = -1;
    PracticeDraftWizard[playerid] = PracticeDraftNew[playerid] = PracticeDraftFixTypes[playerid] = false;
    if (PracticeEditorOwner == playerid) PracticeEditorOwner = INVALID_PLAYER_ID;
    PracticeRemoveEditObject(playerid);
    PracticeHideCatalogue(playerid);
    new empty[E_PRACTICE_POINT];
    PracticeDraft[playerid] = empty;
    return 1;
}

stock PracticeConfigurationChanged(playerid)
{
    PracticeRevision++;
    for (new viewer = 0; viewer < MAX_PLAYERS; viewer++)
        if (PracticePreviewActive[viewer] || PracticeSetupView[viewer]) PracticeSynchronizeView(viewer);
    return PracticeSaveAndNotify(playerid);
}

stock PracticeFreeTargetSlot()
{
    for (new slot = 1; slot <= PRACTICE_MAX_TARGETS; slot++)
        if (!PracticePoints[slot][PracticePointSet]) return slot;
    return -1;
}

stock PracticeBeginDraft(playerid, slot, bool:create = false)
{
    if (!PracticeMayChange(playerid)) return 0;
    if (playerEditingObjectID[playerid] != -1)
    { SendClientMessage(playerid, COLOR_FADE3, "Termina primero la edición del objeto que tienes abierta."); return 0; }
    if (slot < 1 || slot > PRACTICE_MAX_TARGETS || (!create && !PracticePoints[slot][PracticePointSet])) return 0;
    if (!PracticePoints[0][PracticePointSet])
    { SendClientMessage(playerid, COLOR_FADE3, "Marca primero el punto de inicio."); return 0; }
    if (GetPlayerInterior(playerid) != PracticePoints[0][PracticePointInterior]
        || GetPlayerVirtualWorld(playerid) != PracticePoints[0][PracticePointWorld])
    { SendClientMessage(playerid, COLOR_FADE3, "La edición debe realizarse en el interior y mundo del puesto."); return 0; }
    if (!PracticeInSetupRange(playerid))
    { SendClientMessage(playerid, COLOR_FADE3, "Acércate al campo de tiro. Debes estar a menos de 100 metros del inicio para colocar blancos."); return 0; }
    PracticeDiscardDraft(playerid);
    PracticeEditorOwner = playerid;
    PracticeDraftSlot[playerid] = slot;
    PracticeDraftRevision[playerid] = PracticeRevision;
    PracticeDraftNew[playerid] = create;
    PracticeDraftWizard[playerid] = create;
    if (create)
    {
        PracticeCapturePoint(playerid, PracticeDraft[playerid]);
        PracticeDraft[playerid][PracticePointModel] = PRACTICE_TARGET_MODEL;
        // Place the temporary object just ahead, then let the visual editor place it precisely.
        new Float:angle = PracticeDraft[playerid][PracticePointAngle];
        PracticeDraft[playerid][PracticePointX] += 3.5 * floatsin(-angle, degrees);
        PracticeDraft[playerid][PracticePointY] += 3.5 * floatcos(-angle, degrees);
        // Keep the initial editor object within the same final placement bounds.
        if (!PracticeTargetDistanceValid(PracticePoints[0], PracticeDraft[playerid]))
        {
            GetPlayerPos(playerid, PracticeDraft[playerid][PracticePointX], PracticeDraft[playerid][PracticePointY], PracticeDraft[playerid][PracticePointZ]);
            if (!PracticeTargetDistanceValid(PracticePoints[0], PracticeDraft[playerid]))
            {
                PracticeDraft[playerid][PracticePointX] = PracticePoints[0][PracticePointX] + 3.5 * floatsin(-angle, degrees);
                PracticeDraft[playerid][PracticePointY] = PracticePoints[0][PracticePointY] + 3.5 * floatcos(-angle, degrees);
                PracticeDraft[playerid][PracticePointZ] = PracticePoints[0][PracticePointZ];
            }
        }
    }
    else PracticeDraft[playerid] = PracticePoints[slot];
    PracticeEnsureSetupView(playerid);
    return 1;
}

stock bool:PracticeDraftCurrent(playerid)
{
    return PracticeHasDraft(playerid) && PracticeAdminAvailable(playerid) && !PracticeIsBusy()
        && PracticeDraftRevision[playerid] == PracticeRevision
        && GetPlayerInterior(playerid) == PracticeDraft[playerid][PracticePointInterior]
        && GetPlayerVirtualWorld(playerid) == PracticeDraft[playerid][PracticePointWorld]
        // Start capture may create or relocate the station anywhere. The radius
        // belongs only to target drafts relative to an already marked start.
        && (PracticeDraftSlot[playerid] == 0 || PracticeInSetupRange(playerid));
}

stock PracticeCommitDraft(playerid)
{
    if (!PracticeDraftCurrent(playerid))
    {
        PracticeDiscardDraft(playerid);
        return 0;
    }
    new slot = PracticeDraftSlot[playerid];
    if (slot < 1 || !PracticePointValid(PracticeDraft[playerid])
        || PracticeModelIndex(PracticeDraft[playerid][PracticePointModel]) < 0
        || !PracticeSameDimension(PracticePoints[0], PracticeDraft[playerid])
        || !PracticeTargetDistanceValid(PracticePoints[0], PracticeDraft[playerid]))
        return PracticeShowEditError(playerid);
    PracticePoints[slot] = PracticeDraft[playerid];
    if (PracticeDraftFixTypes[playerid]) PracticeRules[PracticeRandomTypes] = 0;
    PracticeDiscardDraft(playerid);
    PracticeConfigurationChanged(playerid);
    return PracticeShowTargets(playerid);
}

stock PracticeStartVisualEdit(playerid)
{
    if (!PracticeDraftCurrent(playerid) || PracticeDraftSlot[playerid] < 1) return 0;
    if (playerEditingObjectID[playerid] != -1)
        return SendClientMessage(playerid, COLOR_FADE3, "Termina primero el otro editor de objetos.");
    PracticeRemoveEditObject(playerid);
    PracticeHideCatalogue(playerid);
    PracticeEditObject[playerid] = PracticeCreatePrivateObject(playerid, PracticeDraft[playerid]);
    Streamer_Update(playerid, STREAMER_TYPE_OBJECT);
    if (!IsValidDynamicObject(PracticeEditObject[playerid])
        || !Streamer_IsItemVisible(playerid, STREAMER_TYPE_OBJECT, PracticeEditObject[playerid]))
    {
        PracticeRemoveEditObject(playerid);
        SendClientMessage(playerid, COLOR_FADE3, "No se pudo abrir el editor. Acércate al blanco y comprueba su posición.");
        return PracticeShowDraftMenu(playerid);
    }
    PracticeEnsureSetupView(playerid);
    SetPVarInt(playerid, "SP_ObjectEditor", 1);
    if (!EditDynamicObject(playerid, PracticeEditObject[playerid]))
    {
        PracticeRemoveEditObject(playerid);
        PracticeEnsureSetupView(playerid);
        return PracticeShowDraftMenu(playerid);
    }
    SendClientMessage(playerid, COLOR_GRAD1, "Mueve y gira el blanco. Confirmar guarda todos sus cambios. Cancelar conserva la configuración anterior.");
    return 1;
}

stock Float:PracticeNormalizeRotation(Float:angle)
{
    return angle - float(floatround(angle / 360.0, floatround_floor)) * 360.0;
}

stock PracticeHandleObjectEdit(playerid, STREAMER_TAG_OBJECT:objectid, response, Float:x, Float:y, Float:z,
    Float:rx, Float:ry, Float:rz)
{
    if (objectid == INVALID_STREAMER_ID || objectid != PracticeEditObject[playerid]) return 0;
    if (!PracticeDraftCurrent(playerid))
    {
        PracticeDiscardDraft(playerid);
        return 1;
    }
    if (response == EDIT_RESPONSE_CANCEL)
    {
        PracticeRemoveEditObject(playerid, false);
        PracticeDiscardDraft(playerid);
        return PracticeShowTargets(playerid);
    }
    // Updates are client previews only. FINAL coordinates are authoritative;
    // the last UPDATE can lag behind the actual released mouse position.
    if (response != EDIT_RESPONSE_FINAL) return 1;
    PracticeRemoveEditObject(playerid, false);
    if (!(x >= -20000.0 && x <= 20000.0 && y >= -20000.0 && y <= 20000.0 && z >= -20000.0 && z <= 20000.0
        && rx >= -36000.0 && rx <= 36000.0 && ry >= -36000.0 && ry <= 36000.0 && rz >= -36000.0 && rz <= 36000.0))
        return PracticeShowEditError(playerid);
    new candidate[E_PRACTICE_POINT];
    candidate = PracticeDraft[playerid];
    candidate[PracticePointX] = x; candidate[PracticePointY] = y; candidate[PracticePointZ] = z;
    candidate[PracticePointRotX] = PracticeNormalizeRotation(rx);
    candidate[PracticePointRotY] = PracticeNormalizeRotation(ry);
    candidate[PracticePointAngle] = PracticeNormalizeRotation(rz);
    if (!PracticePointValid(candidate) || !PracticeTargetDistanceValid(PracticePoints[0], candidate))
        return PracticeShowEditError(playerid);
    PracticeDraft[playerid] = candidate;
    return PracticeCommitDraft(playerid);
}

stock PracticeEditorTick()
{
    for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
    {
        if (PracticeHasDraft(playerid) && (!PracticeDraftCurrent(playerid)
            || (PracticeEditObject[playerid] != INVALID_STREAMER_ID && !IsValidDynamicObject(PracticeEditObject[playerid]))))
            PracticeResetUI(playerid);
        if ((PracticePreviewActive[playerid] || PracticeSetupView[playerid]) && (!PracticeAdminAvailable(playerid) || PracticeIsBusy()
            || GetPlayerInterior(playerid) != PracticePoints[0][PracticePointInterior]
            || GetPlayerVirtualWorld(playerid) != PracticePoints[0][PracticePointWorld]
            || (PracticeSetupView[playerid] && !PracticeInSetupRange(playerid)))) PracticeHidePreview(playerid);
    }
    return 1;
}
