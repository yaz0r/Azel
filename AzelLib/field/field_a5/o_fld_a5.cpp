#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/fieldVisibilityGrid.h"
#include "field/fieldSceneManager.h"
#include "field/fieldModelRender.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "trigo.h"
#include "menu_dragonMorph.h"

s32 playBattleSoundEffect(s32 effectIndex);

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

// Worm 3D object system (size 0x372C — very large)
struct sA5WormParticle
{
    sAnimatedQuad m0_quad;
    sVec3_FP m8_position;       // X, Y, Z world position
    s32 m14_velocityX;
    s32 m18_velocityY;
    s32 m1C_velocityZ;
    s32 m20_pad0;
    s32 m24_gravityY;
    s32 m28_pad1;
    s32 m2C_orbitAngle;
    s32 m30_orbitAngleSpeed;
    s32 m34_pad2;
    s32 m38_orbitRadius;
    s32 m3C_orbitRadiusSpeed;
    s8 m40_lifetime;
    u8 m41_type;
    u8 m42_pad[2];
    // size 0x44
};

struct sA5WormObjectSystem : public s_workAreaTemplate<sA5WormObjectSystem>
{
    s_memoryAreaOutput m0_memoryArea; // 0x00 (8 bytes)
    s16 m8_freeIndices[0xC4];        // 0x08: free index pool (0x188 bytes → ends at 0x190)
    s16 m190_sortedIndices[0xC4];    // 0x190: sorted draw order indices (ends at 0x318)
    sA5WormParticle m318_particles[0xC4]; // 0x318: particle array (0xC4*0x44 → ends at 0x3728)
    s16 m3728_drawCount;             // 0x3728: active particles to draw
    // size 0x372C
};
// 06058E8E
static void a5WormObjectSystem_Init(sA5WormObjectSystem* p)
{
    getMemoryArea(&p->m0_memoryArea, 1);

    // Store pointer past the s_memoryAreaOutput as m8_pWormData
    // (external code accesses particle arrays relative to this offset-8 pointer)
    s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
    pFieldData->m8_pWormData = (void*)p->m8_freeIndices;

    p->m3728_drawCount = 0;

    // Initialize index arrays and particle quads
    u16 vdp1Memory = (u16)((p->m0_memoryArea.m4_characterArea - 0x25C00000) >> 3);
    for (s32 i = 0; i < 0xC4; i += 2)
    {
        p->m8_freeIndices[i] = (s16)i;
        Unimplemented(); // particleInitSub(&p->m318_particles[i].m0_quad, vdp1Memory, randomQuadData)
        p->m8_freeIndices[i + 1] = (s16)(i + 1);
        Unimplemented(); // particleInitSub(&p->m318_particles[i+1].m0_quad, vdp1Memory, randomQuadData)
    }
}
// 060583f0 — mode 0: add exit direction to position
static void a5WormParticle_Clamp_Mode0(sVec3_FP* pPos)
{
    sA5ExitEntity* pExit = ((s_fieldSpecificData_A5*)getFieldTaskPtr()->mC)->m4_pExitEntity;
    pPos->m0_X = fixedPoint(pPos->m0_X.asS32() + pExit->mC_direction.m0_X.asS32());
    pPos->m4_Y = fixedPoint(pPos->m4_Y.asS32() + pExit->mC_direction.m4_Y.asS32());
    pPos->m8_Z = fixedPoint(pPos->m8_Z.asS32() + pExit->mC_direction.m8_Z.asS32());
}

// 0605841c — mode 1: spiral drift around exit target
static void a5WormParticle_Clamp_Mode1(sVec3_FP* pPos)
{
    sA5ExitEntity* pExit = ((s_fieldSpecificData_A5*)getFieldTaskPtr()->mC)->m4_pExitEntity;
    s32 dx = pPos->m0_X.asS32() - pExit->m1C_exitTargetX_mode1;
    s32 dz = pPos->m8_Z.asS32() - pExit->m20_exitTargetZ_mode1;
    pPos->m0_X = fixedPoint(pPos->m0_X.asS32() + ((dz >> 4) - (dx >> 5)));
    pPos->m8_Z = fixedPoint(pPos->m8_Z.asS32() - ((dz >> 5) + (dx >> 4)));
}

// 0605846e — position clamp dispatch (per exit mode)
static void a5WormParticle_ClampPosition(sVec3_FP* pPos)
{
    s32 mode = ((s_fieldSpecificData_A5*)getFieldTaskPtr()->mC)->m4_pExitEntity->m2C_exitMode;
    switch (mode)
    {
    case 0: a5WormParticle_Clamp_Mode0(pPos); break;
    case 1: a5WormParticle_Clamp_Mode1(pPos); break;
    case 2: break; // 06058466 — empty
    case 3: break; // 0605846a — empty
    default: break;
    }
}

// 06058F48 — type 0: linear particle with gravity
static s32 a5WormParticle_Update_Type0(sA5WormParticle* p)
{
    p->m18_velocityY += p->m24_gravityY;
    p->m8_position.m0_X = fixedPoint(p->m8_position.m0_X.asS32() + p->m14_velocityX);
    p->m8_position.m4_Y = fixedPoint(p->m8_position.m4_Y.asS32() + p->m18_velocityY);
    p->m8_position.m8_Z = fixedPoint(p->m8_position.m8_Z.asS32() + p->m1C_velocityZ);
    a5WormParticle_ClampPosition(&p->m8_position);
    return 0;
}

