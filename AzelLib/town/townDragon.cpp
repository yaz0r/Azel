#include "PDS.h"
#include "townDragon.h"
#include "town.h"
#include "kernel/fileBundle.h"
#include "kernel/graphicalObject.h"
#include "town/ruin/twn_ruin.h" // TODO: cleanup

struct sTownDragon : public s_workAreaTemplateWithArg<sTownDragon, sSaturnPtr>, sTownObject
{
    s8 mC;
    s8 mD;
    s16 m14;
    npcFileDeleter* m1C;
    sSaturnPtr m20;
    sSaturnPtr m48;
    sVec3_FP m4C;
    sVec3_FP m58;
    sVec3_FP m64;
    sMainLogic_74 m70;
    s16 mD4_cursorX;
    s16 mD6_cursorY;
    fixedPoint mD8;
    //size: 0xE8
};

bool reinitModel(s_3dModel* pModel, sHotpointBundle* param2)
{
    if (pModel->m40)
    {
        pModel->m44_hotpointData.clear();
    }

    if (param2 == nullptr)
    {
        pModel->m40 = nullptr;
    }
    else
    {
        if (!model_initHotpointBundle(pModel, param2))
            return false;
    }
    initModelDrawFunction(pModel);
    return true;
}

void initDragonForTown(sTownDragon* pThis)
{
    if ((-1 < gDragonState->mC_dragonType) && (gDragonState->mC_dragonType < 8))
    {
        int fileIndex = gDragonState->mC_dragonType * 2 + 10;
        allocateNPC(pThis, fileIndex);
        pThis->mC = gDragonState->mC_dragonType;
        pThis->m14 = -1;
        pThis->m1C = dramAllocatorEnd[fileIndex].mC_fileBundle;

        assert(gCurrentTownOverlay->m_name == "TWN_EXCA.PRG");

        pThis->m20 = readSaturnEA(gCurrentTownOverlay->getSaturnPtr(0x606471C) + gDragonState->mC_dragonType * 4);
        reinitModel(&gDragonState->m28_dragon3dModel, readRiderDefinitionSub(readSaturnEA(gCurrentTownOverlay->getSaturnPtr(0x6064608 + gDragonState->mC_dragonType * 0x1C))));
        
        FunctionUnimplemented();
    }
    gDragonState->m88 = 0;
    pThis->mD4_cursorX = mainGameState.gameStats.m1A_dragonCursorX;
    pThis->mD6_cursorY = mainGameState.gameStats.m1C_dragonCursorY;
}

static void sTownDragon_Init(sTownDragon* pThis, sSaturnPtr arg)
{
    pThis->m48 = arg;
    pThis->m4C = pThis->m58 = readSaturnVec3(arg + 0x8);
    pThis->m64 = readSaturnVec3(arg + 0x14);
    pThis->mD8 = 0x1000;
    pThis->m70.m30_pPosition = &pThis->m58;
    pThis->m70.m34_pRotation = &pThis->m64;
    pThis->m70.m38_pOwner = pThis;
    pThis->m70.m3C_scriptEA = pThis->m20;
    pThis->m70.m40 = 0;
    mainLogicInitSub0(&pThis->m70, 3);

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

    mainLogicInitSub1(&pThis->m70, param1, param2);
    initDragonForTown(pThis);
    pThis->m14 = 1;

    playAnimation(&gDragonState->m28_dragon3dModel, pThis->m1C->m0_fileBundle->getAnimation(readSaturnU16(pThis->m20 + 2)), 0);
}

static void updateTownDragon(sTownDragon* pThis)
{
    if (gDragonState->m88)
    {
        assert(0);
    }
    if (gDragonState->m88 || (pThis->mD4_cursorX != mainGameState.gameStats.m1A_dragonCursorX) || (pThis->mD6_cursorY != mainGameState.gameStats.m1C_dragonCursorY))
    {
        assert(0);
    }
}

static void sTownDragon_Update(sTownDragon* pThis)
{
    pThis->mD = 1;
    updateTownDragon(pThis);
    if ((-1 < gDragonState->mC_dragonType) && (gDragonState->mC_dragonType < 8))
    {
        pThis->m58[0] = pThis->m4C[0];
        pThis->m58[1] = pThis->m4C[1] + pThis->mD8;
        pThis->m58[2] = pThis->m4C[2];
        EdgeUpdateSub0(&pThis->m70);
        updateAndInterpolateAnimation(&gDragonState->m28_dragon3dModel);
        updateAnimationMatrices(&gDragonState->m78_animData, &gDragonState->m28_dragon3dModel);
    }
}

static void sTownDragon_Draw(sTownDragon* pThis)
{
    if ((-1 < gDragonState->mC_dragonType) && (gDragonState->mC_dragonType < 8))
    {
        submitModelAndShadowModelToRendering(&gDragonState->m28_dragon3dModel, gDragonState->m14_modelIndex, gDragonState->m18_shadowModelIndex, &pThis->m58, &pThis->m64, 0);
        if (pThis->mD)
        {
            FunctionUnimplemented();
        }
    }
}

static void sTownDragon_Delete(sTownDragon* pThis)
{
    FunctionUnimplemented();
}

sTownObject* createTownDragon(p_workArea parent, sSaturnPtr arg)
{
    static const sTownDragon::TypedTaskDefinition definition = {
        &sTownDragon_Init,
        &sTownDragon_Update,
        &sTownDragon_Draw,
        &sTownDragon_Delete,
    };

    return createSubTaskWithArg<sTownDragon, sSaturnPtr>(parent, arg, &definition);
}

