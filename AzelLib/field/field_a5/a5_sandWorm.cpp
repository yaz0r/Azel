#include "PDS.h"
#include "o_fld_a5.h"
#include "a5_sandWorm.h"
#include "field/fieldModelRender.h"
#include "field/field_a3/o_fld_a3.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "kernel/monsterPart.h"
#include "audio/systemSounds.h"
#include "menu_dragonMorph.h" // computeLookAt
#include "field/fieldParticlePool.h"
#include "field/fieldDebrisScatter.h"
#include "kernel/vdp1AnimatedQuad.h"
#include <array>
#include <map>

static const std::vector<sVdp1Quad>* a5GetOrParseQuadList(const sSaturnPtr& ea)
{
    static std::map<u32, std::vector<sVdp1Quad>> s_cache;
    auto it = s_cache.find(ea.m_offset);
    if (it == s_cache.end())
    {
        s_cache[ea.m_offset] = initVdp1Quad(ea);
        it = s_cache.find(ea.m_offset);
    }
    return &it->second;
}

#include "3dEngine.h"
#include "3dModels.h"
#include "a5_gridDeferredDraw.h"

// 06077304
static void a5_sandWorm_dispatchCollision(sA5SandWorm* pThis, s16 entryKey)
{
    callGridCellDraw_normalSub2(pThis->m0_memoryArea.m0_mainMemoryBundle, entryKey);
}

// 06077074
static void a5_sandWorm_dispatchCollisionWithTransform(sA5SandWorm* pThis, s16 entryKey, sVec3_FP* pPos, sVec3_FP* pRot)
{
    a5GridDraw_cullAndEnqueueWithTransform_06077074(
        pThis->m0_memoryArea.m0_mainMemoryBundle, entryKey, pPos, pRot);
}

static constexpr std::array<std::array<s8, 5>, 12> kSandWormTurnTable = {{
    {{ 7,  8,  9,  6, 10}},
    {{ 7,  9,  8,  6, 10}},
    {{ 8,  9,  7,  6, 10}},
    {{ 8,  7,  9,  6, 10}},
    {{ 9,  7,  8,  6, 10}},
    {{ 9,  8,  7,  6, 10}},
    {{ 7,  8,  9, 10,  6}},
    {{ 7,  9,  8, 10,  6}},
    {{ 8,  9,  7, 10,  6}},
    {{ 8,  7,  9, 10,  6}},
    {{ 9,  7,  8, 10,  6}},
    {{ 9,  8,  7, 10,  6}},
}};

// 06059ae0
static void a5_sandWorm_pickNextTarget(sVec3_FP* pPos, u8* pArgFlag)
{
    s32 startX = pPos->m0_X.m_value;
    s32 startZ = pPos->m8_Z.m_value;
    u8  startDir = *pArgFlag;

    const u32 turnRow = performModulo2(12, randomNumber());

    s32 attempts = 5;
    s32 attemptIdx = 0;
    while (attempts >= 1)
    {
        pPos->m0_X = fixedPoint(startX);
        pPos->m8_Z = fixedPoint(startZ);

        assert(attemptIdx < 5);
        const s8 delta = kSandWormTurnTable[turnRow][attemptIdx];
        attemptIdx++;
        *pArgFlag = (startDir + delta) & 7;

        pPos->m0_X = fixedPoint(pPos->m0_X.m_value
            + (s32)readSaturnU32(gFLD_A5->getSaturnPtr(0x06099CB8 + (*pArgFlag) * 4)));
        pPos->m8_Z = fixedPoint(pPos->m8_Z.m_value
            + (s32)readSaturnU32(gFLD_A5->getSaturnPtr(0x06099CD8 + (*pArgFlag) * 4)));

        bool oob = pPos->m0_X.m_value < 0x4F5000
                || pPos->m0_X.m_value > 0x6F5000
                || pPos->m8_Z.m_value < -0x700000
                || pPos->m8_Z.m_value >= -0x100000;

        if (oob)
        {
            attempts--;
            continue;
        }

        bool bad = false;
        switch (*pArgFlag)
        {
        case 1:
            if (pPos->m0_X.m_value >= 0x675000 && pPos->m8_Z.m_value <= -0x680000)
                bad = true;
            break;
        case 3:
            if (pPos->m0_X.m_value >= 0x675000 && pPos->m8_Z.m_value >= -0x180000)
                bad = true;
            break;
        case 5:
            if (pPos->m0_X.m_value <= 0x575000 && pPos->m8_Z.m_value >= -0x180000)
                bad = true;
            break;
        case 7:
            if (pPos->m0_X.m_value <= 0x575000 && pPos->m8_Z.m_value <= -0x680000)
                bad = true;
            break;
        default:
            break;
        }

        if (bad)
        {
            attempts--;
        }
        else
        {
            attempts = 0; // accepted
        }
    }
}

