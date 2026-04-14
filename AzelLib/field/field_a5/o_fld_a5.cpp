#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldVisibilityGrid.h"
#include "field/fieldSceneManager.h"
#include "field/fieldModelRender.h"
#include "field/fieldItemBox.h"
#include "field/fieldItemBoxDefinition.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "trigo.h"
#include "field/fieldDragonInput.h"
#include "field/fieldCutsceneTask2.h"
#include "menu_dragonMorph.h"

s32 playBattleSoundEffect(s32 effectIndex);
// 0606CFF0 — checks worm segment visibility in grid + debug worm editor controls
static bool auto_0x0606CFF0() { Unimplemented(); return true; }

FLD_A5_data* gFLD_A5 = nullptr;

// Depth range table for A5 (read from 06089a28)
static std::vector<fixedPoint> a5_depthRangeTable =
{
    fixedPoint(0x180000),
    fixedPoint(0x200000),
    fixedPoint(0x400000),
    fixedPoint(0x7FFFFFFF),
};

// 0605595c — A5 update function 3 (debug: collision/worm object data viewer)
static void a5_updateFunction3()
{
    Unimplemented(); // FUN_FLD_A5__0606dbac with data from 0608c890/0608c894
}

// 06055952 — A5 update function 4 (debug: message/dialog data viewer)
static void a5_updateFunction4()
{
    Unimplemented(); // FUN_FLD_A5__0606d8c0 with data from 0608c888/0608c88c
}

// --- Post-battle sound functions (set as m344->m0 callback) ---

// 06055984 — post-battle sound for subfields 0, 7 (day open areas)
void postBattleSound_A5_day(void)
{
    playPCM(getFieldTaskPtr()->m8_pSubFieldData, 0x66);
}

// 060559a8 — post-battle sound for subfields 2, 8 (night open areas, variant A)
void postBattleSound_A5_nightA(void)
{
    p_workArea pSubFieldData = (p_workArea)getFieldTaskPtr()->m8_pSubFieldData;
    playPCM(pSubFieldData, 0x66);
    if ((mainGameState.bitField[0x96] & 0x10) != 0)
    {
        playPCM(pSubFieldData, 100);
    }
}

// 060559ec — post-battle sound for subfields 4, 9 (night open areas, variant B)
void postBattleSound_A5_nightB(void)
{
    p_workArea pSubFieldData = (p_workArea)getFieldTaskPtr()->m8_pSubFieldData;
    playPCM(pSubFieldData, 0x66);
    if ((mainGameState.bitField[0x96] & 1) != 0)
    {
        playPCM(pSubFieldData, 0x65);
    }
}

// 060804d0 — create 3D scene manager task, store in field data m54
void createA5_3dSceneManager(p_workArea parent)
{
    s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
    pFieldData->m54 = createFieldSceneManager(parent, 1, 0x40);
}

// 060552c0 — empty update (data task has no per-frame logic)
static void fieldSpecificDataUpdate_A5(s_fieldSpecificData_A5* pThis)
{
    // empty — confirmed from Ghidra
}

// 060552c4 — create field-specific data task for A5
void createFieldSpecificDataTask_A5(p_workArea parent)
{
    s_fieldSpecificData_A5* p = createSubTaskFromFunction<s_fieldSpecificData_A5>(parent, &fieldSpecificDataUpdate_A5);
    if (p != nullptr)
    {
        getFieldTaskPtr()->mC = (p_workArea)p;
    }
}

// sA5WormObjectSystem (and sA5WormParticle) are implemented in a5_wormObjectSystem.cpp

// sA5ExitEntity + setFieldExitConfig_A5 are implemented in a5_exitEntity.cpp

// Corridor worm subtask struct (size 0xC)
struct sA5CorridorWormSubtask : public s_workAreaTemplate<sA5CorridorWormSubtask>
{
    s32 m0;
    s32 m4;
    s32 m8;
    // size 0xC
};

// computeDragonSpeed, dragonUpdate_idle, dragonUpdate_normal,
// dragonUpdate_normal_type8, clearDragonScriptFlag, dragonTransitionToNormal,
// processCameraScript, processCutscene, dragonUpdate_cameraScript, dragonUpdate_cutscene

// as setCutsceneKeyFrame, clearCutsceneKeyFrame, enableScriptSkippingAndExit

