#include "PDS.h"
#include "battleGrid.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDebug.h"
#include "town/town.h" // todo: clean

void battleGrid_initSub0()
{
    getBattleManager()->m10_battleOverlay->m8_gridTask->m218 = 0x11c71c7;
    getBattleManager()->m10_battleOverlay->m8_gridTask->m1C8 |= 0x20;
}

void battleGrid_initSub1(s_battleGrid* pThis)
{
    pThis->m214 = 0;
    battleGrid_initSub0();
}

void battleGrid_init(s_battleGrid* pThis)
{
    getBattleManager()->m10_battleOverlay->m8_gridTask = pThis;

    pThis->m0 = 1;
    pThis->m1 = 0;
    pThis->m2 = 4;

    pThis->m4 = 0;
    pThis->mC = 0;
    pThis->m10.zeroize();
    pThis->m64.zeroize();
    pThis->m70.zeroize();
    pThis->mE4.zeroize();
    pThis->mF0.zeroize();
    pThis->mFC.zeroize();
    pThis->m108.zeroize();
    pThis->m114.zeroize();
    pThis->m12C = 0x1999;
    pThis->m130 = 0x6666;

    pThis->m134 = readSaturnVec3(gCurrentBattleOverlay->getSaturnPtr(0x60ac478) + pThis->m1 * 0x24);
    pThis->m140 = readSaturnVec3(gCurrentBattleOverlay->getSaturnPtr(0x60ac478) + pThis->m1 * 0x24 + 0xC);
    pThis->mFC = readSaturnVec3(gCurrentBattleOverlay->getSaturnPtr(0x60ac478) + pThis->m1 * 0x24 + 0x18);

    pThis->mB4_rotation.zeroize();
    pThis->mC0.zeroize();

    pThis->mCC[0] = 0x6666;
    pThis->mCC[1] = 0x2666;
    pThis->mCC[2] = 0x4CCC;

    pThis->mD8[0] = 0x6666;
    pThis->mD8[1] = 0x4CCC;
    pThis->mD8[2] = 0x8000;

    pThis->m198[0] = 0x3333;
    pThis->m198[1] = 0x3333;
    pThis->m198[2] = 0x3333;

    pThis->m1A4[0] = 0x8000;
    pThis->m1A4[1] = 0x8000;
    pThis->m1A4[2] = 0x8000;

    pThis->m14C = 0;
    pThis->m320 = 0;
    pThis->m318 = 0;

    pThis->m314 = 0;

    pThis->m31C = 1;

    pThis->m2F8 = 0;
    pThis->m2FC = 0;
    pThis->m300 = 0;
    pThis->m304 = 0;
    pThis->m308 = 0;
    pThis->m30C = 0;

    for (int i = 0; i < 4; i++)
    {
        pThis->m298[i].zeroize();
        pThis->m2C8[i].zeroize();
    }

    battleGrid_initSub1(pThis);
}

void battleGrid_updateSub0(s_battleGrid* pThis)
{
    pThis->m8 = pThis->mC;
    pThis->mC = 0;
    pThis->m64[0] = getBattleManager()->m10_battleOverlay->m4_battleEngine->m1C8;
    pThis->m64[1] = getBattleManager()->m10_battleOverlay->m4_battleEngine->m1CC;

    pThis->m1B4 = 0;
    pThis->m1B0 = 0;
}

void battleGrid_updateSub1Sub0(s_battleGrid* pThis)
{
    pThis->m40 += MTH_Mul(0x3333, -pThis->m28);
    pThis->m40 -= MTH_Mul(0x8000, pThis->m40);
    pThis->m28 += pThis->m40;

    pThis->m90 += MTH_Mul(0x3333, -pThis->m84);
    pThis->m90 -= MTH_Mul(0x8000, pThis->m90);
    pThis->m84 += pThis->m90;
}

