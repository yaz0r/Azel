#include "PDS.h"
#include "field/fieldCamera.h"
#include "field/fieldCutsceneTask.h"
#include "field/field_a3/o_fld_a3.h"

static void cameraFollowMode_vertical(sFieldCameraStatus* r4);
static void cameraFollowMode3(sFieldCameraStatus* r4);
static void cameraFollowMode4(sFieldCameraStatus* r4);
static void cameraFollowMode5(sFieldCameraStatus* r4);
static void cameraFollowMode6(sFieldCameraStatus* r4);
static void cameraFollowMode7_Draw(sFieldCameraStatus* r4);
static void cameraFollowMode_idle(sFieldCameraStatus* r4);
static void cameraFollowMode_scriptTarget(sFieldCameraStatus* r4);
static void updateZoneCameraFollow(sFieldCameraManager* r4);

static void(*cameraFollowModeUpdateTable[10])(sFieldCameraStatus*) = {
    cameraFollowMode_scriptTarget,           // [0] 06062900
    cameraFollowMode_default,             // [1] 060621C6
    cameraFollowMode_vertical,       // [2] 06062228
    cameraFollowMode3,                       // [3] 0606229E
    cameraFollowMode4,                       // [4] 06062302
    cameraFollowMode5,                       // [5] 06062370
    cameraFollowMode6,                       // [6] 0606240C
    nullptr,
    cameraFollowMode_idle,           // [8] 06062474
    nullptr,
};

static void(*cameraFollowModeDrawTable[10])(sFieldCameraStatus*) = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    cameraFollowMode7_Draw,
    0,
    0,
};

// 0606133c
static void applyCameraStatusToEngine(sFieldCameraManager* pTypedWorkArea)
{
    sFieldCameraStatus* r13 = &pTypedWorkArea->m3E4_cameraSlots[pTypedWorkArea->m50C_activeCameraSlot];
    sVec3_S16 r15;
    r15[0] = r13->mC_rotation[0] >> 16;
    r15[1] = r13->mC_rotation[1] >> 16;
    r15[2] = r13->mC_rotation[2] >> 16;

    updateEngineCamera(&cameraProperties2, r13->m0_position, r15);

    copyMatrix(pCurrentMatrix, &pTypedWorkArea->m384_viewMatrix);
    copyMatrix(&cameraProperties2.m28[0], &pTypedWorkArea->m3B4_projectionMatrix);
}

// 0606139a
void sFieldCameraManager::fieldCameraManagerInit(sFieldCameraManager* pTypedWorkArea)
{
    getFieldTaskPtr()->m8_pSubFieldData->m334 = pTypedWorkArea;

    initCameraSlotWithFunctions(0, gFieldCameraDrawFunc ? gFieldCameraDrawFunc : &cameraFollowMode_default, 0);
    selectCameraSlot(0);
    setupFieldCameraConfigs(readCameraConfig(gFieldCameraConfigEA), 1);

    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex = 1;

    deactivateCameraSlot(1);

    applyCameraStatusToEngine(pTypedWorkArea);

    getFieldTaskPtr()->m8_pSubFieldData->m334->m50D_isInitialized = 1;
}

// 060613f4
static void selectCamera(sFieldCameraManager* r4, s_dragonTaskWorkArea* r5)
{
    s32 r6 = r4->mC;

    while (--r6)
    {
        s_fieldCameraConfig* r7 = &r4->m10[r6];
        if (r6) // don't 2d check on first camera so it's always taken
        {
            if (r5->m8_pos[0] <= r7->m0_min[0])
                continue;
            if (r5->m8_pos[2] <= r7->m0_min[2])
                continue;
            if (r5->m8_pos[0] >= r7->mC_max[0])
                continue;
            if (r5->m8_pos[2] >= r7->mC_max[2])
                continue;
        }

        if ((r4->m0_nextCamera != r6) && (r4->m4_currentCamera == r6))
        {
            r4->m8_numFramesOnCurrentCamera++;
        }
        else
        {
            r4->m4_currentCamera = r6;
            r4->m8_numFramesOnCurrentCamera = 0;
        }

        if (r4->m8_numFramesOnCurrentCamera >= 30)
        {
            r4->m0_nextCamera = r6;
            r4->m8_numFramesOnCurrentCamera = 0;
        }

        return;
    }
}

