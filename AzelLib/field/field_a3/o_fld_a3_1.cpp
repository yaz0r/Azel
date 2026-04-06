#include "PDS.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "a3_background_layer.h"
#include "items.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup
#include "field/fieldRadar.h"
#include "field/exitField.h"
#include "field/fieldItemBox.h"
#include "menu_dragonMorph.h"

fixedPoint interpolateDistance(fixedPoint r11, fixedPoint r12, fixedPoint stack0, fixedPoint r10, s32 r14);

// Above Excavation

void fieldA3_1_startTasks_sub1Task_InitSub0(p_workArea, sLCSTarget*)
{
    getFieldSpecificData_A3()->m9C_A3_1_exits[0] = 1;
}

void fieldA3_1_startTasks_sub1Task_InitSub1(p_workArea, sLCSTarget*)
{
    getFieldSpecificData_A3()->m9C_A3_1_exits[1] = 1;
}

void fieldA3_1_startTasks_sub1Task_InitSub2(p_workArea, sLCSTarget*)
{
    getFieldSpecificData_A3()->m9C_A3_1_exits[2] = 1;
}

void(*fieldA3_1_startTasks_sub1Task_InitFunctionTable[])(p_workArea, sLCSTarget*) = {
    fieldA3_1_startTasks_sub1Task_InitSub0,
    fieldA3_1_startTasks_sub1Task_InitSub1,
    fieldA3_1_startTasks_sub1Task_InitSub2
};

// Map exits
struct sfieldA3_1_startTasks_sub1Task : public s_workAreaTemplate<sfieldA3_1_startTasks_sub1Task>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sfieldA3_1_startTasks_sub1Task::Init, &sfieldA3_1_startTasks_sub1Task::Update, NULL, NULL};
        return &taskDefinition;
    }

    static void Init(sfieldA3_1_startTasks_sub1Task* pThis)
    {
        static const sVec3_FP fieldA3_1_startTasks_sub1Task_InitPositionTable[3] = {
            {0x348000, -0x10000, -0x1EE000},
            {0x2EA000, -0x30180, -0x15B53F4},
            {0x428000, -0x10000, -0x1A96000}
        };
        for (int i = 2; i >= 0; i--)
        {
            createLCSTarget(&pThis->m0[i], pThis, fieldA3_1_startTasks_sub1Task_InitFunctionTable[i], &fieldA3_1_startTasks_sub1Task_InitPositionTable[i], NULL, 3, 0, eItems::mMinusOne, 0, 0);
            getFieldSpecificData_A3()->m9C_A3_1_exits[i] = 0;
        }

        pThis->m0[0].m18_diableFlags |= 1;
    }

    static void Update(sfieldA3_1_startTasks_sub1Task* pThis)
    {
        const sVec3_FP& r4_dragonPos = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos;

        static const sVec2_FP zoneMin = { 0x280000 , -0x1720000 };
        static const sVec2_FP zoneMax = { 0x3E0000 , -0x1400000 };

        // Conana�s Nest check
        if (mainGameState.getBit(0x91 * 8 + 4) && (r4_dragonPos[0] >= zoneMin[0]) && (r4_dragonPos[0] < zoneMax[0]) && (r4_dragonPos[2] >= zoneMin[1]) && (r4_dragonPos[2] < zoneMax[1]))
        {
            pThis->m0[1].m18_diableFlags = 0;
        }
        else
        {
            pThis->m0[1].m18_diableFlags |= 1;
        }

        for (int i = 2; i >= 0; i--)
        {
            updateLCSTarget(&pThis->m0[i]);
        }
    }

    std::array<sLCSTarget, 3> m0; // stride is 0x34
    // size 9C
};

void fieldA3_1_startTasks_sub1(p_workArea workArea)
{
    createSubTask<sfieldA3_1_startTasks_sub1Task>(workArea);
}

struct sFieldA3_1_fieldIntroTask : public s_workAreaTemplate<sFieldA3_1_fieldIntroTask>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { NULL, NULL, NULL, NULL};
        return &taskDefinition;
    }

    static void Update(sFieldA3_1_fieldIntroTask* pThis)
    {
        if (startFieldScript(14, 1453))
        {
            pThis->getTask()->markFinished();
        }
    }
};

void create_fieldA3_1_fieldIntroTask(p_workArea workArea)
{
    if (!mainGameState.getBit(0xB5 * 8 + 5))
    {
        createSubTaskFromFunction<sFieldA3_1_fieldIntroTask>(workArea, &sFieldA3_1_fieldIntroTask::Update);
    }
}

static const std::array<s16,3> A3_1_exitsVars =
{
    1261,1261,1260
};

