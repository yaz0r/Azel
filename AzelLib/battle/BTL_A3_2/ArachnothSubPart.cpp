#include "PDS.h"
#include "ArachnothSubPart.h"
#include "battle/battleTargetable.h"
#include "kernel/graphicalObject.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"

void arachnothCreateSubModel(sArachnothSubModel* pThis, s_workAreaCopy* pParent, npcFileDeleter* param_3, s8 param_4, sSaturnPtr param_5)
{
    sSaturnPtr psVar5 = readSaturnEA(param_5);
    pThis->m0 = param_3;
    pThis->m4 = param_5;

    sModelHierarchy* pHierarchy = pParent->m0_fileBundle->getModelHierarchy(readSaturnU16(psVar5));
    sStaticPoseData* pStaticPose = pParent->m0_fileBundle->getStaticPose(readSaturnU16(psVar5 + 2), pHierarchy->countNumberOfBones());
    pThis->hotpointBundle = new sHotpointBundle(readSaturnEA(psVar5 + 4));
    init3DModelRawData(pParent, &pThis->m8_model, 0, param_3->m0_fileBundle, readSaturnU16(psVar5), nullptr, pStaticPose, nullptr, pThis->hotpointBundle);
    pThis->m60 = pThis->m8_model.m12_numBones;
    pThis->m64 = 0;
    if (!readSaturnEA(psVar5 + 4).isNull())
    {
        for (int i = 0; i < pThis->m60; i++)
        {
            pThis->m64 += (*pThis->m8_model.m40)[i].m4_count;
        }
    }
    pThis->m68 = pParent;
    pThis->m80 = 0;
    pThis->m84 = 0;
    pThis->m88 = 0;
    pThis->m8C = 0;

    int targetableIndex = 0;
    if (pThis->m64)
    {
        pThis->m58_targetables.resize(pThis->m64);
        pThis->m5C_targetablesPosition.resize(pThis->m64);

        for (int i = 0; i < pThis->m60; i++)
        {
            if (pThis->m8_model.m44_hotpointData[i].size())
            {
                for (int j = 0; j < (*pThis->m8_model.m40)[i].m4_count; j++)
                {
                    initTargetable(&pThis->m58_targetables[targetableIndex], pThis->m68, &pThis->m5C_targetablesPosition[targetableIndex], (*pThis->m8_model.m40)[i].m0[j].m10, (*pThis->m8_model.m40)[i].m0[j].m0, param_4, 0, 10);
                    targetableIndex++;
                }
            }
        }
    }

}

void arachnothCreateSubModel2(sArachnothSubModel* pThis, s_workAreaCopy* pParent, npcFileDeleter* param_3, s8 param_4, sSaturnPtr param_5, sSaturnPtr param_6, sSaturnPtr param_7)
{
    sSaturnPtr psVar5 = readSaturnEA(param_5);
    pThis->m0 = param_3;
    pThis->m4 = param_5;

    sModelHierarchy* pHierarchy = pParent->m0_fileBundle->getModelHierarchy(readSaturnU16(psVar5));
    sStaticPoseData* pStaticPose = pParent->m0_fileBundle->getStaticPose(readSaturnU16(psVar5 + 2), pHierarchy->countNumberOfBones());
    pThis->hotpointBundle = new sHotpointBundle(readSaturnEA(psVar5 + 4));
    init3DModelRawData(pParent, &pThis->m8_model, 0, param_3->m0_fileBundle, readSaturnU16(psVar5), nullptr, pStaticPose, nullptr, pThis->hotpointBundle);
    FunctionUnimplemented();
    pThis->m60 = pThis->m8_model.m12_numBones;
    pThis->m64 = 0;
    if (!readSaturnEA(psVar5 + 4).isNull())
    {
        for (int i = 0; i < pThis->m60; i++)
        {
            pThis->m64 += (*pThis->m8_model.m40)[i].m4_count;
        }
    }
    pThis->m68 = pParent;
    pThis->m80 = 0;
    pThis->m84 = 0;
    pThis->m88 = 0;
    pThis->m8C = 0;

    int targetableIndex = 0;
    if (pThis->m64)
    {
        pThis->m58_targetables.resize(pThis->m64);
        pThis->m5C_targetablesPosition.resize(pThis->m64);

        for (int i = 0; i < pThis->m60; i++)
        {
            if (pThis->m8_model.m44_hotpointData[i].size())
            {
                for (int j = 0; j < (*pThis->m8_model.m40)[i].m4_count; j++)
                {
                    initTargetable(&pThis->m58_targetables[targetableIndex], pThis->m68, &pThis->m5C_targetablesPosition[targetableIndex], (*pThis->m8_model.m40)[i].m0[j].m10, (*pThis->m8_model.m40)[i].m0[j].m0, param_4, 0, 10);
                    targetableIndex++;
                }
            }
        }
    }
}

void arachnothInitSubModelAnimation(sArachnothSubModel* pThis, s32, s32)
{
    FunctionUnimplemented();
}

void arachnothInitSubModelFunctions(sArachnothSubModel* pThis, s32, void (*param_2)(sArachnothSubModel*, s32), void (*param_3)(sArachnothSubModel*, s32), void (*param_4)(sArachnothSubModel*, s32))
{
    FunctionUnimplemented();
}

void arachnothSubModelFunction0(sArachnothSubModel* pThis, s32) { FunctionUnimplemented(); }
void arachnothSubModelFunction1(sArachnothSubModel* pThis, s32) { FunctionUnimplemented(); }
void arachnothSubModelFunction2(sArachnothSubModel* pThis, s32) { FunctionUnimplemented(); }

void createArachnothFormationSub0(sArachnothSubModel* pThis, s32)
{
    FunctionUnimplemented();
}

s32 arachnothSubPartGetDamage(sArachnothSubModel* pThis)
{
    if (pThis->m64)
    {
        FunctionUnimplemented();
        return 0;
    }
    return 0;
}