// 06061496
static void selectCameraZone(sFieldCameraManager* r4, s_dragonTaskWorkArea* r5)
{
    if (r4->m2E0_forcedZoneIndex)
    {
        assert(0);
    }

    s32 r13 = r4->m2DC_numCameraZones;
    s32 var8[3];

    for (s32 r13 = r4->m2DC_numCameraZones; r13 >= 0; r13--)
    {
        if (r13)
        {
            sFieldCameraZone* r12 = &r4->m2E4_cameraZones[r13];
            s32 r6 = r5->m8_pos[0] - r12->m0_center[0];
            if (r6 >= 0)
            {
                var8[0] = r6;
            }
            else
            {
                var8[0] = r12->m0_center[0] - r5->m8_pos[0];
            }

            if (var8[0] >= r12->m14_triggerRadius)
                continue;

            if (r5->m8_pos[2] - r12->m0_center[2] < 0)
            {
                var8[2] = r5->m8_pos[2] - r12->m0_center[2];
            }
            else
            {
                var8[2] = r12->m0_center[2] - r5->m8_pos[2];
            }

            if (var8[2] >= r12->m14_triggerRadius)
                continue;

            if (MTH_Mul(var8[0], var8[0]) + MTH_Mul(var8[2], var8[2]) >= r12->m18_maxDistanceSquare)
                continue;
        }

        //6061554
        if ((r4->m2D0_activeZoneIndex != r13) && (r4->m2D4_candidateZoneIndex == r13))
        {
            r4->m2D8_zoneDwellCounter++;
        }
        else
        {
            r4->m2D4_candidateZoneIndex = r13;
            r4->m2D8_zoneDwellCounter = 0;
        }

        if (r4->m2D8_zoneDwellCounter >= 30)
        {
            r4->m2D0_activeZoneIndex = r13;
            r4->m2D8_zoneDwellCounter = 0;
        }

        return;
    }
}

// 060615b6
void updateFieldCameraSlots()
{
    sFieldCameraManager* r13 = getFieldTaskPtr()->m8_pSubFieldData->m334;
    s_dragonTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

    if (r14)
    {
        selectCamera(r13, r14);
        selectCameraZone(r13, r14);
    }

    for (int i = 0; i < 2; i++)
    {
        if (isFieldCameraSlotActive(i))
        {
            if (r13->m37C_isCutsceneCameraActive)
            {
                updateCutsceneCameraInterpolation(r13, &r13->m3E4_cameraSlots[i]);
            }
            else
            {
                if ((r13->m3E4_cameraSlots[i].m74_updateFunc == cameraFollowMode_scriptTarget) || (r13->m3E4_cameraSlots[i].m74_updateFunc == cameraFollowMode_idle))
                {
                    r13->m3E4_cameraSlots[i].m74_updateFunc(&r13->m3E4_cameraSlots[i]);
                }
                else
                {
                    if ((r13->m2E0_forcedZoneIndex > 0) || (r13->m2D0_activeZoneIndex > 0))
                    {
                        updateZoneCameraFollow(r13);
                    }
                    else
                    {
                        if (r13->m3E4_cameraSlots[i].m74_updateFunc)
                        {
                            r13->m3E4_cameraSlots[i].m74_updateFunc(&r13->m3E4_cameraSlots[i]);
                        }
                    }
                }
            }
            r13->m3E4_cameraSlots[i].m84++;
        }
    }
}

// 06061690
void drawFieldCameraSlots()
{
    sFieldCameraManager* r12 = getFieldTaskPtr()->m8_pSubFieldData->m334;

    for (int i = 0; i < 2; i++)
    {
        if (isFieldCameraSlotActive(i))
        {
            if (r12->m37C_isCutsceneCameraActive == 0)
            {
                sFieldCameraStatus* pFieldCameraStatus = &r12->m3E4_cameraSlots[i];
                if (pFieldCameraStatus->m78_drawFunc)
                {
                    pFieldCameraStatus->m78_drawFunc(pFieldCameraStatus);
                }
            }
        }
    }

    applyCameraStatusToEngine(r12);
}

