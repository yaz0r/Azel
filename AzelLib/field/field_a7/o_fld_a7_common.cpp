#include "PDS.h"
#include "o_fld_a7.h"
#include "field/fieldModelRender.h"
#include "field/field_a3/o_fld_a3.h"
#include "audio/soundDriver.h"
#include "kernel/fileBundle.h"
#include "field/fieldSceneManager.h"
#include "field/fieldCutsceneTask2.h"
#include "audio/systemSounds.h"
#include "trigo.h"
#include "a7_effectEntity60.h"
#include "a7_envEntity54Particles.h"
#include "kernel/rayDisplay.h"
#include "field/fieldDebrisScatter.h"
#include "field/fieldDragonInput.h"
#include "a7_spawnedEntityChild.h"
#include "a7_repairBit.h"
#include "a7_beamChargeWobble.h"
#include "kernel/vdp1AnimatedQuad.h"
#include <vector>

s32 playBattleSoundEffect(s32 effectIndex);

// 06054474
void createFieldSpecificDataTask_A7(p_workArea parent)
{
    s_fieldSpecificData_A7* p = createSubTaskFromFunction<s_fieldSpecificData_A7>(parent, nullptr);
    if (p)
    {
        getFieldTaskPtr()->mC = p;
    }
}

// 06059418
static void initDragonLightParams(fixedPoint lightRotation, fixedPoint lightParam2,
    s8 n0, s8 n1, s8 n2,
    s8 f0_0, s8 f0_1, s8 f0_2,
    s8 f1_0, s8 f1_1, s8 f1_2,
    s8 f2_0, s8 f2_1, s8 f2_2,
    s8 f3_0, s8 f3_1, s8 f3_2)
{
    s_dragonTaskWorkArea* p = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    p->mC0_lightRotationAroundDragon = lightRotation;
    p->mC4 = lightParam2;
    p->mC8_normalLightColor = { n0, n1, n2 };
    p->mCB_falloffColor0 = { f0_0, f0_1, f0_2 };
    p->mCE_falloffColor1 = { f1_0, f1_1, f1_2 };
    p->mD1_falloffColor2 = { f2_0, f2_1, f2_2 };
    p->mD4 = { f3_0, f3_1, f3_2 };
}

// 06059538 — dragon light params for subfield 0
void initDragonParams_A7_0()
{
    // Data at 06085aa4
    initDragonLightParams(fixedPoint(0x0871C71C), fixedPoint(0),
        0x03, 0x03, 0x03,
        0x0A, 0x0A, 0x0A,
        (s8)0xE0, (s8)0xFB, (s8)0xFB,
        0x0A, 0x0A, 0x0A,
        0x10, 0x10, 0x10);
}

// 0605953e — dragon light params for subfield 1
void initDragonParams_A7_1()
{
    // Data at 06085abc
    initDragonLightParams(fixedPoint(0x0871C71C), fixedPoint(0),
        0x00, 0x00, 0x00,
        0x0A, 0x10, 0x14,
        0x09, 0x0E, 0x07,
        0x17, 0x17, 0x1D,
        0x10, 0x10, 0x10);
}

// 06059544 — dragon light params for subfield 2
void initDragonParams_A7_2()
{
    // Data at 06085ad4
    initDragonLightParams(fixedPoint(0x0871C71C), fixedPoint(0),
        0x10, 0x1B, 0x1C,
        0x01, 0x05, 0x08,
        (s8)0xE0, (s8)0xF2, (s8)0xEE,
        (s8)0xE0, (s8)0xF4, (s8)0xEE,
        0x10, 0x10, 0x10);
}

// 06055266 — set/clear game state bit from table at 060843d0
static void setGameStateBitFromTable(s32 index, s32 setOrClear)
{
    s16 bitIndex = readSaturnS16(gFLD_A7->getSaturnPtr(0x060843d0) + index * 2);
    u32 adjustedIndex = (bitIndex < 1000) ? (u32)bitIndex : (u32)(bitIndex - 0x236);
    if (setOrClear == 0)
    {
        mainGameState.bitField[adjustedIndex >> 3] &= ~(1 << (adjustedIndex & 7));
    }
    else
    {
        mainGameState.bitField[adjustedIndex >> 3] |= (1 << (adjustedIndex & 7));
    }
}

// 0605600a — set game flags for subfield 0 objects
void setGameFlagsA7_0()
{
    for (s32 i = 0; i < 8; i++)
    {
        setGameStateBitFromTable(i, 1);
    }
    s_fieldSpecificData_A7* pFieldData = (s_fieldSpecificData_A7*)getFieldTaskPtr()->mC;
    pFieldData->m279 = 8;
}

// 0605c650 — read game state bit from table at 06086260
static s32 readGameStateBitFromTable(s32 index)
{
    u32 bitIndex = (u32)readSaturnS16(gFLD_A7->getSaturnPtr(0x06086260) + index * 2);
    u32 adjustedIndex = (bitIndex < 1000) ? bitIndex : (bitIndex - 0x236);
    return (s32)(s8)(mainGameState.bitField[adjustedIndex >> 3] & (1 << (adjustedIndex & 7)));
}

// 0605c75c — count active environment objects
void countActiveObjectsA7()
{
    s_fieldSpecificData_A7* pFieldData = (s_fieldSpecificData_A7*)getFieldTaskPtr()->mC;
    pFieldData->m276 = 0;
    pFieldData->m277_segmentsReturned = 0;
    for (s32 i = 0; i < 8; i += 4)
    {
        for (s32 j = 0; j < 4; j++)
        {
            if (readGameStateBitFromTable(i + j) != 0)
            {
                pFieldData->m276++;
                pFieldData->m277_segmentsReturned++;
            }
        }
    }
    pFieldData->m278 = 0;
}

// 06068a9e — A7 cutscene camera init (local copy of cutsceneTaskInitSub2)
static std::vector<s_scriptData1> a7CutsceneScriptData;

static void a7CutsceneCameraInit(p_workArea parent, const sSaturnPtr& scriptDataEA, s32 r6, sVec3_FP* r7, u32 flags)
{
    loadScriptData1(a7CutsceneScriptData, scriptDataEA);
    cutsceneTaskInitSub2(parent, a7CutsceneScriptData, r6, r7, flags);
}

// Simple subtask struct — 4 bytes extra (one s32 counter)
struct sA7SimpleSubtask : public s_workAreaTemplate<sA7SimpleSubtask>
{
    s32 m0_counter;
    // size 0x4
};

