#include "PDS.h"
#include "campDragon.h"
#include "town/town.h"
#include "town/townDragon.h"
#include "kernel/fileBundle.h"
#include "kernel/graphicalObject.h"
#include "town/collisionBody.h"

void initDragonForTown(sTownDragon* pThis); // TODO: Cleanup
void updateTownDragon(sTownDragon* pThis); // TODO: cleanup

struct sCampDragon_F8 {
    sVec3_FP m4;
    sCollisionBody m10_collisionBody;
    sVec3_FP m74;
    sVec3_FP m80;
    // size 0x8C
};

struct sCampDragon : public sTownDragon
{
    //0-E8: sTownDragon
    sVec3_FP mE8;
    s32 mF4 = 0; // count of F8
    std::array<sCampDragon_F8, 9> mF8;
    //size: 0x624
};

s32 sCampDragon_InitSub1(sTownDragon* pThis) {

    fixedPoint X = readSaturnFP(*(npcData0.m120_stack.end() - 2));
    fixedPoint Y = readSaturnFP(*(npcData0.m120_stack.end() - 1));

    return atan2_FP(X - (pThis->m58_position).m0_X, Y - (pThis->m58_position).m8_Z);
}

void increaseGameResource(int param_1, int param_2) {
    switch (param_1)
    {
    case 0xB:
        mainGameState.gameStats.m7C_overallRating = mainGameState.gameStats.m7C_overallRating + param_2;
        if ((int)mainGameState.gameStats.m7C_overallRating < 0) {
            mainGameState.gameStats.m7C_overallRating = 0;
        }
        if (0x7c < (int)mainGameState.gameStats.m7C_overallRating) {
            mainGameState.gameStats.m7C_overallRating = 0x7c;
        }
        break;
    default:
        assert(0);
        break;
    }
}

s32 sCampDragon_InitSub0() {
    s32 uVar1;
    int iVar2;

    uVar1 = mainGameState.gameStats.m7C_overallRating;
    iVar2 = readPackedBits(mainGameState.bitField, 0x1347, 0x20);
    iVar2 = uVar1 - iVar2;
    if (iVar2 < -0xf) {
        return 2;
    }
    if (iVar2 < -5) {
        return 3;
    }
    if (0xf < iVar2) {
        return 6;
    }
    if (5 < iVar2) {
        return 5;
    }
    return 4;
}

void sCampDragon_InitSub2(sCampDragon* pThis) {
    u32 temp = updateAndInterpolateAnimation(&gDragonState->m28_dragon3dModel);
    if (pThis->m24) {
        assert(0);
    }
}

void sCampDragon_Init(sTownDragon* pThisBase, sSaturnPtr arg) {
    sCampDragon* pThis = (sCampDragon*)pThisBase;
    pThis->m48_entityEA = arg;
    pThis->m4C_basePosition = pThis->m58_position = readSaturnVec3(arg + 0x8);
    pThis->m64_rotation = readSaturnVec3(arg + 0x14);
    npcData0.m70_npcPointerArray[readSaturnU16(arg + 0x20)].workArea = pThis;
    pThis->m16_timer = 0x3C;
    increaseGameResource(0xb, 0);
    pThis->mF_affinityLevel = sCampDragon_InitSub0();
    pThis->m40 = 0x8000000;
    pThis->m3C = 0x8000000;
    pThis->mD8_heightOffset = 0x1000;
    s32 temp = 0x8000000;
    if (mainGameState.getBit(0x263, 0x20) == 0) {
        temp = 0x4000000;
    }
    else {
        temp += performDivision(0x19, mainGameState.gameStats.m7C_overallRating) * 0x2000000;
    }
    pThis->m64_rotation.m4_Y = sCampDragon_InitSub1(pThis) + temp;
    pThis->m70_collisionBody.m30_pPosition = &pThis->m58_position;
    pThis->m70_collisionBody.m34_pRotation = &pThis->m64_rotation;
    pThis->m70_collisionBody.m38_pOwner = pThis;
    pThis->m70_collisionBody.m3C_scriptEA = pThis->m20_scriptEA;
    pThis->m70_collisionBody.m40 = 0;
    setCollisionSetup(&pThis->m70_collisionBody, 3);

    static const sVec3_FP param1 = {
        -0x1800,
        -0x1800,
        -0x1800,
    };

    static const sVec3_FP param2 = {
        0x1800,
        0x1000,
        0x1800,
    };

    setCollisionBounds(&pThis->m70_collisionBody, param1, param2);

    for (int i = 0; i < 9; i++) {
        auto* psVar3 = &pThis->mF8[i];
        (psVar3->m10_collisionBody).m30_pPosition = &psVar3->m4;
        (psVar3->m10_collisionBody).m34_pRotation = &pThis->mE8;
        (psVar3->m10_collisionBody).m38_pOwner = pThis;
        (psVar3->m10_collisionBody).m3C_scriptEA = sSaturnPtr::getNull();
        (psVar3->m10_collisionBody).m40 = nullptr;
        setCollisionSetup(&psVar3->m10_collisionBody, 3);
        setCollisionBounds(&psVar3->m10_collisionBody, psVar3->m74, psVar3->m80);
    }
    initDragonForTown(pThis);
    pThis->m14_readyState = 1;

    playAnimation(&gDragonState->m28_dragon3dModel, pThis->m1C->m0_fileBundle->getAnimation(readSaturnU16(pThis->m20_scriptEA + 2)), 0);
    sCampDragon_InitSub2(pThis);
}

