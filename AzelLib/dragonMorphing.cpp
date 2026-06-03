#include "PDS.h"
#include "dragonMorphing.h"
#include "processModel.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "3dEngine_textureCache.h"
#include "commonOverlay.h"

static fixedPoint morphDragonAccumulator[3];
static s16 morphDragonWeightS16[3];

// 060136c0
static void morphDragonStoreWeights(fixedPoint w0, fixedPoint w1, fixedPoint w2)
{
    morphDragonAccumulator[0] = w0;
    morphDragonAccumulator[1] = w1;
    morphDragonAccumulator[2] = w2;
    morphDragonWeightS16[0] = (s16)((s32)w0 >> 2);
    morphDragonWeightS16[1] = (s16)((s32)w1 >> 2);
    morphDragonWeightS16[2] = (s16)((s32)w2 >> 2);
}

// 0600c050
static void morphDragonSetupWeights(s32 cursorX, s32 cursorY)
{
    fixedPoint w0, w1, w2;
    if (cursorX == 0 && cursorY == 0)
    {
        w0 = 0x10000;
        w1 = fixedPoint(0);
        w2 = fixedPoint(0);
    }
    else
    {
        fixedPoint invTotal = FP_Div(0x10000, fixedPoint((cursorX + cursorY) * 0x800));
        w0 = MTH_Mul(fixedPoint((0x800 - cursorX) * cursorX + (0x800 - cursorY) * cursorY), invTotal);
        w1 = MTH_Mul(fixedPoint(cursorX * cursorX), invTotal);
        w2 = MTH_Mul(fixedPoint(cursorY * cursorY), invTotal);
    }
    morphDragonStoreWeights(w0, w1, w2);
}

// 06013510
static void morphDragonVertice(sVec3_FP& pDst, const sVec3_FP& pA, const sVec3_FP& pB, const sVec3_FP& pC)
{
    pDst = MTH_Mul(morphDragonAccumulator[0], pA)
        + MTH_Mul(morphDragonAccumulator[1], pB)
        + MTH_Mul(morphDragonAccumulator[2], pC);
}

// 060134cc — morph normals (s16 weighted blend)
static void morphDragonNormals(std::vector<sVec3_S16_12_4>& pDst, const std::vector<sVec3_S16_12_4>& pA, const std::vector<sVec3_S16_12_4>& pB, const std::vector<sVec3_S16_12_4>& pC, s32 count)
{
    assert((count % 3) == 0);
    for (s32 i = 0; i < count / 3; i++)
    {
        for (s32 j = 0; j < 3; j++)
        {
            s32 result = (s32)morphDragonWeightS16[0] * (s32)pA[i][j]
                + (s32)morphDragonWeightS16[1] * (s32)pB[i][j]
                + (s32)morphDragonWeightS16[2] * (s32)pC[i][j];
            pDst[i][j] = (s16)(result >> 14);
        }
    }
}

// variants for type-safety
static void morphDragonNormals(std::vector<sProcessed3dModel::sQuadExtra>& pDst, const std::vector<sProcessed3dModel::sQuadExtra>& pA, const std::vector<sProcessed3dModel::sQuadExtra>& pB, const std::vector<sProcessed3dModel::sQuadExtra>& pC, s32 count) {
    if (count >= 4) {
        assert(pDst.size() == 4);
        for (int i = 0; i < 4; i++) {
            for(int j=0; j< 3; j++) {
                s32 result = (s32)morphDragonWeightS16[0] * (s32)pA[i].m0_normals[j]
                    + (s32)morphDragonWeightS16[1] * (s32)pB[i].m0_normals[j]
                    + (s32)morphDragonWeightS16[2] * (s32)pC[i].m0_normals[j];
                pDst[i].m0_normals[j] = (s16)(result >> 14);
            }
        }
        if (count == 8) {
            assert(0); // untested
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 3; j++) {
                    u32 result = (u32)morphDragonWeightS16[0] * (u32)pA[i].m6_colors[j]
                        + (u32)morphDragonWeightS16[1] * (u32)pB[i].m6_colors[j]
                        + (u32)morphDragonWeightS16[2] * (u32)pC[i].m6_colors[j];
                    pDst[i].m6_colors[j] = (s16)(result >> 14);
                }
            }
        }
        else
        {
            assert(count == 4);
        }
    }
    else {
        assert(count == 1);
        s32 i = 0;
        assert(pDst.size() == 1);
        for (int j = 0; j < 3; j++) {
            s32 result = (s32)morphDragonWeightS16[0] * (s32)pA[i].m0_normals[j];
            +(s32)morphDragonWeightS16[1] * (s32)pB[i].m0_normals[j]
                + (s32)morphDragonWeightS16[2] * (s32)pC[i].m0_normals[j];
            pDst[i].m0_normals[j] = (s16)(result >> 14);
        }
    }
}


