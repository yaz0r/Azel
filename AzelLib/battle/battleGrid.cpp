#include "PDS.h"
#include "battleGrid.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDebug.h"
#include "battleOverlay_C.h"
#include "town/town.h" // todo: clean

#include "kernel/debug/trace.h"

void battleGrid_initSub0()
{
    getBattleManager()->m10_battleOverlay->m8_gridTask->m218_fov = 0x11c71c7;
    getBattleManager()->m10_battleOverlay->m8_gridTask->m1C8_flags |= 0x20;
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
    pThis->m64_cameraRotationTarget.zeroize();
    pThis->m70.zeroize();
    pThis->mE4_currentCameraReferenceCenter.zeroize();
    pThis->mF0_currentCameraReferenceForward.zeroize();
    pThis->mFC_cameraReferenceUp.zeroize();
    pThis->m108_deltaCameraPosition.zeroize();
    pThis->m114_deltaCameraTarget.zeroize();
    pThis->m12C_cameraInterpolationSpeed = 0x1999;
    pThis->m130_cameraInterpolationSpeed2 = 0x6666;

    pThis->m134_desiredCameraPosition = readSaturnVec3(gCurrentBattleOverlay->getSaturnPtr(0x60ac478) + pThis->m1 * 0x24);
    pThis->m140_desiredCameraTarget = readSaturnVec3(gCurrentBattleOverlay->getSaturnPtr(0x60ac478) + pThis->m1 * 0x24 + 0xC);
    pThis->mFC_cameraReferenceUp = readSaturnVec3(gCurrentBattleOverlay->getSaturnPtr(0x60ac478) + pThis->m1 * 0x24 + 0x18);

    pThis->mB4_cameraRotation.zeroize();
    pThis->mC0_cameraRotationInterpolation.zeroize();

    pThis->mCC_cameraInterpolationRotationFactor[0] = 0x6666;
    pThis->mCC_cameraInterpolationRotationFactor[1] = 0x2666;
    pThis->mCC_cameraInterpolationRotationFactor[2] = 0x4CCC;

    pThis->mD8_cameraInterpolationRotationFactor2[0] = 0x6666;
    pThis->mD8_cameraInterpolationRotationFactor2[1] = 0x4CCC;
    pThis->mD8_cameraInterpolationRotationFactor2[2] = 0x8000;

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
    pThis->m64_cameraRotationTarget[0] = getBattleManager()->m10_battleOverlay->m4_battleEngine->m1C8;
    pThis->m64_cameraRotationTarget[1] = getBattleManager()->m10_battleOverlay->m4_battleEngine->m1CC;

    pThis->m1B4 = 0;
    pThis->m1B0 = 0;
}

void battleGrid_updateSub1Sub0(s_battleGrid* pThis)
{
    pThis->m40 += MTH_Mul(0x3333, -pThis->m28);
    pThis->m40 -= MTH_Mul(0x8000, pThis->m40);
    pThis->m28 += pThis->m40;

    pThis->m90 += MTH_Mul(0x3333, -pThis->m84_cameraRotationDelta);
    pThis->m90 -= MTH_Mul(0x8000, pThis->m90);
    pThis->m84_cameraRotationDelta += pThis->m90;
}