// 060616f8
u32 selectCameraSlot(u32 r4)
{
    if (isFieldCameraSlotActive(r4))
    {
        sFieldCameraManager* p334 = getFieldTaskPtr()->m8_pSubFieldData->m334;
        p334->m50C_activeCameraSlot = r4;
        p334->m3E4_cameraSlots[r4].m80_frameCounter = 0;
        return 1;
    }

    return 0;
}

// 0606173a
static s32 setCameraFollowFunctions(u32 r4, void(*r5)(sFieldCameraStatus*), void(*r6)(sFieldCameraStatus*))
{
    if (isFieldCameraSlotActive(r4))
    {
        sFieldCameraStatus* pCamera = &getFieldTaskPtr()->m8_pSubFieldData->m334->m3E4_cameraSlots[r4];
        pCamera->m74_updateFunc = r5;
        pCamera->m78_drawFunc = r6;
        pCamera->m8D_reinitMode = 0;
        pCamera->m8E_followSubState = 0;
        return 1;
    }
    return 0;
}

// 06061794
s8 isFieldCameraSlotActive(s32 index)
{
    return getFieldTaskPtr()->m8_pSubFieldData->m334->m3E4_cameraSlots[index].m8C_isActive;
}

// 060617e0
static void resetCameraStatus(sFieldCameraStatus* r4)
{
    r4->m74_updateFunc = 0;
    r4->m78_drawFunc = 0;
    r4->m0_position[0] = 0;
    r4->m0_position[1] = 0;
    r4->m0_position[2] = 0;
    r4->mC_rotation[0] = 0;
    r4->mC_rotation[1] = 0;
    r4->mC_rotation[2] = 0;
    r4->m18 = 0;
    r4->m24_distanceToDestination = 0xF000;
    r4->m28 = 0;
    r4->m2C = 0;
    r4->m30 = 0;
    r4->m34 = 0;
    r4->m40 = 0xF000;
    r4->m80_frameCounter = 0;
    r4->m84 = 0;
    r4->m8C_isActive = 0;
    r4->m8D_reinitMode = 0;
    r4->m8E_followSubState = 0;
}

// 0606181e
void initCameraSlotWithFunctions(u32 r4, void(*r5)(sFieldCameraStatus*), void(*r6)(sFieldCameraStatus*))
{
    sFieldCameraStatus* pFieldCameraStatus = &getFieldTaskPtr()->m8_pSubFieldData->m334->m3E4_cameraSlots[r4];
    resetCameraStatus(pFieldCameraStatus);

    pFieldCameraStatus->m74_updateFunc = r5;
    pFieldCameraStatus->m78_drawFunc = r6;
    pFieldCameraStatus->m8C_isActive = 1;
}

// 06061864
void deactivateCameraSlot(u32 r4)
{
    sFieldCameraManager* p334 = getFieldTaskPtr()->m8_pSubFieldData->m334;
    p334->m3E4_cameraSlots[r4].m74_updateFunc = 0;
    p334->m3E4_cameraSlots[r4].m78_drawFunc = 0;
    p334->m3E4_cameraSlots[r4].m8C_isActive = 0;
}

// 0606189c
sFieldCameraStatus* getFieldCameraStatus()
{
    return &getFieldTaskPtr()->m8_pSubFieldData->m334->m3E4_cameraSlots[getFieldTaskPtr()->m8_pSubFieldData->m334->m50C_activeCameraSlot];
}

// 060618e4
void setCameraFollowMode_cut(s32 followMode)
{
    setCameraFollowFunctions(0, cameraFollowModeUpdateTable[followMode], cameraFollowModeDrawTable[followMode]);
    getFieldCameraStatus()->m8D_reinitMode = 0;
}

// 06061914
void setCameraFollowMode_blend(u32 r4)
{
    setCameraFollowFunctions(0, cameraFollowModeUpdateTable[r4], cameraFollowModeDrawTable[r4]);

    getFieldCameraStatus()->m8D_reinitMode = 1;
}

// 06061990
static void fieldOverlaySubTaskInitSub4Sub0Sub0(s32* r4, s32* r5)
{
    for (int i = 0; i < 7; i++)
    {
        r5[i] = r4[i];
    }
}

