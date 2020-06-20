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
    const sDragonAnimDataSubRanges* m_data;
};

struct sDragonAnimData
{
    const sDragonAnimDataSub* m_0;
    const sDragonAnimDataSub* m_4;
    const sDragonAnimDataSub* m_8;
    const sDragonAnimDataSub* m_C;
};

const sDragonAnimDataSubRanges dragon0AnimsData0 =
{
    {0x2423, 0x2423, 0x2423},
    {0xB800, 0xB800, 0xB800},
    {0xE38E38, 0xE38E38, 0xE38E38},
    {-0xE38E38, -0xE38E38, -0xE38E38},
};
const sDragonAnimDataSubRanges dragon0AnimsData1 =
{
    {0x400, 0x400, 0x400},
    {0x1000, 0x1000, 0x1000},
    {0xE38E38, 0xE38E38, 0xE38E38},
    {-0xE38E38, -0xE38E38, -0xE38E38},
};

const sDragonAnimDataSub dragon0Anims0[1] =
{
    { 0x01, &dragon0AnimsData0 },
};

const sDragonAnimDataSub dragon0Anims1[] =
{
    { 0x0E, &dragon0AnimsData1 },
    { 0x0F, &dragon0AnimsData1 },
    { 0x10, &dragon0AnimsData1 },
    { 0x11, &dragon0AnimsData1 },
    { -1, NULL },
};

const sDragonAnimDataSub dragon0Anims2[] =
{
    { 0x02, &dragon0AnimsData0 },
    { 0x03, &dragon0AnimsData0 },
    { 0x04, &dragon0AnimsData0 },
    { 0x05, &dragon0AnimsData0 },
    { 0x06, &dragon0AnimsData0 },
    { -1, NULL },
};

const sDragonAnimData dragon0Anims =
{
    dragon0Anims0,
    dragon0Anims1,
    dragon0Anims2,
    NULL,
};

const sDragonAnimData* dragonAnimData[DR_ANIM_MAX] =
{
    &dragon0Anims,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
    (const sDragonAnimData*)1,
};

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


u32 countNumAnimData(s3DModelAnimData* pDragonStateData2, const sDragonAnimData* dragonAnims)
{
    pDragonStateData2->count0 = 0;
    pDragonStateData2->count1 = 0;
    pDragonStateData2->count2 = 0;

    const sDragonAnimDataSub* r6 = dragonAnims->m_4;
    if (r6)
    {
        while (r6->count >= 0)
        {
            pDragonStateData2->count0++;
            r6++;
        }
    }

    r6 = dragonAnims->m_8;
    if (r6)
    {
        while (r6->count >= 0)
        {
            pDragonStateData2->count1++;
            r6++;
        }
    }

    r6 = dragonAnims->m_C;
    if (r6)
    {
        while (r6->count >= 0)
        {
            pDragonStateData2->count2++;
            r6++;
        }
    }

    return pDragonStateData2->count0 + pDragonStateData2->count1 + pDragonStateData2->count2 + 1;
}

void copyAnimMatrix(const sDragonAnimDataSubRanges* source, sDragonAnimDataSubRanges* destination)
{
    *destination = *source;
}

void initRuntimeAnimDataSub1(const sDragonAnimDataSub* animDataSub, s_runtimeAnimData* subData)
{
    subData->m0_root.zeroize();
    subData->m_vec_C.zeroize();
    subData->m_vec_18.zeroize();
    subData->m24_rootDelta.zeroize();

    copyAnimMatrix(animDataSub->m_data, &subData->m_factors);

    subData->dataSource = animDataSub;
}

void initRuntimeAnimData(const sDragonAnimData* dragonAnims, s_runtimeAnimData* subData)
{
    initRuntimeAnimDataSub1(dragonAnims->m_0, &subData[0]);
    u32 r14 = 1;

    if (dragonAnims->m_4)
    {
        const sDragonAnimDataSub* r12 = dragonAnims->m_4;

        while (r12->count >= 0)
        {
            initRuntimeAnimDataSub1(r12, &subData[r14]);
            r14++;
            r12++;
        }
    }

    if (dragonAnims->m_8)
    {
        const sDragonAnimDataSub* r12 = dragonAnims->m_8;

        while (r12->count >= 0)
        {
            initRuntimeAnimDataSub1(r12, &subData[r14]);
            r14++;
            r12++;
        }
    }

    if (dragonAnims->m_C)
    {
        const sDragonAnimDataSub* r12 = dragonAnims->m_C;

        while (r12->count >= 0)
        {
            initRuntimeAnimDataSub1(r12, &subData[r14]);
            r14++;
            r12++;
        }
    }
}

void init3DModelAnims(s_dragonState* pDragonState, s_3dModel* pDragonStateData1, s3DModelAnimData* p3DModelAnimData, const sDragonAnimData* dragonAnims)
{
    p3DModelAnimData->m0_animData = dragonAnims;

    p3DModelAnimData->countAnims = countNumAnimData(p3DModelAnimData, dragonAnims);

    p3DModelAnimData->m4_boneMatrices = &pDragonStateData1->m3C_boneMatrices;

    p3DModelAnimData->m8_runtimeAnimData = static_cast<s_runtimeAnimData*>(allocateHeapForTask(pDragonState, p3DModelAnimData->countAnims * sizeof(s_runtimeAnimData)));

    initRuntimeAnimData(dragonAnims, p3DModelAnimData->m8_runtimeAnimData);
}

const sDragonAnimData* getDragonDataByIndex(e_dragonLevel dragonLevel)
{
    return dragonAnimData[dragonLevel];
}

void loadDragonSoundBank(e_dragonLevel dragonLevel)
{
    FunctionUnimplemented();
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
        assert(0);
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

    assert(false);
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
    gDragonState->m1C_dragonArchetype = mainGameState.gameStats.dragonArchetype;

    s_loadDragonWorkArea* pLoadDragonWorkArea = loadDragonModel(pWorkArea, mainGameState.gameStats.m1_dragonLevel);
    const sDragonData3* pDragonData3 = &dragonData3[mainGameState.gameStats.m1_dragonLevel];

    morphDragon(pLoadDragonWorkArea, &gDragonState->m28_dragon3dModel, pLoadDragonWorkArea->m8_MCBInDram, pDragonData3, mainGameState.gameStats.m1A_dragonCursorX, mainGameState.gameStats.m1C_dragonCursorY);

    loadDragonSub1(pLoadDragonWorkArea);
}

void loadDragonDataFromCommon()
{
    sSaturnPtr ptr = gCommonFile.getSaturnPtr(0x2065e8);
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

            if (!hotpointsPtr.isNull() && (j==0))
            {
                subEntry.m_m8 = readRiderDefinitionSub(hotpointsPtr);
            }
        }
    }
}