static const std::array<s8, 3> A3_1_exitsLocations =
{
    7, 8, 9
};

static const std::array<sSaturnPtr, 3> A3_1_exitsCutscenes =
{
    {
        {0, gFLD_A3},
        {0x6091190, gFLD_A3},
        {0x60911C4, gFLD_A3},
    }
};

s32 fieldA3_1_checkExitsTaskUpdate2Sub0()
{
    if (getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m48_cutsceneTask == nullptr)
    {
        return true;
    }
    return false;
}

// TODO: move to kernel
void fieldA3_1_checkExitsTaskUpdate2Sub1(s32 r4)
{
    if (getFieldTaskPtr()->m35 && (r4 > -1))
    {
        setNextGameStatus(r4);
    }
    else
    {
        dispatchTutorialMultiChoiceSub2();
        getFieldTaskPtr()->m3D = r4;
    }
}

struct sfieldA3_1_checkExitsTask : public s_workAreaTemplate<sfieldA3_1_checkExitsTask>
{
    static void Update2(sfieldA3_1_checkExitsTask* pThis)
    {
        if (fieldA3_1_checkExitsTaskUpdate2Sub0())
        {
            if (mainGameState.getBit(0xA * 8 + 6))
            {
                dispatchTutorialMultiChoiceSub2();
            }
            else
            {
                fieldA3_1_checkExitsTaskUpdate2Sub1(6);
            }
        }
    }
    static void Update(sfieldA3_1_checkExitsTask* pThis)
    {
        for (int i = 0; i < 3; i++)
        {
            if (getFieldSpecificData_A3()->m9C_A3_1_exits[i])
            {
                mainGameState.setBit566(A3_1_exitsVars[i]);

                if (i == 2)
                {
                    startCutscene(loadCutsceneData({ 0x6091CC4, gFLD_A3 }));
                    pThis->m_UpdateMethod = &sfieldA3_1_checkExitsTask::Update2;
                }
                else
                {
                    startExitFieldCutscene(pThis, readCameraScript(A3_1_exitsCutscenes[i]), A3_1_exitsLocations[i], i, 0x8000);
                    pThis->m_UpdateMethod = nullptr;
                }
            }
        }
    }
};

void create_fieldA3_1_checkExitsTask(p_workArea workArea)
{
    createSubTaskFromFunction<sfieldA3_1_checkExitsTask>(workArea, &sfieldA3_1_checkExitsTask::Update);
}

void fieldA3_1_startTasks_sub0()
{
    getFieldSpecificData_A3()->m0 = fixedPoint(0x10000);
}


void fieldA3_1_createItemBoxes(p_workArea workArea)
{
    fieldA3_1_createItemBoxes_Sub0(workArea, readItemBoxDefinition({ 0x6092034, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub0(workArea, readItemBoxDefinition({ 0x609207C, gFLD_A3 }));

    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x60920C4, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x6092154, gFLD_A3 }));
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition({ 0x609210C, gFLD_A3 }));
}

void fieldA3_1_task4_init(s_fieldA3_1_task4* pThis)
{
    sSaturnPtr r6 = gFLD_A3->getSaturnPtr(0x06081E04);

    if (mainGameState.getBit(0xA2 * 8 + 3))
    {
        pThis->m0 = readSaturnS32(r6);
        pThis->m4 = readSaturnS32(r6 + 4);
    }
    else
    {
        r6 += 0x18;
        pThis->m0 = readSaturnS32(r6);
        pThis->m4 = readSaturnS32(r6 + 4);
    }

    pThis->m8 = readSaturnS32(r6 + 8);
}

void fieldA3_1_task4_update(s_fieldA3_1_task4* pThis)
{
    return; // but why?
}

static const s_fieldA3_1_task4::TypedTaskDefinition fieldA3_1_task4_definition = {
    fieldA3_1_task4_init,
    fieldA3_1_task4_update,
    nullptr,
    nullptr
};

void create_fieldA3_1_task4(p_workArea workArea)
{
    createSubTask<s_fieldA3_1_task4>(workArea, &fieldA3_1_task4_definition);
}

