#include "PDS.h"
#include "dragonData.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "commonOverlay.h"
#include "3dEngine_textureCache.h"
#include "kernel/vdp1Allocator.h"

sHotpointBundle* readRiderDefinitionSub(sSaturnPtr ptrEA); // TODO cleanup

s_fileBundle* gDragonModel = nullptr;
u8 gDragonVram[0x4000];

struct sDragonAnimDataSub
{
    s32 count;
    sDragonAnimDataSubRanges* m_data;
};

struct sDragonAnimData
{
    sDragonAnimDataSub* m0;
    std::array<std::vector<sDragonAnimDataSub>, 3> m4;
};

std::array<sDragonAnimData, DR_ANIM_MAX> dragonAnimData;

const s_dragonFileConfig dragonFilenameTable[DR_LEVEL_MAX] = {
    //DR_LEVEL_0_BASIC_WING
    {
        { "DRAGON0.MCB",    "DRAGON0.CGB" },
        { NULL,             NULL },
        { "DRAGONC0.MCB",   "DRAGONC0.CGB" },
    },

    //DR_LEVEL_1_VALIANT_WING
    {
        { "DRAGON1.MCB",     "DRAGON1.CGB" },
        { "DRAGONM1.MCB",    "DRAGONM1.CGB" },
        { "DRAGONC1.MCB",    "DRAGONC1.CGB" },
    },

    //DR_LEVEL_2_STRIPE_WING
    {
        { "DRAGON2.MCB",     "DRAGON2.CGB" },
        { "DRAGONM2.MCB",    "DRAGONM2.CGB" },
        { "DRAGONC2.MCB",    "DRAGONC2.CGB" },
    },

    //DR_LEVEL_3_PANZER_WING
    {
        { "DRAGON3.MCB",     "DRAGON3.CGB" },
        { "DRAGONM3.MCB",    "DRAGONM3.CGB" },
        { "DRAGONC3.MCB",    "DRAGONC3.CGB" },
    },

    //DR_LEVEL_4_EYE_WING
    {
        { "DRAGON4.MCB",     "DRAGON4.CGB" },
        { "DRAGONM4.MCB",    "DRAGONM4.CGB" },
        { "DRAGONC4.MCB",    "DRAGONC4.CGB" },
    },

    //DR_LEVEL_5_ARM_WING
    {
        { "DRAGON5.MCB",     "DRAGON5.CGB" },
        { "DRAGONM5.MCB",    "DRAGONM5.CGB" },
        { NULL,              NULL },
    },

    //DR_LEVEL_6_LIGHT_WING
    {
        { "DRAGON6.MCB",    "DRAGON6.CGB" },
        { NULL,             NULL },
        { NULL,             NULL },
    },

    //DR_LEVEL_7_SOLO_WING
    {
        { "DRAGON7.MCB",     "DRAGON7.CGB" },
        { "DRAGONM7.MCB",    "DRAGONM7.CGB" },
        { NULL,              NULL },
    },

    //DR_LEVEL_8_FLOATER
    {
        { "KTEI.MCB",       "KTEI.CGB" },
        { NULL,             NULL },
        { NULL,             NULL },
    },
};

std::array<sDragonData3, DR_LEVEL_MAX> dragonData3;

const u16 dragonData2_0[] = {
    0x10C,
    0x110,
    0x114,
    0x118,
    0x11C,
    0x120,
    0x124,
    0x128,
    0x12C,
    0x130,
    0x134,
    0x138,
    0x13C,
    0x140,
    0x144,
    0x148,
    0x14C,
    0x150
};

const u16 dragonData2_1[] = {
    0x114,
    0x118,
    0x11C,
    0x120,
    0x124,
    0x128,
    0x12C,
    0x130,
    0x134,
    0x138,
    0x13C,
    0x140,
    0x144,
    0x148,
    0x14C,
    0x150,
    0x154,
    0x158,
};

const u16 dragonData2_2[] = {
    0x114,
    0x118,
    0x11C,
    0x120,
    0x124,
    0x128,
    0x12C,
    0x130,
    0x134,
    0x138,
    0x13C,
    0x140,
    0x144,
    0x148,
    0x14C,
    0x150,
    0x154,
    0x158,
};

