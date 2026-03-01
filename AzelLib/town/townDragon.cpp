#include "PDS.h"
#include "townDragon.h"
#include "town.h"
#include "kernel/fileBundle.h"
#include "kernel/graphicalObject.h"
#include "town/ruin/twn_ruin.h" // TODO: cleanup

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
        pThis->mC_dragonType = gDragonState->mC_dragonType;
        pThis->m14_readyState = -1;
        pThis->m1C = dramAllocatorEnd[fileIndex].mC_fileBundle;

        if (gCurrentTownOverlay->m_name == "TWN_EXCA.PRG") {
            pThis->m20_scriptEA = readSaturnEA(gCurrentTownOverlay->getSaturnPtr(0x606471C) + gDragonState->mC_dragonType * 4);
            reinitModel(&gDragonState->m28_dragon3dModel, readRiderDefinitionSub(readSaturnEA(gCurrentTownOverlay->getSaturnPtr(0x6064608 + gDragonState->mC_dragonType * 0x1C))));
        }
        else if (gCurrentTownOverlay->m_name == "TWN_CAMP.PRG") {
            pThis->m20_scriptEA = readSaturnEA(gCurrentTownOverlay->getSaturnPtr(0x607af3c) + gDragonState->mC_dragonType * 4);
            reinitModel(&gDragonState->m28_dragon3dModel, readRiderDefinitionSub(readSaturnEA(gCurrentTownOverlay->getSaturnPtr(0x607abf0 + gDragonState->mC_dragonType * 0x1C))));
        }
        else {
            assert(0);
        }
        
        Unimplemented();
    }
    gDragonState->m88 = 0;
    pThis->mD4_cursorX = mainGameState.gameStats.m1A_dragonCursorX;
    pThis->mD6_cursorY = mainGameState.gameStats.m1C_dragonCursorY;
}

static void sTownDragon_Init(sTownDragon* pThis, sSaturnPtr arg)
{
    pThis->m48_entityEA = arg;
    pThis->m4C_basePosition = pThis->m58_position = readSaturnVec3(arg + 0x8);
    pThis->m64_rotation = readSaturnVec3(arg + 0x14);
    pThis->mD8_heightOffset = 0x1000;
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
    initDragonForTown(pThis);
    pThis->m14_readyState = 1;

    playAnimation(&gDragonState->m28_dragon3dModel, pThis->m1C->m0_fileBundle->getAnimation(readSaturnU16(pThis->m20_scriptEA + 2)), 0);
}

void updateTownDragon(sTownDragon* pThis)
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
    pThis->mD_drawExtras = 1;
    updateTownDragon(pThis);
    if ((-1 < gDragonState->mC_dragonType) && (gDragonState->mC_dragonType < 8))
    {
        pThis->m58_position[0] = pThis->m4C_basePosition[0];
        pThis->m58_position[1] = pThis->m4C_basePosition[1] + pThis->mD8_heightOffset;
        pThis->m58_position[2] = pThis->m4C_basePosition[2];
        registerCollisionBody(&pThis->m70_collisionBody);
        updateAndInterpolateAnimation(&gDragonState->m28_dragon3dModel);
        updateAnimationMatrices(&gDragonState->m78_animData, &gDragonState->m28_dragon3dModel);
    }
}

static void sTownDragon_Draw(sTownDragon* pThis)
{
    if ((-1 < gDragonState->mC_dragonType) && (gDragonState->mC_dragonType < 8))
    {
        submitModelAndShadowModelToRendering(&gDragonState->m28_dragon3dModel, gDragonState->m14_modelIndex, gDragonState->m18_shadowModelIndex, &pThis->m58_position, &pThis->m64_rotation, 0);
        if (pThis->mD_drawExtras)
        {
            Unimplemented();
        }
    }
}

static void sTownDragon_Delete(sTownDragon* pThis)
{
    Unimplemented();
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