// 0605aa10
// Quadratic Bezier interpolation: result = P0*(1-t)^2 + P1*2*t*(1-t) + P2*t^2
static void fieldA3_1_bezierInterpolate(sVec3_FP* pResult, sVec3_FP* pControlPoints, s32 t)
{
    s32 oneMinusT = 0x10000 - t;
    s32 twoTOneMinusT = MTH_Mul(oneMinusT, t) << 1;
    s32 tSquared = MTH_Mul(t, t);
    s32 oneMinusTSquared = MTH_Mul(oneMinusT, oneMinusT);

    (*pResult)[0] = MTH_Mul(pControlPoints[0][0], oneMinusTSquared) + MTH_Mul(pControlPoints[1][0], twoTOneMinusT) + MTH_Mul(pControlPoints[2][0], tSquared);
    (*pResult)[1] = MTH_Mul(pControlPoints[0][1], oneMinusTSquared) + MTH_Mul(pControlPoints[1][1], twoTOneMinusT) + MTH_Mul(pControlPoints[2][1], tSquared);
    (*pResult)[2] = MTH_Mul(pControlPoints[0][2], oneMinusTSquared) + MTH_Mul(pControlPoints[1][2], twoTOneMinusT) + MTH_Mul(pControlPoints[2][2], tSquared);
}

// 0605aaca
// Compute rotation from direction vector, applying pitch offset of -0x8000000 (180 degrees)
static void fieldA3_1_computeRotationFromDirection(const sVec3_FP& direction, sVec3_FP& rotation)
{
    sVec2_FP lookAt;
    computeLookAt(direction, lookAt);
    rotation[0] = -lookAt[0];
    rotation[1] = lookAt[1] + (s32)0xf8000000;
}

// This is the LCS target used to activate the birds to access Conana's nest
struct fieldA3_1_startTasks_subTask : public s_workAreaTemplateWithArg<fieldA3_1_startTasks_subTask, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &fieldA3_1_startTasks_subTask::Init, &fieldA3_1_startTasks_subTask::Update, &fieldA3_1_startTasks_subTask::Draw, NULL };
        return &taskDefinition;
    }

    // 0605b2d0
    static void startBirdCutsceneToConanaNest(p_workArea pWorkArea, sLCSTarget*)
    {
        fieldA3_1_startTasks_subTask* pThis = static_cast<fieldA3_1_startTasks_subTask*>(pWorkArea);
        pThis->m68_lcsTarget.m18_diableFlags |= 1;
        pThis->m_DrawMethod = &fieldA3_1_startTasks_subTask::Draw2;
        pThis->mEC_state = 1;
        getFieldSpecificData_A3()->m130_conanaNestCutsceneTrigger = 1;
        startFieldScript(2, -1);
    }

    // 0605b504
    static void Init(fieldA3_1_startTasks_subTask* pThis, sSaturnPtr arg)
    {
        getMemoryArea(&pThis->m0_memoryArea, 3);

        s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
        sAnimationData* pAnimation = pBundle->getAnimation(0x324);
        sStaticPoseData* pPose = pBundle->getStaticPose(0x2A0, pAnimation->m2_numBones);
        init3DModelRawData(pThis, &pThis->m9C_3dModel, 0, pBundle, 4, pAnimation, pPose, 0, nullptr);
        stepAnimation(&pThis->m9C_3dModel);

        pThis->m8_translation = readSaturnVec3(arg);
        pThis->m54 = 0x67B4;
        pThis->m14_rotation[0] = 0;
        pThis->m14_rotation[1] = 0;
        pThis->m14_rotation[2] = 0;
        pThis->m2C_pPathData = gFLD_A3->getSaturnPtr(0x060907cc);
        pThis->m64 = 2;
        pThis->mEC_state = 0;
        createLCSTarget(&pThis->m68_lcsTarget, pThis, &fieldA3_1_startTasks_subTask::startBirdCutsceneToConanaNest, &pThis->m8_translation, nullptr, 0, 0, eItems::mMinusOne, 0, 0);
        if (!mainGameState.getBit(0xA * 8 + 1))
        {
            pThis->m68_lcsTarget.m18_diableFlags |= 1;
        }
    }

    // 0605b35c
    static void Update(fieldA3_1_startTasks_subTask* pThis)
    {
        updateLCSTarget(&pThis->m68_lcsTarget);

        s32 state = pThis->mEC_state;
        if (state == 0)
        {
            if (getFieldSpecificData_A3()->m130_conanaNestCutsceneTrigger == 1)
            {
                fieldA3_1_startTasks_subTask::startBirdCutsceneToConanaNest(pThis, nullptr);
            }
        }
        else if (state == 1)
        {
            sSaturnPtr pathPtr = pThis->m2C_pPathData;
            if (readSaturnS32(pathPtr + 0xC) == 0)
            {
                pThis->mEC_state = 3;
                return;
            }
            fixedPoint distance = vecDistance(readSaturnVec3(pathPtr), pThis->m8_translation);
            s32 timer = (s32)distance / pThis->m54;
            pThis->m58_timer = timer;
            if (timer != 0)
            {
                pThis->m30_P0 = pThis->m8_translation;
                pThis->m3C_P1 = readSaturnVec3(pathPtr);
                pThis->m2C_pPathData = pathPtr + 0xC;
                pThis->m48_P2 = readSaturnVec3(pThis->m2C_pPathData);
                if (pThis->m64 != 0)
                {
                    pThis->m64 = pThis->m64 - 1;
                    pThis->m58_timer = pThis->m58_timer * 3 >> 1;
                }
                pThis->m5C = 0x8000 / pThis->m58_timer;
                return;
            }
            pThis->mEC_state = 2;
            // fall through to state 2
            goto state2;
        }
        else if (state == 2)
        {
        state2:
            if ((pThis->m64 == 0) || ((pThis->m58_timer & 3) == 0))
            {
                stepAnimation(&pThis->m9C_3dModel);
            }
            sVec3_FP prevPos = pThis->m8_translation;
            s32 accumulator = pThis->m60 + pThis->m5C;
            pThis->m60 = accumulator;
            fieldA3_1_bezierInterpolate(&pThis->m8_translation, &pThis->m30_P0, accumulator);
            sVec3_FP delta;
            delta[0] = pThis->m8_translation[0] - prevPos[0];
            delta[1] = pThis->m8_translation[1] - prevPos[1];
            delta[2] = pThis->m8_translation[2] - prevPos[2];
            fieldA3_1_computeRotationFromDirection(delta, pThis->m14_rotation);
            if (pThis->m58_timer < 1)
            {
                pThis->mEC_state = 1;
            }
            else
            {
                pThis->m58_timer = pThis->m58_timer - 1;
            }
        }
        else if (state == 3)
        {
            getFieldSpecificData_A3()->m130_conanaNestCutsceneTrigger = 2;
        }
    }

    // 0605b276
    static void Draw2(fieldA3_1_startTasks_subTask* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_translation);
        rotateCurrentMatrixYXZ(pThis->m14_rotation);
        pThis->m9C_3dModel.m18_drawFunction(&pThis->m9C_3dModel);
        popMatrix();
    }

    static void Draw(fieldA3_1_startTasks_subTask* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_translation);
        rotateCurrentMatrixZYX(&pThis->m14_rotation);
        addBillBoardToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(0xA0)); // draw a bird
        popMatrix();
    }

    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_translation;
    sVec3_FP m14_rotation;
    sVec3_FP m20;
    sSaturnPtr m2C_pPathData;
    sVec3_FP m30_P0;
    sVec3_FP m3C_P1;
    sVec3_FP m48_P2;
    s32 m54;
    s32 m58_timer;
    s32 m5C;
    s32 m60;
    s32 m64;
    sLCSTarget m68_lcsTarget;
    s_3dModel m9C_3dModel;
    s8 mEC_state;
    // size F0
};