const u16 dragonData2_3[] = {
    0x10C,
    0x110,
    0x114,
    0x118,
    0x11C,
    0x120,
    0x124,
    0x128,
    0x12C,
    0x130,
    0x134,
    0x138,
    0x13C,
    0x140,
    0x144,
    0x148,
    0x14C,
    0x150
};

const u16 dragonData2_4[] = {
    0x11C,
    0x120,
    0x124,
    0x128,
    0x12C,
    0x130,
    0x134,
    0x138,
    0x13C,
    0x140,
    0x144,
    0x148,
    0x14C,
    0x150,
    0x154,
    0x158,
    0x15C,
    0x160
};

const u16 dragonData2_5[] = {
    0x104,
    0x108,
    0x10C,
    0x110,
    0x114,
    0x118,
    0x11C,
    0x120,
    0x124,
    0x128,
    0x12C,
    0x130,
    0x134,
    0x138,
    0x13C,
    0x140,
    0x144,
    0x148,
};

const u16 dragonData2_6[] = {
    0xE4,
    0xE8,
    0xEC,
    0xF0,
    0xF4,
    0xF8,
    0xFC,
    0x100,
    0x104,
    0x108,
    0x10C,
    0x110,
    0x114,
    0x118,
    0x11C,
    0x120,
    0x124,
    0x128,
};

const u16 dragonData2_7[] = {
    0xF4,
    0xF8,
    0xFC,
    0x100,
    0x104,
    0x108,
    0x10C,
    0x110,
    0x114,
    0x118,
    0x11C,
    0x120,
    0x124,
    0x128,
    0x12C,
    0x130,
    0x134,
    0x138,
};

const u16 dragonData2_8[] = {
    0xD0,
    0xD4,
    0xD8,
    0xDC,
};

const s_dragonData2 dragonAnimOffsets[DR_LEVEL_MAX] = {
    { dragonData2_0, 0x12 },
    { dragonData2_1, 0x12 },
    { dragonData2_2, 0x12 },
    { dragonData2_3, 0x12 },
    { dragonData2_4, 0x12 },
    { dragonData2_5, 0x12 },
    { dragonData2_6, 0x12 },
    { dragonData2_7, 0x12 },
    { dragonData2_8, 4 },
};

s_dragonState* gDragonState = NULL;


u32 countNumAnimData(s3DModelAnimData* pDragonStateData2, sDragonAnimData* dragonAnims)
{
    pDragonStateData2->count0 = 0;
    pDragonStateData2->count1 = 0;
    pDragonStateData2->count2 = 0;

    if(dragonAnims->m4[0].size())
    {
        std::vector<sDragonAnimDataSub>::iterator r6 = dragonAnims->m4[0].begin();
        while (r6->count >= 0)
        {
            pDragonStateData2->count0++;
            r6++;
        }
    }

    if (dragonAnims->m4[1].size())
    {
        std::vector<sDragonAnimDataSub>::iterator r6 = dragonAnims->m4[1].begin();
        while (r6->count >= 0)
        {
            pDragonStateData2->count1++;
            r6++;
        }
    }

    if (dragonAnims->m4[2].size())
    {
        std::vector<sDragonAnimDataSub>::iterator r6 = dragonAnims->m4[2].begin();
        while (r6->count >= 0)
        {
            pDragonStateData2->count2++;
            r6++;
        }
    }

    return pDragonStateData2->count0 + pDragonStateData2->count1 + pDragonStateData2->count2 + 1;
}

void copyAnimMatrix(sDragonAnimDataSubRanges* source, sDragonAnimDataSubRanges* destination)
{
    *destination = *source;
}

void initRuntimeAnimDataSub1(sDragonAnimDataSub* animDataSub, s_runtimeAnimData* subData)
{
    subData->m0_root.zeroize();
    subData->m_vec_C.zeroize();
    subData->m_vec_18.zeroize();
    subData->m24_rootDelta.zeroize();

    copyAnimMatrix(animDataSub->m_data, &subData->m_factors);

    subData->dataSource = animDataSub;
}