// 06058F9C — type 1: orbiting particle around exit position
static s32 a5WormParticle_Update_Type1(sA5WormParticle* p)
{
    sA5ExitEntity* pExit = ((s_fieldSpecificData_A5*)getFieldTaskPtr()->mC)->m4_pExitEntity;

    p->m18_velocityY += p->m24_gravityY;
    p->m8_position.m4_Y = fixedPoint(p->m8_position.m4_Y.asS32() + p->m18_velocityY);

    p->m30_orbitAngleSpeed += p->m34_pad2;
    p->m2C_orbitAngle += p->m30_orbitAngleSpeed;

    p->m38_orbitRadius += p->m3C_orbitRadiusSpeed;
    u32 angleIdx = ((u32)p->m2C_orbitAngle >> 16) & 0xFFF;
    p->m8_position.m0_X = fixedPoint(MTH_Mul(fixedPoint(p->m38_orbitRadius), getSin(angleIdx)).asS32() + pExit->m1C_exitTargetX_mode1);
    p->m8_position.m8_Z = fixedPoint(MTH_Mul(fixedPoint(p->m38_orbitRadius), getCos(angleIdx)).asS32() + pExit->m20_exitTargetZ_mode1);
    return 0;
}

// 06059016 — type 2: linear particle, removed when below height threshold
static s32 a5WormParticle_Update_Type2(sA5WormParticle* p)
{
    sA5ExitEntity* pExit = ((s_fieldSpecificData_A5*)getFieldTaskPtr()->mC)->m4_pExitEntity;

    p->m8_position.m0_X = fixedPoint(p->m8_position.m0_X.asS32() + p->m14_velocityX);
    p->m8_position.m4_Y = fixedPoint(p->m8_position.m4_Y.asS32() + p->m18_velocityY);
    p->m8_position.m8_Z = fixedPoint(p->m8_position.m8_Z.asS32() + p->m1C_velocityZ);

    if (p->m8_position.m4_Y.asS32() <= pExit->m28_exitTargetZ_mode3)
    {
        return 1; // below threshold — remove
    }
    return 0;
}

typedef s32(*a5WormParticleUpdateFunc)(sA5WormParticle*);
static const a5WormParticleUpdateFunc a5WormParticleUpdateTable[] = {
    &a5WormParticle_Update_Type0,
    &a5WormParticle_Update_Type1,
    &a5WormParticle_Update_Type2,
};

// 06058E40 — remove particle from draw list (swap-remove)
static void a5WormParticle_Remove(sA5WormObjectSystem* pSystem, s32 drawIndex)
{
    if (pSystem->m3728_drawCount > 0)
    {
        pSystem->m3728_drawCount--;
        pSystem->m8_freeIndices[pSystem->m3728_drawCount] = pSystem->m190_sortedIndices[drawIndex];
        pSystem->m190_sortedIndices[drawIndex] = pSystem->m190_sortedIndices[pSystem->m3728_drawCount];
    }
}

// 06059054
static void a5WormObjectSystem_Update(sA5WormObjectSystem* p)
{
    // Debug display
    if ((getFieldTaskPtr()->m8_pSubFieldData->m370_fieldDebuggerWho & 4) != 0
        && getFieldTaskPtr()->m8_pSubFieldData->m37E_debugMenuStatus2_a == 0
        && getFieldTaskPtr()->m8_pSubFieldData->m369 == 0)
    {
        // Debug sand particle count + mode display (skipped)
    }

    for (s32 i = 0; i < p->m3728_drawCount; i++)
    {
        s16 idx = p->m190_sortedIndices[i];
        sA5WormParticle& particle = p->m318_particles[idx];

        // Per-type update dispatch
        s32 result = a5WormParticleUpdateTable[particle.m41_type](&particle);
        if (result != 0)
        {
            a5WormParticle_Remove(p, i);
            i--;
            continue;
        }

        // Advance animation
        s32 animResult = sGunShotTask_UpdateSub4(&particle.m0_quad);
        if ((animResult & 2) != 0)
        {
            particle.m40_lifetime--;
            if (particle.m40_lifetime < 1)
            {
                a5WormParticle_Remove(p, i);
                i--;
            }
        }
    }
}
// 0605918E
static void a5WormObjectSystem_Draw(sA5WormObjectSystem* p)
{
    for (s32 i = 0; i < p->m3728_drawCount; i++)
    {
        s16 idx = p->m190_sortedIndices[i];
        sA5WormParticle& particle = p->m318_particles[idx];
        drawProjectedParticle(&particle.m0_quad, &particle.m8_position);
    }
}

// 060591de
void createA5_wormObjectTask(p_workArea parent)
{
    static sA5WormObjectSystem::TypedTaskDefinition td = { &a5WormObjectSystem_Init, &a5WormObjectSystem_Update, &a5WormObjectSystem_Draw, nullptr };
    createSubTask<sA5WormObjectSystem>(parent, &td);
}

// 06058514 — exit entity init: sets up orbit params and stores self in field data
static void a5ExitEntity_Init(sA5ExitEntity* p)
{
    p->m30_radius = 0xAAA;
    p->m35 = 0;
    p->m36 = 0;
    p->m18 = 0;
    p->m0 = 0;
    p->m4_angle = 0x1C71C71;
    p->m8 = 0;

    u32 angleIdx = ((u32)p->m4_angle >> 16) & 0xFFF;
    p->mC_direction.m0_X = MTH_Mul(fixedPoint(p->m18), getSin(angleIdx));
    p->mC_direction.m4_Y = 0;
    p->mC_direction.m8_Z = MTH_Mul(fixedPoint(p->m18), getCos(angleIdx));

    p->m34 = 0;

    s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
    pFieldData->m4_pExitEntity = p;
}