void battleGrid_updateSub1Sub1(s_battleGrid* pThis)
{
    pThis->m108_deltaCameraPosition[0] += MTH_Mul(pThis->m134_desiredCameraPosition[0] - pThis->mE4_currentCameraReferenceCenter[0], pThis->m12C_cameraInterpolationSpeed);
    pThis->m108_deltaCameraPosition[0] -= MTH_Mul(pThis->m108_deltaCameraPosition[0], pThis->m130_cameraInterpolationSpeed2);
    pThis->mE4_currentCameraReferenceCenter[0] += pThis->m108_deltaCameraPosition[0];

    pThis->m108_deltaCameraPosition[1] += MTH_Mul(pThis->m134_desiredCameraPosition[1] - pThis->mE4_currentCameraReferenceCenter[1], pThis->m12C_cameraInterpolationSpeed);
    pThis->m108_deltaCameraPosition[1] -= MTH_Mul(pThis->m108_deltaCameraPosition[1], pThis->m130_cameraInterpolationSpeed2);
    pThis->mE4_currentCameraReferenceCenter[1] += pThis->m108_deltaCameraPosition[1];

    pThis->m108_deltaCameraPosition[2] += MTH_Mul(pThis->m134_desiredCameraPosition[2] - pThis->mE4_currentCameraReferenceCenter[2], pThis->m12C_cameraInterpolationSpeed);
    pThis->m108_deltaCameraPosition[2] -= MTH_Mul(pThis->m108_deltaCameraPosition[2], pThis->m130_cameraInterpolationSpeed2);
    pThis->mE4_currentCameraReferenceCenter[2] += pThis->m108_deltaCameraPosition[2];

    pThis->m114_deltaCameraTarget[0] += MTH_Mul(pThis->m140_desiredCameraTarget[0] - pThis->mF0_currentCameraReferenceForward[0], pThis->m12C_cameraInterpolationSpeed);
    pThis->m114_deltaCameraTarget[0] -= MTH_Mul(pThis->m114_deltaCameraTarget[0], pThis->m130_cameraInterpolationSpeed2);
    pThis->mF0_currentCameraReferenceForward[0] += pThis->m114_deltaCameraTarget[0];

    pThis->m114_deltaCameraTarget[1] += MTH_Mul(pThis->m140_desiredCameraTarget[1] - pThis->mF0_currentCameraReferenceForward[1], pThis->m12C_cameraInterpolationSpeed);
    pThis->m114_deltaCameraTarget[1] -= MTH_Mul(pThis->m114_deltaCameraTarget[1], pThis->m130_cameraInterpolationSpeed2);
    pThis->mF0_currentCameraReferenceForward[1] += pThis->m114_deltaCameraTarget[1];

    pThis->m114_deltaCameraTarget[2] += MTH_Mul(pThis->m140_desiredCameraTarget[2] - pThis->mF0_currentCameraReferenceForward[2], pThis->m12C_cameraInterpolationSpeed);
    pThis->m114_deltaCameraTarget[2] -= MTH_Mul(pThis->m114_deltaCameraTarget[2], pThis->m130_cameraInterpolationSpeed2);
    pThis->mF0_currentCameraReferenceForward[2] += pThis->m114_deltaCameraTarget[2];
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

    pThis->m34_cameraPosition = getBattleManager()->m10_battleOverlay->m4_battleEngine->m104_dragonStartPosition + pThis->m1C + pThis->m28;

    pThis->m180_cameraTranslation[0] = (*pThis->m1B8_pCameraTranslationSource)[0];
    pThis->m180_cameraTranslation[1] = (*pThis->m1B8_pCameraTranslationSource)[1];
    
    if (getBattleManager()->m10_battleOverlay->mC->m204_cameraMaxAltitude + 0x1000 >= pThis->m180_cameraTranslation[1])
    {
        pThis->m180_cameraTranslation[1] = getBattleManager()->m10_battleOverlay->mC->m204_cameraMaxAltitude + 0x1000;
    }
    else if (getBattleManager()->m10_battleOverlay->mC->m200_cameraMinAltitude - 0x1000 <= pThis->m180_cameraTranslation[1])
    {
        pThis->m180_cameraTranslation[1] = getBattleManager()->m10_battleOverlay->mC->m200_cameraMinAltitude - 0x1000;
    }
    
    pThis->m180_cameraTranslation[2] = (*pThis->m1B8_pCameraTranslationSource)[2];

    if (isTraceEnabled())
    {
        addTraceLog((*pThis->m1B8_pCameraTranslationSource), "cameraTranslationSource");
        addTraceLog(pThis->m180_cameraTranslation, "cameraTranslation");
    }
}