// 060591fc — corridor worm file-load/animation state machine. Manages
// progressive loading of the worm segment models as the dragon travels
// through a corridor. Called every frame from corridorWormUpdate_1/_3.
//
// pData layout (6 x s32, big-endian):
//   [0] Z position A
//   [1] Z position B
//   [2] day file index A
//   [3] day file index B
//   [4] night file index A
//   [5] night file index B
static void a5_corridorWormAnimUpdate(sA5CorridorWormSubtask* pThis, sSaturnPtr pData, const sVec3_FP* pDragonPos)
{
    s32 state = pThis->m0;
    s32 dragonZ = pDragonPos->m8_Z.asS32();

    if (state == 0)
    {
        // Init: compute center position and threshold distance
        s32 posA = readSaturnS32(pData);
        s32 posB = readSaturnS32(pData + 4);
        pThis->m4 = (s32)((u32)(posA + posB + (u32)((u32)posA + (u32)posB < (u32)posA)) >> 1);
        s32 dist = posA - posB;
        if (dist < 0) dist = posB - posA;
        pThis->m8 = dist / 6;

        if (dragonZ <= pThis->m4)
            pThis->m0 = 2;
        if (pThis->m4 < dragonZ)
            pThis->m0 = 4;
        return;
    }

    if (state == 1)
    {
        // Load file pair (direction A→B)
        s32 fileOff = ((mainGameState.bitField[0xA3] & 0x20) != 0) ? 16 : 8;
        unloadFileFromFileList(readSaturnS32(pData + fileOff + 4));
        loadFileFromFileList(readSaturnS32(pData + fileOff));
        pThis->m0++;
        // Fall through to state 2 threshold check
    }

    if (state == 1 || state == 2)
    {
        // Check if dragon crossed center + threshold → switch to state 3
        if (pThis->m4 + pThis->m8 < dragonZ)
        {
            pThis->m0 = 3;
        }
        return;
    }

    if (state == 3)
    {
        // Load file pair (direction B→A)
        s32 fileOff = ((mainGameState.bitField[0xA3] & 0x20) != 0) ? 16 : 8;
        unloadFileFromFileList(readSaturnS32(pData + fileOff));
        loadFileFromFileList(readSaturnS32(pData + fileOff + 4));
        pThis->m0++;
        // Fall through to state 4 threshold check
    }

    // States 3 (after increment = 4) and 4
    if (pThis->m4 - pThis->m8 < dragonZ)
        return;
    pThis->m0 = 1;
}

// Bounds check helper: returns true if dragonPos is inside the 6-value
// {xMin, xMax, yMin, yMax, zMin, zMax} region stored at the Saturn address.
static bool a5_isDragonInRegion(const sVec3_FP& pos, const s32* bounds)
{
    return pos.m0_X.asS32() >= bounds[0] && pos.m0_X.asS32() <= bounds[1] &&
           pos.m4_Y.asS32() >= bounds[2] && pos.m4_Y.asS32() <= bounds[3] &&
           pos.m8_Z.asS32() >= bounds[4] && pos.m8_Z.asS32() <= bounds[5];
}

// 06099914
static const s32 corridorBounds_1_region0[] = { (s32)0x80000001, 0x85000, (s32)0x80000001, 0x7FFFFFFF, -0x6A4000, -0x5DC000 };
// 0609992C
static const s32 corridorBounds_1_region1[] = { 0x25D000, 0x7FFFFFFF, (s32)0x80000001, 0x7FFFFFFF, -0xE4000, -0x1C000 };
// 06099944
static const s32 corridorBounds_3_region0[] = { 0x4C000, 0x114000, (s32)0x80000001, 0x7FFFFFFF, -0x7D000, 0x7FFFFFFF };
// 0609995C
static const s32 corridorBounds_3_region1[] = { 0x4C000, 0x114000, (s32)0x80000001, 0x7FFFFFFF, (s32)0x80000001, -0x519000 };
// 06099974
static const s32 corridorBounds_5_A[] = { 0x195000, 0x7FFFFFFF, (s32)0x80000001, 0x7FFFFFFF, -0x4C6000, -0x3FE000 };
// 0609998C
static const s32 corridorBounds_6[] = { 0x181000, 0x7FFFFFFF, (s32)0x80000001, 0x7FFFFFFF, -0x413000, -0x34B000 };