void initRuntimeAnimData(sDragonAnimData* dragonAnims, s_runtimeAnimData* subData)
{
    initRuntimeAnimDataSub1(dragonAnims->m0, &subData[0]);
    u32 r14 = 1;

    if (dragonAnims->m4[0].size())
    {
        std::vector<sDragonAnimDataSub>::iterator r12 = dragonAnims->m4[0].begin();

        while (r12->count >= 0)
        {
            initRuntimeAnimDataSub1(&(*r12), &subData[r14]);
            r14++;
            r12++;
        }
    }

    if (dragonAnims->m4[1].size())
    {
        std::vector<sDragonAnimDataSub>::iterator r12 = dragonAnims->m4[1].begin();

        while (r12->count >= 0)
        {
            initRuntimeAnimDataSub1(&(*r12), &subData[r14]);
            r14++;
            r12++;
        }
    }

    if (dragonAnims->m4[2].size())
    {
        std::vector<sDragonAnimDataSub>::iterator r12 = dragonAnims->m4[2].begin();

        while (r12->count >= 0)
        {
            initRuntimeAnimDataSub1(&(*r12), &subData[r14]);
            r14++;
            r12++;
        }
    }
}

void init3DModelAnims(s_dragonState* pDragonState, s_3dModel* pDragonStateData1, s3DModelAnimData* p3DModelAnimData, sDragonAnimData* dragonAnims)
{
    p3DModelAnimData->m0_animData = dragonAnims;

    p3DModelAnimData->countAnims = countNumAnimData(p3DModelAnimData, dragonAnims);

    p3DModelAnimData->m4_boneMatrices = &pDragonStateData1->m3C_boneMatrices;

    p3DModelAnimData->m8_runtimeAnimData = static_cast<s_runtimeAnimData*>(allocateHeapForTask(pDragonState, p3DModelAnimData->countAnims * sizeof(s_runtimeAnimData)));

    initRuntimeAnimData(dragonAnims, p3DModelAnimData->m8_runtimeAnimData);
}

sDragonAnimData* getDragonDataByIndex(e_dragonLevel dragonLevel)
{
    return &dragonAnimData[dragonLevel];
}

// 0600c430
void loadDragonSoundBank(e_dragonLevel dragonLevel)
{
    // Saturn: loads DRG_SE.SEQ + DRG1SE.BIN (or KOGATA.SEQ for Light Wing)
    // to M68K sound RAM at 0x25A0D000. On PC, dragon sound effects are
    // handled by SoLoud — this Saturn-specific sound bank loading is a no-op.
}

void createDragon3DModel(s_workArea* pWorkArea, e_dragonLevel dragonLevel)
{
    const sDragonData3* pDragonData3 = &dragonData3[dragonLevel];
    const s_dragonData2* pDragonAnimOffsets = &dragonAnimOffsets[dragonLevel];

    s_dragonState* pDragonState = createSubTaskFromFunction<s_dragonState>(pWorkArea, nullptr);

    pDragonState->m0_pDragonModelBundle = gDragonModel;
    pDragonState->m4_vdp1Allocation = nullptr;
    pDragonState->m8_parentTask = pWorkArea;
    pDragonState->mC_dragonType = dragonLevel;
    pDragonState->m14_modelIndex = pDragonData3->m_m8[0].m0_modelIndex;
    pDragonState->m18_shadowModelIndex = pDragonData3->m_m8[0].m2_shadowModelIndex;
    pDragonState->m20_dragonAnimOffsets = pDragonAnimOffsets->m_data;
    pDragonState->m24_dragonAnimCount = pDragonAnimOffsets->m_count;
    pDragonState->m88 = 1;

    sAnimationData* pDefaultAnimationData = pDragonState->m0_pDragonModelBundle->getAnimation(pDragonState->m20_dragonAnimOffsets[0]);
    sStaticPoseData* defaultPose = pDragonState->m0_pDragonModelBundle->getStaticPose(pDragonData3->m_m8[0].m4_poseModelIndex, pDefaultAnimationData->m2_numBones);

    init3DModelRawData(pDragonState, &pDragonState->m28_dragon3dModel, 0x300, pDragonState->m0_pDragonModelBundle, pDragonState->m14_modelIndex, pDefaultAnimationData, defaultPose, 0, pDragonData3->m_m8[0].m_m8);

    init3DModelAnims(pDragonState, &pDragonState->m28_dragon3dModel, &pDragonState->m78_animData, getDragonDataByIndex(dragonLevel));

    loadDragonSoundBank(dragonLevel);

    gDragonState = pDragonState;
}

