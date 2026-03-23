#include "PDS.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "a3_2_crashedImperialShip.h"
#include "items.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup
#include "field/fieldDebrisScatter.h"
#include "kernel/vdp1AnimatedQuad.h"
#include "audio/systemSounds.h"

#include "particlePool.h"
s32 playBattleSoundEffect(s32 effectIndex); // TODO: move to header

// 06059584 — random ambient sound at crashed ship position
struct sCrashedShipAmbientSound : public s_workAreaTemplate<sCrashedShipAmbientSound>
{
    static void Update(sCrashedShipAmbientSound* pThis)
    {
        if (!checkPositionVisibilityAgainstFarPlane(&pThis->m0_position))
        {
            if ((randomNumber() & 0xF) == 0)
            {
                playBattleSoundEffect(0x6D);
            }
        }
    }
    sVec3_FP m0_position;
    // size: 0xC
};

// 060595b4
void createCrashedShipAmbientSoundTask(p_workArea parent, sVec3_FP* position)
{
    sCrashedShipAmbientSound* pTask = createSubTaskFromFunction<sCrashedShipAmbientSound>(
        parent, &sCrashedShipAmbientSound::Update);
    if (pTask)
    {
        pTask->m0_position = *position;
    }
}

// 0605A046 — periodic explosion particle spawner on the crashed ship
struct sCrashedShipExplosionSpawner : public s_workAreaTemplate<sCrashedShipExplosionSpawner>
{
    static void Update(sCrashedShipExplosionSpawner* pThis)
    {
        if (pThis->m0_timer == 0)
        {
            // Cycle through explosion points
            pThis->m4_pointIndex++;
            if (pThis->m4_pointIndex > 6)
                pThis->m4_pointIndex = 0;

            // Spawn particle at the current explosion point (if under max count)
            if (pThis->m8_perPointCount[pThis->m4_pointIndex] < 3)
            {
                // Read explosion point position from data table
                sSaturnPtr pointData = gFLD_A3->getSaturnPtr(0x06090644) + pThis->m4_pointIndex * 0xC;
                sVec3_FP pos;
                pos.m0_X = readSaturnS32(pointData) + (s32)(randomNumber() & 0x3FFF) - 0x2000;
                pos.m4_Y = readSaturnS32(pointData + 4);
                pos.m8_Z = readSaturnS32(pointData + 8) + (s32)(randomNumber() & 0x3FFF) - 0x2000;

                sVec3_FP velocity = { 0, 0, 0 };
                sParticleSpawnConfig config;
                config.m0_pPosition = &pos;
                config.m4_pVelocity = &velocity;
                config.m8_pQuadData = &gFLD_A3->m_explosionQuad;
                config.mC_velocityScaleX = 0x8000;
                config.m10_velocityScaleY = -0x10000;
                config.m14_updateFunc = &particleUpdateStatic;
                config.m18_heapSize = 0;
                config.m1C_heapData = nullptr;

                s_fieldSpecificData_A3* pFieldData = getFieldSpecificData_A3();
                if (pFieldData->m168)
                {
                    spawnParticleInPool((sParticlePoolManager*)pFieldData->m168, &config, 1);
                }
                pThis->m8_perPointCount[pThis->m4_pointIndex]++;
            }

            pThis->m0_timer = (randomNumber() & 0xF) + 6;
        }
        else
        {
            pThis->m0_timer--;
        }
    }
    s32 m0_timer;
    s32 m4_pointIndex;
    s8 m8_perPointCount[7];
    // size: 0x10
};

// 0605a17a
void createCrashedShipExplosionSpawner(p_workArea parent)
{
    sCrashedShipExplosionSpawner* pTask = createSubTaskFromFunction<sCrashedShipExplosionSpawner>(
        parent, &sCrashedShipExplosionSpawner::Update);
    if (pTask)
    {
        pTask->m0_timer = 0;
        memset(pTask->m8_perPointCount, 0, 7);
    }
}