// 0601344e — morph a single 3D model node (vertices + normals + polygon data)
static void morphDragonProcessNode(sProcessed3dModel* pDst, sProcessed3dModel* pA, sProcessed3dModel* pB, sProcessed3dModel* pC)
{
    if (pDst->m4_numVertices == 0) // m4_vertexCount
        return;

    pDst->m_vertexBuffersDirty = true;

    // Morph the first vertex (scale)
    fixedPoint blendedScale = MTH_Mul(morphDragonAccumulator[0], pA->m0_radius)
        + MTH_Mul(morphDragonAccumulator[1], pB->m0_radius)
        + MTH_Mul(morphDragonAccumulator[2], pC->m0_radius);
    pDst->m0_radius = (u32)(s32)blendedScale;

    // Morph normal data
    morphDragonNormals(pDst->m8_vertices, pA->m8_vertices, pB->m8_vertices, pC->m8_vertices, pDst->m4_numVertices * 3);

    // Morph polygon vertex coordinates — iterate VDP1 polygon commands
    // Layout: [header 5 u32s][vertex data...] per polygon, terminated by sentinel
    auto pCmdDst = pDst->mC_Quads.begin();
    auto pCmdA = pA->mC_Quads.begin();
    auto pCmdB = pB->mC_Quads.begin();
    auto pCmdC = pC->mC_Quads.begin();

    while (pCmdDst != pDst->mC_Quads.end())
    {
        u8 polyType = (pCmdDst->m8_lightingControl >> 8) & 3;
        switch (polyType) {
        case 3:
            morphDragonNormals(pCmdDst->m14_extraData, pCmdA->m14_extraData, pCmdB->m14_extraData, pCmdC->m14_extraData, 4);
            break;
        case 2:
            morphDragonNormals(pCmdDst->m14_extraData, pCmdA->m14_extraData, pCmdB->m14_extraData, pCmdC->m14_extraData, 8);
            break;
        case 1:
            morphDragonNormals(pCmdDst->m14_extraData, pCmdA->m14_extraData, pCmdB->m14_extraData, pCmdC->m14_extraData, 1);
            break;
        }
        pCmdDst++;
        pCmdA++;
        pCmdB++;
        pCmdC++;
    }
}

// 0600c0c4
static void morphDragonTraverseHierarchy(sModelHierarchy* pDst, sModelHierarchy* pA, sModelHierarchy* pB, sModelHierarchy* pC)
{
    while (true)
    {
        if (pDst->m0_3dModel)
            morphDragonProcessNode(pDst->m0_3dModel, pA->m0_3dModel, pB->m0_3dModel, pC->m0_3dModel);
        if (pDst->m4_subNode)
            morphDragonTraverseHierarchy(pDst->m4_subNode, pA->m4_subNode, pB->m4_subNode, pC->m4_subNode);
        if (!pDst->m8_nextNode)
            break;
        pDst = pDst->m8_nextNode;
        pA = pA->m8_nextNode;
        pB = pB->m8_nextNode;
        pC = pC->m8_nextNode;
    }
}