struct fieldA3_1_startTasks_subTask2 : public s_workAreaTemplateWithArg<fieldA3_1_startTasks_subTask2, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &fieldA3_1_startTasks_subTask2::Init, &fieldA3_1_startTasks_subTask2::Update, &fieldA3_1_startTasks_subTask2::Draw, NULL };
        return &taskDefinition;
    }

    // 0605ae32
    static void subTask2_lcsCallback(p_workArea pWorkArea, sLCSTarget*)
    {
        fieldA3_1_startTasks_subTask2* pThis = static_cast<fieldA3_1_startTasks_subTask2*>(pWorkArea);
        pThis->m44_lcsTarget.m18_diableFlags |= 1;
        pThis->m_DrawMethod = &fieldA3_1_startTasks_subTask2::Draw2;
        pThis->mC8_state = 1;
        getFieldSpecificData_A3()->m130_conanaNestCutsceneTrigger = 1;
    }

    // 0605b1b4
    static void Init(fieldA3_1_startTasks_subTask2* pThis, sSaturnPtr arg)
    {
        getMemoryArea(&pThis->m0_memoryArea, 3);

        s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
        sAnimationData* pAnimation = pBundle->getAnimation(0x324);
        sStaticPoseData* pPose = pBundle->getStaticPose(0x2A0, pAnimation->m2_numBones);

        init3DModelRawData(pThis, &pThis->m78_3dModel, 0, pBundle, 4, pAnimation, pPose, 0, nullptr);
        stepAnimation(&pThis->m78_3dModel);

        pThis->m8_translation = readSaturnVec3(arg);
        pThis->m30 = readSaturnS32(arg + 0xC);
        pThis->m34 = readSaturnS32(arg + 0x10);
        pThis->m3C = readSaturnS32(arg + 0x14);
        pThis->m40 = (randomNumber() & 0x1f) + 0x10;
        pThis->m2C = 0x8cbd;
        pThis->mC8_state = 0;
        createLCSTarget(&pThis->m44_lcsTarget, pThis, &fieldA3_1_startTasks_subTask2::subTask2_lcsCallback, &pThis->m8_translation, nullptr, 0, 0, eItems::mMinusOne, 0, 0);
        if (!mainGameState.getBit(0xA * 8 + 1))
        {
            pThis->m44_lcsTarget.m18_diableFlags |= 1;
        }
    }

    // 0605ae5a
    static void Update(fieldA3_1_startTasks_subTask2* pThis)
    {
        updateLCSTarget(&pThis->m44_lcsTarget);

        s32 state = pThis->mC8_state;
        if (state == 0)
        {
            if (getFieldSpecificData_A3()->m130_conanaNestCutsceneTrigger == 1)
            {
                subTask2_lcsCallback(pThis, nullptr);
            }
        }
        else if (state == 1)
        {
            s_fieldSpecificData_A3* pFieldData = getFieldSpecificData_A3();
            s32 angleOffset = pThis->m34;
            s32 fieldAngle = pFieldData->mC0[8].m8_Z;

            sVec3_FP& dragonPos = pFieldData->mC0[7];
            pThis->m20[0] = dragonPos.m0_X - pThis->m8_translation[0];
            pThis->m20[1] = dragonPos.m4_Y - pThis->m8_translation[1];
            pThis->m20[2] = dragonPos.m8_Z - pThis->m8_translation[2];

            s32 angle = (s32)(u16)(((u32)(fieldAngle + angleOffset) >> 16) & 0xFFF);
            s32 sinAngle = getSin(angle);
            s32 cosAngle = getCos(angle);
            pThis->m20[0] = pThis->m20[0] + MTH_Mul(pThis->m30, sinAngle);
            pThis->m20[2] = pThis->m20[2] + MTH_Mul(pThis->m30, cosAngle);
            fieldA3_1_computeRotationFromDirection(pThis->m20, pThis->m14_rotation);

            fixedPoint distSq = MTH_Product3d_FP(pThis->m20, pThis->m20);
            fixedPoint dist = sqrt_F(distSq);
            if (dist < pThis->m2C)
            {
                pThis->mC8_state = 2;
            }
            else if (dist < fixedPoint(0x3c000))
            {
                pThis->m2C = 0x7684;
            }
        }
        else if (state == 2)
        {
            s32 fieldAngle = getFieldSpecificData_A3()->mC0[8].m8_Z;
            s32 angleDiff = pThis->m38 - fieldAngle;
            s32 absDiff = angleDiff < 0 ? -angleDiff : angleDiff;
            if (pThis->m3C < absDiff)
            {
                bool positive = angleDiff > 0;
                bool smallEnough = absDiff <= (s32)0x3FFFFFF;
                if (positive != smallEnough)
                {
                    // Add m3C to m38
                    u32 newAngle = pThis->m38 + pThis->m3C;
                    if ((newAngle & 0x8000000) == 0)
                        newAngle = newAngle & 0xFFFFFFF;
                    else
                        newAngle = newAngle | 0xF0000000;
                    pThis->m38 = newAngle;
                }
                else
                {
                    // Subtract m3C from m38
                    u32 newAngle = pThis->m38 - pThis->m3C;
                    if ((newAngle & 0x8000000) == 0)
                        newAngle = newAngle & 0xFFFFFFF;
                    else
                        newAngle = newAngle | 0xF0000000;
                    pThis->m38 = newAngle;
                }
            }

            s32 angleOffset = pThis->m34;
            s32 angle2 = pThis->m38;
            s32 dragonX = getFieldSpecificData_A3()->mC0[7].m0_X;
            s32 angleIdx = (s32)(u16)(((u32)(angle2 + angleOffset) >> 16) & 0xFFF);
            pThis->m8_translation[0] = MTH_Mul(pThis->m30, getSin(angleIdx)) + dragonX;
            pThis->m8_translation[1] = getFieldSpecificData_A3()->mC0[7].m4_Y;
            pThis->m8_translation[2] = MTH_Mul(pThis->m30, getCos(angleIdx)) + getFieldSpecificData_A3()->mC0[7].m8_Z;
        }
    }

    // 0605ad78
    static void Draw2(fieldA3_1_startTasks_subTask2* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_translation);
        rotateCurrentMatrixYXZ(pThis->m14_rotation);
        pThis->m78_3dModel.m18_drawFunction(&pThis->m78_3dModel);
        popMatrix();
    }

    static void Draw(fieldA3_1_startTasks_subTask2* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_translation);
        rotateCurrentMatrixZYX(&pThis->m14_rotation);
        addBillBoardToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(0x9C));
        popMatrix();
    }

    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_translation;
    sVec3_FP m14_rotation;
    sVec3_FP m20;
    s32 m2C;
    s32 m30;
    s32 m34;
    s32 m38;
    s32 m3C;
    s32 m40;
    sLCSTarget m44_lcsTarget;
    s_3dModel m78_3dModel;
    s8 mC8_state;
    // size CC
};