// 060544e8
static void a7SimpleSubtaskUpdate(sA7SimpleSubtask* pThis)
{
    if (pThis->m0_counter == 0)
    {
        if ((mainGameState.bitField[0x74] & 0x80) == 0)
        {
            a7CutsceneCameraInit(pThis, gFLD_A7->getSaturnPtr(0x060810b4), 0, nullptr, 1);
            startFieldScript(0xe, 0x5d6);
        }
        if (pThis != nullptr)
        {
            pThis->getTask()->markFinished();
        }
    }
    else
    {
        pThis->m0_counter--;
    }
}

// 06054536 — create simple subtask (conditional)
void createA7_simpleSubtask(p_workArea parent)
{
    if (getFieldTaskPtr()->m2C_currentFieldIndex != 0x16)
    {
        sA7SimpleSubtask* pTask = createSubTaskFromFunction<sA7SimpleSubtask>(parent, &a7SimpleSubtaskUpdate);
        if (pTask != nullptr)
        {
            pTask->m0_counter = 0x5E;
        }
    }
}

// Field script subtask struct — 0 bytes extra
struct sA7FieldScriptSubtask : public s_workAreaTemplate<sA7FieldScriptSubtask>
{
    // size 0x0
};

// 0605e8ba — field script subtask update (runs once then self-destructs)
static void fieldScriptSubtaskUpdate_A7(sA7FieldScriptSubtask* pThis)
{
    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m2C_currentFieldIndex == 4 && (mainGameState.bitField[0x74] & 0x80) == 0)
    {
        return;
    }
    s32 result = startFieldScript(0x20, -1);
    if (result != 0 && pThis != nullptr)
    {
        // self-destruct this task
        pThis->getTask()->markFinished();
    }
}

// 0605e930 — create field script environment subtask
void createA7_fieldScriptSubtask(p_workArea parent)
{
    if (getFieldTaskPtr()->m2C_currentFieldIndex == 4 && (mainGameState.bitField[0xa3] & 0x10) != 0)
    {
        return;
    }
    createSubTaskFromFunction<sA7FieldScriptSubtask>(parent, &fieldScriptSubtaskUpdate_A7);
}

// 060545ac — proximity check update (random battle trigger)
struct sA7ProximityTask : public s_workAreaTemplate<sA7ProximityTask>
{
    sVec3_FP m0_lastDragonPos;
    s32 mC_counter;
    // size 0x10
};

// 060545ac
static void proximityCheckUpdate(sA7ProximityTask* pThis)
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    s32 dist = vecDistance(pDragon->m8_pos, pThis->m0_lastDragonPos);

    pThis->m0_lastDragonPos.m0_X = pDragon->m8_pos.m0_X;
    pThis->m0_lastDragonPos.m4_Y = pDragon->m8_pos.m4_Y;
    pThis->m0_lastDragonPos.m8_Z = pDragon->m8_pos.m8_Z;

    s_fieldSpecificData_A7* pFieldData = (s_fieldSpecificData_A7*)getFieldTaskPtr()->mC;
    if (pDragon->m235_dragonSpeedIndex == 0 && dist > 0x2c71
        /* && additional checks for FUN_FLD_A7__0606b5d4, 060682c4, 060692e8 would go here */)
    {
        pThis->mC_counter++;
        if (pThis->mC_counter > 0x96 && pFieldData->m277_segmentsReturned == 8)
        {
            startFieldScript(0, 0x5c8);
        }
        if (pThis->mC_counter > 0x24 && pFieldData->m277_segmentsReturned < 8)
        {
            startFieldScript(1, 0x5c9);
            return;
        }
    }
    else
    {
        pThis->mC_counter = 0;
    }
}

// 06054684 — create proximity check subtask
void createA7_proximityCheck(p_workArea parent)
{
    sA7ProximityTask* pTask = createSubTaskFromFunction<sA7ProximityTask>(parent, &proximityCheckUpdate);
    if (pTask != nullptr)
    {
        pTask->mC_counter = 0;
    }
}

// --- Model render context (0606b45c / 0606b4b0) --- moved to field/fieldModelRender.cpp

static sFieldSceneManager* createA7_3dSceneManager(p_workArea parent, s32 areaIndex, s32 count)
{
    return createFieldSceneManager(parent, areaIndex, count);
}

// 06056bb0 — start tasks for subfield 0
void startTasksA7_0(p_workArea parent)
{
    createFieldSpecificDataTask_A7(parent);
    s_fieldSpecificData_A7* pFieldData = (s_fieldSpecificData_A7*)getFieldTaskPtr()->mC;
    pFieldData->m280 = createA7_3dSceneManager(parent, 1, 0x40);
    createA7_simpleSubtask(parent);
    setGameFlagsA7_0();
    // 060565a6 — empty
    // 0606a65c — empty
    createA7_itemBoxes_0(parent);
    countActiveObjectsA7();
    createA7_fieldScriptSubtask(parent);
    createA7_proximityCheck(parent);
}

// --- Environment object subtask structs ---

// Empty update subtask (4 bytes, unused counter)
struct sA7EmptySubtask : public s_workAreaTemplate<sA7EmptySubtask>
{
    s32 m0;
    // size 0x4
};

// 060565a2
static void a7EmptySubtaskUpdate(sA7EmptySubtask* pThis)
{
    // empty
}

// 060565b2 — create empty update subtask
void createA7_emptySubtask(p_workArea parent)
{
    createSubTaskFromFunction<sA7EmptySubtask>(parent, &a7EmptySubtaskUpdate);
}

// Encounter check subtask (4 bytes)
struct sA7EncounterCheckSubtask : public s_workAreaTemplate<sA7EncounterCheckSubtask>
{
    s32 m0;
    // size 0x4
};

// Encounter notification task (size 0x10)
struct sA7EncounterNotification : public s_workAreaTemplate<sA7EncounterNotification>
{
    s32 m0_param0;
    s32 m4_param1;
    s32 m8;
    u8 mC;
    // size 0x10
};