// 060599a8
static void a5_sandWorm_commitPose(sA5SandWorm* pThis)
{
    s32 radius = pThis->m1C_radius;
    s32 timer;
    if ((pThis->m141_argFlag & 1) == 0)
    {
        timer = pThis->m2C_param2C;
    }
    else
    {
        timer = (pThis->m2C_param2C * 3) >> 1;
    }
    pThis->m30_timer = timer;

    sVec3_FP delta;
    delta.m0_X = fixedPoint(pThis->m10_pos.m0_X.m_value - pThis->m40_renderPosition.m0_X.m_value);
    delta.m4_Y = fixedPoint(pThis->m10_pos.m4_Y.m_value - pThis->m40_renderPosition.m4_Y.m_value);
    delta.m8_Z = fixedPoint(pThis->m10_pos.m8_Z.m_value - pThis->m40_renderPosition.m8_Z.m_value);
    pThis->m5C_velocity.m0_X = delta.m0_X;
    pThis->m5C_velocity.m4_Y = delta.m4_Y;
    pThis->m5C_velocity.m8_Z = delta.m8_Z;

    fixedPoint lengthSq = MTH_Product3d_FP(delta, delta);
    fixedPoint length   = sqrt_F(lengthSq);
    s32 speed = performDivision(pThis->m30_timer, length.m_value);

    s32 radiusScaled = MTH_Mul(fixedPoint(radius), fixedPoint(0x40000)).m_value;
    pThis->m5C_velocity.m4_Y = performDivision(pThis->m30_timer, radiusScaled);

    s32 angle = atan2_FP(delta.m8_Z.m_value, delta.m0_X.m_value);
    u16 idx = (u16)((u32)angle >> 16) & 0xFFF;

    pThis->m5C_velocity.m0_X = MTH_Mul(fixedPoint(speed), getCos(idx));
    pThis->m5C_velocity.m8_Z = MTH_Mul(fixedPoint(speed), getSin(idx));
}

// 06059a86
static void a5_sandWorm_stepRotation(sVec3_FP* pVec, sVec3_FP* pTarget)
{
    sVec2_FP lookAt;
    computeLookAt(*pVec, lookAt);
    pTarget->m0_X = -lookAt[0].m_value;
    pTarget->m4_Y = lookAt[1].m_value + 0x8000000;
}

// 060597da
static void a5_sandWorm_renderContextHook(sA5SandWorm* pThis)
{
    playSystemSoundEffect(0x6B);

    u32 count = mainGameState.readPackedBits(0x578, 8);
    mainGameState.setPackedBits(0x578, 8, count + 1);

    pThis->m10C_modelCtx.m18_visibilityFlags |= 1;
    pThis->m142_subState = 1;

    if ((mainGameState.bitField[0xB] & 0x80) == 0)
    {
        startFieldScript(1, 0x5B6);
    }
}