struct fieldA3_1_startTasks_subTask3 : public s_workAreaTemplateWithArg<fieldA3_1_startTasks_subTask3, sSaturnPtr>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &fieldA3_1_startTasks_subTask3::Init, &fieldA3_1_startTasks_subTask3::Update, &fieldA3_1_startTasks_subTask3::Draw, NULL };
        return &taskDefinition;
    }

    static void Init(fieldA3_1_startTasks_subTask3* pThis, sSaturnPtr arg)
    {
        getMemoryArea(&pThis->m0_memoryArea, 3);

        s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
        sAnimationData* pAnimation = pBundle->getAnimation(0x324);
        sStaticPoseData* pPose = pBundle->getStaticPose(0x2A0, pAnimation->m2_numBones);

        init3DModelRawData(pThis, &pThis->m3C_3dModel, 0, pBundle, 4, pAnimation, pPose, 0, nullptr);
        stepAnimation(&pThis->m3C_3dModel);

        pThis->m8_translation = readSaturnVec3(arg);
        pThis->m2C = readSaturnVec3(arg + 0xC);
        pThis->m20_rotation = readSaturnVec3(arg + 0xC * 2);
    }

    // 0605abba
    static void Update(fieldA3_1_startTasks_subTask3* pThis)
    {
        sVec3_FP& dragonPos = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m8_pos;
        s32 dx = dragonPos[0] - pThis->m8_translation[0];
        if (dx < 0) dx = -dx;
        if (dx < (s32)0x3c000)
        {
            s32 dz = dragonPos[2] - pThis->m8_translation[2];
            if (dz < 0) dz = -dz;
            if (dz < (s32)0x3c000)
            {
                // Bird is close to dragon - compute flight direction toward target
                sVec3_FP direction;
                direction[0] = pThis->m2C[0] - pThis->m8_translation[0];
                direction[1] = pThis->m2C[1] - pThis->m8_translation[1];
                direction[2] = pThis->m2C[2] - pThis->m8_translation[2];

                fixedPoint distSq = MTH_Product3d_FP(direction, direction);
                fixedPoint dist = sqrt_F(distSq);
                fixedPoint pitchAngle = atan_FP(FP_Div(direction[1], dist));
                u16 pitchIdx = (u16)((u32)pitchAngle >> 16) & 0xFFF;

                s32 yawAngle = atan2_FP(direction[0], direction[2]);
                pThis->m20_rotation[1] = yawAngle;

                s32 yawIdx = (s16)((u16)((u32)yawAngle >> 16) & 0xFFF);
                pThis->m14[0] = MTH_Mul(0x1da1, getSin(yawIdx));
                pThis->m14[1] = MTH_Mul(0x1da1, getSin(pitchIdx));
                pThis->m14[2] = MTH_Mul(0x1da1, getCos(yawIdx));

                pThis->m_UpdateMethod = &fieldA3_1_startTasks_subTask3::Update2;
                pThis->m_DrawMethod = &fieldA3_1_startTasks_subTask3::Draw2;
            }
        }
    }

    // 0605ab74
    static void Update2(fieldA3_1_startTasks_subTask3* pThis)
    {
        stepAnimation(&pThis->m3C_3dModel);
        pThis->m8_translation[0] = pThis->m8_translation[0] + pThis->m14[0];
        pThis->m8_translation[1] = pThis->m8_translation[1] + pThis->m14[1];
        pThis->m8_translation[2] = pThis->m8_translation[2] + pThis->m14[2];
        pThis->m14[1] = pThis->m14[1] + 0x25;
        if (pThis->m8_translation[1] > (s32)0x190000)
        {
            pThis->getTask()->markFinished();
        }
    }

    // 0605aaf4
    static void Draw2(fieldA3_1_startTasks_subTask3* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_translation);
        rotateCurrentMatrixYXZ(pThis->m20_rotation);
        pThis->m3C_3dModel.m18_drawFunction(&pThis->m3C_3dModel);
        popMatrix();
    }

    static void Draw(fieldA3_1_startTasks_subTask3* pThis)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_translation);
        rotateCurrentMatrixZYX(&pThis->m20_rotation);
        addBillBoardToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(0xA0)); // draw a bird
        popMatrix();
    }

    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_translation;
    sVec3_FP m14;
    sVec3_FP m20_rotation;
    sVec3_FP m2C;
    s_3dModel m3C_3dModel;
    // size 8C
};