void sCampDragon_UpdateSub0(sTownDragon* pThis) {
    Unimplemented();
}

void sCampDragon_UpdateMode4(sTownDragon* pThis) {
    switch (pThis->m11_subState) {
    case 0:
        pThis->m12_eventFlag = 1;
        pThis->m11_subState = 1;
        break;
    case 1:
        Unimplemented();
        break;
    default:
        assert(0);
        break;
    }
}

void sCampDragon_Update(sTownDragon* pThisBase) {
    sCampDragon* pThis = (sCampDragon*)pThisBase;

    pThis->mD_drawExtras = 1;
    updateTownDragon(pThis);

    if ((gDragonState->mC_dragonType < 0) || (7 < gDragonState->mC_dragonType)) {
        return;
    }

    pThis->m58_position = pThis->m4C_basePosition;
    pThis->m58_position[1] += pThis->mD8_heightOffset;
    registerCollisionBody(&pThis->m70_collisionBody);

    for (int i = 0; i < pThis->mF4; i++) {
        assert(0);
    }

    sCampDragon_UpdateSub0(pThis);

    if (pThis->mE == 0) {
        switch (pThis->mF_affinityLevel + pThis->m10_modeOffset) {
        case 4:
            sCampDragon_UpdateMode4(pThis);
            break;
        default:
            assert(0);
            break;
        }
    }
    else if ((pThis->mE & 1) == 0) {
        assert(0);
    }
    else {
        assert(0);
    }

    if (pThis->m12_eventFlag == 0) {
        Unimplemented();
    }

    Unimplemented();
}

void updateAnimationMatricesSub1WithScale(s3DModelAnimData* r4, s_3dModel* r5)
{
    u32 r9 = r5->m12_numBones;
    if (r9)
    {
        std::vector<sMatrix4x3>::iterator r14 = r4->m4_boneMatrices->begin();
        if (r5->m48_poseDataInterpolation.size())
        {
            std::vector<sPoseDataInterpolation>::iterator r13 = r5->m48_poseDataInterpolation.begin();

            do
            {
                initMatrixToIdentity(&(*r14));
                translateMatrix(&r13->m0_translation, &(*r14));
                rotateMatrixZYX(&r13->mC_rotation, &(*r14));
                //scaleMatrixRow0(r13->m18_scale[0], &(*r14));
                //scaleMatrixRow1(r13->m18_scale[1], &(*r14));
                //scaleMatrixRow2(r13->m18_scale[2], &(*r14));
                r13++;
                r14++;
            } while (--r9);
        }
        else
        {
            std::vector<sPoseData>::iterator r13 = r5->m2C_poseData.begin();

            do
            {
                initMatrixToIdentity(&(*r14));
                translateMatrix(&r13->m0_translation, &(*r14));
                rotateMatrixZYX(&r13->mC_rotation, &(*r14));
                //scaleMatrixRow0(r13->m18_scale[0], &(*r14));
                //scaleMatrixRow1(r13->m18_scale[1], &(*r14));
                //scaleMatrixRow2(r13->m18_scale[2], &(*r14));
                r13++;
                r14++;
            } while (--r9);
        }
    }
}

void updateAnimationMatricesSub2(s3DModelAnimData* r4); // todo: cleanup

void sCampDragon_Draw(sTownDragon* pThisBase) {
    sCampDragon* pThis = (sCampDragon*)pThisBase;

    if ((-1 < gDragonState->mC_dragonType) && (gDragonState->mC_dragonType < 8))
    {
        updateAnimationMatricesSub1WithScale(&gDragonState->m78_animData, &gDragonState->m28_dragon3dModel);
        updateAnimationMatricesSub2(&gDragonState->m78_animData);

        Unimplemented();
        submitModelAndShadowModelToRendering(&gDragonState->m28_dragon3dModel, gDragonState->m14_modelIndex, gDragonState->m18_shadowModelIndex, &pThis->m58_position, &pThis->m64_rotation, 0);
        if (pThis->mD_drawExtras)
        {
            Unimplemented();
        }
        if (enableDebugTask) {
            assert(0);
        }
    }
}

void sCampDragon_Delete(sTownDragon* pThis) {
    Unimplemented();
}

sTownObject* createCampDragon(p_workArea parent, sSaturnPtr arg) {
    static const sCampDragon::TypedTaskDefinition definition = {
        &sCampDragon_Init,
        &sCampDragon_Update,
        &sCampDragon_Draw,
        &sCampDragon_Delete,
    };

    return createSubTaskWithArg<sCampDragon, sSaturnPtr>(parent, arg, &definition);
}