void battleGrid_updateSub1Sub1(s_battleGrid* pThis)
{
    pThis->m108[0] += MTH_Mul(pThis->m134[0] - pThis->mE4[0], pThis->m12C);
    pThis->m108[0] -= MTH_Mul(pThis->m108[0], pThis->m130);
    pThis->mE4[0] += pThis->m108[0];

    pThis->m108[1] += MTH_Mul(pThis->m134[1] - pThis->mE4[1], pThis->m12C);
    pThis->m108[1] -= MTH_Mul(pThis->m108[1], pThis->m130);
    pThis->mE4[1] += pThis->m108[1];

    pThis->m108[2] += MTH_Mul(pThis->m134[2] - pThis->mE4[2], pThis->m12C);
    pThis->m108[2] -= MTH_Mul(pThis->m108[2], pThis->m130);
    pThis->mE4[2] += pThis->m108[2];

    pThis->m114[0] += MTH_Mul(pThis->m140[0] - pThis->mF0[0], pThis->m12C);
    pThis->m114[0] -= MTH_Mul(pThis->m114[0], pThis->m130);
    pThis->mF0[0] += pThis->m114[0];

    pThis->m114[1] += MTH_Mul(pThis->m140[1] - pThis->mF0[1], pThis->m12C);
    pThis->m114[1] -= MTH_Mul(pThis->m114[1], pThis->m130);
    pThis->mF0[1] += pThis->m114[1];

    pThis->m114[2] += MTH_Mul(pThis->m140[2] - pThis->mF0[2], pThis->m12C);
    pThis->m114[2] -= MTH_Mul(pThis->m114[2], pThis->m130);
    pThis->mF0[2] += pThis->m114[2];
}

void battleGrid_updateSub1Sub2(s_battleGrid* pThis)
{
    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0xA] == 0)
    {
        if ((getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags & 0x400000) == 0)
        {
            pThis->m198[0] = 0x3333;
            pThis->m198[1] = 0x3333;
            pThis->m198[2] = 0x3333;
        }
        else
        {
            pThis->m198[0] = 0x4000;
            pThis->m198[1] = 0x4000;
            pThis->m198[2] = 0x4000;
        }

        pThis->m1A4[0] = 0x8000;
        pThis->m1A4[1] = 0x8000;
        pThis->m1A4[2] = 0x8000;
    }
    else
    {
        assert(0);
    }

    pThis->m34 = getBattleManager()->m10_battleOverlay->m4_battleEngine->m104_dragonStartPosition + pThis->m1C + pThis->m28;

    pThis->m180_translation[0] = (*pThis->m1B8)[0];
    pThis->m180_translation[1] = (*pThis->m1B8)[1];

    if (getBattleManager()->m10_battleOverlay->mC->m204 + 0x1000 < pThis->m180_translation[1])
    {
        if (getBattleManager()->m10_battleOverlay->mC->m200 - 0x1000 <= pThis->m180_translation[1])
        {
            pThis->m180_translation[1] = getBattleManager()->m10_battleOverlay->mC->m200 - 0x1000;
        }
    }
    else
    {
        pThis->m180_translation[1] = getBattleManager()->m10_battleOverlay->mC->m204 + 0x1000;
    }

    pThis->m180_translation[2] = (*pThis->m1B8)[2];
}

void battleGrid_updateSub1(s_battleGrid* pThis)
{
    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0xB] == 0)
    {
        battleGrid_updateSub1Sub0(pThis);
        battleGrid_updateSub1Sub1(pThis);
        initMatrixToIdentity(&pThis->m150_matrix);
        battleGrid_updateSub1Sub2(pThis);
        translateMatrix(&pThis->m180_translation, &pThis->m150_matrix);
        if ((pThis->m1C8 & 2) == 0)
        {
            assert(0);
            //battleGrid_updateSub1Sub3(iParm1);
        }
        else
        {
            assert(0);
            //battleGrid_updateSub1Sub4(iParm1);
        }
        rotateMatrixZYX(&pThis->mB4_rotation, &pThis->m150_matrix);
    }
}