struct sfieldA3_1_startTasks_sub3_func_task : public s_workAreaTemplate<sfieldA3_1_startTasks_sub3_func_task>
{
    fieldA3_1_startTasks_subTask* m0;
    // size 4
};

// 0605B5BE
void fieldA3_1_startTasks_sub3_func(sfieldA3_1_startTasks_sub3_func_task* pThis)
{
    s_fieldSpecificData_A3* pFieldData = getFieldSpecificData_A3();
    fieldA3_1_startTasks_subTask* pSubTask = pThis->m0;

    // Copy bird position/rotation into field-specific data
    pFieldData->mC0[7][0] = pSubTask->m8_translation[0];
    pFieldData->mC0[7][1] = pSubTask->m8_translation[1];
    pFieldData->mC0[7][2] = pSubTask->m8_translation[2];
    pFieldData->mC0[8][1] = pSubTask->m14_rotation[0];
    pFieldData->mC0[8][2] = pSubTask->m14_rotation[1];
    pFieldData->mC0[9][0] = pSubTask->m14_rotation[2];
    pFieldData->mC0[8][0] = fixedPoint(pSubTask->m2C_pPathData.m_offset);

    if (pFieldData->m130_conanaNestCutsceneTrigger == 2)
    {
        mainGameState.setBit(0x91 * 8 + 3);
        pThis->getTask()->markFinished();
    }
}