// 06059854
static void a5SandWorm_Draw(sA5SandWorm* pThis)
{
    sMonsterBody* pBody = pThis->m58_body;
    sMonsterBodyPart* pHead = &pBody->m30_parts[0];

    if (pHead->m4_worldPosition.m4_Y.m_value >= 0)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pHead->m4_worldPosition);
        rotateCurrentMatrixYXZ(&pHead->m1C_rotation);
        pThis->m6C_modelHead.m18_drawFunction(&pThis->m6C_modelHead);
        a5_sandWorm_dispatchCollision(pThis, 0x140);
        popMatrix();
    }

    for (s32 i = pThis->m68_partCount - 2; i > 0; i--)
    {
        sMonsterBodyPart* pPart = &pBody->m30_parts[i];
        if (pPart->m4_worldPosition.m4_Y.m_value < 0)
            continue;

        pBody->m2C_drawPart(pThis->m0_memoryArea.m0_mainMemoryBundle, pPart);
        a5_sandWorm_dispatchCollisionWithTransform(pThis, 0x144, &pPart->m4_worldPosition, &pPart->m1C_rotation);

        pThis->m34 = pPart->m4_worldPosition.m0_X.m_value;
        pThis->m3C = pPart->m4_worldPosition.m8_Z.m_value;

        pushCurrentMatrix();
        translateCurrentMatrix(reinterpret_cast<sVec3_FP*>(&pThis->m34));
        rotateCurrentMatrixShiftedX(fixedPoint(0x4000000));
        addObjectToDrawList(pThis->m0_memoryArea.m0_mainMemoryBundle->get3DModel(0xB0));
        popMatrix();
    }

    sMonsterBodyPart* pTail = &pBody->m30_parts[pThis->m68_partCount - 1];
    s32 cutoff = (pTail->m1C_rotation.m0_X.m_value < 0) ? 0x8000 : -0x8000;
    if (cutoff <= pTail->m4_worldPosition.m4_Y.m_value)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(&pTail->m4_worldPosition);
        rotateCurrentMatrixYXZ(&pTail->m1C_rotation);
        pThis->mBC_modelTail.m18_drawFunction(&pThis->mBC_modelTail);
        a5_sandWorm_dispatchCollision(pThis, 0x148);
        popMatrix();
    }
}

// 060597a0
static void a5_sandWorm_bodyUpdate(sMonsterBody* pBody, sVec3_FP* pPos, sVec3_FP* pRot)
{
    pBody->m0_translation = *pPos;
    pBody->mC_rotation = *pRot;
    pBody->m18_rotationTarget = *pRot;
    pBody->m24_update(&pBody->m30_parts[0], &pBody->m0_translation, &pBody->mC_rotation, &pBody->m18_rotationTarget);
}

// 0605f734
static void a5_sandWorm_spawnBreachSand(sA5SandWorm* /*pThis*/, sVec3_FP* pPos)
{
    static const std::vector<sVdp1Quad>* s_dustQuadList = nullptr;
    if (!s_dustQuadList)
        s_dustQuadList = a5GetOrParseQuadList(gFLD_A5->getSaturnPtr(0x0608C508));

    for (s32 i = 1; i < 9; i++)
    {
        sVec3_FP vel;
        vel.m0_X = fixedPoint(centeredRandom(0x7FF));
        vel.m4_Y = fixedPoint(randomNumber() & 0x7FF);
        vel.m8_Z = fixedPoint(centeredRandom(0x7FF));

        sParticleSpawnConfig desc = {};
        desc.m0_pPosition = pPos;
        desc.m4_pVelocity = &vel;
        desc.m8_pQuadData = s_dustQuadList;
        desc.m14_updateFunc = &particleUpdateMoving;
        desc.m18_heapSize = 4;
        desc.m1C_heapData = &i;

        s_fieldSpecificData_A5* pFieldData = (s_fieldSpecificData_A5*)getFieldTaskPtr()->mC;
        sParticlePoolManager* pManager = pFieldData->m54_particlePool;
        spawnParticleInPool(pManager, &desc, 0);
    }
}