// 060619ae
static void copyFieldCameraConfig(s_fieldCameraConfig* r4, s_fieldCameraConfig* r5)
{
    r5->m0_min = r4->m0_min;
    r5->mC_max = r4->mC_max;

    fieldOverlaySubTaskInitSub4Sub0Sub0(r4->m18, r5->m18);
    fieldOverlaySubTaskInitSub4Sub0Sub0(r4->m34, r5->m34);

    r5->m50 = r4->m50;
    r5->m54 = r4->m54;
}

// 06061a2c
void setupFieldCameraConfigs(s_fieldCameraConfig* r4, u32 r5)
{
    sFieldCameraManager* p334 = getFieldTaskPtr()->m8_pSubFieldData->m334;

    p334->mC = 8;
    p334->m2DC_numCameraZones = 4;

    for (int i = 0; i < r5; i++)
    {
        copyFieldCameraConfig(&r4[i], &p334->m10[i]);
    }
}

// 06061b7e
static void cameraFollowMode_finalStep(sFieldCameraStatus* r14, s_dragonTaskWorkArea* r9)
{
    r14->m5C_rotationSpring[0] = interpolateRotation(r14->m5C_rotationSpring[0], 0, 0x2000, 0x444444, 0);
    r14->m5C_rotationSpring[1] = interpolateRotation(r14->m5C_rotationSpring[1], 0, 0x2000, 0x444444, 0);
    r14->m5C_rotationSpring[2] = interpolateRotation(r14->m5C_rotationSpring[2], 0, 0x2000, 0x444444, 0);

    r14->m5C_rotationSpring += r14->m68_rotationImpulse;
    r14->mC_rotation += r14->m5C_rotationSpring;

    r14->m68_rotationImpulse.zeroize();

    r14->mC_rotation[0] = r14->mC_rotation[0].normalized();
    r14->m18 = r14->m18.normalized();

    fixedPoint r11 = r14->mC_rotation[0] + r14->m18;

    if (r11 < -0x31C71C7)
    {
        r14->mC_rotation[0] = -r14->m18 - 0x31C71C7;
        r11 = -0x31C71C7;
    }
    else if (r11 > 0x31C71C7)
    {
        r14->mC_rotation[0] = -r14->m18 + 0x31C71C7;
        r11 = 0x31C71C7;
    }

    sVec3_FP var10;
    var10[0] = -MTH_Mul_5_6(r14->m24_distanceToDestination, getCos(r11.getInteger() & 0xFFF), getSin(r14->mC_rotation[1].getInteger() & 0xFFF));
    var10[1] = MTH_Mul(r14->m24_distanceToDestination, getSin(r11.getInteger() & 0xFFF));
    var10[2] = -MTH_Mul_5_6(r14->m24_distanceToDestination, getCos(r11.getInteger() & 0xFFF), getCos(r14->mC_rotation[1].getInteger() & 0xFFF));

    r14->m0_position = r9->m8_pos - var10;

    r14->m44_positionSpring[0] = interpolateDistance(r14->m44_positionSpring[0], 0, 0x2000, 0xAAA, 0);
    r14->m44_positionSpring[1] = interpolateDistance(r14->m44_positionSpring[1], 0, 0x2000, 0xAAA, 0);
    r14->m44_positionSpring[2] = interpolateDistance(r14->m44_positionSpring[2], 0, 0x2000, 0xAAA, 0);

    r14->m44_positionSpring += r14->m50_positionImpulse;

    r14->m0_position += r14->m44_positionSpring;
    r14->m50_positionImpulse.zeroize();
}

// 06061de0
static void cameraFollowMode_posFromDragon(sFieldCameraStatus* r11, s_dragonTaskWorkArea* stack_4)
{
    sFieldCameraManager* pCameraData = getFieldTaskPtr()->m8_pSubFieldData->m334;
    s_fieldCameraConfig* r14 = &pCameraData->m10[pCameraData->m0_nextCamera];

    fixedPoint r14FP = FP_Div(stack_4->m8_pos[1] - r14->m50, r14->m54 - r14->m50);

    if (r14FP < 0)
    {
        r14FP = 0;
    }
    else if (r14FP > 0x10000)
    {
        r14FP = 0x10000;
    }

    fixedPoint r4 = r14->m34[0] - r14->m18[0];
    r4 = r4.normalized();
    r11->m28 = r14->m18[0] + MTH_Mul(r4, r14FP);

    r4 = r14->m34[3] - r14->m18[3];
    r4 = r4.normalized();
    r11->m34 = r14->m18[3] + MTH_Mul(r4, r14FP);

    r4 = r14->m34[6] - r14->m18[6];
    r4 = r4.normalized();
    r11->m40 = r14->m18[6] + MTH_Mul(r4, r14FP);
}