static void a5_corridorTryTransition(sA5CorridorWormSubtask* pThis, s32 dayDest, s32 nightDest, bool checkFileLoading)
{
    if (checkFileLoading && fileInfoStruct.m2C_allocatedHead != nullptr)
    {
        initDragonMovementMode();
        return;
    }
    s32 dest = (mainGameState.bitField[0xA3] & 0x20) ? nightDest : dayDest;
    triggerSubfieldChange(dest, -0xC64);
    if (pThis != nullptr)
    {
        pThis->getTask()->markFinished();
    }
}

// 0605930c — corridor 1 worm update
static void corridorWormUpdate_1(sA5CorridorWormSubtask* pThis)
{
    sVec3_FP dragonPos;
    getDragonPosition(&dragonPos);

    if (a5_isDragonInRegion(dragonPos, corridorBounds_1_region0))
    {
        a5_corridorTryTransition(pThis, 0, 7, true);
    }
    if (a5_isDragonInRegion(dragonPos, corridorBounds_1_region1))
    {
        a5_corridorTryTransition(pThis, 2, 8, true);
    }
    a5_corridorWormAnimUpdate(pThis, gFLD_A5->getSaturnPtr(0x060999A4), &dragonPos);
}

// 0605947a — corridor 3 worm update
static void corridorWormUpdate_3(sA5CorridorWormSubtask* pThis)
{
    sVec3_FP dragonPos;
    getDragonPosition(&dragonPos);

    if (a5_isDragonInRegion(dragonPos, corridorBounds_3_region0))
    {
        a5_corridorTryTransition(pThis, 2, 8, true);
    }
    if (a5_isDragonInRegion(dragonPos, corridorBounds_3_region1))
    {
        a5_corridorTryTransition(pThis, 4, 9, true);
    }
    a5_corridorWormAnimUpdate(pThis, gFLD_A5->getSaturnPtr(0x060999BC), &dragonPos);
}

// 060595b4 — corridor 5 worm update
static void corridorWormUpdate_5(sA5CorridorWormSubtask* pThis)
{
    sVec3_FP dragonPos;
    getDragonPosition(&dragonPos);

    if (a5_isDragonInRegion(dragonPos, corridorBounds_5_A))
    {
        a5_corridorTryTransition(pThis, 0, 7, false);
    }
}

// 0605962c — corridor 6 worm update
static void corridorWormUpdate_6(sA5CorridorWormSubtask* pThis)
{
    sVec3_FP dragonPos;
    getDragonPosition(&dragonPos);

    if (a5_isDragonInRegion(dragonPos, corridorBounds_6))
    {
        a5_corridorTryTransition(pThis, 2, 8, false);
    }
}

// 06059750 — create corridor 1 worm subtask
void createA5_corridorWorm_1(p_workArea parent)
{
    createSubTaskFromFunction<sA5CorridorWormSubtask>(parent, &corridorWormUpdate_1);
}

// 06059760 — create corridor 3 worm subtask
void createA5_corridorWorm_3(p_workArea parent)
{
    createSubTaskFromFunction<sA5CorridorWormSubtask>(parent, &corridorWormUpdate_3);
}

// 0605976c — create corridor 5 worm subtask
void createA5_corridorWorm_5(p_workArea parent)
{
    createSubTaskFromFunction<sA5CorridorWormSubtask>(parent, &corridorWormUpdate_5);
}

// 06059774 — create corridor 6 worm subtask
void createA5_corridorWorm_6(p_workArea parent)
{
    createSubTaskFromFunction<sA5CorridorWormSubtask>(parent, &corridorWormUpdate_6);
}

// a5CreateDecorEmitter — A5 overlay-local copy of the shared item box
// creation path. The decor emitter struct is binary-identical to
// s_itemBoxType1, so we use the shared infrastructure directly.
static void a5CreateDecorEmitter(sSaturnPtr pData)
{
    fieldA3_1_createItemBoxes_Sub1(readItemBoxDefinition(pData));
}

// 0605fe7e — create subfield 5 environment object
void createA5_envObject_5()
{
    a5CreateDecorEmitter(gFLD_A5->getSaturnPtr(0x0609D380));
}

// 0605fe86 — create subfield 6 environment object
void createA5_envObject_6()
{
    a5CreateDecorEmitter(gFLD_A5->getSaturnPtr(0x0609D3C8));
}