void loadDragonFiles(s_workArea* pWorkArea, e_dragonLevel dragonLevel)
{
    loadFile(dragonFilenameTable[dragonLevel].m_base.MCB, &gDragonModel, 0x2400);
    loadFile(dragonFilenameTable[dragonLevel].m_base.CGB, getVdp1Pointer(0x25C12000), 0);

    // VDP1 VRAM content changed at fixed address — invalidate cached textures in this range
    invalidateVdp1TextureRange(0x12000, 0x3D00);

    createDragon3DModel(pWorkArea, dragonLevel);
}

s_loadDragonWorkArea* loadDragonModel(s_workArea* pWorkArea, e_dragonLevel dragonLevel)
{
    s_loadDragonWorkArea* pLoadDragonWorkArea = createSubTaskFromFunction<s_loadDragonWorkArea>(pWorkArea, nullptr);

    pLoadDragonWorkArea->m8_dramAllocation = dramAllocate(0x1F600);
    pLoadDragonWorkArea->m4_vramAllocation = NULL;
    pLoadDragonWorkArea->m8_MCBInDram = pLoadDragonWorkArea->m8_dramAllocation + 0x18E00;

    if (dragonFilenameTable[dragonLevel].m_M.MCB)
    {
        loadFile(dragonFilenameTable[dragonLevel].m_M.MCB, pLoadDragonWorkArea->m8_dramAllocation, 0);
        loadFile(dragonFilenameTable[dragonLevel].m_M.CGB, pLoadDragonWorkArea->m8_MCBInDram, 0);
    }
    return pLoadDragonWorkArea;
}

static fixedPoint morphDragonAccumulator[3];
static s16 morphDragonWeightS16[3];

// 060136c0
static void morphDragonSub1Sub0(fixedPoint w0, fixedPoint w1, fixedPoint w2)
{
    morphDragonAccumulator[0] = w0;
    morphDragonAccumulator[1] = w1;
    morphDragonAccumulator[2] = w2;
    morphDragonWeightS16[0] = (s16)((s32)w0 >> 2);
    morphDragonWeightS16[1] = (s16)((s32)w1 >> 2);
    morphDragonWeightS16[2] = (s16)((s32)w2 >> 2);
}

// 0600c050
static void morphDragonSub1(s32 cursorX, s32 cursorY)
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
    morphDragonSub1Sub0(w0, w1, w2);
}

// 06013510
static void morphDragonVertice(fixedPoint* pDst, const fixedPoint* pA, const fixedPoint* pB, const fixedPoint* pC)
{
    for (int i = 0; i < 3; i++)
    {
        pDst[i] = MTH_Mul(morphDragonAccumulator[0], pA[i])
                + MTH_Mul(morphDragonAccumulator[1], pB[i])
                + MTH_Mul(morphDragonAccumulator[2], pC[i]);
    }
}

// 060134cc — morph normals (s16 weighted blend)
static void morphDragonNormals(s16* pDst, const s16* pA, const s16* pB, const s16* pC, s32 count)
{
    for (s32 i = 0; i < count; i++)
    {
        s32 result = (s32)morphDragonWeightS16[0] * (s32)pA[i]
                   + (s32)morphDragonWeightS16[1] * (s32)pB[i]
                   + (s32)morphDragonWeightS16[2] * (s32)pC[i];
        pDst[i] = (s16)(result >> 14);
    }
}

