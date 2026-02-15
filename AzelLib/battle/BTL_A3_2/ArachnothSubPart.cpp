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
    pThis->m60_numBones = pThis->m8_model.m12_numBones;
    pThis->m64 = 0;
    if (!readSaturnEA(psVar5 + 4).isNull())
    {
        for (int i = 0; i < pThis->m60_numBones; i++)
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

        for (int i = 0; i < pThis->m60_numBones; i++)
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
    Unimplemented();
    pThis->m60_numBones = pThis->m8_model.m12_numBones;
    pThis->m64 = 0;
    if (!readSaturnEA(psVar5 + 4).isNull())
    {
        for (int i = 0; i < pThis->m60_numBones; i++)
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

        for (int i = 0; i < pThis->m60_numBones; i++)
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

void arachnothInitSubModelAnimation(sArachnothSubModel* pThis, s32 animationIndex, s32 unk)
{
    sAnimationData* pAnimation = pThis->m0->m0_fileBundle->getAnimation(readSaturnU16(readSaturnEA(pThis->m4 + 4) + animationIndex * 2));
    initAnimation(&pThis->m8_model, pAnimation);
    resetAnimation(&pThis->m8_model);
    pThis->m74 = 1;

    if (unk == -2)
    {
        assert(0);
    }
    else
    {
        pThis->m7C = unk;
    }    
}

void arachnothInitSubModelFunctions(sArachnothSubModel* pThis, void (*param_2)(sArachnothSubModel*, s32), void (*param_3)(sArachnothSubModel*, s32), void (*param_4)(sArachnothSubModel*, s32), void (*param_5)(sArachnothSubModel*, s32))
{
    pThis->m80 = param_2;
    pThis->m84 = param_3;
    pThis->m88 = param_4;
    pThis->m8C = param_5;
}

void arachnothSubModelFunction0(sArachnothSubModel* pThis, s32) { Unimplemented(); }
void arachnothSubModelFunction1(sArachnothSubModel* pThis, s32) { Unimplemented(); }
void arachnothSubModelFunction2(sArachnothSubModel* pThis, s32) { Unimplemented(); }

void createArachnothFormationSub0(sArachnothSubModel* pThis, s32 param_2)
{
    sSaturnPtr iVar1 = readSaturnEA(readSaturnEA(pThis->m4) + 4);
    int counter = 0;
    if (!iVar1.isNull())
    {
        for (int i = 0; i < pThis->m60_numBones; i++)
        {
            sSaturnPtr data = iVar1 + 8 * i;
            for (int j = 0; j < readSaturnS32(data + 4); j++)
            {
                u32 uVar3 = readSaturnU32(readSaturnEA(data) + j * 0x14) >> 0x1C;
                switch (param_2)
                {
                case 1:
                    pThis->m58_targetables[counter].m50_flags = (((uVar3 >> 1) | ((uVar3 & 1) << 3)) << 0x1C) | (readSaturnU32(readSaturnEA(data) + j * 0x14) & 0x0fffffff);
                    break;
                case 2:
                    pThis->m58_targetables[counter].m50_flags = (((uVar3 >> 2) | ((uVar3 & 1) << 2)) << 0x1C) | (readSaturnU32(readSaturnEA(data) + j * 0x14) & 0x0fffffff);
                    break;
                case 3:
                    pThis->m58_targetables[counter].m50_flags = (((uVar3 << 1) | ((uVar3 & 1) >> 3)) << 0x1C) | (readSaturnU32(readSaturnEA(data) + j * 0x14) & 0x0fffffff);
                    break;
                default:
                    assert(0);
                    break;
                }
                counter++;
            }
        }
    }
}

void arachnothSubPart_updateTargetablesPosition(sArachnothSubModel* pThis)
{
    int currentTargetableId = 0;
    for (int i=0; i<pThis->m60_numBones; i++)
    {
        const std::vector<sVec3_FP>& boneData = pThis->m8_model.m44_hotpointData[i];
        if (boneData.size())
        {
            for (int j=0; j<boneData.size(); j++)
            {
                pThis->m5C_targetablesPosition[currentTargetableId++] = boneData[j];
            }
        }
    }
}

s32 arachnothSubPart_getTargetablesDamage(sArachnothSubModel* pThis)
{
    Unimplemented();
    return 0;
}

s32 arachnothSubPartGetDamage(sArachnothSubModel* pThis)
{
    if (pThis->m64)
    {
        arachnothSubPart_updateTargetablesPosition(pThis);
        return arachnothSubPart_getTargetablesDamage(pThis);
    }
    return 0;
}