// 06056498
static void a7EncounterNotificationUpdate(sA7EncounterNotification* pThis)
{
    u8 state = pThis->mC;
    if (state == 0)
    {
        a7CutsceneCameraInit(pThis, gFLD_A7->getSaturnPtr(0x06081528), 0, nullptr, 1);
        pThis->mC = 1;
        return;
    }

    if (state == 1)
    {
        s32 prev = pThis->m8;
        pThis->m8 = prev + 1;
        if (pThis->m0_param0 < prev)
        {
            u32 fromColor = convertColorToU32ForFade(*(sVec3_FP*)&g_fadeControls);
            fadePalette(&g_fadeControls.m0_fade0, fromColor, 0xFFFF, pThis->m4_param1);
            pThis->mC = 2;
        }
    }
    else if (state != 2)
    {
        return;
    }

    // States 1 and 2: check for cutscene completion then start battle
    if (isNoCutsceneActive())
    {
        graphicEngineStatus.m40AC.m1_isMenuAllowed = 1;
        enableScriptSkippingAndExit(6, 0, 0, -1);
        pThis->getTask()->markFinished();
    }
}

// 06056522 — create encounter notification sibling task
static void a7CreateEncounterNotification(sA7EncounterCheckSubtask* pThis, s32* pParams)
{
    sA7EncounterNotification* pNotif = createSiblingTaskFromFunction<sA7EncounterNotification>(pThis, &a7EncounterNotificationUpdate);
    if (pNotif != nullptr)
    {
        pNotif->m0_param0 = pParams[0];
        pNotif->m4_param1 = pParams[1];
        pNotif->m8 = 0;
        pNotif->mC = 0;
    }
}

// 06056556
static void a7EncounterCheckUpdate(sA7EncounterCheckSubtask* pThis)
{
    if ((mainGameState.bitField[0x97] & 0x80) != 0
        && isNoCutsceneActive()
        && isScriptActive() == 0)
    {
        s32 params[] = { 0x6C, 0x18 };
        a7CreateEncounterNotification(pThis, params);
        if (pThis != nullptr)
        {
            pThis->getTask()->markFinished();
        }
    }
}

// 060565aa — create encounter check subtask
void createA7_encounterCheckSubtask(p_workArea parent)
{
    createSubTaskFromFunction<sA7EncounterCheckSubtask>(parent, &a7EncounterCheckUpdate);
}

// Sound cleanup tasks — size 0, only have delete handlers

struct sA7SoundCleanupTask : public s_workAreaTemplate<sA7SoundCleanupTask>
{
    // size 0x0
};

// 0605eb90 — delete handler: play sound 0x65
static void a7SoundCleanup_delete_0x65(sA7SoundCleanupTask* pThis)
{
    playBattleSoundEffect(0x65);
}

// 0605eb9e — delete handler: play sound 0x6f
static void a7SoundCleanup_delete_0x6f(sA7SoundCleanupTask* pThis)
{
    playBattleSoundEffect(0x6f);
}

// 0605eb96 — create sound cleanup subtask (sound 0x65)
void createA7_soundCleanup_0x65(p_workArea parent)
{
    static sA7SoundCleanupTask::TypedTaskDefinition td = { nullptr, nullptr, nullptr, &a7SoundCleanup_delete_0x65 };
    createSubTask<sA7SoundCleanupTask>(parent, &td);
}

// 0605eba4 — create sound cleanup subtask (sound 0x6f)
void createA7_soundCleanup_0x6f(p_workArea parent)
{
    static sA7SoundCleanupTask::TypedTaskDefinition td = { nullptr, nullptr, nullptr, &a7SoundCleanup_delete_0x6f };
    createSubTask<sA7SoundCleanupTask>(parent, &td);
}

// Environment entity 0x50 (waterfall)
struct sA7EnvEntity50 : public s_workAreaTemplate<sA7EnvEntity50>
{
    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_position;
    s32 m14_oscillateY;
    s32 m18_oscillateAngle;
    sFieldModelRenderContext m1C_modelCtx;
    // size 0x50
};

// 0605A410
static void a7EnvEntity50_Init(sA7EnvEntity50* pThis)
{
    getMemoryArea(&pThis->m0_memoryArea, 3);
    pThis->m8_position.m0_X = fixedPoint(0x59C000);
    pThis->m8_position.m4_Y = fixedPoint(0x50000);
    pThis->m8_position.m8_Z = fixedPoint(-0x600000);
    initFieldModelRenderContext(&pThis->m1C_modelCtx, pThis, (void*)0x0605A3A4,
        &pThis->m8_position, nullptr, 0, 0, -1, 0, 0);
}

// 0605A3C6
static void a7EnvEntity50_Update(sA7EnvEntity50* pThis)
{
    if ((mainGameState.bitField[0x97] & 0x80) != 0)
    {
        pThis->m8_position.m0_X = fixedPoint(pThis->m8_position.m0_X.asS32() + 0x400);
    }

    u32 angleIdx = ((u32)pThis->m18_oscillateAngle >> 16) & 0xFFF;
    pThis->m14_oscillateY = MTH_Mul(fixedPoint(0x9F49F4), getSin(angleIdx)).asS32() - 0x9F49F4;
    pThis->m18_oscillateAngle += 0x16C16C;

    updateFieldModelRenderContext(&pThis->m1C_modelCtx);
}

// 0605A378
static void a7EnvEntity50_Draw(sA7EnvEntity50* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m8_position);
    rotateCurrentMatrixShiftedZ(fixedPoint(pThis->m14_oscillateY));
    addObjectToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(0x1E4));
    popMatrix();
}

// 0605a456
void createA7_envEntity_a456(p_workArea parent)
{
    static sA7EnvEntity50::TypedTaskDefinition td = { &a7EnvEntity50_Init, &a7EnvEntity50_Update, &a7EnvEntity50_Draw, nullptr };
    createSubTask<sA7EnvEntity50>(parent, &td);
}

// 06059e6e
void createA7_envEntity_9e6e(p_workArea parent, sSaturnPtr arg)
{
    static sRepairBitFormation::TypedTaskDefinition td = { &repairBitFormation_Init, &repairBitFormation_Update, nullptr, nullptr };
    createSubTaskWithArg<sRepairBitFormation>(parent, arg, &td);
}

// Environment entity 0x20 (tower/pillar scenery)
struct sA7EnvEntity20 : public s_workAreaTemplate<sA7EnvEntity20>
{
    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_position;
    s32 m14_rotationY;
    s16 m18_encounterDelay;
    s16 m1A_encounterDelay2;
    u16 m1C_animCounter;
    u8 m1E_fogLevel;
    // size 0x20
};

// 0605A104
static void a7EnvEntity20_Init(sA7EnvEntity20* pThis)
{
    loadFileFromFileList(3);
    getMemoryArea(&pThis->m0_memoryArea, 3);
    pThis->m8_position.m0_X = fixedPoint(0x600000);
    pThis->m8_position.m4_Y = 0;
    pThis->m8_position.m8_Z = fixedPoint(-0x600000);
}