// 0601344e — morph a single 3D model node (vertices + normals + polygon data)
static void morphDragonSub2_processNode(u32* pDst, u32* pA, u32* pB, u32* pC)
{
    if (pDst[1] == 0) // m4_vertexCount
        return;

    // Morph the first vertex (scale)
    fixedPoint blendedScale = MTH_Mul(morphDragonAccumulator[0], fixedPoint((s32)pA[0]))
                            + MTH_Mul(morphDragonAccumulator[1], fixedPoint((s32)pB[0]))
                            + MTH_Mul(morphDragonAccumulator[2], fixedPoint((s32)pC[0]));
    pDst[0] = (u32)(s32)blendedScale;

    // Morph normal data
    morphDragonNormals((s16*)pDst[2], (const s16*)pA[2], (const s16*)pB[2], (const s16*)pC[2],
                       pDst[1] * 3);

    // Morph polygon vertex coordinates — iterate VDP1 polygon commands
    // Layout: [header 5 u32s][vertex data...] per polygon, terminated by sentinel
    u32* pCmdDst = pDst + 3;
    u32* pCmdA = pA + 3;
    u32* pCmdB = pB + 3;
    u32* pCmdC = pC + 3;
    while (pCmdDst[0] != pCmdDst[1])
    {
        u8 polyType = *((u8*)(pCmdDst + 2)) & 3;
        pCmdDst += 5;
        pCmdA += 5;
        pCmdB += 5;
        pCmdC += 5;
        if (polyType != 0)
        {
            // FUN_060134cc — blend per-polygon normal/vertex data
            // Each polygon has vertex normals as s16 triplets (X,Y,Z)
            // polyType 2 = triangle (3 vertices), 3 = quad (4 vertices), 1 = untextured
            s32 numVerts = (polyType == 2) ? 3 : 4;
            s16* pNormDst = (s16*)pCmdDst;
            const s16* pNormA = (const s16*)pCmdA;
            const s16* pNormB = (const s16*)pCmdB;
            const s16* pNormC = (const s16*)pCmdC;
            for (s32 v = 0; v < numVerts; v++)
            {
                for (s32 c = 0; c < 3; c++)
                {
                    s32 blended = (s32)morphDragonWeightS16[0] * (s32)pNormA[v * 3 + c]
                                + (s32)morphDragonWeightS16[1] * (s32)pNormB[v * 3 + c]
                                + (s32)morphDragonWeightS16[2] * (s32)pNormC[v * 3 + c];
                    pNormDst[v * 3 + c] = (s16)((blended << 2) >> 16);
                }
            }

            if (polyType == 1)
            {
                pCmdDst = (u32*)((u8*)pCmdDst + 2);
                pCmdA = (u32*)((u8*)pCmdA + 2);
                pCmdB = (u32*)((u8*)pCmdB + 2);
                pCmdC = (u32*)((u8*)pCmdC + 2);
            }
        }
    }
}

// 0600c0c4
static void morphDragonSub2(sModelHierarchy* pDst, sModelHierarchy* pA, sModelHierarchy* pB, sModelHierarchy* pC)
{
    while (true)
    {
        if (pDst->m0_3dModel)
            morphDragonSub2_processNode((u32*)pDst->m0_3dModel, (u32*)pA->m0_3dModel, (u32*)pB->m0_3dModel, (u32*)pC->m0_3dModel);
        if (pDst->m4_subNode)
            morphDragonSub2(pDst->m4_subNode, pA->m4_subNode, pB->m4_subNode, pC->m4_subNode);
        if (!pDst->m8_nextNode)
            break;
        pDst = pDst->m8_nextNode;
        pA = pA->m8_nextNode;
        pB = pB->m8_nextNode;
        pC = pC->m8_nextNode;
    }
}

// 0600c138
static void morphDragonSub3(s_3dModel* pModel, fixedPoint* pA, fixedPoint* pB, fixedPoint* pC)
{
    s32 numBones = pModel->m12_numBones;
    sPoseData* pPose = &pModel->m2C_poseData[0];
    for (s32 i = 1; i < numBones; i++)
    {
        pA += 9;
        pPose++;
        pB += 9;
        pC += 9;
        morphDragonVertice(&pPose->m0_translation[0], pA, pB, pC);
    }
}

// 060136a8
static fixedPoint morphDragonBlendColor(fixedPoint a, fixedPoint b, fixedPoint c)
{
    return MTH_Mul(morphDragonAccumulator[0], a)
         + MTH_Mul(morphDragonAccumulator[1], b)
         + MTH_Mul(morphDragonAccumulator[2], c);
}

// 0600c180
static void morphDragonSub4(s32* pDstPairs, s32* pAPairs, s32* pBPairs, s32* pCPairs, s32 count)
{
    for (s32 i = 0; i < count; i++)
    {
        s32 numEntries = pDstPairs[1];
        if (numEntries != 0)
        {
            s32* pDst = (s32*)pDstPairs[0];
            s32* pA = (s32*)pAPairs[0];
            s32* pB = (s32*)pBPairs[0];
            s32* pC = (s32*)pCPairs[0];
            for (s32 j = 0; j < numEntries; j++)
            {
                morphDragonVertice((fixedPoint*)(pDst + 1), (fixedPoint*)(pA + 1), (fixedPoint*)(pB + 1), (fixedPoint*)(pC + 1));
                fixedPoint blended = morphDragonBlendColor(fixedPoint(pA[4]), fixedPoint(pB[4]), fixedPoint(pC[4]));
                pDst[4] = (s32)blended;
                pDst += 5; pA += 5; pB += 5; pC += 5;
            }
        }
        pDstPairs += 2; pAPairs += 2; pBPairs += 2; pCPairs += 2;
    }
}

