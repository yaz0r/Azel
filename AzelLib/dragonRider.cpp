#include "PDS.h"
#include "dragonRider.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "commonOverlay.h"

sHotpointBundle* readRiderDefinitionSub(sSaturnPtr ptrEA); // TODO cleanup

struct s_RiderDefinition
{
    const char* m_MCBName; //0
    const char* m_CGBName; //4
    u16 m_flags; //8
    u16 mA_offsetToDefaultPose; //A
    sHotpointBundle* m_pExtraData; //C
};

s_RiderDefinition gRiderTable[] = {
    { "RIDER0.MCB",  NULL,          0x4,    0x08, NULL},
    { "EDGE.MCB",   "EDGE.CGB",     0x4,    0x28, (sHotpointBundle*)1 },
    { "GUSH.MCB",   "GUSH.CGB",     0x4,    0x20, NULL },
    { "PAET.MCB",   "PAET.CGB",     0x4,    0x20, NULL },
    { "AZCT.MCB",   "AZCT.CGB",     0x4,    0x20, NULL },
    { "AZEL.MCB",   "AZEL.CGB",     0x4,    0x20, NULL },
    { NULL,         NULL,           0x8,    0xC4, (sHotpointBundle*)1 },
    { NULL,         NULL,           0xC,    0xC8, NULL },
};

s_loadRiderWorkArea* pRider1State = NULL;
s_loadRiderWorkArea* pRider2State = NULL;

s_fileBundle* riderModel = nullptr;
s_fileBundle* rider2Model = nullptr;

s_loadRiderWorkArea* loadRider(s_workArea* pWorkArea, u8 riderType)
{
    const s_RiderDefinition* r13 = &gRiderTable[riderType];

    sAnimationData* pAnimation = NULL;

    s_loadRiderWorkArea* pLoadRiderWorkArea = createSubTaskFromFunction < s_loadRiderWorkArea>(pWorkArea, nullptr);

    pLoadRiderWorkArea->m4 = 0;
    pLoadRiderWorkArea->m_ParentWorkArea = pWorkArea;
    pLoadRiderWorkArea->mC_riderType = riderType;
    pLoadRiderWorkArea->m10_modelIndex = r13->m_flags;

    pRider1State = pLoadRiderWorkArea;

    if (riderType < 6)
    {
        if (riderType == 1)
        {
            pLoadRiderWorkArea->m14_weaponModelIndex = 0x24;
        }
        else
        {
            pLoadRiderWorkArea->m14_weaponModelIndex = 0;
        }

        loadFile(r13->m_MCBName, &riderModel, 0x2C00);
        pLoadRiderWorkArea->m0_riderBundle = riderModel;

        if (r13->m_CGBName)
        {
            loadFile(r13->m_CGBName, getVdp1Pointer(0x25C16000), 0);
        }
    }
    else
    {
        assert(0);
    }

    s_fileBundle* pBundle = pLoadRiderWorkArea->m0_riderBundle;
    sStaticPoseData* pDefaultPose = pBundle->getStaticPose(r13->mA_offsetToDefaultPose, pBundle->getModelHierarchy(pLoadRiderWorkArea->m10_modelIndex)->countNumberOfBones());

    init3DModelRawData(pLoadRiderWorkArea, &pLoadRiderWorkArea->m18_3dModel, 0, pBundle, pLoadRiderWorkArea->m10_modelIndex, pAnimation, pDefaultPose, 0, r13->m_pExtraData);

    return pLoadRiderWorkArea;
}

s_loadRiderWorkArea* loadRider2(s_workArea* pWorkArea, u8 riderType)
{
    const s_RiderDefinition* r13 = &gRiderTable[riderType];

    sAnimationData* pAnimation = NULL;

    s_loadRiderWorkArea* pLoadRiderWorkArea = createSubTaskFromFunction < s_loadRiderWorkArea>(pWorkArea, nullptr);

    pLoadRiderWorkArea->m4 = 0;
    pLoadRiderWorkArea->m_ParentWorkArea = pWorkArea;
    pLoadRiderWorkArea->mC_riderType = riderType;
    pLoadRiderWorkArea->m10_modelIndex = r13->m_flags;

    pRider2State = pLoadRiderWorkArea;

    if (riderType < 6)
    {
        if (riderType == 1)
        {
            pLoadRiderWorkArea->m14_weaponModelIndex = 0x24;
        }
        else
        {
            pLoadRiderWorkArea->m14_weaponModelIndex = 0;
        }

        loadFile(r13->m_MCBName, &rider2Model, 0x2E80);
        pLoadRiderWorkArea->m0_riderBundle = rider2Model;

        if (r13->m_CGBName)
        {
            loadFile(r13->m_CGBName, getVdp1Pointer(0x25C17400), 0);
        }
    }
    else
    {
        assert(0);
    }

    s_fileBundle* pBundle = pLoadRiderWorkArea->m0_riderBundle;
    sStaticPoseData* pDefaultPose = pBundle->getStaticPose(r13->mA_offsetToDefaultPose, pBundle->getModelHierarchy(pLoadRiderWorkArea->m10_modelIndex)->countNumberOfBones());

    init3DModelRawData(pLoadRiderWorkArea, &pLoadRiderWorkArea->m18_3dModel, 0, pBundle, pLoadRiderWorkArea->m10_modelIndex, pAnimation, pDefaultPose, 0, r13->m_pExtraData);

    return pLoadRiderWorkArea;
}

void loadCurrentRider(s_workArea* pWorkArea)
{
    loadRider(pWorkArea, mainGameState.gameStats.m2_rider1);
}

void loadCurrentRider2(s_workArea* pWorkArea)
{
    loadRider2(pWorkArea, mainGameState.gameStats.m3_rider2);
}

void loadDragonRiderDataFromCommon()
{
    gRiderTable[1].m_pExtraData = readRiderDefinitionSub(gCommonFile->getSaturnPtr(0x2020fc));
}