// 0605A12A — encounter approach state: fade fog to zero while rotating
static void a7EnvEntity20_EncounterUpdate(sA7EnvEntity20* pThis)
{
    pThis->m14_rotationY -= 0x888888;

    u16 nextAnim = pThis->m1C_animCounter + 1;
    pThis->m1C_animCounter = (nextAnim < 0x16) ? nextAnim : 0;

    if (pThis->m18_encounterDelay < 1 && pThis->m1E_fogLevel > 0)
    {
        sVdp2PlaneTask* pVdp2Task = getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask;
        pThis->m18_encounterDelay = pThis->m1A_encounterDelay2;
        pThis->m1E_fogLevel--;
        pVdp2Task->m74_colorNBG = pThis->m1E_fogLevel;
        vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA =
            (vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA & 0xFFE0) | (s16)pVdp2Task->m74_colorNBG;
        vdp2Controls.m_isDirty = 1;
    }
    else
    {
        pThis->m18_encounterDelay--;
    }
}

// 0605A1A6
static void a7EnvEntity20_Update(sA7EnvEntity20* pThis)
{
    pThis->m14_rotationY -= 0x888888;

    u16 nextAnim = pThis->m1C_animCounter + 1;
    pThis->m1C_animCounter = (nextAnim < 0x16) ? nextAnim : 0;

    if ((pThis->m1C_animCounter & 7) == 0)
    {
        Unimplemented(); // FUN_FLD_A7__0605a002 — periodic visual update
    }

    // Compute distance to tower for fog level
    sVdp2PlaneTask* pVdp2Task = getFieldTaskPtr()->m8_pSubFieldData->m350_fieldPaletteTask;
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

    sVec2_FP delta;
    delta[0] = fixedPoint(pDragon->m8_pos.m0_X.asS32() - 0x600000);
    delta[1] = fixedPoint(pDragon->m8_pos.m8_Z.asS32() + 0x600000);
    fixedPoint distSq = MulVec2(delta, delta);
    s32 dist = sqrt_F(distSq).asS32();

    // Fog table lookup — distance >> 0x13 gives index (0-17)
    static const u8 fogTable[] = {
        0x1B, 0x1A, 0x19, 0x18, 0x18, 0x17, 0x17, 0x17,
        0x16, 0x16, 0x16, 0x15, 0x15, 0x14, 0x13, 0x12,
        0x11, 0x10,
    };
    s16 fogIdx = (s16)(dist >> 0x13);
    if (fogIdx < 0) fogIdx = 0;
    if (fogIdx > 17) fogIdx = 17;
    pVdp2Task->m74_colorNBG = fogTable[fogIdx];
    vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA =
        (vdp2Controls.m4_pendingVdp2Regs->m108_CCRNA & 0xFFE0) | (s16)pVdp2Task->m74_colorNBG;
    vdp2Controls.m_isDirty = 1;

    // Check encounter conditions
    if ((mainGameState.bitField[0x97] & 0x80) != 0
        && isNoCutsceneActive()
        && isScriptActive() == 0)
    {
        // Transition to encounter approach state
        pThis->m1E_fogLevel = pVdp2Task->m74_colorNBG;
        pThis->m18_encounterDelay = FP_Div((s32)pThis->m1E_fogLevel, fixedPoint(0x8C)).asS32();
        pThis->m1A_encounterDelay2 = pThis->m18_encounterDelay;
        pThis->m_UpdateMethod = &a7EnvEntity20_EncounterUpdate;
    }
}

// 0605A2A8
static void a7EnvEntity20_Draw(sA7EnvEntity20* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m8_position);
    rotateCurrentMatrixShiftedY(fixedPoint(pThis->m14_rotationY));
    scaleCurrentMatrixRow0(0x40000);
    scaleCurrentMatrixRow1(0x40000);
    scaleCurrentMatrixRow2(0x40000);

    // Read model/pose indices from table based on anim counter
    const u8* tableBase = getSaturnPtr(gFLD_A7->getSaturnPtr(0x06086122));
    s16 animIdx = (s16)pThis->m1C_animCounter;
    s16 modelIdx = READ_BE_S16(tableBase + animIdx * 4);
    s16 poseIdx = READ_BE_S16(tableBase + animIdx * 4 + 2);

    LCSItemBox_DrawType0Sub0(pThis->m0_memoryArea.m0_mainMemoryBundle, modelIdx, poseIdx);
    popMatrix();
}

// 0605a2fe
void createA7_envEntity_a2fe(p_workArea parent)
{
    static sA7EnvEntity20::TypedTaskDefinition td = { &a7EnvEntity20_Init, &a7EnvEntity20_Update, &a7EnvEntity20_Draw, nullptr };
    createSubTask<sA7EnvEntity20>(parent, &td);
}

// Entity spawner task struct (Saturn size 0xC: ptr4 + u16 + u16 + u8)
struct sA7EntitySpawnerTask : public s_workAreaTemplate<sA7EntitySpawnerTask>
{
    sSaturnPtr m0_dataTable;  // Saturn offset 0 (4 bytes on Saturn, 8 in C++)
    u16 m4_index;             // Saturn offset 4
    u16 m6_delay;             // Saturn offset 6
    u8 m8_loopFlag;           // Saturn offset 8
    // Saturn size 0xC
};

// Spawned entity (size 0x208). Offsets 0x08..0x1E7 form a 40-slot sVec3_FP
// trail buffer; m8_position aliases slot[0] and the update regenerates the
// entire trail from the angle/radius accumulators each frame.
struct sA7SpawnedEntity : public s_workAreaTemplateWithArg<sA7SpawnedEntity, sSaturnPtr>
{
    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_position;        // trail slot[0]
    u8 m14_trail[0x1D4];         // trail slots[1..39] (39 x sVec3_FP)
    s32 m1E8_angleToTower;       // angle accumulator (init: atan2 toward tower)
    p_workArea m1EC_pChildTask;  // spawned sibling reference task, nullable
    s32 m1F0_distToTower;        // radius (init: sqrt distance to tower)
    s32 m1F4_param4;             // per-frame angle bump
    s32 m1F8_param3;             // per-frame Y gravity
    s32 m1FC_velocityConst0;     // per-slot angle step (-0x7107C)
    s32 m200_velocityConst1;     // per-slot Y step (-0x3555)
    u16 m204_animIndex;
    u8 m206_pad[2];
    // Saturn size 0x208
};