void battleGrid_updateSub1Sub3(s_battleGrid* pThis)
{
    if (isTraceEnabled())
    {
        addTraceLog(pThis->m1BC_cameraRotationStep, "m1BC_cameraRotationStep");
        addTraceLog(pThis->m84_cameraRotationDelta, "m84_cameraRotationDelta");
        addTraceLog(pThis->mB4_cameraRotation, "mB4_cameraRotation");
        addTraceLog(pThis->mC0_cameraRotationInterpolation, "mC0_cameraRotationInterpolation");
    }

    pThis->m64_cameraRotationTarget[0] += pThis->m1BC_cameraRotationStep[0];
    pThis->m64_cameraRotationTarget[1] += pThis->m1BC_cameraRotationStep[1];

    pThis->mCC_cameraInterpolationRotationFactor[0] = 0x1999;
    pThis->mCC_cameraInterpolationRotationFactor[1] = 0x1999;
    pThis->mCC_cameraInterpolationRotationFactor[2] = 0x1999;

    pThis->mD8_cameraInterpolationRotationFactor2[0] = 0x8000;
    pThis->mD8_cameraInterpolationRotationFactor2[1] = 0x8000;
    pThis->mD8_cameraInterpolationRotationFactor2[2] = 0x8000;

    pThis->mB4_cameraRotation += pThis->m84_cameraRotationDelta;

    pThis->mC0_cameraRotationInterpolation[0] += MTH_Mul(fixedPoint(std::min(int(pThis->m64_cameraRotationTarget[0]), 0x382d82d) - pThis->mB4_cameraRotation[0]).normalized(), pThis->mCC_cameraInterpolationRotationFactor[0]);
    pThis->mC0_cameraRotationInterpolation[0] -= MTH_Mul(pThis->mC0_cameraRotationInterpolation[0], pThis->mD8_cameraInterpolationRotationFactor2[0]);
    pThis->mB4_cameraRotation[0] += pThis->mC0_cameraRotationInterpolation[0];

    pThis->mC0_cameraRotationInterpolation[1] += MTH_Mul(fixedPoint(pThis->m64_cameraRotationTarget[1] - pThis->mB4_cameraRotation[1]).normalized(), pThis->mCC_cameraInterpolationRotationFactor[1]);
    pThis->mC0_cameraRotationInterpolation[1] -= MTH_Mul(pThis->mC0_cameraRotationInterpolation[1], pThis->mD8_cameraInterpolationRotationFactor2[1]);
    pThis->mB4_cameraRotation[1] += pThis->mC0_cameraRotationInterpolation[1];

    pThis->mC0_cameraRotationInterpolation[2] += MTH_Mul(fixedPoint(pThis->m64_cameraRotationTarget[2] - pThis->mB4_cameraRotation[2]).normalized(), pThis->mCC_cameraInterpolationRotationFactor[2]);
    pThis->mC0_cameraRotationInterpolation[2] -= MTH_Mul(pThis->mC0_cameraRotationInterpolation[2], pThis->mD8_cameraInterpolationRotationFactor2[2]);
    pThis->mB4_cameraRotation[2] += pThis->mC0_cameraRotationInterpolation[2];
}

void battleGrid_updateSub1(s_battleGrid* pThis)
{
    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0xB] == 0)
    {
        battleGrid_updateSub1Sub0(pThis);
        battleGrid_updateSub1Sub1(pThis);
        initMatrixToIdentity(&pThis->m150_cameraMatrix);
        battleGrid_updateSub1Sub2(pThis);
        translateMatrix(&pThis->m180_cameraTranslation, &pThis->m150_cameraMatrix);
        if ((pThis->m1C8_flags & 2) == 0)
        {
            battleGrid_updateSub1Sub3(pThis);
        }
        else
        {
            assert(0);
            //battleGrid_updateSub1Sub4(iParm1);
        }
        rotateMatrixZYX(&pThis->mB4_cameraRotation, &pThis->m150_cameraMatrix);

        if (isTraceEnabled())
        {
            addTraceLog(pThis->m180_cameraTranslation, "cameraTranslation");
            addTraceLog(pThis->mB4_cameraRotation, "cameraRotation");
        }
    }
}

void battleGrid_updateSub2(s_battleGrid* pThis)
{
    if (pThis->m1C8_flags & 4)
    {
        assert(0);
    }
    if (pThis->m1C8_flags & 8)
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

    if ((pThis->m1C8_flags & 0x10) && !(pThis->m1C8_flags & 0x40))
    {
        mainLogicUpdateSub0(pThis->m180_cameraTranslation[0], pThis->m180_cameraTranslation[2]);
    }
}

