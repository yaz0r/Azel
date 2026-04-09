#include "PDS.h"
#include "fieldModelRender.h"

// 0606b45c
void initFieldModelRenderContext(sFieldModelRenderContext* pCtx, void* parent, void* callback,
    sVec3_FP* pPosition, sVec3_FP* pNormal, u16 flags, s16 modelIdx, s16 poseIdx, u8 param9, u8 param10)
{
    pCtx->m0_parentWorkArea = parent;
    pCtx->m4_initCallback = callback;
    pCtx->m8_pPosition = pPosition;
    pCtx->mC_pNormal = pNormal;
    pCtx->m12_modelIndex = modelIdx;
    pCtx->m14_poseIndex = poseIdx;
    if (pCtx->m14_poseIndex < 0)
    {
        pCtx->m16_param9 = 0;
    }
    else
    {
        pCtx->m16_param9 = param9;
    }
    pCtx->m10_flags = flags;
    pCtx->m17_param10 = param10;
    pCtx->m18_visibilityFlags = 0;
    pCtx->m19_drawFlags = 0;
    pCtx->m1A = 0;
    pCtx->m1B = 0;
    pCtx->m1C = 0;
    pCtx->m20 = 0;
}

// 0606b4b0
void updateFieldModelRenderContext(sFieldModelRenderContext* pCtx)
{
    s_LCSTask* pLCS = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;

    pCtx->m19_drawFlags &= 0xCF;
    pCtx->m1C = 0;
    pCtx->m18_visibilityFlags &= ~4;

    if ((pCtx->m18_visibilityFlags & 1) == 0)
    {
        if (pCtx->mC_pNormal != nullptr)
        {
            sVec3_FP viewPos;
            if ((pCtx->m10_flags & 0x100) == 0)
            {
                transformAndAddVecByCurrentMatrix(pCtx->m8_pPosition, &viewPos);
            }
            else
            {
                viewPos = *pCtx->m8_pPosition;
            }

            sVec3_FP viewNormal;
            if ((pCtx->m10_flags & 0x200) == 0)
            {
                transformVecByCurrentMatrix(*pCtx->mC_pNormal, viewNormal);
            }
            else
            {
                viewNormal = *pCtx->mC_pNormal;
            }

            fixedPoint dotResult = dot3_FP(&viewPos, &viewNormal);
            if (dotResult.asS32() >= 0)
            {
                pCtx->m18_visibilityFlags |= 4;
            }
        }

        // Register in LCS draw list
        s_LCSTask* pLCSTask = getFieldTaskPtr()->m8_pSubFieldData->m340_pLCS;
        pLCSTask->m10++;
        if (pLCSTask != nullptr && pLCSTask->mC < 0x100)
        {
            s_LCSTask_14& entry = pLCSTask->m14[pLCSTask->mC];
            entry.m0 = (sLCSTarget*)pCtx;
            entry.m4_next = nullptr;
            pLCSTask->mC++;
        }
    }
}
