#include "PDS.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldDragonMovement.h"
#include "field/fieldCutsceneTask2.h"
#include "field/fieldCutsceneTask3.h"

fixedPoint interpolateDistance(fixedPoint r11, fixedPoint r12, fixedPoint stack0, fixedPoint r10, s32 r14);
void fieldOverlaySubTaskInitSub5(u32 r4);
void activateCameraFollowMode(u32 r4);

// 06072fce
u32 cutsceneTaskInitSub0(std::vector<s_scriptData3>& r4, std::vector<s_scriptData3>& r5)
{
    if (r4.size() == 0)
        return 0;

    u32 r6 = 0;
    int i = 0;
    while (r4[i].m0_duration)
    {
        r6 += r4[i].m0_duration;
        r5[i] = r4[i];

        i++;
    }

    r5[i].m0_duration = r4[i].m0_duration;
    return r6;
}

// 0607301e
void cutsceneTaskInitSub1(s_scriptData3* r15)
{
    s_dragonTaskWorkArea* r4 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    r4->m1E4_cutsceneKeyFrame = r15;
    r4->m104_dragonScriptStatus = 0;

    r4->mF0 = dragonCutsceneUpdate;
    r4->mF8_Flags &= ~0x400;
}

// 060625d8
void updateCutsceneCameraInterpolation(sFieldCameraManager* r4, sFieldCameraStatus* r5)
{
    r4->m378_cutsceneFrameCounter++;
    r5->m5C_rotationSpring[0] = interpolateRotation(r5->m5C_rotationSpring[0], 0, 0x2000, 0x444444, 0);
    r5->m5C_rotationSpring[1] = interpolateRotation(r5->m5C_rotationSpring[1], 0, 0x2000, 0x444444, 0);
    r5->m5C_rotationSpring[2] = interpolateRotation(r5->m5C_rotationSpring[2], 0, 0x2000, 0x444444, 0);

    r5->m5C_rotationSpring += r5->m68_rotationImpulse;
    r5->m68_rotationImpulse.zeroize();

    r5->m44_positionSpring[0] = interpolateDistance(r5->m44_positionSpring[0], 0, 0x2000, 0xAAA, 0);
    r5->m44_positionSpring[1] = interpolateDistance(r5->m44_positionSpring[1], 0, 0x2000, 0xAAA, 0);
    r5->m44_positionSpring[2] = interpolateDistance(r5->m44_positionSpring[2], 0, 0x2000, 0xAAA, 0);

    r5->m44_positionSpring += r5->m50_positionImpulse;
    r5->m50_positionImpulse.zeroize();

    r5->m0_position = r5->m44_positionSpring + *r4->m370_cutsceneLookAtPtr;

    sVec3_FP var18;
    var18 = *r4->m374_cutsceneCameraPos - *r4->m370_cutsceneLookAtPtr;

    sVec2_FP dummy;
    computeLookAt(var18, dummy);

    r5->mC_rotation[0] = r5->m5C_rotationSpring[0] + dummy[0];
    r5->mC_rotation[1] = r5->m5C_rotationSpring[1] + dummy[1];
    r5->mC_rotation[2] += r5->m5C_rotationSpring[2];

    r5->m24_distanceToDestination = vecDistance(*r4->m370_cutsceneLookAtPtr, *r4->m374_cutsceneCameraPos);
}

void startCutsceneCameraTracking(sVec3_FP* r4, sVec3_FP* r5)
{
    sFieldCameraManager* r14 = getFieldTaskPtr()->m8_pSubFieldData->m334;
    r14->m370_cutsceneLookAtPtr = r4;
    r14->m374_cutsceneCameraPos = r5;
    r14->m378_cutsceneFrameCounter = 0;
    r14->m37C_isCutsceneCameraActive = 1;

    initDragonMovementMode();

    initCameraSlotWithFunctions(1, NULL, NULL);
    fieldOverlaySubTaskInitSub3(1);

    updateCutsceneCameraInterpolation(r14, getFieldCameraStatus());
}