void fieldA3_1_startTasks_sub3(p_workArea workArea)
{
    sfieldA3_1_startTasks_sub3_func_task* r14 = createSubTaskFromFunction<sfieldA3_1_startTasks_sub3_func_task>(workArea, &fieldA3_1_startTasks_sub3_func);

    r14->m0 = createSubTaskWithArg<fieldA3_1_startTasks_subTask>(r14, gFLD_A3->getSaturnPtr(0x6090778));

    for (int i = 0; i < 7; i++)
    {
        createSubTaskWithArg<fieldA3_1_startTasks_subTask2>(r14, gFLD_A3->getSaturnPtr(0x60906D0 + i * 0xC));
    }

    for (int i = 0; i < 14; i++)
    {
        createSubTaskWithArg<fieldA3_1_startTasks_subTask3>(r14, gFLD_A3->getSaturnPtr(0x6090B74 + i * 0xC));
    }

    getFieldSpecificData_A3()->m130_conanaNestCutsceneTrigger = 0; 
}

void fieldA3_1_startTasks(p_workArea workArea)
{
    fieldA3_0_createTask0(workArea);

    fieldA3_1_startTasks_sub1(workArea);

    create_fieldA3_backgroundLayer(workArea);

    create_fieldA3_1_fieldIntroTask(workArea);
    create_fieldA3_1_checkExitsTask(workArea);

    create_fieldA3_1_task4(workArea);

    fieldA3_1_startTasks_sub3(workArea);

    fieldA3_1_createItemBoxes(workArea);

    fieldA3_1_startTasks_sub0();
}