void morphDragon(s_loadDragonWorkArea* pLoadDragonWorkArea, s_3dModel* pDragonStateSubData1, u8* pMCB, const sDragonData3* pDragonData3, s16 cursorX, s16 cursorY)
{
    if (pDragonData3->m_m0 == 0)
        return;

    const sDragonData3Sub* pCenter = &pDragonData3->m_m8[1];
    const sDragonData3Sub* pRight  = &pDragonData3->m_m8[5];
    const sDragonData3Sub* pLeft   = &pDragonData3->m_m8[3];
    const sDragonData3Sub* pUp     = &pDragonData3->m_m8[4];
    const sDragonData3Sub* pDown   = &pDragonData3->m_m8[2];

    const sDragonData3Sub* pX = pRight;
    const sDragonData3Sub* pY = pUp;

    s32 absX = cursorX;
    s32 absY = cursorY;

    if (cursorX < 0) { absX = -cursorX; pX = pLeft; }
    if (cursorY < 0) { absY = -cursorY; pY = pDown; }

    morphDragonSub1(absX, absY);

    morphDragonSub1(absX, absY);

    // m_m8 layout: [0]=output, [1]=center, [2]=down, [3]=left, [4]=up, [5]=right
    const sDragonData3Sub* pOutput = &pDragonData3->m_m8[0];
    const sDragonData3Sub* pCenterSub = &pDragonData3->m_m8[1];
    s_fileBundle* pBundle = (s_fileBundle*)pLoadDragonWorkArea->m8_dramAllocation;
    s_fileBundle* pModelBundle = pDragonStateSubData1->m4_pModelFile;

    // Morph model hierarchy vertices (first call — sub-models)
    morphDragonSub2(
        pModelBundle->getModelHierarchy(pOutput->m0_modelIndex),
        pBundle->getModelHierarchy(pCenterSub->m0_modelIndex),
        pBundle->getModelHierarchy(pX->m0_modelIndex),
        pBundle->getModelHierarchy(pY->m0_modelIndex)
    );

    // Morph model hierarchy vertices (second call — shadow models)
    morphDragonSub2(
        pModelBundle->getModelHierarchy(pOutput->m2_shadowModelIndex),
        pBundle->getModelHierarchy(pCenterSub->m2_shadowModelIndex),
        pBundle->getModelHierarchy(pX->m2_shadowModelIndex),
        pBundle->getModelHierarchy(pY->m2_shadowModelIndex)
    );

    // Morph bone pose data — reads raw pose offsets from bundle
    u8* pBundleRaw = (u8*)pBundle;
    morphDragonSub3(
        pDragonStateSubData1,
        (fixedPoint*)(pBundleRaw + pCenterSub->m4_poseModelIndex),
        (fixedPoint*)(pBundleRaw + pX->m4_poseModelIndex),
        (fixedPoint*)(pBundleRaw + pY->m4_poseModelIndex)
    );

    // Morph VDP1 palette colors
    {
        s32 numColors = pDragonData3->m_m0 << 4; // each entry = 16 colors
        u16* pDstPal = (u16*)(getVdp1Pointer(0x25C00000) + pOutput->m6 * 8);
        u16* pCenterPal = (u16*)(pMCB + pCenterSub->m6 * 8);
        u16* pXPal = (u16*)(pMCB + pX->m6 * 8);
        u16* pYPal = (u16*)(pMCB + pY->m6 * 8);
        for (s32 i = 0; i < numColors; i++)
        {
            u16 cA = pCenterPal[i], cB = pXPal[i], cC = pYPal[i];
            s32 r = (s32)MTH_Mul(morphDragonAccumulator[0], fixedPoint(cA & 0x1F))
                  + (s32)MTH_Mul(morphDragonAccumulator[1], fixedPoint(cB & 0x1F))
                  + (s32)MTH_Mul(morphDragonAccumulator[2], fixedPoint(cC & 0x1F));
            s32 g = (s32)MTH_Mul(morphDragonAccumulator[0], fixedPoint((cA >> 5) & 0x1F))
                  + (s32)MTH_Mul(morphDragonAccumulator[1], fixedPoint((cB >> 5) & 0x1F))
                  + (s32)MTH_Mul(morphDragonAccumulator[2], fixedPoint((cC >> 5) & 0x1F));
            s32 b = (s32)MTH_Mul(morphDragonAccumulator[0], fixedPoint((cA >> 10) & 0x1F))
                  + (s32)MTH_Mul(morphDragonAccumulator[1], fixedPoint((cB >> 10) & 0x1F))
                  + (s32)MTH_Mul(morphDragonAccumulator[2], fixedPoint((cC >> 10) & 0x1F));
            pDstPal[i] = 0x8000 | ((b >> 16) & 0x1F) << 10 | ((g >> 16) & 0x1F) << 5 | ((r >> 16) & 0x1F);
        }
    }

    // Morph hotpoint data
    if (pOutput->m_m8 && pCenterSub->m_m8 && pX->m_m8 && pY->m_m8)
    {
        morphDragonSub4((s32*)pOutput->m_m8, (s32*)pCenterSub->m_m8, (s32*)pX->m_m8, (s32*)pY->m_m8, pDragonData3->m_m4);
    }
}