s32 a7CenteredRandom(u32 mask)
{
    return centeredRandom(mask);
}

// 06054C34
static void a7SpawnedEntity_Init(sA7SpawnedEntity* pThis, sSaturnPtr arg)
{
    getMemoryArea(&pThis->m0_memoryArea, 3);

    // Read position from Saturn data (3 s32 = sVec3_FP)
    pThis->m8_position.m0_X = fixedPoint(readSaturnS32(arg));
    pThis->m8_position.m4_Y = fixedPoint(readSaturnS32(arg + 4));
    pThis->m8_position.m8_Z = fixedPoint(readSaturnS32(arg + 8));

    // Compute distance and angle to tower at (0x600000, -0x600000)
    sVec2_FP delta;
    delta[0] = fixedPoint(pThis->m8_position.m0_X.asS32() - 0x600000);
    delta[1] = fixedPoint(pThis->m8_position.m8_Z.asS32() + 0x600000);
    pThis->m1F0_distToTower = sqrt_F(MulVec2(delta, delta)).asS32();
    pThis->m1E8_angleToTower = atan2_FP(delta[1].asS32(), delta[0].asS32());

    pThis->m1F4_param4 = readSaturnS32(arg + 16);
    pThis->m1F8_param3 = readSaturnS32(arg + 12);

    // Animation index from lookup table
    u8 typeIdx = readSaturnU8(arg + 20);
    pThis->m204_animIndex = READ_BE_U16(getSaturnPtr(gFLD_A7->getSaturnPtr(0x060842C8)) + typeIdx * 2);

    pThis->m1FC_velocityConst0 = (s32)0xFF8E38E4;
    pThis->m200_velocityConst1 = (s32)0xFFFFCAAB;

    // Compute direction vector toward tower
    s32 dirAngle = atan2_FP(-0x600000 - pThis->m8_position.m8_Z.asS32(),
                             0x600000 - pThis->m8_position.m0_X.asS32());
    u16 dirIdx = ((u16)((u32)(dirAngle + 0xC000000) >> 16)) & 0xFFF;
    sVec3_FP spawnDir;
    spawnDir.m0_X = MTH_Mul(fixedPoint(0x4000), getCos(dirIdx));
    spawnDir.m4_Y = 0;
    spawnDir.m8_Z = MTH_Mul(fixedPoint(0x4000), getSin(dirIdx));

    // Spawn 8 particles via scene manager (4 outer iterations, 2 inlined spawns each)
    s_fieldSpecificData_A7* pFieldData = (s_fieldSpecificData_A7*)getFieldTaskPtr()->mC;
    static std::vector<sVdp1Quad> s_spawnedQuadList;
    if (s_spawnedQuadList.empty())
    {
        s_spawnedQuadList = initVdp1Quad(gFLD_A7->getSaturnPtr(0x060804e4));
    }

    sSceneParticleDesc desc = {};
    desc.m8_pQuadList = &s_spawnedQuadList;

    sVec3_FP spawnPos;
    spawnPos.m4_Y = fixedPoint(0);

    for (s32 i = 8; i != 0; i -= 2)
    {
        spawnPos.m0_X = fixedPoint(a7CenteredRandom(0x3FFFF) + pThis->m8_position.m0_X.m_value);
        spawnPos.m8_Z = fixedPoint(a7CenteredRandom(0x3FFFF) + pThis->m8_position.m8_Z.m_value);
        sceneParticle_spawnProjected((sFieldSceneManager*)pFieldData->m280, &desc, &spawnPos, &spawnDir);

        spawnPos.m0_X = fixedPoint(a7CenteredRandom(0x3FFFF) + pThis->m8_position.m0_X.m_value);
        spawnPos.m8_Z = fixedPoint(a7CenteredRandom(0x3FFFF) + pThis->m8_position.m8_Z.m_value);
        sceneParticle_spawnProjected((sFieldSceneManager*)pFieldData->m280, &desc, &spawnPos, &spawnDir);
    }

    // Play sound if visible on screen
    sVec3_FP viewPos;
    transformAndAddVecByCurrentMatrix(&pThis->m8_position, &viewPos);
    if (isPointOnScreen(&viewPos, graphicEngineStatus.m405C.m14_farClipDistance))
    {
        playSystemSoundEffect(0x6B);
    }
}

// 06054E18
static void a7SpawnedEntity_Update(sA7SpawnedEntity* pThis)
{
    // Regenerate the full 40-slot trail each frame from the angle+radius accumulators,
    // pivoting around (+0x600000, y, -0x600000) in world space.
    sVec3_FP* slots = &pThis->m8_position;
    s32 angle = pThis->m1E8_angleToTower;
    s32 y = pThis->m8_position.m4_Y.m_value;
    fixedPoint radius(pThis->m1F0_distToTower);

    for (s32 i = 0; i < 20; i++)
    {
        u16 idxA = (u16)((u32)angle >> 16) & 0xFFF;
        slots[i * 2 + 0].m0_X = fixedPoint(MTH_Mul(radius, getCos(idxA)).m_value + 0x600000);
        slots[i * 2 + 0].m4_Y = fixedPoint(y);
        slots[i * 2 + 0].m8_Z = fixedPoint(MTH_Mul(radius, getSin(idxA)).m_value - 0x600000);

        s32 dAngle = pThis->m1FC_velocityConst0;
        s32 dY     = pThis->m200_velocityConst1;
        u16 idxB = (u16)((u32)(angle + dAngle) >> 16) & 0xFFF;
        slots[i * 2 + 1].m0_X = fixedPoint(MTH_Mul(radius, getCos(idxB)).m_value + 0x600000);
        slots[i * 2 + 1].m4_Y = fixedPoint(y + dY);
        slots[i * 2 + 1].m8_Z = fixedPoint(MTH_Mul(radius, getSin(idxB)).m_value - 0x600000);

        angle += dAngle + pThis->m1FC_velocityConst0;
        y     += dY     + pThis->m200_velocityConst1;
    }

    pThis->m1E8_angleToTower += pThis->m1F4_param4;
    s32 newY = pThis->m8_position.m4_Y.m_value + pThis->m1F8_param3;
    if (newY >= 0x190001)
    {
        pThis->getTask()->markFinished();
        return;
    }
    pThis->m8_position.m4_Y = fixedPoint(newY);

    if ((mainGameState.bitField[0x97] & 0x80) != 0)
    {
        s32 r = pThis->m1F0_distToTower - 0x4000;
        pThis->m1F0_distToTower = (r < 0) ? 0 : r;
        return;
    }

    if (isScriptActive() != 0 || !isNoCutsceneActive())
    {
        return;
    }

    // Dragon proximity check: the threshold is a bundle-relative s32 looked up
    // via the s16 offset stored at m204_animIndex (+ 0x4000 padding).
    sSaturnPtr tablePtr = (sSaturnPtr&)pThis->m0_memoryArea.m0_mainMemoryBundle;
    (void)tablePtr;
    s32 threshold = 0x4000; // TODO: proper file-relative lookup, see FUN_06054E18

    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

    if (pThis->m1EC_pChildTask == nullptr)
    {
        s32 dx = pThis->m8_position.m0_X.m_value - pDragon->m8_pos.m0_X.m_value;
        if (dx < 0) dx = -dx;
        if (dx > threshold) return;

        s32 dy = pThis->m8_position.m4_Y.m_value - pDragon->m8_pos.m4_Y.m_value;
        if (dy < 0) dy = -dy;
        if (dy > threshold) return;

        s32 dz = pThis->m8_position.m8_Z.m_value - pDragon->m8_pos.m8_Z.m_value;
        if (dz < 0) dz = -dz;
        if (dz > threshold) return;

        pThis->m1EC_pChildTask = (p_workArea)createSiblingTask<sA7SpawnedEntityChild>((p_workArea)pThis);
        playSystemSoundEffect(0x6A);
    }
    else
    {
        if (pThis->m1EC_pChildTask->getTask()->isFinished())
        {
            pThis->m1EC_pChildTask = nullptr;
        }
    }
}

