#include "PDS.h"
#include "a7_effectEntity60.h"
#include "a7_beamChargeWobble.h"
#include "trigo.h"
#include "kernel/fileBundle.h"
#include "field/field_a7/o_fld_a7.h"
#include "field/fieldDebrisScatter.h"
#include "3dModels.h"

// 0605E0F6
void a7EffectEntity60_Init(sA7EffectEntity60* pThis)
{
    getMemoryArea(&pThis->m0_memoryArea, 3);
    pThis->m3C_progress = 0;
    pThis->m38_angleB = 0;
    pThis->m40_fade = 0;
    pThis->m5C_state = 0;
    pThis->m8_pos0.m4_Y  = fixedPoint(0x40000);
    pThis->m14_pos1.m4_Y = fixedPoint(0x40000);
    pThis->m20_pos2.m4_Y = fixedPoint(0x40000);
    pThis->m2C_pos3.m4_Y = fixedPoint(0x40000);
    pThis->m44_scaleY = 0x10000;
}

// Angle offsets for the 4 position slots
static const s32 kProgressOffsets[4] = { 0, 0x8000000, 0x4000000, 0xC000000 };
static const s32 kAngleBOffsets[4]   = { 0, 0x4000000, 0xC000000, 0x2000000 };
// XZ center positions for each slot
static const s32 kCenterX[4] = { 0xA00000, 0xA00000, 0xB00000, 0x900000 };
static const s32 kCenterZ[4] = { -0xB00000, -0xB00000, -0xA00000, -0xA00000 };
// Note: Z signs are negative because the negation applies differently per slot
// Slot 0: X=+0xA00000, Z=-0xB00000 (sin result subtracted from base → +sin+base = base-(-sin))
// The Ghidra code shows: slot0 Z = sinA*0x100000 + sinB*fade*0x28000 + (-0xB00000)

