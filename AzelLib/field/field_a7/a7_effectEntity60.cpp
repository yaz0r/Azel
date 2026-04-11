#include "PDS.h"
#include "a7_effectEntity60.h"
#include "trigo.h"
#include "kernel/fileBundle.h"
#include "field/field_a7/o_fld_a7.h"
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

// 0605E126
void a7EffectEntity60_Update(sA7EffectEntity60* pThis)
{
    s32* pIndices[4] = {
        &pThis->m58_animIdx3,
        &pThis->m54_animIdx2,
        &pThis->m50_animIdx1,
        &pThis->m4C_animIdx0,
    };
    for (s32 slot = 0; slot < 4; slot++)
    {
        s32 v = *pIndices[slot];
        *pIndices[slot] = (v < 0x16) ? (v + 1) : 0;
    }

    bool checkProgress = false;
    if (pThis->m5C_state == 0)
    {
        pThis->m40_fade += 0x100;
        if (pThis->m40_fade >= 0x10000)
        {
            pThis->m5C_state++;
        }
        checkProgress = true;
    }
    else if (pThis->m5C_state == 1)
    {
        checkProgress = true;
    }
    else if (pThis->m5C_state == 2)
    {
        if (pThis->m40_fade < 1)
        {
            pThis->getTask()->markFinished();
        }
        else
        {
            pThis->m40_fade -= 0x80;
        }
    }

    if (checkProgress && pThis->m3C_progress > 0x38E38E3)
    {
        pThis->m5C_state++;
    }

    if (pThis->m3C_progress < 0x4000000)
    {
        pThis->m3C_progress += 0x308B9;
    }
    pThis->m38_angleB -= 0x5B05B0;

    u16 idxA = (u16)((u32)pThis->m3C_progress >> 16) & 0xFFF;
    u16 idxB = (u16)((u32)pThis->m38_angleB >> 16) & 0xFFF;

    fixedPoint tA = MTH_Mul(fixedPoint(0x100000), getCos(idxA));
    fixedPoint tB = MTH_Mul_5_6(fixedPoint(0x28000), fixedPoint(pThis->m40_fade), getCos(idxB));
    pThis->m8_pos0.m0_X = fixedPoint(tB.m_value + tA.m_value + 0xA00000);

    // Final sin multiply — result is discarded in the original binary.
    (void)MTH_Mul(fixedPoint(0x100000), getSin(idxA));
}

// 0605E4C4
void a7EffectEntity60_Draw(sA7EffectEntity60* pThis)
{
    if (pThis->m40_fade == 0)
    {
        return;
    }

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