// 060586a0 — wind state machine: manages wind speed transitions
static void a5ExitEntity_WindStateMachine(sA5ExitEntity* p)
{
    switch (p->m35)
    {
    case 0: // calm
        p->m30_radius = 0x800;
        p->m36--;
        if (p->m36 < 1)
        {
            p->m36 = 0;
            if ((randomNumber() & 0xFFFF) < 0x3334)
            {
                p->m36 = 0x78;
                p->m35 = 1;
            }
        }
        break;
    case 1: // moderate
        p->m30_radius = 0x1000;
        p->m36--;
        if (p->m36 < 1)
        {
            p->m36 = 0;
            if ((randomNumber() & 0xFFFF) < 0x7AF)
            {
                p->m35 = 2;
            }
            else if ((randomNumber() & 0xFFFF) < 0x199A)
            {
                p->m36 = 0x78;
                p->m35 = 0;
            }
        }
        break;
    case 2: // ramp up
        p->m30_radius = 0x2AAA;
        p->m35 = 3;
        break;
    case 3: // strong — wait for speed to reach threshold
        if (p->m18 > 0x27FF)
        {
            playSystemSoundEffect(0x67);
            p->m36 = 0x5A;
            p->m35 = 4;
        }
        break;
    case 4: // cooldown
        p->m36--;
        if (p->m36 < 1)
        {
            p->m36 = 0x96;
            p->m35 = 1;
        }
        break;
    default:
        p->m35 = 0;
        break;
    }
}

// 060587A8 — exit mode 0: wind particles near camera
static void a5ExitEntity_Mode0(sA5ExitEntity* p)
{
    a5ExitEntity_WindStateMachine(p);
    Unimplemented(); // particle spawning near camera position based on wind speed
}

// 0605887C — exit mode 1: directional particles near exit position
static void a5ExitEntity_Mode1(sA5ExitEntity* p)
{
    a5ExitEntity_WindStateMachine(p);
    Unimplemented(); // particle spawning near exit target position
}

// 06058944 — exit mode 3: periodic particle spawning
static void a5ExitEntity_Mode3(sA5ExitEntity* p)
{
    p->m34++;
    if (p->m34 > 8)
    {
        p->m34 = 0;
        Unimplemented(); // spawn particle near camera with height from m24_exitTargetX_mode3
    }
}

// 0605857E
static void a5ExitEntity_Update(sA5ExitEntity* p)
{
    // Debug display (only when debug flags active)
    if ((getFieldTaskPtr()->m8_pSubFieldData->m370_fieldDebuggerWho & 4) != 0
        && getFieldTaskPtr()->m8_pSubFieldData->m37E_debugMenuStatus2_a == 0
        && getFieldTaskPtr()->m8_pSubFieldData->m369 == 0)
    {
        // Debug wind info display — skipped in non-debug builds
    }

    // Dispatch to exit mode handler
    switch (p->m2C_exitMode)
    {
    case 0: a5ExitEntity_Mode0(p); break;
    case 1: a5ExitEntity_Mode1(p); break;
    case 2: break; // 06058940 — empty
    case 3: a5ExitEntity_Mode3(p); break;
    default: break;
    }

    // Interpolate approach speed toward target radius
    p->m18 = p->m18 + ((p->m30_radius - p->m18) >> 4);

    // Recompute direction vector from angle
    u32 angleIdx = ((u32)p->m4_angle >> 16) & 0xFFF;
    p->mC_direction.m0_X = MTH_Mul(fixedPoint(p->m18), getSin(angleIdx));
    p->mC_direction.m8_Z = MTH_Mul(fixedPoint(p->m18), getCos(angleIdx));
}

// 060589be
void createA5_exitEntityTask(p_workArea parent)
{
    static sA5ExitEntity::TypedTaskDefinition td = { &a5ExitEntity_Init, &a5ExitEntity_Update, nullptr, nullptr };
    createSubTask<sA5ExitEntity>(parent, &td);
}

// 060584ca — set field exit configuration
void setFieldExitConfig_A5(s32 mode, s32* pParams)
{
    s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
    sA5ExitEntity* pExit = pFieldData->m4_pExitEntity;
    if (mode < 4)
    {
        pExit->m2C_exitMode = mode;
        if (mode == 1)
        {
            pExit->m1C_exitTargetX_mode1 = pParams[0];
            pExit->m20_exitTargetZ_mode1 = pParams[2];
        }
        else if (mode == 3)
        {
            pExit->m24_exitTargetX_mode3 = pParams[0];
            pExit->m28_exitTargetZ_mode3 = pParams[1];
        }
    }
}

// Corridor worm subtask struct (size 0xC)
struct sA5CorridorWormSubtask : public s_workAreaTemplate<sA5CorridorWormSubtask>
{
    s32 m0;
    s32 m4;
    s32 m8;
    // size 0xC
};

// 0605930c — corridor 1 worm update
static void corridorWormUpdate_1(sA5CorridorWormSubtask* pThis) { Unimplemented(); }
// 0605947a — corridor 3 worm update
static void corridorWormUpdate_3(sA5CorridorWormSubtask* pThis) { Unimplemented(); }
// 060595b4 — corridor 5 worm update
static void corridorWormUpdate_5(sA5CorridorWormSubtask* pThis) { Unimplemented(); }
// 0605962c — corridor 6 worm update
static void corridorWormUpdate_6(sA5CorridorWormSubtask* pThis) { Unimplemented(); }

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