// 0606202c
static void cameraFollowMode_initDirect(sFieldCameraStatus* r14, s_dragonTaskWorkArea* r12)
{
    sVec2_FP var0;
    sVec3_FP stack_8;

    stack_8[0] = -r12->m88_matrix.m[0][2];
    stack_8[1] = -r12->m88_matrix.m[1][2];
    stack_8[2] = -r12->m88_matrix.m[2][2];

    computeLookAt(stack_8, var0);

    cameraFollowMode_posFromDragon(r14, r12);

    if (r14->m7C & 1)
    {
        r14->mC_rotation[0] = r14->m28 - r12->m20_angle[0];
    }
    else
    {
        r14->mC_rotation[0] = r14->m28;
    }

    if (r14->m7C & 2)
    {
        r14->mC_rotation[1] = var0[1];
    }

    if (r14->m7C & 4)
    {
        r14->mC_rotation[2] = r14->m30;
    }

    r14->m18 = r14->m34;

    r14->m24_distanceToDestination = r14->m40;

    cameraFollowMode_finalStep(r14, r12);
}

// 060620ac
static void cameraFollowMode_interpSmooth(sFieldCameraStatus* r14, s_dragonTaskWorkArea* r12)
{
    sVec2_FP var0;
    sVec3_FP stack_8;

    stack_8[0] = -r12->m88_matrix.m[0][2];
    stack_8[1] = -r12->m88_matrix.m[1][2];
    stack_8[2] = -r12->m88_matrix.m[2][2];

    computeLookAt(stack_8, var0);

    cameraFollowMode_posFromDragon(r14, r12);

    if (r14->m7C & 1)
    {
        r14->mC_rotation[0] = interpolateRotation(r14->mC_rotation[0], r14->m28 - r12->m20_angle[0], 0x2000, 0x111111, 0);
    }
    else
    {
        r14->mC_rotation[0] = interpolateRotation(r14->mC_rotation[0], r14->m28, 0x2000, 0x111111, 0);
    }

    // TODO: recheck, this is sketchy (the m34[2])
    if (getFieldTaskPtr()->m8_pSubFieldData->m334->m10[0].m34[2])
    {
        r14->m7C &= 0xFFFFFFFD;
    }
    else
    {
        r14->m7C |= 2;
    }

    if (r14->m7C & 2)
    {
        r14->mC_rotation[1] = interpolateRotation(r14->mC_rotation[1], var0[1], 0x2000, 0x222222, 0);
    }

    if ((r14->m7C & 4) == 0)
    {
        r14->m30 = 0;
    }

    r14->mC_rotation[2] = interpolateRotation(r14->mC_rotation[2], r14->m30, 0x2000, 0x222222, 0);

    r14->m24_distanceToDestination = interpolateDistance(r14->m24_distanceToDestination, r14->m40, 0x2000, 0xAAA, 0);

    cameraFollowMode_finalStep(r14, r12);
}

// 060621c6
void cameraFollowMode_default(sFieldCameraStatus* r14)
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (pDragonTask == NULL)
        return;

    switch (r14->m8D_reinitMode)
    {
    case 0:
        r14->m8F_followType = 1;
        r14->m90_followMode = 1;
        r14->m7C = 2;
        r14->m8D_reinitMode = 2;
        cameraFollowMode_initDirect(r14, pDragonTask);
        return;
    case 1:
        r14->m8F_followType = 1;
        r14->m90_followMode = 1;
        r14->m7C = 2;
        r14->m8D_reinitMode = 2;
    case 2:
        cameraFollowMode_interpSmooth(r14, pDragonTask);
        return;
    default:
        assert(0);
    }
}