// 060596a4 — corridor A worm update (same bounds as corridor 5)
static void corridorWormUpdate_A(sA5CorridorWormSubtask* pThis)
{
    sVec3_FP dragonPos;
    getDragonPosition(&dragonPos);

    if (a5_isDragonInRegion(dragonPos, corridorBounds_5_A))
    {
        a5_corridorTryTransition(pThis, 0, 7, false);
    }
}

// 0605977c — create corridor A worm subtask
void createA5_corridorWorm_A(p_workArea parent)
{
    createSubTaskFromFunction<sA5CorridorWormSubtask>(parent, &corridorWormUpdate_A);
}

// 0605fe8e — create subfield A environment object
void createA5_envObject_A()
{
    a5CreateDecorEmitter(gFLD_A5->getSaturnPtr(0x0609D410));
}

// --- Shared A5 gameplay helpers ---
// as getDragonPosition, getDragonAngle, triggerSubfieldChange

// --- Shared open-area environment/entity creation functions ---

// sA5WormDustTask is implemented in a5_wormDustTask.cpp

// sA5WormSegmentEntity is implemented in a5_wormSegmentEntity.cpp

// sA5ProximityAlertTask is implemented in a5_proximityAlertTask.cpp

// sA5FieldEventCheckTask is implemented in a5_fieldEventCheck.cpp

// 06057c0c — create Baldor worm body (complex 3D entity, subfields 2, 8)
void createA5_baldorWormBody(p_workArea parent)
{
    Unimplemented(); // createSubTaskFromFunction size 0x138, loads Baldor body with 3D models/animation
}

// 0605fdb8 — create environment decor objects for day subfields (0, 7)
// 0605f6f0
static void a5DecorDrawCallback() { Unimplemented(); }

void createA5_decorObjects_day()
{
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m9D4_customDrawFunc = &a5DecorDrawCallback;
    static const u32 addrs[] = { 0x0609CE28, 0x0609CE70, 0x0609CEB8, 0x0609CF00, 0x0609CF48, 0x0609CF90 };
    for (int i = 0; i < 6; i++)
        a5CreateDecorEmitter(gFLD_A5->getSaturnPtr(addrs[i]));
}

// --- Per-subfield environment objects ---

// 0605971c — empty (subfield 0 placeholder)
void createA5_envObjects_empty_0971c() {}

// 06059768 — empty (subfield 4/9 placeholder)
void createA5_envObjects_empty_09768() {}

// createA5_envObjects_sub0 is implemented in a5_wormHoleEntity.cpp

// createA5_envObjects_sub0_exits is implemented in a5_exitTriggerEntity.cpp

// Particle system entity (size 0x150)
struct sA5ParticleEntity : public s_workAreaTemplate<sA5ParticleEntity>
{
    u8 m0_data[0x150]; // size 0x150
};
static void a5ParticleEntity_Init(sA5ParticleEntity* p) { Unimplemented(); } // 0605E99C
static void a5ParticleEntity_Update(sA5ParticleEntity* p) { Unimplemented(); } // 0605E8EE
static void a5ParticleEntity_Draw(sA5ParticleEntity* p) { Unimplemented(); } // 0605E7E8

// 0605ebc6
void createA5_envObjects_sub4_particle(p_workArea parent)
{
    static sA5ParticleEntity::TypedTaskDefinition td = { &a5ParticleEntity_Init, &a5ParticleEntity_Update, &a5ParticleEntity_Draw, nullptr };
    createSubTask<sA5ParticleEntity>(parent, &td);
}

// Light entity (size 1 — minimal)
struct sA5LightEntity : public s_workAreaTemplate<sA5LightEntity>
{
    u8 m0; // size 0x1
};
// 060601AC
static void a5LightEntity_Init(sA5LightEntity* p)
{
    p->m0 = 0;
}

// 0x4C-byte light wing evolution effect subtask
// Task def at 0609d488 = {init=0605FF00, update=0605FF5C, draw=06060114, delete=null}
struct sA5LightEffectSubtask : public s_workAreaTemplate<sA5LightEffectSubtask>
{
    u8 m0_data[0x4C];
    // size 0x4C
};
static void a5LightEffectSubtask_Init(sA5LightEffectSubtask*) { Unimplemented(); }  // 0605FF00
static void a5LightEffectSubtask_Update(sA5LightEffectSubtask*) { Unimplemented(); } // 0605FF5C
static void a5LightEffectSubtask_Draw(sA5LightEffectSubtask*) { Unimplemented(); }   // 06060114