void endCutsceneCameraWithRestore()
{
    sFieldCameraManager* r13 = getFieldTaskPtr()->m8_pSubFieldData->m334;
    r13->m378_cutsceneFrameCounter = 0;
    r13->m37C_isCutsceneCameraActive = 0;
    dragonFieldTaskInitSub4Sub4();
    fieldOverlaySubTaskInitSub5(1);

    r13->m3E4_cameraSlots[0].m0_position = r13->m3E4_cameraSlots[1].m0_position;
    r13->m3E4_cameraSlots[0].mC_rotation = r13->m3E4_cameraSlots[1].mC_rotation;
    r13->m3E4_cameraSlots[0].m18 = r13->m3E4_cameraSlots[1].m18;
    r13->m3E4_cameraSlots[0].m1C = r13->m3E4_cameraSlots[1].m1C;
    r13->m3E4_cameraSlots[0].m20 = r13->m3E4_cameraSlots[1].m20;
    r13->m3E4_cameraSlots[0].m24_distanceToDestination = r13->m3E4_cameraSlots[1].m24_distanceToDestination;

    fieldOverlaySubTaskInitSub3(0);

    activateCameraFollowMode(getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex);
}

void endCutsceneCameraNoRestore()
{
    sFieldCameraManager* r13 = getFieldTaskPtr()->m8_pSubFieldData->m334;
    r13->m378_cutsceneFrameCounter = 0;
    r13->m37C_isCutsceneCameraActive = 0;
    dragonFieldTaskInitSub4Sub4();
    fieldOverlaySubTaskInitSub5(1);
    fieldOverlaySubTaskInitSub3(0);
    dragonFieldTaskInitSub4Sub3(getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex);
}

void s_cutsceneTask::Init(s_cutsceneTask* pThis, s_cutsceneData* pCutsceneData)
{
    s_dragonTaskWorkArea* r14 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    u32 r11 = (pCutsceneData->m8 & ~3) / 2;

    if (r14->m1D8_cutscene)
    {
        r14->m1D8_cutscene->getTask()->markFinished();
    }

    r14->mF8_Flags &= ~0x400;
    r14->mF8_Flags |= 0x40000;

    r14->m1D8_cutscene = pThis;

    r14->m1D4_cutsceneData = pCutsceneData;

    r14->m1EC = cutsceneTaskInitSub0(pCutsceneData->m0, *getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m90);

    r14->m1EE = r14->m1EC;

    cutsceneTaskInitSub1(&pCutsceneData->m0[pThis->m0]);

    r14->mF0(r14);

    switch (pCutsceneData->m8 & 3)
    {
    case 1:
        assert(pCutsceneData->m4.size());
        cutsceneTaskInitSub2(pThis, pCutsceneData->m4, 0, &r14->m8_pos, r11);
        break;
    case 2:
        assert(pCutsceneData->m4.size());
        cutsceneTaskInitSub2(pThis, pCutsceneData->m4, 0, &r14->m8_pos, 1 | r11);
        break;
    case 3:
        assert(pCutsceneData->m4bis.size());
        cutsceneTaskInitSub3(pThis, pCutsceneData->m4bis, 0, &r14->m8_pos, 1 | r11);
        break;
    default:
        assert(0);
        break;
    }
}

void cutsceneTaskUpdateSub0()
{
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    pDragonTask->m1E4_cutsceneKeyFrame = 0;
}

static void cutsceneTaskUpdateSub1(s32 fieldIndex, s32 fieldParam, s32 fieldExitIndex, s32 r7)
{
    exitCutsceneTaskUpdateSub0Sub0();
    exitCutsceneTaskUpdateSub0Sub1(fieldIndex, fieldParam, fieldExitIndex, r7);
}

void s_cutsceneTask::Update(s_cutsceneTask* pThis)
{
    s_dragonTaskWorkArea* r13 = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (r13->m1D4_cutsceneData == NULL)
    {
        cutsceneTaskUpdateSub0();
        dragonFieldTaskInitSub4Sub4();
        pThis->getTask()->markFinished();
        r13->m1D8_cutscene = 0;
        return;
    }
    if (r13->m1E4_cutsceneKeyFrame == NULL)
    {
        s_scriptData3* pData = &r13->m1D4_cutsceneData->m0[++pThis->m0];
        if (pData->m0_duration)
        {
            cutsceneTaskInitSub1(pData);
        }
        else
        {
            r13->m1D4_cutsceneData = NULL;
        }
    }

    pThis->m18_frameCount++;

    if (pThis->m4_changeField)
    {
        if (r13->m1EE <= 20)
        {
            cutsceneTaskUpdateSub1(pThis->m8_fieldIndex, pThis->mC_fieldParam, pThis->m10_fieldExitIndex, pThis->m14);
            pThis->m4_changeField = 0;
        }
    }
}