// Decor/sound emitter task struct (size 0xF0, init-only)
struct sA5DecorEmitter : public s_workAreaTemplateWithArg<sA5DecorEmitter, sSaturnPtr>
{
    u8 m0_data[0xF0];
    // size 0xF0
};

// 06084338
static void a5DecorEmitter_Init(sA5DecorEmitter* pThis, sSaturnPtr arg) { Unimplemented(); }

// 0607938c — moved to shared field.cpp as findGridParentForEntity

// 06084de2 — create one decor/sound emitter at grid position
static void a5CreateDecorEmitter(sSaturnPtr pData)
{
    static sA5DecorEmitter::TypedTaskDefinition td = { &a5DecorEmitter_Init, nullptr, nullptr, nullptr };
    p_workArea parent = findGridParentForEntity(pData);
    createSubTaskWithArg<sA5DecorEmitter>(parent, pData, &td);
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

// 060596a4 — corridor A worm update
static void corridorWormUpdate_A(sA5CorridorWormSubtask* pThis) { Unimplemented(); }

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
// getDragonPosition_A5, getDragonAngle_A5, triggerSubfieldChange_A5 moved to shared field.cpp
// as getDragonPosition, getDragonAngle, triggerSubfieldChange

// --- Shared open-area environment/entity creation functions ---

// Worm dust particle spawner (size 4)
struct sA5WormDustTask : public s_workAreaTemplate<sA5WormDustTask>
{
    s32 m0_counter;
    // size 0x4
};

// 060589f4
// 060589f4
static void a5WormDustUpdate(sA5WormDustTask* pThis)
{
    sVec3_FP dragonPos;
    getDragonPosition(&dragonPos);

    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    s32 speedVal = pDragon->m154_dragonSpeed.asS32() * 0x1E;
    s16 speedShift = (s16)(speedVal >> 0x11);

    if (dragonPos.m4_Y.asS32() < 0x19000)
    {
        u32 threshold = rotateRightR0ByR1((u32)dragonPos.m4_Y.asS32(), speedShift + 0xC);
        pThis->m0_counter++;
        if ((s32)threshold <= pThis->m0_counter)
        {
            pThis->m0_counter = 0;

            sVec3_FP dragonAngle;
            getDragonAngle(&dragonAngle);

            // Offset spawn position behind dragon
            s32 offset = speedVal >> 3;
            u32 angleIdx = ((u32)dragonAngle.m4_Y.asS32() >> 16) & 0xFFF;
            dragonPos.m0_X = fixedPoint(dragonPos.m0_X.asS32() - MTH_Mul(fixedPoint(offset), getSin(angleIdx)).asS32());
            dragonPos.m8_Z = fixedPoint(dragonPos.m8_Z.asS32() - MTH_Mul(fixedPoint(offset), getCos(angleIdx)).asS32());

            Unimplemented(); // FUN_FLD_A5__06058ae0 — spawn dust particle (needs worm system initialized)
        }
    }
}

// 06058ad8 — create worm dust particle spawner (used by subfields 0, 4, 7, 9)
void createA5_wormDustTask(p_workArea parent)
{
    createSubTaskFromFunction<sA5WormDustTask>(parent, &a5WormDustUpdate);
}

// Worm segment entity (Saturn size 0xAC, with arg)
struct sA5WormSegmentEntity : public s_workAreaTemplateWithArg<sA5WormSegmentEntity, sSaturnPtr>
{
    s_memoryAreaOutput m0_memoryArea;
    sSaturnPtr m8_dataPtr;
    sFieldModelRenderContext mC_modelCtx;
    s_3dModel m40_3dModel;
    sVec3_FP m90_normal;
    s32 m9C_config;
    s32 mA0_state;
    s32 mA4_pad;
    s16 mA8_modelIdx;
    s16 mAA_poseIdx;
    // Saturn size 0xAC
};

// 06056EFC — worm segment draw (alternate, when model loaded)
static void a5WormSegmentEntity_DrawWithModel(sA5WormSegmentEntity* pThis) { Unimplemented(); }

// 06056A9A
static void a5WormSegmentEntity_Init(sA5WormSegmentEntity* pThis, sSaturnPtr arg)
{
    s16 areaIdx = readSaturnS16(arg + 0x1E);
    getMemoryArea(&pThis->m0_memoryArea, areaIdx);
    pThis->m8_dataPtr = arg;
    pThis->m9C_config = readSaturnS16(arg + 0x20);

    // If data has a 3D model index
    s16 modelOffset = readSaturnS16(arg + 0x5A);
    if (modelOffset != 0)
    {
        s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
        sStaticPoseData* pPose = pBundle->getStaticPose(readSaturnS16(arg + 0x5C), pBundle->getModelHierarchy(modelOffset)->countNumberOfBones());
        init3DModelRawData(pThis, &pThis->m40_3dModel, 0, pBundle, modelOffset, nullptr, pPose, nullptr, nullptr);
        stepAnimation(&pThis->m40_3dModel);
        pThis->m_DrawMethod = &a5WormSegmentEntity_DrawWithModel;

        // Check game state bit from data
        u32 bitIdx = (u32)readSaturnS16(arg + 0x5E);
        u32 adjIdx = (bitIdx < 1000) ? bitIdx : (bitIdx - 0x236);
        if ((mainGameState.bitField[adjIdx >> 3] & (1 << (bitIdx & 7))) == 0)
        {
            pThis->mA8_modelIdx = readSaturnS16(arg + 0x18);
            pThis->mAA_poseIdx = readSaturnS16(arg + 0x1A);
            pThis->mA0_state = 1;
        }
        else
        {
            pThis->mA8_modelIdx = readSaturnS16(arg + 0x56);
            pThis->mAA_poseIdx = readSaturnS16(arg + 0x58);
            pThis->mA0_state = 2;
        }
    }

    // Set up model render context
    s16 rotFlag = readSaturnS16(arg + 0x26);
    if (rotFlag == 0)
    {
        initFieldModelRenderContext(&pThis->mC_modelCtx, pThis, nullptr,
            (sVec3_FP*)getSaturnPtr(arg), nullptr, 3, 0, -1, 0, 0);
    }
    else
    {
        // Compute rotated normal vector
        sMatrix4x3 rotMatrix;
        initMatrixToIdentity(&rotMatrix);
        rotateMatrixShiftedY(fixedPoint(readSaturnS32(arg + 0x10)), &rotMatrix);
        transformVec(*(sVec3_FP*)getSaturnPtr(gFLD_A5->getSaturnPtr(0x06098AE4)), pThis->m90_normal, rotMatrix);
        initFieldModelRenderContext(&pThis->mC_modelCtx, pThis, nullptr,
            (sVec3_FP*)getSaturnPtr(arg), &pThis->m90_normal, 3, 0, -1, 0, 0);
    }
}

// 06056C16
static void a5WormSegmentEntity_Update(sA5WormSegmentEntity* pThis)
{
    sSaturnPtr data = pThis->m8_dataPtr;
    s32 state = pThis->m9C_config;

    if (state == -1)
    {
        if (readSaturnS16(data + 0x24) == 0)
            return;
        Unimplemented(); // FUN_FLD_A5__0607a2cc — proximity check
        return;
    }
    if (state == 0)
    {
    {
        // Check activation game state bits
        bool activated = true;
        s16 bit1 = readSaturnS16(data + 0x22);
        if (bit1 != 0)
        {
            u32 adj1 = (bit1 < 1000) ? (u32)bit1 : (u32)(bit1 - 0x236);
            if ((mainGameState.bitField[adj1 >> 3] & (1 << (bit1 & 7))) == 0)
                activated = false;
        }
        if (activated)
        {
            s16 bit2 = readSaturnS16(data + 0x5E);
            if (bit2 != 0)
            {
                u32 adj2 = (bit2 < 1000) ? (u32)bit2 : (u32)(bit2 - 0x236);
                if ((mainGameState.bitField[adj2 >> 3] & (1 << (bit2 & 7))) == 0)
                    activated = false;
            }
        }
        if (activated)
            pThis->m9C_config++;
    }
        if (readSaturnS16(data + 0x24) != 0)
        {
            Unimplemented(); // FUN_FLD_A5__0607a2cc — proximity check
        }
        return;
    }
    if (state == 1)
    {
        updateFieldModelRenderContext(&pThis->mC_modelCtx);
        if (readSaturnS16(data + 0x24) != 0)
        {
            Unimplemented(); // FUN_FLD_A5__0607a2cc — proximity check
        }
        return;
    }
    if (state == 2)
    {
        Unimplemented(); // dragon autopilot setup through worm tunnel
        pThis->m9C_config++;
        return;
    }
    if (state == 3)
    {
        pThis->mA4_pad--;
        if (pThis->mA4_pad > 0)
            return;
        // Trigger subfield change
        s16 dest = readSaturnS16(data + 0x28);
        if (dest < 0)
        {
            Unimplemented(); // tutorial/special handling
        }
        else
        {
            triggerSubfieldChange(dest, -0x8000);
            playBattleSoundEffect(100);
            playBattleSoundEffect(0x65);
        }
        pThis->m9C_config++;
        return;
    }
}

// 06056EFC
static void a5WormSegmentEntity_Draw(sA5WormSegmentEntity* pThis)
{
    sSaturnPtr data = pThis->m8_dataPtr;
    if (readSaturnS16(data + 0x18) == 0)
        return;

    pushCurrentMatrix();

    sVec3_FP pos = readSaturnVec3(data);
    translateCurrentMatrix(&pos);

    sVec3_FP rot = { fixedPoint(readSaturnS32(data + 0xC)),
                     fixedPoint(readSaturnS32(data + 0x10)),
                     fixedPoint(readSaturnS32(data + 0x14)) };
    rotateCurrentMatrixZYX(&rot);

    Unimplemented(); // FUN_FLD_A5__06078fa6 LOD check + addObjectToDrawList + FUN_FLD_A5__06077304

    popMatrix();
}

// Shared: create one worm segment entity with specific data
void createA5_wormSegment(p_workArea parent, sSaturnPtr data)
{
    static sA5WormSegmentEntity::TypedTaskDefinition td = { &a5WormSegmentEntity_Init, &a5WormSegmentEntity_Update, &a5WormSegmentEntity_Draw, nullptr };
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, data, &td);
}