// 06055068
static void a7SpawnedEntity_Draw(sA7SpawnedEntity* pThis)
{
    if (checkPositionVisibilityAgainstFarPlane(&pThis->m8_position) == 0)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pThis->m8_position);
        s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
        s16 modelOffset = (s16)pThis->m204_animIndex;
        addObjectToDrawList(pBundle->get3DModel((u32)modelOffset));
        popMatrix();
    }

    // Saturn constants (pool @ FLD_A7::060551a2):
    //   width      = 0x2800
    //   cmdcolr    = 0x2050
    //   cmdsize    = 0x0210
    //   colorMode  = 0
    //   cmdsrca body base   = 0x1168 (added to VDP1 char area offset)
    //   cmdsrca final base  = 0x1178
    const s32 width     = 0x2800;
    const u16 cmdcolr   = 0x2050;
    const u16 cmdsize   = 0x0210;
    const u16 colorMode = 0;
    const u16 vdp1Base  = (u16)((pThis->m0_memoryArea.m4_characterArea - 0x25C00000) >> 3);
    const u16 cmdsrcaBody  = (u16)(vdp1Base + 0x1168);
    const u16 cmdsrcaFinal = (u16)(vdp1Base + 0x1178);

    sVec3_FP* trail = &pThis->m8_position;
    for (s32 i = 0; i < 0x26; i++)
    {
        // Sentinel on trail[i+1].Y < 0 — early-terminates the trail
        if (trail[i + 1].m4_Y.m_value < 0)
        {
            return;
        }
        displayRaySegmentFromWorldSpace(&trail[i], width, cmdsrcaBody, cmdsize, cmdcolr, colorMode);
    }
    displayRaySegmentFromWorldSpace(&trail[38], width, cmdsrcaFinal, cmdsize, cmdcolr, colorMode);
}

// 0605513c — spawner active update: creates entities from table
static void a7EntitySpawnerActiveUpdate(sA7EntitySpawnerTask* pThis)
{
    if (pThis->m6_delay == 0)
    {
        // Create entity from current table entry
        sSaturnPtr entryEA = pThis->m0_dataTable + (u32)pThis->m4_index * 0x1C;

        static sA7SpawnedEntity::TypedTaskDefinition td = { &a7SpawnedEntity_Init, &a7SpawnedEntity_Update, &a7SpawnedEntity_Draw, nullptr };
        createSubTaskWithArg<sA7SpawnedEntity>((p_workArea)pThis, entryEA, &td);

        // Read delay for next spawn from table entry offset 0x18
        pThis->m6_delay = readSaturnU16(entryEA + 0x18);

        // Advance index
        if (pThis->m4_index < 7)
        {
            pThis->m4_index++;
        }
        else
        {
            pThis->m4_index = 0;
            if (pThis->m8_loopFlag != 0)
            {
                // Stop spawning: clear update function
                pThis->m_UpdateMethod = nullptr;
            }
        }
    }
    else
    {
        pThis->m6_delay--;
    }
}

// 060551d8 — spawner initial update: waits for game flag then activates
static void a7EntitySpawnerWaitUpdate(sA7EntitySpawnerTask* pThis)
{
    if ((mainGameState.bitField[0xA3] & 2) != 0)
    {
        pThis->m_UpdateMethod = &a7EntitySpawnerActiveUpdate;
    }
}

// 060551ee — create entity spawner subtask
void createA7_envEntity_51ee(p_workArea parent, sSaturnPtr arg, u8 param3)
{
    sA7EntitySpawnerTask* pTask = createSubTaskFromFunction<sA7EntitySpawnerTask>(parent, &a7EntitySpawnerWaitUpdate);
    if (pTask != nullptr)
    {
        pTask->m0_dataTable = arg;
        pTask->m4_index = 0;
        pTask->m6_delay = 0;
        pTask->m8_loopFlag = param3;
    }
}

// 0607c18e, 0607c120 — initDebrisScatterConfig, readMaxScalarFromBundleTree

// Forward declarations
struct sA7EnvEntity60;
static void a7EnvEntity60_debrisDrawCallback(p_workArea pDebrisTask, sVec3_FP* pPosition);
static void a7EnvEntity60_SpawnExplosion(sA7EnvEntity60* pThis);

// Environment entity 0x60 (interactive scenery — fossil/crystal)
struct sA7EnvEntity60 : public s_workAreaTemplate<sA7EnvEntity60>
{
    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_position;
    sVec3_FP m14_rotation;
    s32 m20_scale;
    struct sA7EffectEntity60* m24_pEffect;
    sFieldModelRenderContext m28_modelCtx;
    u8 m5C_state;
    // size 0x60
};