// 06059e44 — spawn explosion particles at a position
static void spawnCrashedShipExplosion(sVec3_FP* position)
{
    sVec3_FP velocity = { 0, 0, 0 };
    sParticleSpawnConfig config;
    config.m0_pPosition = position;
    config.m4_pVelocity = &velocity;
    config.m8_pQuadData = &gFLD_A3->m_explosionQuad;
    config.mC_velocityScaleX = 0x10000;
    config.m10_velocityScaleY = -0x10000;
    config.m14_updateFunc = &particleUpdateMoving;
    config.m18_heapSize = 0;
    config.m1C_heapData = nullptr;

    s_fieldSpecificData_A3* pFieldData = getFieldSpecificData_A3();
    if (pFieldData->m168)
    {
        spawnParticleInPool((sParticlePoolManager*)pFieldData->m168, &config, 1);
    }
}

struct fieldA3_2_crashedImpertialShip : public s_workAreaTemplate<fieldA3_2_crashedImpertialShip>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, &fieldA3_2_crashedImpertialShip::Update, &fieldA3_2_crashedImpertialShip::Draw, NULL };
        return &taskDefinition;
    }

    static void Update(fieldA3_2_crashedImpertialShip* pThis)
    {
        stepAnimation(&pThis->m40_3dModel);
        switch (pThis->m3C_status)
        {
        case 0:
            if (getFieldTaskPtr()->m8_pSubFieldData->m370_fieldDebuggerWho & 2)
            {
                assert(0);
            }
            break;
        case 1: // recording activated
            pThis->m90_LCSTarget.m18_diableFlags |= 1;
            if (mainGameState.getBit(0xA * 8 + 7))
            {
                startFieldScript(16, 1452);
            }
            else
            {
                startFieldScript(15, 1451);
            }
            createCrashedShipExplosionSpawner(pThis);
            createCrashedShipAmbientSoundTask(pThis, &pThis->mC_position);
            pThis->m3C_status++;
            //fall
        case 2: // wait for end of script
            if (!fieldScriptTaskUpdateSub4())
            {
                pThis->m3C_status++;
            }
            break;
        case 3: // done
            break;
        default:
            assert(0);
            break;
        }

        updateLCSTarget(&pThis->m90_LCSTarget);
        pThis->m3D_visible = !checkPositionVisibilityAgainstFarPlane(&pThis->mC_position);
    }

    static void Draw(fieldA3_2_crashedImpertialShip* pThis)
    {
        if (pThis->m3D_visible)
        {
            pushCurrentMatrix();
            translateCurrentMatrix(pThis->mC_position);
            rotateCurrentMatrixZYX_s16(pThis->m24_rotation);
            pThis->m40_3dModel.m18_drawFunction(&pThis->m40_3dModel);
            callGridCellDraw_normalSub2(pThis->m0_memoryArea.m0_mainMemoryBundle, 1164);
            popMatrix();
        }
    }

    s_memoryAreaOutput m0_memoryArea;
    s_DataTable2Sub0* m8;
    sVec3_FP mC_position;
    sVec3_S16_12_4 m24_rotation;
    sVec3_FP m2C_LCSTargetLocation;
    s8 m3C_status;
    s8 m3D_visible;
    s_3dModel m40_3dModel;
    sLCSTarget m90_LCSTarget;
    // size 0xC4
};

// 06059712
void initCrashedShipDestruction(fieldA3_2_crashedImpertialShip* r4)
{
    // Set up debris scatter params from Ghidra data
    sVec3_FP rotation;
    rotation.m0_X = (s32)r4->m24_rotation[0] << 16;
    rotation.m4_Y = (s32)r4->m24_rotation[1] << 16;
    rotation.m8_Z = (s32)r4->m24_rotation[2] << 16;

    sDebrisScatterParams params;
    memset(&params, 0, sizeof(params));
    params.m0_gravity = 0x6E;
    params.m4_bounce = 0x4CCC;
    params.m8_spread = 0x1FFFFF;
    params.mC_randomMask = 0;
    params.m10_pPosition = &r4->mC_position;
    params.m14_pRotation = &rotation;
    params.m34_modelOffset = 0xE8;
    params.m36_poseOffset = 0x574;
    params.m38_groundY = -0x320000; // 0xFFCE0000
    params.m3C_scale = 0; // no scale
    params.m44_soundEffect = 0xFFFF; // no sound on first task
    params.m_pBundle = r4->m0_memoryArea.m0_mainMemoryBundle;

    createDebrisScatterTask(r4, &params, false);

    // Spawn explosion particles at two positions
    spawnCrashedShipExplosion(&r4->m2C_LCSTargetLocation);

    static sVec3_FP explosionPos2;
    explosionPos2 = readSaturnVec3(gFLD_A3->getSaturnPtr(0x060905E4));
    spawnCrashedShipExplosion(&explosionPos2);

    playSystemSoundEffect(0x6C);
}