// 06059c20
void a5SandWorm_Update(sA5SandWorm* pThis)
{
    pThis->m30_timer--;
    updateFieldModelRenderContext(&pThis->m10C_modelCtx);

    s8 state = (s8)pThis->m140_state;
    bool inPositiveLane = false;
    bool runStep = true;

    if (state == 0)
    {
        a5_sandWorm_pickNextTarget(&pThis->m10_pos, &pThis->m141_argFlag);
        pThis->m40_renderPosition.m4_Y = fixedPoint(0);
        a5_sandWorm_commitPose(pThis);
        pThis->m140_state++;
        inPositiveLane = true;
    }
    else if (state == 1)
    {
        inPositiveLane = true;
    }
    else if (state == 2)
    {
        pThis->m10_pos.m0_X = fixedPoint(pThis->m10_pos.m0_X.m_value
            + (s32)readSaturnU32(gFLD_A5->getSaturnPtr(0x06099CB8 + pThis->m141_argFlag * 4)));
        pThis->m10_pos.m8_Z = fixedPoint(pThis->m10_pos.m8_Z.m_value
            + (s32)readSaturnU32(gFLD_A5->getSaturnPtr(0x06099CD8 + pThis->m141_argFlag * 4)));
        a5_sandWorm_commitPose(pThis);
        pThis->m5C_velocity.m4_Y = fixedPoint(-pThis->m5C_velocity.m4_Y.m_value);
        pThis->m140_state++;
        inPositiveLane = false;
    }
    else if (state == 3)
    {
        inPositiveLane = false;
    }
    else
    {
        runStep = false;
    }

    if (runStep)
    {
        if (inPositiveLane)
        {
            pThis->m5C_velocity.m4_Y = fixedPoint(pThis->m5C_velocity.m4_Y.m_value - pThis->m20_param8);
            a5_sandWorm_stepRotation(&pThis->m5C_velocity, &pThis->m4C_rotationTarget);
            if (pThis->m30_timer < 1)
                pThis->m140_state++;

            u32 r = randomNumber();
            if ((r & 0xFF) == 0 && isWorldPositionOnScreen(&pThis->m40_renderPosition) != 0)
            {
                playSystemSoundEffect((r & 2) == 0 ? 0x6D : 0x6C);
            }
        }
        else
        {
            pThis->m5C_velocity.m4_Y = fixedPoint(pThis->m5C_velocity.m4_Y.m_value + pThis->m20_param8);
            a5_sandWorm_stepRotation(&pThis->m5C_velocity, &pThis->m4C_rotationTarget);
            if (pThis->m30_timer < 1)
                pThis->m140_state = 0;
        }
    }

    pThis->m4C_rotationTarget.m8_Z = fixedPoint(pThis->m4C_rotationTarget.m8_Z.m_value + pThis->m28_param10);

    u8 subState = pThis->m142_subState;
    if (subState == 0)
    {
        if (pThis->m40_renderPosition.m4_Y.m_value < 0)
            pThis->m10C_modelCtx.m18_visibilityFlags |= 1;
        else
            pThis->m10C_modelCtx.m18_visibilityFlags = 0;
    }
    else if (subState == 1)
    {
        pThis->m24 += 0x800000;
        if (pThis->m24 > 0x1FFFFFF)
            pThis->m142_subState++;
    }
    else if (subState == 0x30)
    {
        pThis->m24 -= 0x800000;
        if (pThis->m24 < 1)
        {
            pThis->m142_subState = 0;
            pThis->m24 = 0;
            pThis->m10C_modelCtx.m18_visibilityFlags = 0;
        }
    }
    else
    {
        pThis->m142_subState++;
    }

    pThis->m4C_rotationTarget.m4_Y = fixedPoint(pThis->m4C_rotationTarget.m4_Y.m_value + pThis->m24);

    s32 oldY = pThis->m40_renderPosition.m4_Y.m_value;

    pThis->m40_renderPosition.m0_X = fixedPoint(pThis->m40_renderPosition.m0_X.m_value + pThis->m5C_velocity.m0_X.m_value);
    pThis->m40_renderPosition.m4_Y = fixedPoint(pThis->m40_renderPosition.m4_Y.m_value + pThis->m5C_velocity.m4_Y.m_value);
    pThis->m40_renderPosition.m8_Z = fixedPoint(pThis->m40_renderPosition.m8_Z.m_value + pThis->m5C_velocity.m8_Z.m_value);

    if (MTH_Mul(fixedPoint(oldY), pThis->m40_renderPosition.m4_Y).m_value < 1)
    {
        if (isWorldPositionOnScreen(&pThis->m40_renderPosition) != 0)
        {
            playSystemSoundEffect(0x6E);
        }
        a5_sandWorm_spawnBreachSand(pThis, &pThis->m40_renderPosition);
    }

    s16 stiffness = (pThis->m142_subState == 0) ? 0xE00 : 0x1C00;
    for (s32 i = 0; i < pThis->m68_partCount; i++)
    {
        sMonsterBodyPart* pPart = &pThis->m58_body->m30_parts[i];
        pPart->m44_springStiffness.m4_Y = fixedPoint(stiffness);
        pThis->m8_perPartInts[i] = pPart->m4_worldPosition.m4_Y.m_value;
    }

    a5_sandWorm_bodyUpdate(pThis->m58_body, &pThis->m40_renderPosition, &pThis->m4C_rotationTarget);

    s32 yThreshold = MTH_Mul(fixedPoint(pThis->m1C_radius), fixedPoint((s32)0xFFFF8000)).m_value;
    for (u32 i = 0; i < (u32)pThis->m68_partCount; i++)
    {
        sMonsterBodyPart* pPart = &pThis->m58_body->m30_parts[i];
        s32 crossProduct = MTH_Mul(fixedPoint(pPart->m4_worldPosition.m4_Y.m_value),
                                   fixedPoint(pThis->m8_perPartInts[i])).m_value;
        if (crossProduct < 1)
        {
            // 0605f86c
            Unimplemented(); // a5_sandWorm_spawnSegmentSand(pThis, &pPart->m4_worldPosition)
            pThis->mC_perPartBytes[i] = 0;
            break;
        }
        if (((i & 1) != 0) && (pThis->mC_perPartBytes[i] == 0) &&
            (pPart->m4_worldPosition.m4_Y.m_value < yThreshold))
        {
            // 0605f804
            Unimplemented(); // a5_sandWorm_spawnSegmentDebris(pThis, &pPart->m4_worldPosition)
            pThis->mC_perPartBytes[i] = 1;
            break;
        }
    }

    stepAnimation(&pThis->m6C_modelHead);
    stepAnimation(&pThis->mBC_modelTail);
}