// 0605e594 — spawn explosion debris scatter when the crystal breaks
static void a7EnvEntity60_SpawnExplosion(sA7EnvEntity60* pThis)
{
    sDebrisScatterParams params;
    initDebrisScatterConfig(&params, 4, 0x248);

    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    u8* pRaw = pBundle->getRawBuffer();
    u32 treeRootOffset = READ_BE_U32(pRaw + 4);
    params.m8_spread = fixedPoint(readMaxScalarFromBundleTree(pRaw, treeRootOffset));

    params.m10_pPosition = &pThis->m8_position;
    params.m14_pRotation = &pThis->m14_rotation;
    params.m0_gravity = fixedPoint(0x14a);
    params.m4_bounce = fixedPoint((s32)0xffffffff);
    params.mC_randomMask = fixedPoint(0xffffff);
    params.m8_spread = MTH_Mul(params.m8_spread, fixedPoint(0x40000));
    params.m3C_scale = fixedPoint(pThis->m20_scale);
    params.m_pBundle = pBundle;

    createDebrisScatterTask((p_workArea)pThis, &params, false);
}

// 0605e0ec — debris draw callback: spawn a projected scene particle at randomized position
static void a7EnvEntity60_debrisDrawCallback(p_workArea /*pDebrisTask*/, sVec3_FP* pPosition)
{
    sVec3_FP vel = {};
    sVec3_FP pos;
    pos.m0_X = fixedPoint(pPosition->m0_X.m_value + centeredRandom(0x1FFFF));
    pos.m4_Y = 0;
    pos.m8_Z = fixedPoint(pPosition->m8_Z.m_value + centeredRandom(0x1FFFF));

    s_fieldSpecificData_A7* pFieldData = (s_fieldSpecificData_A7*)getFieldTaskPtr()->mC;
    sFieldSceneManager* pManager = (sFieldSceneManager*)pFieldData->m280;

    sSceneParticleDesc desc = {};
    desc.m8_pQuadList = a7GetOrParseQuadList(gFLD_A7->getSaturnPtr(0x060804E4));

    sceneParticle_spawnProjected(pManager, &desc, &pos, &vel);
}

// 0605E768
static void a7EnvEntity60_Init(sA7EnvEntity60* pThis)
{
    getMemoryArea(&pThis->m0_memoryArea, 3);
    pThis->m8_position.m0_X = fixedPoint(0xA00000);
    pThis->m8_position.m4_Y = 0;
    pThis->m8_position.m8_Z = fixedPoint(-0xA00000);
    pThis->m14_rotation.m0_X = fixedPoint(-0x3800000);
    pThis->m14_rotation.m4_Y = 0;
    pThis->m14_rotation.m8_Z = fixedPoint(0x900000);
    initFieldModelRenderContext(&pThis->m28_modelCtx, pThis, (void*)0x0605E70A,
        &pThis->m8_position, nullptr, 2, -1, -1, 0, 6);
    pThis->m20_scale = 0x10000;
    pThis->m5C_state = 0;
}

// 0605E682
static void a7EnvEntity60_Update(sA7EnvEntity60* pThis)
{
    if (pThis->m5C_state == 0)
    {
        u8 flags = pThis->m28_modelCtx.m19_drawFlags;
        if ((flags & 1) != 0 && (flags & 4) != 0)
        {
            static sA7EffectEntity60::TypedTaskDefinition td = { &a7EffectEntity60_Init, &a7EffectEntity60_Update, &a7EffectEntity60_Draw, nullptr };
            pThis->m24_pEffect = createSiblingTask<sA7EffectEntity60>((p_workArea)pThis, &td);
            pThis->m5C_state = 1;
        }
    }
    else if (pThis->m5C_state == 1)
    {
        sA7EffectEntity60* pEffect = pThis->m24_pEffect;
        if (pEffect->m3C_progress > 0x38E38E3)
        {
            pThis->m_DrawMethod = nullptr;
            pThis->m28_modelCtx.m18_visibilityFlags |= 1;
            playSystemSoundEffect(0x6D);
            a7EnvEntity60_SpawnExplosion(pThis);
            pThis->m5C_state = 2;
        }
    }
    else if (pThis->m5C_state == 2)
    {
        pThis->getTask()->markFinished();
    }

    updateFieldModelRenderContext(&pThis->m28_modelCtx);
}

// 0605E644
static void a7EnvEntity60_Draw(sA7EnvEntity60* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m8_position);
    rotateCurrentMatrixZYX(&pThis->m14_rotation);
    scaleCurrentMatrixRow0(pThis->m20_scale);
    scaleCurrentMatrixRow1(pThis->m20_scale);
    scaleCurrentMatrixRow2(pThis->m20_scale);
    LCSItemBox_DrawType0Sub0(pThis->m0_memoryArea.m0_mainMemoryBundle, 4, 0x248);
    popMatrix();
}

// 0605e7c2
void createA7_envEntity_e7c2(p_workArea parent)
{
    static sA7EnvEntity60::TypedTaskDefinition td = { &a7EnvEntity60_Init, &a7EnvEntity60_Update, &a7EnvEntity60_Draw, nullptr };
    createSubTask<sA7EnvEntity60>(parent, &td);
}

// Environment entity 0x54 (sibling — growing crystal/plant)
struct sA7EnvEntity54 : public s_workAreaTemplate<sA7EnvEntity54>
{
    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_position;
    s32 m14_scale;
    s32 m18_rotationY;
    s32 m1C_animCounter;
    sFieldModelRenderContext m20_modelCtx;
    // size 0x54 (with m38 flag byte inside model data)
};

// 0605DBDA
static void a7EnvEntity54_Init(sA7EnvEntity54* pThis)
{
    getMemoryArea(&pThis->m0_memoryArea, 4);
    pThis->m8_position.m0_X = 0;
    pThis->m8_position.m4_Y = 0;
    pThis->m8_position.m8_Z = 0;
    pThis->m14_scale = 0;

    sA7EnvEntity54ParticlesArg particlesArg;
    particlesArg.m0_pPosition = &pThis->m8_position;
    particlesArg.m4_count = 0x80;
    particlesArg.m6_val = 0xC;
    a7EnvEntity54Particles_create((p_workArea)pThis, &particlesArg);

    initFieldModelRenderContext(&pThis->m20_modelCtx, pThis, (void*)0x0605DBB8,
        &pThis->m8_position, nullptr, 3, -1, -1, 0, 0);
    mainGameState.bitField[0xA3] |= 8;
}