void battleGrid_updateSub2(s_battleGrid* pThis)
{
    if (pThis->m1C8 & 4)
    {
        assert(0);
    }
    if (pThis->m1C8 & 8)
    {
        assert(0);
    }
}

void battleGrid_update(s_battleGrid* pThis)
{
    pThis->m4++;

    battleGrid_updateSub0(pThis);
    battleGrid_updateSub1(pThis);
    battleGrid_updateSub2(pThis);

    if ((pThis->m1C8 & 0x10) && !(pThis->m1C8 & 0x40))
    {
        mainLogicUpdateSub0(pThis->m180_translation[0], pThis->m180_translation[2]);
    }
}

void battleGrid_draw(s_battleGrid* pThis)
{
    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0xB])
    {
        assert(0);
    }
    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0xC])
    {
        assert(0);
    }

    sVec3_FP localVec0;
    sVec3_FP localVec1;
    sVec3_FP localVec2;
    transformAndAddVec(pThis->mFC, localVec0, pThis->m150_matrix);
    transformAndAddVec(pThis->mF0, localVec1, pThis->m150_matrix);
    transformAndAddVec(pThis->mFC, localVec2, pThis->m150_matrix);

    fixedPoint iVar3 = localVec2[1] - 0x1000;
    if (iVar3 < getBattleManager()->m10_battleOverlay->mC->m204)
    {
        localVec1[1] -= iVar3 - getBattleManager()->m10_battleOverlay->mC->m204;
        localVec2[1] -= iVar3 - getBattleManager()->m10_battleOverlay->mC->m204;
    }

    if (getBattleManager()->m10_battleOverlay->mC->m200 + pThis->m134[2] < localVec2[1])
    {
        localVec1[1] -= localVec2[1] - getBattleManager()->m10_battleOverlay->mC->m200 - pThis->m134[2];
        localVec2[1] -= localVec2[1] - getBattleManager()->m10_battleOverlay->mC->m200 - pThis->m134[2];
    }

    generateCameraMatrix(&cameraProperties2, localVec2, localVec1, localVec0);

    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0x12])
    {
        assert(0);
    }

    sVec3_FP iStack104;
    iStack104[0] = -MTH_Mul_5_6(0x10000, getCos(pThis->m280.getInteger()), getSin(pThis->m284.getInteger()));
    iStack104[1] = MTH_Mul(0x10000, getSin(pThis->m280.getInteger()));
    iStack104[2] = -MTH_Mul_5_6(0x10000, getCos(pThis->m280.getInteger()), getCos(pThis->m284.getInteger()));

    sVec3_FP uStack116;
    transformVecByCurrentMatrix(iStack104, uStack116);
    setupLight(uStack116[0], uStack116[1], uStack116[2], s_RGB8::fromVector(pThis->m1CC_lightColor).toU32());
    generateLightFalloffMap(
        s_RGB8::fromVector(pThis->m1E4_lightFalloff0).toU32(),
        s_RGB8::fromVector(pThis->m1FC_lightFalloff1).toU32(),
        s_RGB8::fromVector(pThis->m208_lightFalloff2).toU32()
    );

    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0xD])
    {
        assert(0);
    }

    if (pThis->m1C8 & 0x20)
    {
        initVDP1Projection(pThis->m218, 0);
        pThis->m1C8 &= ~0x20;
    }

    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0x12])
    {
        assert(0);
    }

    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0x11])
    {
        assert(0);
    }
}

void battleGrid_delete(s_battleGrid* pThis)
{
    FunctionUnimplemented();
}

p_workArea createBattleEnvironmentGridTask(p_workArea parent)
{
    static const s_battleGrid::TypedTaskDefinition definition = {
        &battleGrid_init,
        &battleGrid_update,
        &battleGrid_draw,
        &battleGrid_delete
    };

    return createSubTask<s_battleGrid>(parent, &definition);
}