// Static buffer for cutscene camera script data (like A7's a7CutsceneScriptData)
static std::vector<s_scriptData1> a5CutsceneScriptData;

void a5CutsceneCameraInit(p_workArea parent, const sSaturnPtr& scriptDataEA, s32 r6, sVec3_FP* r7, u32 flags)
{
    loadScriptData1(a5CutsceneScriptData, scriptDataEA);
    cutsceneTaskInitSub2(parent, a5CutsceneScriptData, r6, r7, flags);
}

// 060601B2
static void a5LightEntity_Update(sA5LightEntity* p)
{
    if (p->m0 == 0
        && (mainGameState.bitField[0xA3] & 0x40) == 0
        && (mainGameState.bitField[0x96] & 8) != 0
        && (mainGameState.bitField[0x96] & 4) != 0)
    {
        startFieldScript(10, 0x5BF);

        static sA5LightEffectSubtask::TypedTaskDefinition td = {
            &a5LightEffectSubtask_Init, &a5LightEffectSubtask_Update, &a5LightEffectSubtask_Draw, nullptr
        };
        sA5LightEffectSubtask* pEffect = createSubTask<sA5LightEffectSubtask>((p_workArea)p, &td);

        sVec3_FP* pEffectPos = (pEffect != nullptr) ? (sVec3_FP*)&pEffect->m0_data[8] : nullptr;
        a5CutsceneCameraInit((p_workArea)p, gFLD_A5->getSaturnPtr(0x0608CD5C), 0, pEffectPos, 0);

        p->m0++;
    }
}

// 06060220
void createA5_envObjects_sub4_light(p_workArea parent)
{
    static sA5LightEntity::TypedTaskDefinition td = { &a5LightEntity_Init, &a5LightEntity_Update, nullptr, nullptr };
    createSubTask<sA5LightEntity>(parent, &td);
}

// createA5_envObjects_sub4_wormSegments is implemented in a5_wormSegmentEntity.cpp

// 06059430 — corridor worm update for subfield 2/8: checks bitfields and triggers cutscene
static void corridorWormUpdate_2(sA5CorridorWormSubtask* pThis)
{
    if ((mainGameState.bitField[0x96] & 0x10) != 0 &&
        (mainGameState.bitField[0x96] & 0x20) == 0 &&
        auto_0x0606CFF0() &&
        startFieldScript(3, 0x5b8) != 0)
    {
        a5CutsceneCameraInit((p_workArea)pThis, gFLD_A5->getSaturnPtr(0x0608CEC4), 0, nullptr, 0);
    }
}

// 06059758 — create corridor worm subtask for subfield 2/8
void createA5_corridorWorm_2(p_workArea parent)
{
    createSubTaskFromFunction<sA5CorridorWormSubtask>(parent, &corridorWormUpdate_2);
}

// 06059768 reuse for sub 4 — empty, already handled

// 0605efb2 — init worm color tables for day subfields (4)
void initA5_wormColorTables_day()
{
    s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
    pFieldData->mC_wormColorTable0 = getSaturnPtr(gFLD_A5->getSaturnPtr(0x0609C628));
    pFieldData->m10_wormColorTable1 = getSaturnPtr(gFLD_A5->getSaturnPtr(0x0609C634));
    pFieldData->m14_wormColorState0 = 0;
    pFieldData->m18_wormColorState1 = 0;
}

// 0605ef84 — init worm color tables for night subfields (2)
void initA5_wormColorTables_night()
{
    s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
    pFieldData->mC_wormColorTable0 = getSaturnPtr(gFLD_A5->getSaturnPtr(0x0609C610));
    pFieldData->m10_wormColorTable1 = getSaturnPtr(gFLD_A5->getSaturnPtr(0x0609C61C));
    pFieldData->m14_wormColorState0 = 0;
    pFieldData->m18_wormColorState1 = 0;
}

// 0605fe38 — create decor objects for night subfield 4/9
void createA5_decorObjects_night_4()
{
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m9D4_customDrawFunc = &a5DecorDrawCallback;
    static const u32 addrs[] = { 0x0609D188, 0x0609D1D0, 0x0609D218, 0x0609D260, 0x0609D2A8, 0x0609D2F0, 0x0609D338 };
    for (int i = 0; i < 7; i++)
        a5CreateDecorEmitter(gFLD_A5->getSaturnPtr(addrs[i]));
}