// 06062228
static void cameraFollowMode_vertical(sFieldCameraStatus* r14)
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (pDragonTask == NULL)
        return;

    switch (r14->m8D_reinitMode)
    {
    case 0:
        r14->m8F_followType = 2;
        r14->m90_followMode = 2;
        r14->m7C = 3;
        r14->m8D_reinitMode = 2;
        cameraFollowMode_initDirect(r14, pDragonTask);
        return;
    case 1:
        r14->m8F_followType = 2;
        r14->m90_followMode = 2;
        r14->m7C = 3;
        r14->m8D_reinitMode = 2;
    case 2:
        cameraFollowMode_interpSmooth(r14, pDragonTask);
        return;
    default:
        assert(0);
    }
}

// 0606229e
static void cameraFollowMode3(sFieldCameraStatus* r14)
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (pDragonTask == NULL)
        return;

    switch (r14->m8D_reinitMode)
    {
    case 0:
        r14->m8F_followType = 3;
        r14->m90_followMode = 3;
        r14->m7C = 6;
        r14->m8D_reinitMode = 2;
        cameraFollowMode_initDirect(r14, pDragonTask);
        return;
    case 1:
        r14->m8F_followType = 3;
        r14->m90_followMode = 3;
        r14->m7C = 6;
        r14->m8D_reinitMode = 2;
    case 2:
        cameraFollowMode_interpSmooth(r14, pDragonTask);
        return;
    default:
        assert(0);
    }
}

// 06062302
static void cameraFollowMode4(sFieldCameraStatus* r14)
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (pDragonTask == NULL)
        return;

    switch (r14->m8D_reinitMode)
    {
    case 0:
        r14->m8F_followType = 4;
        r14->m90_followMode = 4;
        r14->m7C = 7;
        r14->m8D_reinitMode = 2;
        cameraFollowMode_initDirect(r14, pDragonTask);
        return;
    case 1:
        r14->m8F_followType = 4;
        r14->m90_followMode = 4;
        r14->m7C = 7;
        r14->m8D_reinitMode = 2;
    case 2:
        cameraFollowMode_interpSmooth(r14, pDragonTask);
        return;
    default:
        assert(0);
    }
}

// 06062370
static void cameraFollowMode5(sFieldCameraStatus* r14)
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (pDragonTask == NULL)
        return;

    if (r14->m8D_reinitMode == 0 || r14->m8D_reinitMode == 1)
    {
        r14->m8F_followType = 5;
        r14->m90_followMode = 5;
        r14->m7C = 0;
        r14->m8D_reinitMode = 2;
    }

    sVec3_FP delta;
    delta.m0_X = fixedPoint(pDragonTask->m8_pos.m0_X.m_value - r14->m0_position.m0_X.m_value);
    delta.m4_Y = fixedPoint(pDragonTask->m8_pos.m4_Y.m_value - r14->m0_position.m4_Y.m_value);
    delta.m8_Z = fixedPoint(pDragonTask->m8_pos.m8_Z.m_value - r14->m0_position.m8_Z.m_value);
    sVec2_FP lookAt;
    computeLookAt(delta, lookAt);
    r14->mC_rotation.m0_X = lookAt[0];
    r14->mC_rotation.m4_Y = lookAt[1];
    r14->m24_distanceToDestination = vecDistance(r14->m0_position, pDragonTask->m8_pos).m_value;
}

// 0606240c
static void cameraFollowMode6(sFieldCameraStatus* r14)
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (pDragonTask == NULL)
        return;

    if (r14->m8D_reinitMode == 0 || r14->m8D_reinitMode == 1)
    {
        r14->m8F_followType = 6;
        r14->m90_followMode = 6;
        r14->m7C = 0;
        r14->m8D_reinitMode = 2;
    }
}

// 06062474
static void cameraFollowMode_idle(sFieldCameraStatus* r4)
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (pDragonTask == nullptr)
        return;

    switch (r4->m8D_reinitMode)
    {
    case 0:
        r4->m8F_followType = 8;
        r4->m90_followMode = 8;
        r4->m7C = 0;
        r4->m8D_reinitMode = 2;
    case 1:
        cameraFollowMode_finalStep(r4, pDragonTask);
        return;
    default:
        assert(0);
    }
}