void battleGrid_draw(s_battleGrid* pThis)
{
    if (isTraceEnabled())
    {
        addTraceLog(pThis->m150_cameraMatrix, "pThis->m150_cameraMatrix");
    }

    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0xB])
    {
        assert(0);
    }
    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0xC])
    {
        assert(0);
    }

    sVec3_FP transformedCameraUp;
    sVec3_FP transformedCameraTarget;
    sVec3_FP transformedCameraPosition;
    transformAndAddVec(pThis->mFC_cameraReferenceUp, transformedCameraUp, pThis->m150_cameraMatrix);
    transformAndAddVec(pThis->mF0_currentCameraReferenceForward, transformedCameraTarget, pThis->m150_cameraMatrix);
    transformAndAddVec(pThis->mE4_currentCameraReferenceCenter, transformedCameraPosition, pThis->m150_cameraMatrix);

    fixedPoint iVar3 = transformedCameraPosition[1] - 0x1000;
    if (iVar3 < getBattleManager()->m10_battleOverlay->mC->m204_cameraMaxAltitude)
    {
        transformedCameraTarget[1] -= iVar3 - getBattleManager()->m10_battleOverlay->mC->m204_cameraMaxAltitude;
        transformedCameraPosition[1] -= iVar3 - getBattleManager()->m10_battleOverlay->mC->m204_cameraMaxAltitude;
    }

    if (getBattleManager()->m10_battleOverlay->mC->m200_cameraMinAltitude + pThis->m134_desiredCameraPosition[2] < transformedCameraPosition[1])
    {
        transformedCameraTarget[1] -= getBattleManager()->m10_battleOverlay->mC->m200_cameraMinAltitude - pThis->m134_desiredCameraPosition[2];
        transformedCameraPosition[1] -= getBattleManager()->m10_battleOverlay->mC->m200_cameraMinAltitude - pThis->m134_desiredCameraPosition[2];
    }

    generateCameraMatrix(&cameraProperties2, transformedCameraPosition, transformedCameraTarget, transformedCameraUp);

    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0x12])
    {
        assert(0);
    }

    sVec3_FP lightVector;
    lightVector[0] = -MTH_Mul_5_6(0x10000, getCos(pThis->m280_lightAngle1.getInteger()), getSin(pThis->m284_lightAngle2.getInteger()));
    lightVector[1] = MTH_Mul(0x10000, getSin(pThis->m280_lightAngle1.getInteger()));
    lightVector[2] = -MTH_Mul_5_6(0x10000, getCos(pThis->m280_lightAngle1.getInteger()), getCos(pThis->m284_lightAngle2.getInteger()));

    sVec3_FP lightVectorWorld;
    transformVecByCurrentMatrix(lightVector, lightVectorWorld);
    setupLight(lightVectorWorld[0], lightVectorWorld[1], lightVectorWorld[2], s_RGB8::fromVector(pThis->m1CC_lightColor).toU32());
    generateLightFalloffMap(
        s_RGB8::fromVector(pThis->m1E4_lightFalloff0).toU32(),
        s_RGB8::fromVector(pThis->m1FC_lightFalloff1).toU32(),
        s_RGB8::fromVector(pThis->m208_lightFalloff2).toU32()
    );

    if (getBattleManager()->m10_battleOverlay->m10_inBattleDebug->mFlags[0xD])
    {
        assert(0);
    }

    if (pThis->m1C8_flags & 0x20)
    {
        initVDP1Projection(pThis->m218_fov, 0);
        pThis->m1C8_flags &= ~0x20;
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

#ifndef SHIPPING_BUILD

static const std::vector<s_memberDefinitions> members = {
    MEMBER_TYPE(s_battleGrid, m0),
    MEMBER_TYPE(s_battleGrid, m1),
    MEMBER_TYPE(s_battleGrid, m2),
    MEMBER_TYPE(s_battleGrid, m4),
    MEMBER_TYPE(s_battleGrid, m8),
    MEMBER_TYPE(s_battleGrid, mC),
    MEMBER_TYPE(s_battleGrid, m10),
    MEMBER_TYPE(s_battleGrid, m1C),
    MEMBER_TYPE(s_battleGrid, m28),
    MEMBER_TYPE(s_battleGrid, m34_cameraPosition),
    MEMBER_TYPE(s_battleGrid, m40),
    MEMBER_TYPE(s_battleGrid, m64_cameraRotationTarget),
    MEMBER_TYPE(s_battleGrid, m70),
    MEMBER_TYPE(s_battleGrid, m84_cameraRotationDelta),
    MEMBER_TYPE(s_battleGrid, m90),
    MEMBER_TYPE(s_battleGrid, mB4_cameraRotation),
    MEMBER_TYPE(s_battleGrid, mC0_cameraRotationInterpolation),
    MEMBER_TYPE(s_battleGrid, mCC_cameraInterpolationRotationFactor),
    MEMBER_TYPE(s_battleGrid, mD8_cameraInterpolationRotationFactor2),
    MEMBER_TYPE(s_battleGrid, mE4_currentCameraReferenceCenter),
    MEMBER_TYPE(s_battleGrid, mF0_currentCameraReferenceForward),
    MEMBER_TYPE(s_battleGrid, mFC_cameraReferenceUp),
    MEMBER_TYPE(s_battleGrid, m108_deltaCameraPosition),
    MEMBER_TYPE(s_battleGrid, m114_deltaCameraTarget),
};

void s_battleGrid::Inspect()
{
    InspectTask(str(s_battleGrid), this, members);
}
#endif