// 0600c138
static void morphDragonBonePoses(s_3dModel* pModel, struct sStaticPoseData* pA, struct sStaticPoseData* pB, struct sStaticPoseData* pC)
{
    s32 numBones = pModel->m12_numBones;
    for (s32 i = 1; i < numBones; i++)
    {
        auto& pPose = pModel->m2C_poseData.at(i);
        auto& pPoseA = pA->m0_bones.at(i);
        auto& pPoseB = pB->m0_bones.at(i);
        auto& pPoseC = pC->m0_bones.at(i);

        morphDragonVertice(pPose.m0_translation, pPoseA.m0_translation, pPoseB.m0_translation, pPoseC.m0_translation);
    }
}

// 060136a8
static s32 morphDragonBlendColor(s32 a, s32 b, s32 c)
{
    return MTH_Mul(morphDragonAccumulator[0], a)
        + MTH_Mul(morphDragonAccumulator[1], b)
        + MTH_Mul(morphDragonAccumulator[2], c);
}

// 0600c180
static void morphDragonHotpoints(std::vector<s_hotpointDefinition>* pDstPairs, std::vector<s_hotpointDefinition>* pAPairs, std::vector<s_hotpointDefinition>* pBPairs, std::vector<s_hotpointDefinition>* pCPairs, s32 count)
{
    for (s32 i = 0; i < count; i++)
    {
        s32 numEntries = pDstPairs->at(i).m4_count;
        if (numEntries != 0)
        {
            auto pDst = pDstPairs->at(i).m0.begin();
            auto pA = pAPairs->at(i).m0.begin();
            auto pB = pBPairs->at(i).m0.begin();
            auto pC = pCPairs->at(i).m0.begin();
            for (s32 j = 0; j < numEntries; j++)
            {
                morphDragonVertice(pDst->m4, pA->m4, pB->m4, pC->m4);
                pDst->m10 = morphDragonBlendColor(pA->m10, pB->m10, pC->m10);
            }
        }
    }
}

// 060133a8 — blend three RGB555 palettes by the morph weights (each channel blended in place)
void morphDragonTexture(u16* dest, u16* pA, u16* pB, u16* pC, s32 count)
{
    s32 numColors = count << 4; // each entry = 16 colors
    do
    {
        u16 cC = *pC++;
        u16 cB = *pB++;
        u16 cA = *pA++;

        // Each channel is weighted in its native bit position (R: 0x1f, G: 0x3e0, B: 0x7c00)
        s32 red = (s32)MTH_Mul(morphDragonAccumulator[0], fixedPoint((s32)(cA & 0x1f)))
            + (s32)MTH_Mul(morphDragonAccumulator[1], fixedPoint((s32)(cB & 0x1f)))
            + (s32)MTH_Mul(morphDragonAccumulator[2], fixedPoint((s32)(cC & 0x1f)));
        s32 green = (s32)MTH_Mul(morphDragonAccumulator[0], fixedPoint((s32)(cA & 0x3e0)))
            + (s32)MTH_Mul(morphDragonAccumulator[1], fixedPoint((s32)(cB & 0x3e0)))
            + (s32)MTH_Mul(morphDragonAccumulator[2], fixedPoint((s32)(cC & 0x3e0)));
        s32 blue = (s32)MTH_Mul(morphDragonAccumulator[0], fixedPoint((s32)(cA & 0x7c00)))
            + (s32)MTH_Mul(morphDragonAccumulator[1], fixedPoint((s32)(cB & 0x7c00)))
            + (s32)MTH_Mul(morphDragonAccumulator[2], fixedPoint((s32)(cC & 0x7c00)));

        *dest++ = ((blue >> 16) & 0x7c00) | ((green >> 16) & 0x3e0) | ((red >> 16) & 0x1f) | 0x8000;
        numColors--;
    } while (numColors != 0);
}

