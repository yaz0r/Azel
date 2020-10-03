#include "PDS.h"
#include "dragonData.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "commonOverlay.h"

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

void loadDragonSoundBank(e_dragonLevel dragonLevel)
{
    Unimplemented();
}

void createDragon3DModel(s_workArea* pWorkArea, e_dragonLevel dragonLevel)
{
    const sDragonData3* pDragonData3 = &dragonData3[dragonLevel];
    const s_dragonData2* pDragonAnimOffsets = &dragonAnimOffsets[dragonLevel];

    s_dragonState* pDragonState = createSubTaskFromFunction<s_dragonState>(pWorkArea, nullptr);

    pDragonState->m0_pDragonModelBundle = gDragonModel;
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
    loadFile(dragonFilenameTable[dragonLevel].m_base.CGB, getVdp1Pointer(0x25C12000), NULL);

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
        Unimplemented();
        loadFile(dragonFilenameTable[dragonLevel].m_M.MCB, pLoadDragonWorkArea->m8_dramAllocation, 0/*pLoadDragonWorkArea->m8_MCBInDram >> 3*/);
        loadFile(dragonFilenameTable[dragonLevel].m_M.CGB, pLoadDragonWorkArea->m8_MCBInDram, 0);

    }
    return pLoadDragonWorkArea;
}

void morphDragon(s_loadDragonWorkArea* pLoadDragonWorkArea, s_3dModel* pDragonStateSubData1, u8* pMCB, const sDragonData3* pDragonData3, s16 cursorX, s16 cursorY)
{
    if (pDragonData3->m_m0 == 0)
    {
        return;
    }

    const sDragonData3Sub* r13 = &pDragonData3->m_m8[1];
    const sDragonData3Sub* r11 = &pDragonData3->m_m8[5];

    Unimplemented();
}

void dramFree(u8* ptr)
{
    PDS_unimplemented("dramFree not implemented");
}

void vdp1Free(u8* ptr)
{
    PDS_unimplemented("vdp1Free not implemented");
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