void startCutscene(s_cutsceneData* r4)
{
    createSubTaskWithArg<s_cutsceneTask>(getFieldTaskPtr()->m8_pSubFieldData, r4);
}

void startExitFieldCutscene2Sub0(p_workArea parent, s_cutsceneData* pScript, s32 fieldIndex, s32 param, s32 exitIndex, s32 arg0)
{
    s_cutsceneTask* pCutsceneTask = createSubTaskWithArg<s_cutsceneTask>(getFieldTaskPtr()->m8_pSubFieldData, pScript);
    pCutsceneTask->m4_changeField = 1;
    pCutsceneTask->m8_fieldIndex = fieldIndex;
    pCutsceneTask->mC_fieldParam = param;
    pCutsceneTask->m10_fieldExitIndex = exitIndex;
    pCutsceneTask->m14 = arg0;
}

void startExitFieldCutscene2(p_workArea parent, s_cutsceneData* pScript, s32 param, s32 exitIndex, s32 arg0)
{
    startExitFieldCutscene2Sub0(parent, pScript, getFieldTaskPtr()->m2C_currentFieldIndex, param, exitIndex, arg0);
}

void loadScriptData1(std::vector<s_scriptData1>& output, const sSaturnPtr& constEA)
{
    sSaturnPtr EA = constEA;

    int numEntries = 0;
    while (readSaturnU32(EA))
    {
        numEntries++;
        EA = EA + 0x3C;
    }
    EA -= 0x3C * numEntries;
    numEntries++;

    output.resize(numEntries);
    for (int i = 0; i < numEntries; i++)
    {
        output[i].m0 = readSaturnS32(EA); EA = EA + 4;
        output[i].m4 = readSaturnVec3(EA); EA = EA + 4 * 3;
        output[i].m10 = readSaturnVec3(EA); EA = EA + 4 * 3;
        output[i].m1C = readSaturnVec3(EA); EA = EA + 4 * 3;
        output[i].m28 = readSaturnVec3(EA); EA = EA + 4 * 3;
        output[i].m34 = readSaturnS32(EA); EA = EA + 4;
        output[i].m38 = readSaturnS32(EA); EA = EA + 4;
    }

}

s_cutsceneData* loadCutsceneData(sSaturnPtr EA)
{
    s_cutsceneData* pData = new s_cutsceneData;

    sSaturnPtr table0 = readSaturnEA(EA); EA = EA + 4;
    sSaturnPtr table1 = readSaturnEA(EA); EA = EA + 4;
    pData->m8 = readSaturnU8(EA); EA = EA + 1;

    // read table 0
    {
        int numEntries = 0;
        while (readSaturnU32(table0))
        {
            numEntries++;
            table0 = table0 + 0x20;
        }
        table0 -= 0x20 * numEntries;
        numEntries++;

        pData->m0.resize(numEntries);
        for (int i = 0; i < numEntries; i++)
        {
            pData->m0[i].m0_duration = readSaturnS32(table0); table0 = table0 + 4;
            pData->m0[i].m4_pos = readSaturnVec3(table0); table0 = table0 + 4 * 3;
            pData->m0[i].m10_rotationDuration = readSaturnS32(table0); table0 = table0 + 4;
            pData->m0[i].m14_rot = readSaturnVec3(table0); table0 = table0 + 4 * 3;
        }
    }

    // read table 1
    switch (pData->m8 & 3)
    {
    case 1:
    case 2:
    {
        loadScriptData1(pData->m4, table1);
        break;
    }
    case 3:
    {
        int numEntries = 0;
        while (readSaturnU32(table1))
        {
            numEntries++;
            table1 = table1 + 0x24;
        }
        table1 -= 0x24 * numEntries;
        numEntries++;

        pData->m4bis.resize(numEntries);
        for (int i = 0; i < numEntries; i++)
        {
            pData->m4bis[i].m0 = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].m4 = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].m8 = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].mC = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].m10 = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].m14 = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].m18 = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].m1C = readSaturnS32(table1); table1 = table1 + 4;
            pData->m4bis[i].m20 = readSaturnS32(table1); table1 = table1 + 4;
        }
        break;
    }
    default:
        assert(0);
    }

    return pData;
}