// 0605E126
void a7EffectEntity60_Update(sA7EffectEntity60* pThis)
{
    // Update all 8 animation indices (4 pairs at m4C..m58)
    for (s32 slot = 3; slot >= 0; slot--)
    {
        s32* pIdxA = (s32*)((u8*)pThis + 0x4C + slot * 4);
        *pIdxA = (*pIdxA < 0x16) ? (*pIdxA + 1) : 0;
        s32* pIdxB = (s32*)((u8*)pThis + 0x48 + slot * 4);
        *pIdxB = (*pIdxB < 0x16) ? (*pIdxB + 1) : 0;
    }

    // State machine
    s32 state = pThis->m5C_state;
    bool checkProgress = false;
    if (state == 0)
    {
        pThis->m40_fade += 0x100;
        if (pThis->m40_fade > 0xFFFF)
            pThis->m5C_state++;
        checkProgress = true;
    }
    else if (state == 1)
    {
        checkProgress = true;
    }
    else if (state == 2)
    {
        if (pThis->m40_fade < 1)
            pThis->getTask()->markFinished();
        else
            pThis->m40_fade -= 0x80;
    }

    if (checkProgress && pThis->m3C_progress > 0x38E38E3)
        pThis->m5C_state++;

    // Advance progress and angle
    if (pThis->m3C_progress < 0x4000000)
        pThis->m3C_progress += 0x308B9;
    pThis->m38_angleB -= 0x5B05B0;

    // Compute all 4 position slots
    sVec3_FP* positions[4] = { &pThis->m8_pos0, &pThis->m14_pos1, &pThis->m20_pos2, &pThis->m2C_pos3 };

    for (s32 i = 0; i < 4; i++)
    {
        u16 idxA = (u16)((u32)(pThis->m3C_progress + kProgressOffsets[i]) >> 16) & 0xFFF;
        u16 idxB = (u16)((u32)(pThis->m38_angleB + kAngleBOffsets[i]) >> 16) & 0xFFF;

        fixedPoint cosA = MTH_Mul(fixedPoint(0x100000), getCos(idxA));
        fixedPoint wobbleX = MTH_Mul_5_6(fixedPoint(0x28000), fixedPoint(pThis->m40_fade), getCos(idxB));
        positions[i]->m0_X = fixedPoint(wobbleX.m_value + cosA.m_value + kCenterX[i]);

        fixedPoint sinA = MTH_Mul(fixedPoint(0x100000), getSin(idxA));
        fixedPoint wobbleZ = MTH_Mul_5_6(fixedPoint(0x28000), fixedPoint(pThis->m40_fade), getSin(idxB));
        positions[i]->m8_Z = fixedPoint(wobbleZ.m_value + sinA.m_value + kCenterZ[i]);
    }

    // Compute Y (shared across all slots) from fade -> scaleY interpolation
    s32 scaleY;
    if (pThis->m40_fade < 0x2000)
        scaleY = pThis->m40_fade << 3;
    else
        scaleY = 0x10000;
    pThis->m44_scaleY = scaleY;

    s32 yVal = MTH_Mul(fixedPoint(0x10000 - scaleY), fixedPoint(0x8C000)).m_value;
    yVal = setDividend(yVal, 0x4C000, 0x8C000);
    yVal += 0x40000;
    pThis->m8_pos0.m4_Y  = fixedPoint(yVal);
    pThis->m14_pos1.m4_Y = fixedPoint(yVal);
    pThis->m20_pos2.m4_Y = fixedPoint(yVal);
    pThis->m2C_pos3.m4_Y = fixedPoint(yVal);

    // Spawn a scene particle at one of the 4 positions (round-robin via m48)
    pThis->m48_particleCounter++;
    s32 slotIdx = (s32)(pThis->m4B_particleSlot & 3);
    sVec3_FP* pSpawnPos = positions[slotIdx];

    sVec3_FP vel = {};
    sVec3_FP pos;
    pos.m0_X = fixedPoint(pSpawnPos->m0_X.m_value + centeredRandom(0x1FFFF));
    pos.m4_Y = 0;
    pos.m8_Z = fixedPoint(pSpawnPos->m8_Z.m_value + centeredRandom(0x1FFFF));

    s_fieldSpecificData_A7* pFieldData = (s_fieldSpecificData_A7*)getFieldTaskPtr()->mC;
    sFieldSceneManager* pManager = (sFieldSceneManager*)pFieldData->m280;

    sSceneParticleDesc desc = {};
    desc.m8_pQuadList = a7GetOrParseQuadList(gFLD_A7->getSaturnPtr(0x060804E4));
    sceneParticle_spawnProjected(pManager, &desc, &pos, &vel);
}

// 0605E4C4
void a7EffectEntity60_Draw(sA7EffectEntity60* pThis)
{
    if (pThis->m40_fade == 0)
        return;

    s_fileBundle* pBundle = pThis->m0_memoryArea.m0_mainMemoryBundle;
    sSaturnPtr lut = gFLD_A7->getSaturnPtr(0x06086520);

    struct { sVec3_FP* posA; s32 idxA; sVec3_FP* posB; s32 idxB; } iters[2] = {
        { &pThis->m2C_pos3, pThis->m58_animIdx3, &pThis->m20_pos2, pThis->m54_animIdx2 },
        { &pThis->m14_pos1, pThis->m50_animIdx1, &pThis->m8_pos0,  pThis->m4C_animIdx0 },
    };

    for (s32 i = 0; i < 2; i++)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(iters[i].posA);
        scaleCurrentMatrixRow0(pThis->m40_fade);
        scaleCurrentMatrixRow1(pThis->m44_scaleY);
        scaleCurrentMatrixRow2(pThis->m40_fade);
        s16 modelOffA = readSaturnS16(lut + iters[i].idxA * 2);
        addObjectToDrawList(pBundle->get3DModel(modelOffA));
        popMatrix();

        pushCurrentMatrix();
        translateCurrentMatrix(iters[i].posB);
        scaleCurrentMatrixRow0(pThis->m40_fade);
        scaleCurrentMatrixRow1(pThis->m44_scaleY);
        scaleCurrentMatrixRow2(pThis->m40_fade);
        s16 modelOffB = readSaturnS16(lut + iters[i].idxB * 2);
        addObjectToDrawList(pBundle->get3DModel(modelOffB));
        popMatrix();
    }
}