// 060624c0
static void updateZoneCameraFollow(sFieldCameraManager* r4)
{
    assert(0);
}

// 06062900
static void cameraFollowMode_scriptTarget(sFieldCameraStatus* r4)
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    switch (r4->m8D_reinitMode)
    {
    case 0:
    case 1:
        r4->m8F_followType = 0;
        r4->m90_followMode = 0;
        r4->m8D_reinitMode = 2;
    case 2:
    default:
        if (pDragonTask->m1D0_cameraScript)
        {
            sVec3_FP r15_8 = pDragonTask->m8_pos - pDragonTask->m1D0_cameraScript->m24_pos2;

            sVec2_FP r15;
            computeLookAt(r15_8, r15);

            r4->mC_rotation[0] = r15[0];
            r4->mC_rotation[1] = r15[1];

            r4->m24_distanceToDestination = vecDistance(pDragonTask->m1D0_cameraScript->m24_pos2, pDragonTask->m8_pos);

            if (r4->m24_distanceToDestination < pDragonTask->m1D0_cameraScript->m30_thresholdDistance)
            {
                r4->m0_position = pDragonTask->m1D0_cameraScript->m24_pos2;
            }
            else
            {
                r4->m24_distanceToDestination = pDragonTask->m1D0_cameraScript->m30_thresholdDistance;

                fixedPoint var20 = -MTH_Mul_5_6(pDragonTask->m1D0_cameraScript->m30_thresholdDistance, getCos(r4->mC_rotation[0].getInteger() & 0xFFF), getSin(r4->mC_rotation[1].getInteger() & 0xFFF));
                fixedPoint var1C = MTH_Mul(r4->m24_distanceToDestination, getSin(r4->mC_rotation[0].getInteger() & 0xFFF));
                fixedPoint var18 = -MTH_Mul_5_6(pDragonTask->m1D0_cameraScript->m30_thresholdDistance, getCos(r4->mC_rotation[0].getInteger() & 0xFFF), getCos(r4->mC_rotation[1].getInteger() & 0xFFF));

                r4->m0_position[0] = pDragonTask->m8_pos[0] - var20;
                r4->m0_position[1] = pDragonTask->m8_pos[1] - var1C;
                r4->m0_position[2] = pDragonTask->m8_pos[2] - var18;
            }
        }
        else
        {
            assert(0);
        }
    }
}