// Replace Saturn DRAM free with standard free
void dramFree(u8* ptr)
{
    free(ptr);
}

void vdp1Free(u8* ptr)
{
    s_vdp1AllocationNode* pNode = (s_vdp1AllocationNode*)ptr;
    if (!pNode) return;

    // clear the VDP1 VRAM region being freed and invalidate cached textures
    u32 vramOffset = (u32)pNode->m4_vdp1Memory << 3;
    u32 vramSize = (u32)pNode->m6_size << 3;
    memset(getVdp1Pointer(0x25C00000 + vramOffset), 0, vramSize);
    invalidateVdp1TextureRange(vramOffset, vramSize);

    u16 nodeOffset = pNode->m4_vdp1Memory;

    // Walk free list (at vdp1AllocatorHead + 8) to find insertion point sorted by vdp1 offset
    s_vdp1AllocationNode* pPrev = &vdp1AllocatorHead->m8_freeHead;
    s_vdp1AllocationNode* pCur = pPrev->m_0;
    while (pCur != nullptr && pCur->m4_vdp1Memory <= nodeOffset) {
        pPrev = pCur;
        pCur = pCur->m_0;
    }

    // Try to coalesce with next block
    if (pCur != nullptr && nodeOffset + pNode->m6_size == pCur->m4_vdp1Memory) {
        // Merge: expand next to cover freed node, return freed node to pool
        pCur->m4_vdp1Memory = pNode->m4_vdp1Memory;
        pCur->m6_size += pNode->m6_size;
        pNode->m_0 = vdp1AllocatorHead->m4_nextNode;
        vdp1AllocatorHead->m4_nextNode = pNode;
        pNode = pCur;
    } else {
        pNode->m_0 = pCur;
    }

    // Try to coalesce with prev block
    if (pPrev->m4_vdp1Memory + pPrev->m6_size == nodeOffset) {
        // Merge: expand prev to cover freed node, return freed node to pool
        pPrev->m_0 = pNode->m_0;
        pPrev->m6_size += pNode->m6_size;
        pNode->m_0 = vdp1AllocatorHead->m4_nextNode;
        vdp1AllocatorHead->m4_nextNode = pNode;
    } else {
        pPrev->m_0 = pNode;
    }
}

void loadDragonSub1Sub1(s_loadDragonWorkArea* pLoadDragonWorkArea)
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
    if (pLoadDragonWorkArea->m8_dramAllocation)
    {
        dramFree(pLoadDragonWorkArea->m8_dramAllocation);
        pLoadDragonWorkArea->m8_dramAllocation = NULL;
    }

    if (pLoadDragonWorkArea->m4_vramAllocation)
    {
        vdp1Free(pLoadDragonWorkArea->m4_vramAllocation);
        pLoadDragonWorkArea->m4_vramAllocation = NULL;
    }
}

void loadDragonSub1(s_loadDragonWorkArea* pLoadDragonWorkArea)
{
    if (pLoadDragonWorkArea)
    {
        loadDragonSub1Sub1(pLoadDragonWorkArea);

        pLoadDragonWorkArea->getTask()->markFinished();
    }
}