struct sSandWormSpawn
{
    s16 m0_modelTypeIdx;
    s16 m2_partCount;
    s32 m4_posX;
    s32 m8_posY;
    s32 mC_posZ;
    s32 m10_radius;
    s32 m14_param10;
    s32 m18_param2C;
    s32 m1C;
    u8 m20_argFlag;
    // size 0x24
};

// 06059f1c
static void a5_createSandWorm(p_workArea parent, const sSandWormSpawn& arg)
{
    static sA5SandWorm::TypedTaskDefinition td = {
        nullptr, &a5SandWorm_Update, &a5SandWorm_Draw, nullptr
    };
    sA5SandWorm* pThis = createSubTask<sA5SandWorm>(parent, &td);
    if (pThis == nullptr)
        return;

    getMemoryArea(&pThis->m0_memoryArea, 1);

    const s16 modelTypeIdx = arg.m0_modelTypeIdx;
    const s16 partCount    = arg.m2_partCount;
    pThis->m68_partCount = partCount;

    sMonsterBody* pBody = monsterBody_create(pThis, partCount);
    pThis->m58_body = pBody;
    monsterBody_loadPartData(pBody, 0, sSaturnPtr());

    for (s32 iPart = 0; iPart < partCount; iPart++)
    {
        u32 templateTable;
        if (iPart == 0)
            templateTable = 0x06099C1C;
        else if (iPart == partCount - 1)
            templateTable = 0x06099C34;
        else
            templateTable = 0x06099C28;

        sMonsterBodyPart* pPart = &pBody->m30_parts[iPart];

        sSaturnPtr transEA = gFLD_A5->getSaturnPtr(0x06099C88 + (u8)modelTypeIdx * 12);
        pPart->m10_translation = readSaturnVec3(transEA);

        sSaturnPtr partTemplateEA = gFLD_A5->getSaturnPtr(
            readSaturnU32(gFLD_A5->getSaturnPtr(templateTable + (u8)modelTypeIdx * 4)));
        monsterBodyPart_loadTemplate(pPart, partTemplateEA);

        pPart->m44_springStiffness.m0_X = fixedPoint(0xE00);
        pPart->m44_springStiffness.m4_Y = fixedPoint(0xE00);
        pPart->m44_springStiffness.m8_Z = fixedPoint(0xE00);
        pPart->m50_damping = fixedPoint(0x5000);
    }

    monsterBody_initPose(pBody, &pThis->m40_renderPosition, &pThis->m4C_rotationTarget);

    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    {
        u32 idx4 = (u32)modelTypeIdx * 4;
        s16 modelOffset = (s16)readSaturnU32(gFLD_A5->getSaturnPtr(0x06099C40 + idx4));
        u32 animOffset  = readSaturnU32(gFLD_A5->getSaturnPtr(0x06099C70 + idx4));
        u32 poseOffset  = readSaturnU32(gFLD_A5->getSaturnPtr(0x06099C58 + idx4));
        sAnimationData*  pAnim = pBundle->getAnimation(animOffset);
        sStaticPoseData* pPose = pBundle->getStaticPose(poseOffset, pAnim->m2_numBones);
        init3DModelRawData(pThis, &pThis->m6C_modelHead, 0, pBundle, (u16)modelOffset, pAnim, pPose, nullptr, nullptr);
        stepAnimation(&pThis->m6C_modelHead);
    }

    {
        u32 idx4 = (u32)modelTypeIdx * 4;
        s16 modelOffset = (s16)readSaturnU32(gFLD_A5->getSaturnPtr(0x06099C4C + idx4));
        u32 animOffset  = readSaturnU32(gFLD_A5->getSaturnPtr(0x06099C7C + idx4));
        u32 poseOffset  = readSaturnU32(gFLD_A5->getSaturnPtr(0x06099C64 + idx4));
        sAnimationData*  pAnim = pBundle->getAnimation(animOffset);
        sStaticPoseData* pPose = pBundle->getStaticPose(poseOffset, pAnim->m2_numBones);
        init3DModelRawData(pThis, &pThis->mBC_modelTail, 0, pBundle, (u16)modelOffset, pAnim, pPose, nullptr, nullptr);
        stepAnimation(&pThis->mBC_modelTail);
    }

    pThis->m8_perPartInts  = static_cast<s32*>(allocateHeapForTask(pThis, partCount * 4));
    pThis->mC_perPartBytes = static_cast<u8*> (allocateHeapForTask(pThis, partCount));
    if (pThis->m8_perPartInts == nullptr || pThis->mC_perPartBytes == nullptr)
    {
        pThis->getTask()->markFinished();
        return;
    }

    const s32 posX = arg.m4_posX;
    const s32 posY = arg.m8_posY;
    const s32 posZ = arg.mC_posZ;
    pThis->m10_pos.m0_X = fixedPoint(posX); pThis->m40_renderPosition.m0_X = fixedPoint(posX);
    pThis->m10_pos.m4_Y = fixedPoint(posY); pThis->m40_renderPosition.m4_Y = fixedPoint(posY);
    pThis->m10_pos.m8_Z = fixedPoint(posZ); pThis->m40_renderPosition.m8_Z = fixedPoint(posZ);

    pThis->m1C_radius   = arg.m10_radius;
    pThis->m2C_param2C  = arg.m18_param2C;
    pThis->m141_argFlag = arg.m20_argFlag;
    pThis->m140_state   = 0;
    pThis->m28_param10  = arg.m14_param10;
    pThis->m5C_velocity.m0_X = 0;
    pThis->m5C_velocity.m4_Y = 0;
    pThis->m5C_velocity.m8_Z = 0;
    pThis->m24          = 0;
    pThis->m142_subState = 0;
    pThis->m38          = 0;

    initFieldModelRenderContext(
        &pThis->m10C_modelCtx,
        pThis,
        reinterpret_cast<void*>(&a5_sandWorm_renderContextHook),
        &pThis->m40_renderPosition,
        nullptr,
        1, 0, -1, 0, 0);
}

// 0605a190
void createA5_envObjects_sub0(p_workArea parent)
{
    // 06099de8
    static constexpr std::array<sSandWormSpawn, 4> kSandWormSpawns = { {
        {1,  6, 0x006F5000, 0, (s32)0xFFA00000, 0x00046000, 0x00000000, 0x3C, 6, 0},
        {1,  8, 0x006F5000, 0, (s32)0xFFF00000, 0x00032000, 0x00400000, 0x48, 6, 0},
        {1, 11, 0x005F5000, 0, (s32)0xFFB00000, 0x0003C000, 0x00000000, 0x3F, 1, 0},
        {2,  4, 0x004F5000, 0, (s32)0xFFA00000, 0x0004B000, 0x00000000, 0x33, 2, 0},
    } };

    for (const sSandWormSpawn& spawn : kSandWormSpawns)
    {
        a5_createSandWorm(parent, spawn);
    }
}