void fieldA3_2_crashedImpertialShip_LCSCallback(p_workArea r4, sLCSTarget*)
{
    fieldA3_2_crashedImpertialShip* pThis = static_cast<fieldA3_2_crashedImpertialShip*>(r4);
    pThis->m3C_status = true;
}

void create_A3_2_crashedImperialShip(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    if (mainGameState.getBit(0x6E * 8 + 5))
        return;

    if (mainGameState.getBit(0x6E * 8 + 6))
        return;

    fieldA3_2_crashedImpertialShip* pNewTask = createSubTask<fieldA3_2_crashedImpertialShip>(r4);
    getMemoryArea(&pNewTask->m0_memoryArea, r6);

    pNewTask->m8 = &r5;
    pNewTask->mC_position = r5.m4_position;
    pNewTask->m24_rotation = r5.m10_rotation;
    pNewTask->m2C_LCSTargetLocation = pNewTask->mC_position + sVec3_FP(0, -0x3C000, 0xA000);

    s_fileBundle* pBundle = pNewTask->m0_memoryArea.m0_mainMemoryBundle;
    sAnimationData* pAnimation = pBundle->getAnimation(0x580);
    sStaticPoseData* pPose = pBundle->getStaticPose(readSaturnU16(r5.m1C_modelData + 2), pAnimation->m2_numBones);

    init3DModelRawData(pNewTask, &pNewTask->m40_3dModel, 0, pBundle, readSaturnS16(r5.m1C_modelData), pAnimation, pPose, nullptr, nullptr);
    stepAnimation(&pNewTask->m40_3dModel);

    createLCSTarget(&pNewTask->m90_LCSTarget, pNewTask, &fieldA3_2_crashedImpertialShip_LCSCallback, &pNewTask->m2C_LCSTargetLocation, nullptr, 0, 0, eItems::m8C_recording, 1, 0);
    pNewTask->m3C_status = 0;

    getFieldSpecificData_A3()->m164_A3_2_crashedImperialShipTask = pNewTask;
}

struct fieldA3_2_crashedImpertialShip2 : public s_workAreaTemplate<fieldA3_2_crashedImpertialShip2>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, &fieldA3_2_crashedImpertialShip2::Update, &fieldA3_2_crashedImpertialShip2::Draw, NULL };
        return &taskDefinition;
    }

    static void Update(fieldA3_2_crashedImpertialShip2* pThis)
    {
        stepAnimation(&pThis->m34_3dModel);
        pThis->m31_visible = !checkPositionVisibilityAgainstFarPlane(&pThis->mC_position);
    }

    static void Draw(fieldA3_2_crashedImpertialShip2* pThis)
    {
        if (pThis->m31_visible)
        {
            pushCurrentMatrix();
            translateCurrentMatrix(pThis->mC_position);
            rotateCurrentMatrixZYX_s16(pThis->m24_rotation);
            pThis->m34_3dModel.m18_drawFunction(&pThis->m34_3dModel);
            callGridCellDraw_normalSub2(pThis->m0_memoryArea.m0_mainMemoryBundle, 1136);
            popMatrix();
        }
    }

    s_memoryAreaOutput m0_memoryArea;
    s_DataTable2Sub0* m8;
    sVec3_FP mC_position;
    sVec3_S16_12_4 m24_rotation;
    s8 m30_status;
    s8 m31_visible;
    s_3dModel m34_3dModel;
    // size 0x84
};