void morphDragon(sDragonMorphData* pLoadDragonWorkArea, s_3dModel* pOutputDragonModel, u8* pMCB, const sDragonMorphDataPerLevel* pDragonData3, s16 cursorX, s16 cursorY)
{
    if (pDragonData3->m0_numMCBEntries == 0)
        return;

    const sDragonMorphModels* pCenter = &pDragonData3->m_m8[1];
    const sDragonMorphModels* pRight = &pDragonData3->m_m8[5];
    const sDragonMorphModels* pLeft = &pDragonData3->m_m8[3];
    const sDragonMorphModels* pUp = &pDragonData3->m_m8[4];
    const sDragonMorphModels* pDown = &pDragonData3->m_m8[2];

    const sDragonMorphModels* pB = pRight;
    const sDragonMorphModels* pC = pUp;

    s32 absX = cursorX;
    s32 absY = cursorY;

    if (cursorX < 0) { absX = -cursorX; pB = pLeft; }
    if (cursorY < 0) { absY = -cursorY; pC = pDown; }

    morphDragonSetupWeights(absX, absY);

    // m_m8 layout: [0]=output, [1]=center, [2]=down, [3]=left, [4]=up, [5]=right
    const sDragonMorphModels* pOutput = &pDragonData3->m_m8[0];
    const sDragonMorphModels* pA = pCenter;
    s_fileBundle* pBundle = pLoadDragonWorkArea->m0_dramAllocation->m0_dramData;

    // Morph model hierarchy vertices (first call — sub-models)
    morphDragonTraverseHierarchy(
        pOutputDragonModel->m4_pModelFile->getModelHierarchy(pOutput->m0_modelIndex),
        pBundle->getModelHierarchy(pA->m0_modelIndex),
        pBundle->getModelHierarchy(pB->m0_modelIndex),
        pBundle->getModelHierarchy(pC->m0_modelIndex)
    );

    // Morph model hierarchy vertices (second call — shadow models)
    morphDragonTraverseHierarchy(
        pOutputDragonModel->m4_pModelFile->getModelHierarchy(pOutput->m2_shadowModelIndex),
        pBundle->getModelHierarchy(pA->m2_shadowModelIndex),
        pBundle->getModelHierarchy(pB->m2_shadowModelIndex),
        pBundle->getModelHierarchy(pC->m2_shadowModelIndex)
    );

    // Morph bone pose data — reads raw pose offsets from bundle
    int numBones = pOutputDragonModel->m4_pModelFile->getModelHierarchy(pOutput->m2_shadowModelIndex)->countNumberOfBones();
    morphDragonBonePoses(
        pOutputDragonModel,
        pBundle->getStaticPose(pA->m4_poseModelIndex, numBones),
        pBundle->getStaticPose(pB->m4_poseModelIndex, numBones),
        pBundle->getStaticPose(pC->m4_poseModelIndex, numBones)
    );

    // Morph VDP1 palette colors
    morphDragonTexture(
        (u16*)(getVdp1Pointer(0x25C00000) + pOutput->m6_textureOffset * 8),
        (u16*)(pMCB + pA->m6_textureOffset * 8),
        (u16*)(pMCB + pB->m6_textureOffset * 8),
        (u16*)(pMCB + pC->m6_textureOffset * 8),
        pDragonData3->m0_numMCBEntries);
    invalidateVdp1TextureRange(0x25C00000 + pOutput->m6_textureOffset * 8, 8 * 8 * 2);

    // Morph hotpoint data
    morphDragonHotpoints(
        pOutput->m8_hotPoints->getData(numBones),
        pA->m8_hotPoints->getData(numBones),
        pB->m8_hotPoints->getData(numBones),
        pC->m8_hotPoints->getData(numBones),
        pDragonData3->m4_numHotPoints);
}