// 06057066 — create 2 worm segment entities (subfields 0, 7)
void createA5_wormSegments_day(p_workArea parent)
{
    static sA5WormSegmentEntity::TypedTaskDefinition td = { &a5WormSegmentEntity_Init, &a5WormSegmentEntity_Update, &a5WormSegmentEntity_Draw, nullptr };
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, gFLD_A5->getSaturnPtr(0x06098AF0), &td);
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, gFLD_A5->getSaturnPtr(0x06098B50), &td);
}

// 0605708e — create 3 worm segment entities (subfields 2, 8)
void createA5_wormSegments_night(p_workArea parent)
{
    static sA5WormSegmentEntity::TypedTaskDefinition td = { &a5WormSegmentEntity_Init, &a5WormSegmentEntity_Update, &a5WormSegmentEntity_Draw, nullptr };
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, gFLD_A5->getSaturnPtr(0x06098C10), &td);
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, gFLD_A5->getSaturnPtr(0x06098BB0), &td);
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, gFLD_A5->getSaturnPtr(0x06098C70), &td);
}

// Proximity alert task (size 8, update-only)
struct sA5ProximityAlertTask : public s_workAreaTemplate<sA5ProximityAlertTask>
{
    s32 m0;
    s32 m4;
    // size 0x8
};