void loadDragon(s_workArea* pWorkArea)
{
    loadDragonFiles(pWorkArea, mainGameState.gameStats.m1_dragonLevel);

    updateDragonStatsFromLevel();

    gDragonState->m10_cursorX = mainGameState.gameStats.m1A_dragonCursorX;
    gDragonState->m12_cursorY = mainGameState.gameStats.m1C_dragonCursorY;
    gDragonState->m1C_dragonArchetype = mainGameState.gameStats.mB6_dragonArchetype;

    s_loadDragonWorkArea* pLoadDragonWorkArea = loadDragonModel(pWorkArea, mainGameState.gameStats.m1_dragonLevel);
    const sDragonData3* pDragonData3 = &dragonData3[mainGameState.gameStats.m1_dragonLevel];

    morphDragon(pLoadDragonWorkArea, &gDragonState->m28_dragon3dModel, pLoadDragonWorkArea->m8_MCBInDram, pDragonData3, mainGameState.gameStats.m1A_dragonCursorX, mainGameState.gameStats.m1C_dragonCursorY);

    loadDragonSub1(pLoadDragonWorkArea);
}

void loadDragonDataFromCommon()
{
    // Load the dragon models stuff
    {
        sSaturnPtr ptr = gCommonFile->getSaturnPtr(0x2065e8);
        for (int i = 0; i < e_dragonLevel::DR_LEVEL_MAX; i++)
        {
            s_fileBundle* pDragonModel = nullptr;
            loadFile(dragonFilenameTable[i].m_base.MCB, &pDragonModel, 0x2400);

            sDragonData3& entry = dragonData3[i];
            entry.m_m0 = readSaturnU32(ptr + 0); ptr += 4;
            entry.m_m4 = readSaturnU16(ptr + 2); ptr += 2;
            entry.m_m6 = readSaturnU16(ptr + 2); ptr += 2;
            for (int j = 0; j < 7; j++)
            {
                sDragonData3Sub& subEntry = entry.m_m8[j];

                subEntry.m0_modelIndex = readSaturnU16(ptr); ptr += 2;
                subEntry.m2_shadowModelIndex = readSaturnU16(ptr); ptr += 2;
                subEntry.m4_poseModelIndex = readSaturnU16(ptr); ptr += 2;
                subEntry.m6 = readSaturnU16(ptr); ptr += 2;
                subEntry.m_m8 = nullptr;

                sSaturnPtr hotpointsPtr = readSaturnEA(ptr); ptr += 4;

                if (!hotpointsPtr.isNull() && (j == 0))
                {
                    subEntry.m_m8 = readRiderDefinitionSub(hotpointsPtr);
                }
            }
        }
    }


    // Load the dragon anim stuff
    {
        for (int i = 0; i < DR_ANIM_MAX; i++)
        {
            sSaturnPtr ptr = readSaturnEA(gCommonFile->getSaturnPtr(0x00202054 + 4 * i));

            for (int j=0; j<4; j++)
            {
                sSaturnPtr subPtr = readSaturnEA(ptr + j * 4);
                if (!subPtr.isNull())
                {
                    while (1)
                    {
                        sDragonAnimDataSub* pOutput = nullptr;

                        if (j == 0)
                        {
                            dragonAnimData[i].m0 = new sDragonAnimDataSub;
                            pOutput = dragonAnimData[i].m0;
                        }
                        else
                        {
                            dragonAnimData[i].m4[j - 1].resize(dragonAnimData[i].m4[j - 1].size() + 1);
                            pOutput = &dragonAnimData[i].m4[j - 1][dragonAnimData[i].m4[j - 1].size() - 1];
                        }

                        pOutput->count = readSaturnS32(subPtr + 0);
                        pOutput->m_data = nullptr;

                        if (pOutput->count != -1)
                        {
                            sSaturnPtr subSubPtr = readSaturnEA(subPtr + 4);

                            sDragonAnimDataSubRanges* subSubEntry = new sDragonAnimDataSubRanges;
                            subSubEntry->m_vec0 = readSaturnVec3(subSubPtr + 0);
                            subSubEntry->m_vecC = readSaturnVec3(subSubPtr + 0xC);
                            subSubEntry->m_max = readSaturnVec3(subSubPtr + 0x18);
                            subSubEntry->m_min = readSaturnVec3(subSubPtr + 0x24);

                            pOutput->m_data = subSubEntry;
                        }
                        else
                        {
                            break;
                        }

                        if (j == 0)
                        {
                            break;
                        }

                        subPtr += 8;
                    }
                }
            }
        }
    }
}