void freeDragonMorphData(sDragonMorphData* pDragonMorphData)
{
    /*
    if (pLoadDragonWorkArea->MCBOffsetInDram >= 0)
    {
        deleteLoadedFile(pLoadDragonWorkArea->MCBOffsetInDram);
        pLoadDragonWorkArea->MCBOffsetInDram = -1;
    }

    if (pLoadDragonWorkArea->CGBOffsetInDram >= 0)
    {
        deleteLoadedFile(pLoadDragonWorkArea->CGBOffsetInDram);
        CGBOffsetInDram->MCBOffsetInDram = -1;
    }
    */
    if (pDragonMorphData->m0_dramAllocation)
    {
        delete pDragonMorphData->m0_dramAllocation;
        pDragonMorphData->m0_dramAllocation = nullptr;
    }

    if (pDragonMorphData->m4_vramAllocation)
    {
        vdp1Free(pDragonMorphData->m4_vramAllocation);
        pDragonMorphData->m4_vramAllocation = NULL;
    }
}

void deleteDragonMorphData(sDragonMorphData* pLoadDragonWorkArea)
{
    if (pLoadDragonWorkArea)
    {
        freeDragonMorphData(pLoadDragonWorkArea);

        pLoadDragonWorkArea->getTask()->markFinished();
    }
}

sDragonMorphData* createDragonMorphData(s_workArea* pWorkArea, e_dragonLevel dragonLevel)
{
    sDragonMorphData* pDragonMorphData = createSubTaskFromFunction<sDragonMorphData>(pWorkArea, nullptr);

    pDragonMorphData->m0_dramAllocation = new sDragonBuffer;
    pDragonMorphData->m4_vramAllocation = nullptr;
    pDragonMorphData->m8_MCBInDram = pDragonMorphData->m0_dramAllocation->m18E00_MCBInDram.data();

    if (dragonFilenameTable[dragonLevel].m_morph.MCB)
    {
        loadFile(dragonFilenameTable[dragonLevel].m_morph.MCB, &pDragonMorphData->m0_dramAllocation->m0_dramData, 0);
        loadFile(dragonFilenameTable[dragonLevel].m_morph.CGB, pDragonMorphData->m8_MCBInDram, 0);
    }
    return pDragonMorphData;
}

// 0600c324
void updateDragonStats(int type, sVec3_FP* pOutput)
{
    auto& pDragonLevelStats = gCommonFile->dragonLevelStats[mainGameState.gameStats.m1_dragonLevel];

    auto pcVar4 = pDragonLevelStats.m18.begin();
    auto pcVar5 = pDragonLevelStats.m12.begin();
    auto pcVar6 = pDragonLevelStats.m0.begin();

    if (type)
    {
        pcVar4 = pDragonLevelStats.m1B.begin();
        pcVar5 = pDragonLevelStats.m15.begin();
        pcVar6 = pDragonLevelStats.m3.begin();
    }

    s16 iVar1 = mainGameState.gameStats.m1A_dragonCursorX;
    s16 iVar2 = mainGameState.gameStats.m1C_dragonCursorY;

    if (iVar1 < 0)
    {
        iVar1 = -iVar1;
        pcVar4 = pDragonLevelStats.mC.begin();
        if (type)
        {
            pcVar4 = pDragonLevelStats.mF.begin();
        }
    }

    if (iVar2 < 0)
    {
        iVar2 = -iVar2;
        pcVar5 = pDragonLevelStats.m6.begin();
        if (type)
        {
            pcVar5 = pDragonLevelStats.m9.begin();
        }
    }

    morphDragonSetupWeights(iVar1, iVar2);

    (*pOutput)[0] = pcVar6[0] * (s32)morphDragonAccumulator[0] + pcVar4[0] * (s32)morphDragonAccumulator[1] + pcVar5[0] * (s32)morphDragonAccumulator[2];
    (*pOutput)[1] = pcVar6[1] * (s32)morphDragonAccumulator[0] + pcVar4[1] * (s32)morphDragonAccumulator[1] + pcVar5[1] * (s32)morphDragonAccumulator[2];
    (*pOutput)[2] = pcVar6[2] * (s32)morphDragonAccumulator[0] + pcVar4[2] * (s32)morphDragonAccumulator[1] + pcVar5[2] * (s32)morphDragonAccumulator[2];
}