// 0605fdf8 — create decor objects for night subfield 2/8
void createA5_decorObjects_night_2()
{
    getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS->m9D4_customDrawFunc = &a5DecorDrawCallback;
    static const u32 addrs[] = { 0x0609CFD8, 0x0609D020, 0x0609D068, 0x0609D0B0, 0x0609D0F8, 0x0609D140 };
    for (int i = 0; i < 6; i++)
        a5CreateDecorEmitter(gFLD_A5->getSaturnPtr(addrs[i]));
}

// sA5SandfallEntity is implemented in a5_sandfallEntity.cpp
// sA5SkyEntity is implemented in a5_skyEntity.cpp

// sA5WeatherTask is implemented in a5_weatherTask.cpp

// --- Encounter/radar config tasks ---

// Encounter config args
struct sA5EncounterConfig
{
    s32 m0_subfieldId;
    s16 m4_baseOffset;
    s16 m6_colorMask;
    s16 m8_range0;
    s16 mA_range1;
    s16 mC_range2;
    s16 mE_range3;
};

// Encounter config task struct (size 0x20)
struct sA5EncounterConfigTask : public s_workAreaTemplateWithArg<sA5EncounterConfigTask, sA5EncounterConfig*>
{
    s_memoryAreaOutput m0_memoryArea;
    s16 m10_radarOffset;
    s16 m12_colorMask;
    s16 m14_range0;
    s16 m16_range1;
    s16 m18_range2;
    s16 m1A_range3;
    u8 m1C_active;
    // size 0x20
};

// 060552F4
static void a5EncounterConfigInit(sA5EncounterConfigTask* pThis, sA5EncounterConfig* pConfig)
{
    getMemoryArea(&pThis->m0_memoryArea, pConfig->m0_subfieldId);
    pThis->m10_radarOffset = pConfig->m4_baseOffset + (s16)((pThis->m0_memoryArea.m4_characterArea - 0x25C00000) >> 3);
    pThis->m12_colorMask = pConfig->m6_colorMask;
    pThis->m14_range0 = pConfig->m8_range0;
    pThis->m16_range1 = pConfig->mA_range1;
    pThis->m18_range2 = pConfig->mC_range2;
    pThis->m1A_range3 = pConfig->mE_range3;
    pThis->m1C_active = 1;
    ((s_fieldSpecificData_A5*)getFieldTaskPtr()->mC)->m0_pEncounterConfig = pThis;
}

// 060554f8 — shared: create encounter config task
static void createA5_encounterConfigInner(p_workArea parent, sA5EncounterConfig* pConfig)
{
    static sA5EncounterConfigTask::TypedTaskDefinition td = { &a5EncounterConfigInit, nullptr, nullptr, nullptr };
    createSubTaskWithArg<sA5EncounterConfigTask>(parent, pConfig, &td);
}

// 06055510/0605554c/06055588/060555c4/06055600/0605563c — per-subfield encounter config wrappers
void createA5_encounterConfig(p_workArea parent, s32 subfieldId, s16 param1, s16 colorMask, s16 param3, s16 param4, s16 param5, s16 param6)
{
    sA5EncounterConfig config;
    config.m0_subfieldId = subfieldId;
    config.m4_baseOffset = param1;
    config.m6_colorMask = colorMask;
    config.m8_range0 = param3;
    config.mA_range1 = param4;
    config.mC_range2 = param5;
    config.mE_range3 = param6;
    createA5_encounterConfigInner(parent, &config);
}

// --- Trigger zone tasks (subfields 2, 8) ---

// Zero-size script check task
struct sA5ScriptCheckTask : public s_workAreaTemplate<sA5ScriptCheckTask>
{
    // size 0x0
};

// 06054e34 — script check: proximity sound for trigger zones
static void a5ScriptCheckUpdate(sA5ScriptCheckTask* pThis)
{
    static const u32 posAddrs[] = { 0x06089B98, 0x06089BA4, 0x06089BB0, 0x06089BBC };
    for (u32 i = 0; i < 4; i++)
    {
        if (isWorldPositionOnScreen(gFLD_A5->getSaturnPtr(posAddrs[i])))
        {
            s32 sndHandle = findSound(0x75);
            if (sndHandle < 0)
            {
                playSystemSoundEffect(0x75);
            }
            return;
        }
    }
    playBattleSoundEffect(0x75);
}