// 06059be0 — smoke/steam particle spawner for the second crashed ship piece
struct sCrashedShip2SmokeSpawner : public s_workAreaTemplate<sCrashedShip2SmokeSpawner>
{
    static void Update(sCrashedShip2SmokeSpawner* pThis)
    {
        if ((pThis->m0_counter & 3) == 0)
        {
            pThis->m4_pointIndex++;
            if (pThis->m4_pointIndex > 1)
                pThis->m4_pointIndex = 0;

            // Read smoke position from data table
            sSaturnPtr pointData = gFLD_A3->getSaturnPtr(0x06090620) + pThis->m4_pointIndex * 0xC;
            sVec3_FP pos;
            pos.m0_X = readSaturnS32(pointData);
            pos.m4_Y = readSaturnS32(pointData + 4);
            pos.m8_Z = readSaturnS32(pointData + 8);

            sVec3_FP velocity;
            velocity.m0_X = (s32)(randomNumber() & 0x1FF) - 0x100;
            velocity.m4_Y = 0xB33;
            velocity.m8_Z = (s32)(randomNumber() & 0x1FF) - 0x100;

            sParticleSpawnConfig config;
            config.m0_pPosition = &pos;
            config.m4_pVelocity = &velocity;
            config.m8_pQuadData = &gFLD_A3->m_smokeQuad;
            // Random scale from table at 0x06090610
            static const fixedPoint scaleTable[] = { 0x8000, 0xC000, 0x10000, 0x6000 };
            config.mC_velocityScaleX = scaleTable[randomNumber() & 3];
            config.m10_velocityScaleY = -0x10000;
            config.m14_updateFunc = &particleUpdateMoving;
            config.m18_heapSize = 0;
            config.m1C_heapData = nullptr;

            s_fieldSpecificData_A3* pFieldData = getFieldSpecificData_A3();
            sParticlePoolManager* pPool = (sParticlePoolManager*)pFieldData->m168;
            if (pPool)
            {
                spawnParticleInPool(pPool, &config, 1);
            }
        }
        pThis->m0_counter++;
    }
    u32 m0_counter;
    u32 m4_pointIndex;
    // size: 8
};

// 06059ca4
void createCrashedShip2SmokeSpawner(p_workArea parent)
{
    sCrashedShip2SmokeSpawner* pTask = createSubTaskFromFunction<sCrashedShip2SmokeSpawner>(
        parent, &sCrashedShip2SmokeSpawner::Update);
    if (pTask)
    {
        pTask->m0_counter = 0;
    }
}

void create_A3_2_crashedImperialShip2(s_visdibilityCellTask* r4, s_DataTable2Sub0& r5, s32 r6)
{
    fieldA3_2_crashedImpertialShip2* pNewTask = createSubTask<fieldA3_2_crashedImpertialShip2>(r4);
    getMemoryArea(&pNewTask->m0_memoryArea, r6);
    pNewTask->m8 = &r5;
    pNewTask->mC_position = r5.m4_position;
    pNewTask->m24_rotation = r5.m10_rotation;

    s_fileBundle* pBundle = pNewTask->m0_memoryArea.m0_mainMemoryBundle;
    sAnimationData* pAnimation = pBundle->getAnimation(0x57C);
    sStaticPoseData* pPose = pBundle->getStaticPose(readSaturnU16(r5.m1C_modelData + 2), pAnimation->m2_numBones);

    init3DModelRawData(pNewTask, &pNewTask->m34_3dModel, 0, pBundle, readSaturnS16(r5.m1C_modelData), pAnimation, pPose, nullptr, nullptr);
    stepAnimation(&pNewTask->m34_3dModel);

    pNewTask->m30_status = 0;

    createCrashedShip2SmokeSpawner(pNewTask);
}

s32 initCrashedShipDestruction()
{
    getFieldSpecificData_A3()->m164_A3_2_crashedImperialShipTask->m_DrawMethod = nullptr;
    initCrashedShipDestruction(getFieldSpecificData_A3()->m164_A3_2_crashedImperialShipTask);
    cutsceneTaskInitSub2(getFieldSpecificData_A3()->m164_A3_2_crashedImperialShipTask, gFLD_A3->m6083244, 0, 0, 0);

    return 0;
}