// 06067FDC
static void a5ProximityAlertUpdate_day(sA5ProximityAlertTask* pThis) { Unimplemented(); }

// 06068182 — create proximity alert task for day subfields (0, 7)
void createA5_proximityAlert_day(p_workArea parent)
{
    static sA5ProximityAlertTask::TypedTaskDefinition td = { nullptr, &a5ProximityAlertUpdate_day, nullptr, nullptr };
    createSubTask<sA5ProximityAlertTask>(parent, &td);
}

// 0606811E
static void a5ProximityAlertUpdate_night(sA5ProximityAlertTask* pThis) { Unimplemented(); }

// 0606818a — create proximity alert task for night subfields (2, 8)
void createA5_proximityAlert_night(p_workArea parent)
{
    static sA5ProximityAlertTask::TypedTaskDefinition td = { nullptr, &a5ProximityAlertUpdate_night, nullptr, nullptr };
    createSubTask<sA5ProximityAlertTask>(parent, &td);
}

// Field event check task (size 4)
struct sA5FieldEventCheckTask : public s_workAreaTemplate<sA5FieldEventCheckTask>
{
    s16 m0_timer;
    s16 m2_flags;
    // size 0x4
};

// 060656F8 — empty init
static void a5FieldEventCheckInit(sA5FieldEventCheckTask* pThis)
{
    // empty — confirmed from Ghidra
}

// 060656FC — check if dragon is inside a triangle defined by two data points (XZ plane)
static s32 a5CheckProximityToPoint(sSaturnPtr pData)
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

    // Read two triangle vertices from Saturn data (each is sVec3_FP, only X and Z used)
    s32 v0x = readSaturnS32(pData);
    s32 v0z = readSaturnS32(pData + 8);
    s32 v1x = readSaturnS32(pData + 12);
    s32 v1z = readSaturnS32(pData + 20);

    // Reference point from dragon rotation
    s32 refX = pDragon->m14_oldPos.m0_X.asS32();
    s32 refZ = pDragon->m14_oldPos.m8_Z.asS32();

    // Triangle edges relative to reference
    s32 e0x = v0x - refX;
    s32 e0z = v0z - refZ;
    s32 e1x = v1x - refX;
    s32 e1z = v1z - refZ;

    // Dragon position relative to reference
    s32 px = pDragon->m8_pos.m0_X.asS32() - refX;
    s32 pz = pDragon->m8_pos.m8_Z.asS32() - refZ;

    // Cross product for barycentric coordinates
    s32 cross = MTH_Mul(fixedPoint(e0x), fixedPoint(e1z)).asS32()
              - MTH_Mul(fixedPoint(e0z), fixedPoint(e1x)).asS32();

    s32 u = FP_Div(
        MTH_Mul(fixedPoint(e1z), fixedPoint(px)).asS32() - MTH_Mul(fixedPoint(e1x), fixedPoint(pz)).asS32(),
        fixedPoint(cross)).asS32();

    s32 v = FP_Div(
        MTH_Mul(fixedPoint(e0x), fixedPoint(pz)).asS32() - MTH_Mul(fixedPoint(e0z), fixedPoint(px)).asS32(),
        fixedPoint(cross)).asS32();

    if (u < 0 || v < 0 || (u + v) < 0x10000)
    {
        return 0;
    }
    return 1;
}

// 0606580C — field event check: detects dragon in specific zone and sets game flag
static void a5FieldEventCheckUpdate(sA5FieldEventCheckTask* pThis)
{
    s16 prevFlags = pThis->m2_flags;

    sVec3_FP dragonPos;
    getDragonPosition(&dragonPos);

    // Check if dragon is in the detection zone
    if (dragonPos.m4_Y.asS32() < 0x2E000
        && dragonPos.m0_X.asS32() > 0x100000 && dragonPos.m0_X.asS32() < 0x300000
        && dragonPos.m8_Z.asS32() > -0x400000 && dragonPos.m8_Z.asS32() < -0x200000)
    {
        if (a5CheckProximityToPoint(gFLD_A5->getSaturnPtr(0x0609D548)) != 0)
        {
            pThis->m2_flags |= 1;
        }
        if (a5CheckProximityToPoint(gFLD_A5->getSaturnPtr(0x0609D560)) != 0)
        {
            pThis->m2_flags |= 2;
        }
        if (a5CheckProximityToPoint(gFLD_A5->getSaturnPtr(0x0609D578)) != 0)
        {
            pThis->m2_flags |= 4;
        }
    }

    // Start countdown when first condition detected
    if (prevFlags == 0 && pThis->m2_flags != 0)
    {
        pThis->m0_timer = 0x96;
    }

    // Decrement timer
    pThis->m0_timer--;
    if (pThis->m0_timer < 0)
    {
        pThis->m0_timer = 0;
        pThis->m2_flags = 0;
    }

    // If all 3 flags set within timer window, set game state bit
    if (pThis->m2_flags == 7 && pThis->m0_timer != 0)
    {
        mainGameState.bitField[0xA2] |= 8;
    }
}