// Trigger zone entity (size 0x40)
struct sA5TriggerZoneEntity : public s_workAreaTemplateWithArg<sA5TriggerZoneEntity, void*>
{
    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_position;
    s32 m18_animSpeed;
    s16 m1C_param0;
    u8 m1E_pad[0x1C];
    s16 m3A_param1;
    u8 m3C_index;
    // size 0x40
};

// Day variants (task def at 06089bc8)
// Trigger zone entity struct with named members (size 0x40)
// The arg is sA5TriggerZoneArgs* — {sSaturnPtr data, s32 index}

struct sA5TriggerZoneArgs
{
    sSaturnPtr m0_data;
    s32 m4_index;
};

// 06054EA0 — day trigger zone init
static void a5TriggerZoneDay_Init(sA5TriggerZoneEntity* pThis, void* arg)
{
    sA5TriggerZoneArgs* pArgs = (sA5TriggerZoneArgs*)arg;
    getMemoryArea(&pThis->m0_memoryArea, 3);
    pThis->m8_position = readSaturnVec3(pArgs->m0_data);
    pThis->m3C_index = (u8)pArgs->m4_index;
    pThis->m18_animSpeed = 0x16C16C;
    pThis->m1C_param0 = (s16)0x8000;
    pThis->m3A_param1 = (s16)0xADF1;
}

// 060550BC — night trigger zone init
static void a5TriggerZoneNight_Init(sA5TriggerZoneEntity* pThis, void* arg)
{
    sA5TriggerZoneArgs* pArgs = (sA5TriggerZoneArgs*)arg;
    getMemoryArea(&pThis->m0_memoryArea, 3);
    pThis->m8_position = readSaturnVec3(pArgs->m0_data);
    pThis->m3C_index = (u8)pArgs->m4_index;
    pThis->m18_animSpeed = 0x16C16C;
    pThis->m1C_param0 = 0;
    pThis->m3A_param1 = 0xC42;
}

// 06055020 — create trigger zones for day subfield 2
void createA5_triggerZones_day(p_workArea parent)
{
    createSubTaskFromFunction<sA5ScriptCheckTask>(parent, &a5ScriptCheckUpdate);

    static sA5TriggerZoneEntity::TypedTaskDefinition td = { (void(*)(sA5TriggerZoneEntity*, void*))&a5TriggerZoneDay_Init, nullptr, nullptr, nullptr };
    static const struct { u32 addr; s32 idx; } zones[] = {
        { 0x06089B98, 0 }, { 0x06089BA4, 3 }, { 0x06089BB0, 6 }, { 0x06089BBC, 9 },
    };
    for (int i = 0; i < 4; i++)
    {
        sA5TriggerZoneArgs args = { gFLD_A5->getSaturnPtr(zones[i].addr), zones[i].idx };
        createSubTaskWithArg<sA5TriggerZoneEntity>(parent, &args, &td);
    }
}

// 06055224 — create trigger zones for night subfield 8
void createA5_triggerZones_night(p_workArea parent)
{
    createSubTaskFromFunction<sA5ScriptCheckTask>(parent, &a5ScriptCheckUpdate);

    static sA5TriggerZoneEntity::TypedTaskDefinition td = { (void(*)(sA5TriggerZoneEntity*, void*))&a5TriggerZoneNight_Init, nullptr, nullptr, nullptr };
    static const struct { u32 addr; s32 idx; } zones[] = {
        { 0x06089B98, 0 }, { 0x06089BA4, 3 }, { 0x06089BB0, 6 }, { 0x06089BBC, 9 },
    };
    for (int i = 0; i < 4; i++)
    {
        sA5TriggerZoneArgs args = { gFLD_A5->getSaturnPtr(zones[i].addr), zones[i].idx };
        createSubTaskWithArg<sA5TriggerZoneEntity>(parent, &args, &td);
    }
}