// 0605DAD8
static void a7EnvEntity54_Update(sA7EnvEntity54* pThis)
{
    // Distance check — hide if dragon is too close
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    s32 dx = pDragon->m8_pos.m0_X.asS32() - pThis->m8_position.m0_X.asS32();
    if (dx < 0) dx = -dx;
    s32 dz = pDragon->m8_pos.m8_Z.asS32() - pThis->m8_position.m8_Z.asS32();
    if (dz < 0) dz = -dz;

    if (dx < 0xFC000 && dz < 0xFC000)
    {
        pThis->m20_modelCtx.m18_visibilityFlags = 0;
    }
    else
    {
        pThis->m20_modelCtx.m18_visibilityFlags |= 1;
    }

    updateFieldModelRenderContext(&pThis->m20_modelCtx);

    // Grow scale
    if (pThis->m14_scale < 0x40000)
    {
        pThis->m14_scale += 0x200;
    }

    // Rotate and animate
    pThis->m18_rotationY -= 0x888888;
    pThis->m1C_animCounter = (pThis->m1C_animCounter < 0x16) ? pThis->m1C_animCounter + 1 : 0;
}

// 0605DA64
static void a7EnvEntity54_Draw(sA7EnvEntity54* pThis)
{
    if (pThis->m14_scale <= 0x1999)
        return;

    s32 scaleY = (pThis->m14_scale < 0x10000) ? pThis->m14_scale << 2 : 0x40000;

    pushCurrentMatrix();
    translateCurrentMatrix(&pThis->m8_position);
    rotateCurrentMatrixShiftedY(fixedPoint(pThis->m18_rotationY));
    scaleCurrentMatrixRow0(pThis->m14_scale);
    scaleCurrentMatrixRow1(scaleY);
    scaleCurrentMatrixRow2(pThis->m14_scale);

    const u8* modelTable = getSaturnPtr(gFLD_A7->getSaturnPtr(0x06086440));
    const u8* poseTable = getSaturnPtr(gFLD_A7->getSaturnPtr(0x0608646E));
    s16 modelIdx = READ_BE_S16(modelTable + pThis->m1C_animCounter * 2);
    s16 poseIdx = READ_BE_S16(poseTable + pThis->m1C_animCounter * 2);
    LCSItemBox_DrawType0Sub0(pThis->m0_memoryArea.m0_mainMemoryBundle, modelIdx, poseIdx);
    popMatrix();
}

// 0605e034
void createA7_envEntity_e034(p_workArea parent)
{
    static sA7EnvEntity54::TypedTaskDefinition td = { &a7EnvEntity54_Init, &a7EnvEntity54_Update, &a7EnvEntity54_Draw, nullptr };
    createSiblingTask<sA7EnvEntity54>(parent, &td);
}

// 0605600a — same function as setGameFlagsA7_0, called from both startTasks 0 and 2
void createA7_visibilityObjects(void)
{
    setGameFlagsA7_0();
}

// 06056c0a — start tasks for subfield 1
void startTasksA7_1(p_workArea parent)
{
    createFieldSpecificDataTask_A7(parent);
    {
        s_fieldSpecificData_A7* pFieldData = (s_fieldSpecificData_A7*)getFieldTaskPtr()->mC;
        pFieldData->m280 = createA7_3dSceneManager(parent, 3, 0x20);
    }
    createA7_envEntity_a456(parent); // Shelcoof
    createA7_envEntity_9e6e(parent, gFLD_A7->getSaturnPtr(0x06085aec)); // repair bits
    createA7_itemBoxes_1();
    createA7_envEntity_a2fe(parent); // tornado
    createA7_encounterCheckSubtask(parent);
    createA7_soundCleanup_0x6f(parent);
    createA7_envEntity_51ee(parent, gFLD_A7->getSaturnPtr(0x060842d0), 0); // spinning debris
    createA7_envEntity_e7c2(parent);
}

// 06056c6a — start tasks for subfield 2
void startTasksA7_2(p_workArea parent)
{
    createFieldSpecificDataTask_A7(parent);
    {
        s_fieldSpecificData_A7* pFieldData = (s_fieldSpecificData_A7*)getFieldTaskPtr()->mC;
        pFieldData->m280 = createA7_3dSceneManager(parent, 4, 0x20);
    }
    createA7_visibilityObjects();
    createA7_emptySubtask(parent);
    createA7_envEntity_e034(parent);
    createA7_soundCleanup_0x65(parent);
}

// 06056d0c — push dragon away from a point based on proximity
static void dragonPushFromPoint_A7(s_dragonTaskWorkArea* pDragon, sVec3_FP* pTarget)
{
    sVec3_FP target;
    target.m0_X = pTarget->m0_X;
    target.m4_Y = pDragon->m8_pos.m4_Y;
    target.m8_Z = pTarget->m8_Z;

    s32 dist = vecDistance(pDragon->m8_pos, target);
    s32 pushRange = 0xA5000 - dist;

    s32 pushX = 0;
    s32 pushZ = 0;
    if (pushRange >= 0)
    {
        s32 dx = pDragon->m8_pos.m0_X.asS32() - target.m0_X.asS32();
        s32 dz = pDragon->m8_pos.m8_Z.asS32() - target.m8_Z.asS32();
        pushX = setDividend(pushRange, dx, dist);
        pushZ = setDividend(pushRange, dz, dist);
    }

    pDragon->m160_deltaTranslation.m0_X = fixedPoint(pDragon->m160_deltaTranslation.m0_X.asS32() + pushX);
    // m164 (Y) unchanged
    pDragon->m160_deltaTranslation.m8_Z = fixedPoint(pDragon->m160_deltaTranslation.m8_Z.asS32() + pushZ);
}

// 06056dbc — dragon mF4 callback for subfield 1
void dragonCallback_A7_1(s_dragonTaskWorkArea* pDragon)
{
    static const sVec3_FP pushTarget = { fixedPoint(0x600000), fixedPoint(0x40000), fixedPoint(-0x600000) };
    dragonPushFromPoint_A7(pDragon, const_cast<sVec3_FP*>(&pushTarget));
    mainGameState.bitField[0xA3] |= 2;
    clearCameraZoneTargets(1);
}

// 06056dec — dragon mF4 callback for subfield 2
void dragonCallback_A7_2(s_dragonTaskWorkArea* pDragon)
{
    if ((mainGameState.bitField[0xA3] & 8) != 0)
    {
        static const sVec3_FP pushTarget = { fixedPoint(0), fixedPoint(0), fixedPoint(0) };
        dragonPushFromPoint_A7(pDragon, const_cast<sVec3_FP*>(&pushTarget));
    }
}