// 060658c4 — create field event check task (subfields 0, 7)
void createA5_fieldEventCheck(p_workArea parent)
{
    static sA5FieldEventCheckTask::TypedTaskDefinition td = { &a5FieldEventCheckInit, &a5FieldEventCheckUpdate, nullptr, nullptr };
    createSubTask<sA5FieldEventCheckTask>(parent, &td);
}

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

// 0605a190 — create 4 worm hole environment objects (subfield 0)
void createA5_envObjects_sub0(p_workArea parent)
{
    Unimplemented(); // calls FUN_FLD_A5__06059f1c 4 times with data from 06099de8..06099e54
}

// 06056970 — create exit trigger entity (subfield 0)
void createA5_envObjects_sub0_exits(p_workArea parent)
{
    Unimplemented(); // calls FUN_FLD_A5__06056870 with {2, 4, 0x538, 0x2AC, 8, 0x53C, 0x2B0}
}

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

// 060601B2
static void a5LightEntity_Update(sA5LightEntity* p)
{
    if (p->m0 == 0
        && (mainGameState.bitField[0xA3] & 0x40) == 0
        && (mainGameState.bitField[0x96] & 8) != 0
        && (mainGameState.bitField[0x96] & 4) != 0)
    {
        startFieldScript(10, 0x5BF);
        Unimplemented(); // creates subtask (task def at 0609d488, size 0x4C) + FUN_FLD_A5__060720c6
        p->m0++;
    }
}

// 06060220
void createA5_envObjects_sub4_light(p_workArea parent)
{
    static sA5LightEntity::TypedTaskDefinition td = { &a5LightEntity_Init, &a5LightEntity_Update, nullptr, nullptr };
    createSubTask<sA5LightEntity>(parent, &td);
}

// 060570c2 — create 2 worm segment entities (subfield 4)
void createA5_envObjects_sub4_wormSegments(p_workArea parent)
{
    static sA5WormSegmentEntity::TypedTaskDefinition td = { &a5WormSegmentEntity_Init, &a5WormSegmentEntity_Update, &a5WormSegmentEntity_Draw, nullptr };
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, gFLD_A5->getSaturnPtr(0x06098CD0), &td);
    createSubTaskWithArg<sA5WormSegmentEntity>(parent, gFLD_A5->getSaturnPtr(0x06098D30), &td);
}

