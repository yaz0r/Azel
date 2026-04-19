#include "PDS.h"
#include "battleEnemyModels.h"
#include "battleTargetable.h"
#include "kernel/graphicalObject.h"
#include "kernel/animation.h"
#include "kernel/fileBundle.h"
#include "mainMenuDebugTasks.h"

// 060a33a2 (BTL_A5) / 0609c5b2 (BTL_A3) — shared across all battle overlays
s32 createBattleEnemyModels(s_workArea* pOwner, s_fileBundle* pBundle, u8& modelCount, std::vector<s_3dModel>& models, sSaturnPtr dataTable, u8 count)
{
    modelCount = count;
    models.resize(count);

    for (int i = 0; i < count; i++)
    {
        sSaturnPtr entry = dataTable + i * 8;
        u16 modelOffset = readSaturnU16(entry);
        u16 poseOffset = readSaturnU16(entry + 2);
        sSaturnPtr hotspotDataEA = readSaturnEA(entry + 4);
        s_3dModel* pModel = &models[i];

        sModelHierarchy* pHierarchy = pBundle->getModelHierarchy(modelOffset);
        u32 numBones = pHierarchy->countNumberOfBones();

        sHotpointBundle* pHotSpotsData = nullptr;
        if (!hotspotDataEA.isNull())
        {
            pModel->m_hotpointBundles.reserve(numBones);
            for (u32 b = 0; b < numBones; b++)
                pModel->m_hotpointBundles.emplace_back(hotspotDataEA + b * 8);
            pHotSpotsData = pModel->m_hotpointBundles.data();
        }

        s32 result = init3DModelRawData(pOwner, pModel, 0, pBundle, modelOffset,
                                         nullptr, pBundle->getStaticPose(poseOffset, numBones),
                                         nullptr, pHotSpotsData);
        if (result == 0)
        {
            pOwner->getTask()->markFinished();
            return 0;
        }
        stepAnimation(pModel);
    }
    return 1;
}

// 060a35bc (BTL_A5) — shared across all battle overlays
static void initModelTargetables(s_workArea* pOwner, s_3dModel* pModel)
{
    pModel->m50_targetBoneCount = pModel->m12_numBones;
    pModel->m52_targetableCount = 0;
    for (int i = 0; i < pModel->m50_targetBoneCount; i++)
    {
        pModel->m52_targetableCount += (s16)pModel->m44_hotpointData[i].size();
    }
    pModel->m54_targetables = (sBattleTargetable*)allocateHeapForTask(pOwner, pModel->m52_targetableCount * sizeof(sBattleTargetable));
    pModel->m58_targetablePositions = (sVec3_FP*)allocateHeapForTask(pOwner, pModel->m52_targetableCount * sizeof(sVec3_FP));

    int idx = 0;
    for (int i = 0; i < pModel->m50_targetBoneCount; i++)
    {
        if (pModel->m44_hotpointData[i].size() != 0)
        {
            for (int j = 0; j < (int)pModel->m44_hotpointData[i].size(); j++)
            {
                initTargetable(&pModel->m54_targetables[idx], (s_workAreaCopy*)pOwner,
                    &pModel->m58_targetablePositions[idx],
                    pModel->m44_hotpointData[i][j][3],
                    pModel->m44_hotpointData[i][j][0],
                    0, 0, 10);
                idx++;
            }
        }
    }
}

// 060a344e (BTL_A5) — shared across all battle overlays
void initBattleEnemyTargetableData(u8* pEnemy)
{
    *(u16*)(pEnemy + 0xB4) = 0;
    *(u32*)(pEnemy + 0xB8) = 0x060a3a4c;
    *(u32*)(pEnemy + 0xBC) = 0x060a3a50;
    *(u32*)(pEnemy + 0xC0) = 0x060a3a74;
    *(u32*)(pEnemy + 0xC4) = 0x060a3a98;
    *(u16*)(pEnemy + 0x144) = 10;
    *(u16*)(pEnemy + 0x146) = 10;
    *(u16*)(pEnemy + 0x148) = 10;
    *(u16*)(pEnemy + 0x14A) = 10;
    *(u8*)(pEnemy + 0x150) = 0;
    *(u32*)(pEnemy + 0x154) = 0;
}

// 060a3488 (BTL_A5) — shared across all battle overlays
void initBattleEnemyTargetables(s_workArea* pEnemy, u8 modelCount, std::vector<s_3dModel>& models)
{
    initBattleEnemyTargetableData((u8*)pEnemy);

    for (int i = 0; i < modelCount; i++)
    {
        s_3dModel* pModel = &models[i];
        if (pModel->m40 == nullptr)
        {
            pModel->m52_targetableCount = 0;
            pModel->m50_targetBoneCount = pModel->m12_numBones;
            pModel->m54_targetables = nullptr;
            pModel->m58_targetablePositions = nullptr;
        }
        else
        {
            initModelTargetables(pEnemy, pModel);
        }
    }
}