// 0605816c
void subfieldA3_1Sub0Sub1(s_dragonTaskWorkArea* r4)
{
    s_fieldSpecificData_A3* fieldSpecData = getFieldSpecificData_A3();
    sVec3_FP& targetPos = fieldSpecData->mC0[7];
    s_FieldSubTaskWorkArea* pSubFieldData = getFieldTaskPtr()->m8_pSubFieldData;

    sVec3_FP newPos;
    newPos[0] = interpolateDistance(r4->m8_pos[0], targetPos[0], 0x1000, 0x9425, 0);
    newPos[1] = interpolateDistance(r4->m8_pos[1], targetPos[1] - 0x1E000, 0x1000, 0x9425, 0);
    newPos[2] = interpolateDistance(r4->m8_pos[2], targetPos[2], 0x1000, 0x9425, 0);

    r4->m160_deltaTranslation[0] = newPos[0] - r4->m8_pos[0];
    r4->m160_deltaTranslation[1] = newPos[1] - r4->m8_pos[1];
    r4->m160_deltaTranslation[2] = newPos[2] - r4->m8_pos[2];

    r4->m8_pos = newPos;

    sVec2_FP lookAt;
    computeLookAt(r4->m160_deltaTranslation, lookAt);

    r4->m20_angle[0] = interpolateRotation(r4->m20_angle[0], lookAt[0], 0x1000, 0x222222, 0);
    r4->m20_angle[1] = interpolateRotation(r4->m20_angle[1], lookAt[1], 0x1000, 0x222222, 0);
    r4->m20_angle[2] = interpolateRotation(r4->m20_angle[2], fieldSpecData->mC0[9][0], 0x1000, 0x222222, 0);

    if ((pSubFieldData->m370_fieldDebuggerWho & 1) == 0)
        return;
    if (pSubFieldData->m37C_debugMenuStatus1[1] != 0)
        return;
    if (pSubFieldData->m369 != 0)
        return;

    vdp2DebugPrintSetPosition(3, 0xF);
    vdp2PrintfSmallFont("%4d ", (s32)targetPos[0] >> 12);
    vdp2DebugPrintSetPosition(3, 0x10);
    vdp2PrintfSmallFont("%4d ", (s32)targetPos[1] >> 12);
    vdp2DebugPrintSetPosition(3, 0x11);
    vdp2PrintfSmallFont("%4d ", (s32)targetPos[2] >> 12);

    vdp2DebugPrintSetPosition(3, 0x13);
    vdp2PrintfSmallFont("%08x ", (s32)r4->m160_deltaTranslation[0] >> 12);
    vdp2DebugPrintSetPosition(3, 0x14);
    vdp2PrintfSmallFont("%08x ", (s32)r4->m160_deltaTranslation[1] >> 12);
    vdp2DebugPrintSetPosition(3, 0x15);
    vdp2PrintfSmallFont("%08x ", (s32)r4->m160_deltaTranslation[2] >> 12);

    vdp2DebugPrintSetPosition(3, 0x17);
    vdp2PrintfSmallFont("%4d ", (s32)vecDistance(r4->m8_pos, targetPos) >> 12);
}

void subfieldA3_1Sub0(s_dragonTaskWorkArea* r4)
{
    switch (r4->m108)
    {
    case 0:
        if (getFieldSpecificData_A3()->m130_conanaNestCutsceneTrigger != 1)
            return;
        initDragonMovementMode();
        r4->m108++;
    case 1:
        subfieldA3_1Sub0Sub1(r4);
        if (getFieldSpecificData_A3()->m130_conanaNestCutsceneTrigger != 2)
            return;
        subfieldA3_1Sub0Sub2(4, 0x8000);
        r4->m108++;
    case 2:
        return;
    default:
        assert(0);
        break;
    }
}

void setupFieldCameraConfig_A3_1()
{
    setupFieldCameraConfigs(readCameraConfig({ 0x6081F9C, gFLD_A3 }), 1);
}

void subfieldA3_1(p_workArea workArea)
{
    playPCM(workArea, 100);
    playPCM(workArea, 101);

    s_DataTable3* pDataTable3 = readDataTable3({ 0x608838C, gFLD_A3 });
    std::vector<std::vector<sCameraVisibility>>* pVisibility = readCameraVisbility({ 0x608F628, gFLD_A3 }, pDataTable3);
    s_DataTable2* pDataTable2 = readDataTable2({ 0x60866D0, gFLD_A3 });
    setupField(pDataTable3, pDataTable2, fieldA3_1_startTasks, pVisibility);

    fieldTaskPtr->m8_pSubFieldData->m338_pDragonTask->mF4 = subfieldA3_1Sub0;

    {
        sVec3_FP position = { 0x448000, 0x69000, -0x1932000 };
        sVec3_FP rotation = { 0x0, 0x0, 0x0 };
        setupDragonPosition(&position, &rotation);
    }

    if (fieldTaskPtr->m30_fieldEntryPoint != -1)
    {
        //6054472
        switch (fieldTaskPtr->m32_previousSubField)
        {
        case 8:
            fieldTaskPtr->m8_pSubFieldData->m338_pDragonTask->m1D0_cameraScript = readCameraScript({ 0x60911f8, gFLD_A3 }); // coming back from A3_0
            break;
        case 9:
            startCutscene(loadCutsceneData({ 0x6091be0, gFLD_A3 })); // coming back from nest
            break;
        default:
            // play above excavation intro already?
            if (mainGameState.getBit(0xA * 8 + 6))
            {
                startCutscene(loadCutsceneData({ 0x6091688, gFLD_A3 }));
            }
            else
            {
                startCutscene(loadCutsceneData({ 0x60915A4, gFLD_A3 }));
            }
            break;
        }
    }
    //6054526
    initFieldDragonLight();

    createFieldPaletteTask(workArea);

    setupFieldCameraConfig_A3_1();

    adjustVerticalLimits(-0x58000, 0x76000);

    fieldRadar_enableAltitudeGauge();

    getFieldTaskPtr()->m8_pSubFieldData->m344_randomBattleTask->m0 = nullBattle;

    startFieldScript(18, -1);

    fieldRadar_initDestinations(0x19);
}