// 06059758 — create corridor worm subtask for subfield 2/8
void createA5_corridorWorm_2(p_workArea parent)
{
    createSubTaskFromFunction<sA5CorridorWormSubtask>(parent, &corridorWormUpdate_3); // uses shared update stub
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

// Sandfall entity (Saturn size 0x70)
struct sA5SandfallEntity : public s_workAreaTemplate<sA5SandfallEntity>
{
    s_memoryAreaOutput m0_memoryArea;
    sVec3_FP m8_position;
    sFieldModelRenderContext m14_modelCtx;
    sVec3_FP m48_renderPosition;
    s32 m54;
    s32 m58;
    s32 m5C_animState;
    s32 m60;
    s32 m64;
    s32 m68;
    u8 m6C_flag0;
    u8 m6D_flag1;
    u8 m6E_state;
    // Saturn size 0x70
};

// Sandfall subtask (size 0xC)
struct sA5SandfallSubtask : public s_workAreaTemplate<sA5SandfallSubtask>
{
    s32 m0;
    s32 m4;
    sVec3_FP* m8_pPosition;
    // size 0xC
};
static void a5SandfallSubtask_Update(sA5SandfallSubtask* p) { Unimplemented(); } // 0605A1D8

// 0605A2A8
static void a5SandfallEntity_Init(sA5SandfallEntity* p)
{
    getMemoryArea(&p->m0_memoryArea, 3);
    p->m8_position.m0_X = fixedPoint(0x500000);
    p->m8_position.m4_Y = 0;
    p->m8_position.m8_Z = fixedPoint(-0x600000);
    p->m6C_flag0 = 1;
    p->m54 = 0;
    p->m58 = 0;
    p->m5C_animState = 2;
    p->m60 = 0;
    p->m64 = 0;
    p->m68 = 0;

    sA5SandfallSubtask* pSub = createSubTaskFromFunction<sA5SandfallSubtask>((p_workArea)p, &a5SandfallSubtask_Update);
    if (pSub)
        pSub->m8_pPosition = &p->m8_position;

    if ((mainGameState.bitField[0x95] & 2) == 0)
    {
        p->m6D_flag1 = 1;
    }

    p->m48_renderPosition = p->m8_position;
    initFieldModelRenderContext(&p->m14_modelCtx, p, nullptr,
        &p->m48_renderPosition, nullptr, 0, 0, -1, 0xFF, 0);
    p->m14_modelCtx.m18_visibilityFlags |= 1;

    if ((mainGameState.bitField[0x96] & 0x40) == 0)
    {
        p->m14_modelCtx.m18_visibilityFlags = 0;
        p->m6E_state = 0;
    }
    else if ((mainGameState.bitField[0x96] & 0x20) == 0)
    {
        p->m6E_state = 1;
    }
    else
    {
        mainGameState.bitField[0x95] |= 2;
    }
}

static void a5SandfallEntity_Update(sA5SandfallEntity* p) { Unimplemented(); } // 0605A476
static void a5SandfallEntity_Draw(sA5SandfallEntity* p) { Unimplemented(); } // 0605A52E

// 0605a68a
void createA5_envObjects_sub2_sandfall(p_workArea parent)
{
    static sA5SandfallEntity::TypedTaskDefinition td = { &a5SandfallEntity_Init, &a5SandfallEntity_Update, &a5SandfallEntity_Draw, nullptr };
    createSubTask<sA5SandfallEntity>(parent, &td);
}

// Sky entity (size 0x120, with arg)
struct sA5SkyEntity : public s_workAreaTemplateWithArg<sA5SkyEntity, sSaturnPtr>
{
    s_memoryAreaOutput m0_memoryArea;
    sFieldModelRenderContext m8_modelCtx;
    sVec3_FP m3C_renderPosition;
    s_3dModel m48_model1;
    s_3dModel m98_model2;
    sVec3_FP mE8_position;
    s32 mF4;
    sVec3_FP mF8_rotation;
    s32 m104;
    s32 m108;
    s32 m10C;
    s32 m110;
    s32 m114;
    u8 m118_pad[4];
    u8 m11C_flag;
    // Saturn size 0x120
};
// 0605ABD4
static void a5SkyEntity_Init(sA5SkyEntity* p, sSaturnPtr arg)
{
    getMemoryArea(&p->m0_memoryArea, 3);
    s_fileBundle* pBundle = p->m0_memoryArea.m0_mainMemoryBundle;

    // Copy position and rotation from Saturn arg
    p->mE8_position = readSaturnVec3(arg);
    p->mF8_rotation.m0_X = fixedPoint(readSaturnS32(arg + 12));
    p->mF8_rotation.m4_Y = fixedPoint(readSaturnS32(arg + 16));
    p->mF8_rotation.m8_Z = fixedPoint(readSaturnS32(arg + 20));

    p->mF4 = 0;
    p->m104 = 0;
    p->m108 = 0;
    p->m10C = 0;
    p->m110 = 0;
    p->m114 = 0;
    p->m11C_flag = 0;

    // Init primary 3D model at model offset 0x1C
    sStaticPoseData* pPose1 = pBundle->getStaticPose(0xB54, pBundle->getModelHierarchy(0x1C)->countNumberOfBones());
    init3DModelRawData(p, &p->m48_model1, 0, pBundle, 0x1C, nullptr, pPose1, nullptr, nullptr);
    stepAnimation(&p->m48_model1);

    if ((mainGameState.bitField[0x96] & 0x20) == 0)
    {
        // Init secondary 3D model at model offset 0x14
        sStaticPoseData* pPose2 = pBundle->getStaticPose(0xB4C, pBundle->getModelHierarchy(0x14)->countNumberOfBones());
        init3DModelRawData(p, &p->m98_model2, 0, pBundle, 0x14, nullptr, pPose2, nullptr, nullptr);
        stepAnimation(&p->m98_model2);
    }
    else
    {
        Unimplemented(); // FUN_FLD_A5__0605aef4 alternate setup + change update method
    }

    // Render position = world position + Y offset
    p->m3C_renderPosition.m0_X = p->mE8_position.m0_X;
    p->m3C_renderPosition.m4_Y = fixedPoint(p->mE8_position.m4_Y.asS32() + 0x21000);
    p->m3C_renderPosition.m8_Z = p->mE8_position.m8_Z;

    initFieldModelRenderContext(&p->m8_modelCtx, p, nullptr,
        &p->m3C_renderPosition, nullptr, 2, 0, -1, 0, 0);
}
static void a5SkyEntity_Update(sA5SkyEntity* p) { Unimplemented(); } // 0605ADB6
static void a5SkyEntity_Draw(sA5SkyEntity* p) { Unimplemented(); } // 0605AD24

// 0605abc4
void createA5_envObjects_sub2_skyEntity(p_workArea parent, sSaturnPtr arg)
{
    static sA5SkyEntity::TypedTaskDefinition td = { &a5SkyEntity_Init, &a5SkyEntity_Update, &a5SkyEntity_Draw, nullptr };
    createSubTaskWithArg<sA5SkyEntity>(parent, arg, &td);
}

// Weather/fog task (size 0xD0)
struct sA5WeatherTask : public s_workAreaTemplate<sA5WeatherTask>
{
    s_memoryAreaOutput m0_memoryArea;
    sAnimatedQuad m8_quad;
    s32 m10_velocities[0x2F]; // 47 random velocity offsets (0x10..0xCC)
    s32 mCC_state;
    // Saturn size 0xD0
};
// 0605BD08
static void a5WeatherTask_Init(sA5WeatherTask* p)
{
    getMemoryArea(&p->m0_memoryArea, 3);
    u16 vdp1Mem = (u16)((p->m0_memoryArea.m4_characterArea - 0x25C00000) >> 3);
    Unimplemented(); // particleInitSub(&p->m8_quad, vdp1Mem, quadData at 0609A25C)

    p->mCC_state = 0;
    for (s32 i = 0; i < 0x2F; i++)
    {
        p->m10_velocities[i] = (s32)(randomNumber() & 0x1F) - 0x10;
    }
}
static void a5WeatherTask_Update(sA5WeatherTask* p) { Unimplemented(); } // 0605BDF4
static void a5WeatherTask_Draw(sA5WeatherTask* p) { Unimplemented(); } // 0605BD70

// 0605c006
void createA5_envObjects_sub2_weatherTask(p_workArea parent)
{
    static sA5WeatherTask::TypedTaskDefinition td = { &a5WeatherTask_Init, &a5WeatherTask_Update, &a5WeatherTask_Draw, nullptr };
    createSubTask<sA5WeatherTask>(parent, &td);
}

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

// 0607906a / 060790e4 — moved to shared field.cpp as isPointOnScreen / isWorldPositionOnScreen

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