// 06062ac0
static void debugFreeCamera()
{
    sFieldCameraStatus* pCam = getFieldCameraStatus();
    auto& input = graphicEngineStatus.m4514.m0_inputDevices[1].m0_current;

    // Rotation: d-pad slow, d-pad+held fast
    if (input.m6_buttonDown & 0x2000) pCam->mC_rotation.m0_X.m_value += 0xB60B6;
    if (input.m6_buttonDown & 0x0001) pCam->mC_rotation.m0_X.m_value -= 0xB60B6;
    if (input.m6_buttonDown & 0x8000) pCam->mC_rotation.m4_Y.m_value -= 0xB60B6;
    if (input.m6_buttonDown & 0x0800) pCam->mC_rotation.m4_Y.m_value += 0xB60B6;

    if (input.mE & 0x2000) pCam->mC_rotation.m0_X.m_value += 0x2D82D8;
    if (input.mE & 0x0001) pCam->mC_rotation.m0_X.m_value -= 0x2D82D8;
    if (input.mE & 0x8000) pCam->mC_rotation.m4_Y.m_value -= 0x2D82D8;
    if (input.mE & 0x0800) pCam->mC_rotation.m4_Y.m_value += 0x2D82D8;

    u16 yawIdx = (u16)((u32)pCam->mC_rotation.m4_Y.m_value >> 16) & 0xFFF;

    // Movement: forward/back/strafe slow
    if (input.m6_buttonDown & 0x10)
    {
        pCam->m0_position.m0_X.m_value -= MTH_Mul(fixedPoint(0x1000), getSin(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value -= MTH_Mul(fixedPoint(0x1000), getCos(yawIdx)).m_value;
    }
    if (input.m6_buttonDown & 0x20)
    {
        pCam->m0_position.m0_X.m_value += MTH_Mul(fixedPoint(0x1000), getSin(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value += MTH_Mul(fixedPoint(0x1000), getCos(yawIdx)).m_value;
    }
    if (input.m6_buttonDown & 0x80)
    {
        pCam->m0_position.m0_X.m_value += MTH_Mul(fixedPoint(0x1000), getCos(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value -= MTH_Mul(fixedPoint(0x1000), getSin(yawIdx)).m_value;
    }
    if (input.m6_buttonDown & 0x40)
    {
        pCam->m0_position.m0_X.m_value -= MTH_Mul(fixedPoint(0x1000), getCos(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value += MTH_Mul(fixedPoint(0x1000), getSin(yawIdx)).m_value;
    }

    // Up/down slow
    if (input.m6_buttonDown & 0x4000) pCam->m0_position.m4_Y.m_value += 0x1000;
    if (input.m6_buttonDown & 0x0004) pCam->m0_position.m4_Y.m_value -= 0x1000;

    // Movement: forward/back/strafe fast (held buttons)
    if (input.mE & 0x10)
    {
        pCam->m0_position.m0_X.m_value -= MTH_Mul(fixedPoint(0x4000), getSin(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value -= MTH_Mul(fixedPoint(0x4000), getCos(yawIdx)).m_value;
    }
    if (input.mE & 0x20)
    {
        pCam->m0_position.m0_X.m_value += MTH_Mul(fixedPoint(0x4000), getSin(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value += MTH_Mul(fixedPoint(0x4000), getCos(yawIdx)).m_value;
    }
    if (input.mE & 0x80)
    {
        pCam->m0_position.m0_X.m_value += MTH_Mul(fixedPoint(0x4000), getCos(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value -= MTH_Mul(fixedPoint(0x4000), getSin(yawIdx)).m_value;
    }
    if (input.mE & 0x40)
    {
        pCam->m0_position.m0_X.m_value -= MTH_Mul(fixedPoint(0x4000), getCos(yawIdx)).m_value;
        pCam->m0_position.m8_Z.m_value += MTH_Mul(fixedPoint(0x4000), getSin(yawIdx)).m_value;
    }

    // Up/down fast
    if (input.mE & 0x4000) pCam->m0_position.m4_Y.m_value += 0x4000;
    if (input.mE & 0x0004) pCam->m0_position.m4_Y.m_value -= 0x4000;
}

// 06062d90
static void cameraFollowMode7_Draw(sFieldCameraStatus*)
{
    s_FieldSubTaskWorkArea* pSub = getFieldTaskPtr()->m8_pSubFieldData;
    if (pSub->m380_debugMenuStatus3 != 0 && pSub->m37E_debugMenuStatus2_a == 1)
    {
        debugFreeCamera();
    }

    if (pSub->m37C_debugMenuStatus1[0] == 0)
    {
        sFieldCameraManager* pCam = pSub->m334;
        setCameraFollowMode_blend((u32)(s8)pCam->m50E_followModeIndex);
    }
}

s_fieldCameraConfig* readCameraConfig(sSaturnPtr EA)
{
    s_fieldCameraConfig* pCameraConfig = new s_fieldCameraConfig;

    pCameraConfig->m0_min[0] = readSaturnS32(EA); EA = EA + 4;
    pCameraConfig->m0_min[1] = readSaturnS32(EA); EA = EA + 4;
    pCameraConfig->m0_min[2] = readSaturnS32(EA); EA = EA + 4;

    pCameraConfig->mC_max[0] = readSaturnS32(EA); EA = EA + 4;
    pCameraConfig->mC_max[1] = readSaturnS32(EA); EA = EA + 4;
    pCameraConfig->mC_max[2] = readSaturnS32(EA); EA = EA + 4;

    for (int i = 0; i < 7; i++)
    {
        pCameraConfig->m18[i] = readSaturnS32(EA); EA = EA + 4;
    }

    for (int i = 0; i < 7; i++)
    {
        pCameraConfig->m34[i] = readSaturnS32(EA); EA = EA + 4;
    }

    pCameraConfig->m50 = readSaturnS32(EA); EA = EA + 4;
    pCameraConfig->m54 = readSaturnS32(EA); EA = EA + 4;

    return pCameraConfig;
}
