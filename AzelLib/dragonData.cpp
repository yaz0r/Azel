#include "PDS.h"
#include "dragonData.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "commonOverlay.h"
#include "3dEngine_textureCache.h"
#include "kernel/vdp1Allocator.h"
#include "processModel.h"
#include "dragonMorphing.h"

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

std::array<sDragonMorphDataPerLevel, DR_LEVEL_MAX> gDragonMorphDataPerLevel;

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
    const sDragonMorphDataPerLevel* pDragonData3 = &gDragonMorphDataPerLevel[dragonLevel];
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

    init3DModelRawData(pDragonState, &pDragonState->m28_dragon3dModel, 0x300, pDragonState->m0_pDragonModelBundle, pDragonState->m14_modelIndex, pDefaultAnimationData, defaultPose, 0, pDragonData3->m_m8[0].m8_hotPoints);

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

void loadDragon(s_workArea* pWorkArea)
{
    loadDragonFiles(pWorkArea, mainGameState.gameStats.m1_dragonLevel);

    updateDragonStatsFromLevel();

    gDragonState->m10_cursorX = mainGameState.gameStats.m1A_dragonCursorX;
    gDragonState->m12_cursorY = mainGameState.gameStats.m1C_dragonCursorY;
    gDragonState->m1C_dragonArchetype = mainGameState.gameStats.mB6_dragonArchetype;

    sDragonMorphData* pLoadDragonWorkArea = createDragonMorphData(pWorkArea, mainGameState.gameStats.m1_dragonLevel);

    morphDragon(pLoadDragonWorkArea,
        &gDragonState->m28_dragon3dModel,
        pLoadDragonWorkArea->m8_MCBInDram,
        &gDragonMorphDataPerLevel[mainGameState.gameStats.m1_dragonLevel],
        mainGameState.gameStats.m1A_dragonCursorX,
        mainGameState.gameStats.m1C_dragonCursorY);

    deleteDragonMorphData(pLoadDragonWorkArea);
}

void loadDragonDataFromCommon()
{
    // Load the dragon models stuff
    {
        sSaturnPtr ptr = gCommonFile->getSaturnPtr(0x2065e8);
        for (int i = 0; i < e_dragonLevel::DR_LEVEL_MAX; i++)
        {
            sDragonMorphDataPerLevel& entry = gDragonMorphDataPerLevel[i];
            entry.m0_numMCBEntries = readSaturnU32(ptr + 0); ptr += 4;
            entry.m4_numHotPoints = readSaturnU16(ptr + 2); ptr += 2;
            entry.m_m6 = readSaturnU16(ptr + 2); ptr += 2;
            for (int j = 0; j < 7; j++)
            {
                sDragonMorphModels& subEntry = entry.m_m8[j];

                subEntry.m0_modelIndex = readSaturnU16(ptr); ptr += 2;
                subEntry.m2_shadowModelIndex = readSaturnU16(ptr); ptr += 2;
                subEntry.m4_poseModelIndex = readSaturnU16(ptr); ptr += 2;
                subEntry.m6_textureOffset = readSaturnU16(ptr); ptr += 2;
                subEntry.m8_hotPoints = nullptr;

                sSaturnPtr hotpointsPtr = readSaturnEA(ptr); ptr += 4;

                if (!hotpointsPtr.isNull())
                {
                    subEntry.m8_hotPoints = readRiderDefinitionSub(hotpointsPtr);
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