static const s_MCB_CGB fieldFileList[] =
{
    { "FLDCMN.MCB", "FLDCMN.CGB" },
    { "FLD_A5.MCB", "FLD_A5.CGB" },
    { "FLD_A5_0.MCB", "FLD_A5_0.CGB" },
    { "FLD_A5_2.MCB", "FLD_A5_2.CGB" },
    { "FLD_A5_4.MCB", "FLD_A5_4.CGB" },
    { "FLD_A5_7.MCB", "FLD_A5_7.CGB" },
    { "FLD_A5_8.MCB", "FLD_A5_8.CGB" },
    { "FLD_A5_9.MCB", "FLD_A5_9.CGB" },
    { "FLD_A5_B.MCB", "FLD_A5_B.CGB" },
    { (const char*)-1, nullptr }
};

// 06054000
p_workArea overlayStart_FLD_A5(p_workArea workArea, u32 arg)
{
    if (gFLD_A5 == nullptr)
    {
        gFLD_A5 = new FLD_A5_data();
    }

    // FLD_A3.PRG contains shared field infrastructure data (dragon anim tables, etc.)
    if (gFLD_A3 == nullptr)
    {
        gFLD_A3 = new FLD_A3_data();
    }

    gCorridorEntryScriptEA = gFLD_A5->getSaturnPtr(0x06088AAC);

    // 06072e36 — override subfield for specific game state
    if (mainGameState.bitField[0x1B] & 8)
    {
        getFieldTaskPtr()->m2E_currentSubFieldIndex = 0xB;
        getFieldTaskPtr()->m30_fieldEntryPoint = 0;
    }

    gFieldCameraConfigEA = gFLD_A5->getSaturnPtr(0x0609E938);
    gFieldDragonAnimTableEA = { 0x06094134, gFLD_A3 };
    gFieldCameraDrawFunc = &fieldOverlaySubTaskInitSub2;
    gCurrentFieldOverlay = gFLD_A3;

    if (!initField(workArea, fieldFileList, arg))
    {
        return nullptr;
    }

    s16 subFieldIndex = getFieldTaskPtr()->m2E_currentSubFieldIndex;

    if (subFieldIndex == 0xB)
    {
        loadSoundBanks(1, 0);
        playPCM(workArea, 100);
    }
    else
    {
        loadSoundBanks(7, 0);
    }

    graphicEngineStatus.m405C.m10_nearClipDistance = 0x3000;
    graphicEngineStatus.m405C.m30_oneOverNearClip = FP_Div(0x10000, graphicEngineStatus.m405C.m10_nearClipDistance);

    graphicEngineStatus.m405C.m14_farClipDistance = 0x400000;
    graphicEngineStatus.m405C.m38_oneOverFarClip = FP_Div(0x8000, graphicEngineStatus.m405C.m14_farClipDistance);

    graphicEngineStatus.m405C.m34_oneOverFarClip256 = graphicEngineStatus.m405C.m38_oneOverFarClip << 8;

    getFieldTaskPtr()->m8_pSubFieldData->m334->m50E_followModeIndex = 1;
    getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m0_pScripts = ReadScripts(gFLD_A5->getSaturnPtr(0x0609D4A8));

    // Per-subfield dispatch
    static void (*subfieldTable[])(p_workArea) = {
        subfieldA5_0, subfieldA5_1, subfieldA5_2, subfieldA5_3,
        subfieldA5_4, subfieldA5_5, subfieldA5_6, subfieldA5_7,
        subfieldA5_8, subfieldA5_9, subfieldA5_A, subfieldA5_B,
    };

    s16 subfield = getFieldTaskPtr()->m2E_currentSubFieldIndex;
    if (subfield < (s16)(sizeof(subfieldTable) / sizeof(subfieldTable[0])))
    {
        subfieldTable[subfield](workArea);
    }
    else
    {
        assert(0); // unimplemented subfield
    }

    // Post-subfield init: set collision depth table, visibility flag, and update functions
    getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1->m2C_depthRangeTable = &a5_depthRangeTable;
    getFieldTaskPtr()->m8_pSubFieldData->m348_pFieldCameraTask1->m1300 = 1;
    getFieldTaskPtr()->m8_pSubFieldData->pUpdateFunction3 = &a5_updateFunction3;
    getFieldTaskPtr()->m8_pSubFieldData->m378_pUpdateFunction4 = &a5_updateFunction4;

    if (subfield != 0xB)
    {
        startFieldScript(subfield + 0x19, -1);
    }

    return nullptr;
}
