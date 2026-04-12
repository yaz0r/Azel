#include "PDS.h"
#include "kernel/monsterPart.h"
#include "kernel/fileBundle.h"
#include "3dEngine.h"
#include "3dModels.h"

// 06013E58 — recursive spring/damping rotation update. Identical to
// baldorPart_update except the local-space rotation is applied with
// rotateMatrixYXZ instead of rotateMatrixZYX. Walks the part chain via
// m0_child, passing each child its parent's worldPosition/rotation as
// the new base and the parent's rotation as its target.
void monsterPart_defaultUpdate(sMonsterBodyPart* pThis, const sVec3_FP* pTranslation, const sVec3_FP* pRotation, const sVec3_FP* param4)
{
    pThis->m34_rotationAcceleration += MTH_Mul((*param4 - pThis->m1C_rotation).normalized(), pThis->m44_springStiffness);
    pThis->m34_rotationAcceleration -= MTH_Mul(pThis->m50_damping, pThis->m28_rotationVelocity);

    pThis->m28_rotationVelocity += pThis->m34_rotationAcceleration;
    pThis->m1C_rotation          += pThis->m28_rotationVelocity;

    pThis->m34_rotationAcceleration.zeroize();

    sMatrix4x3 m;
    initMatrixToIdentity(&m);
    translateMatrix(*pTranslation, &m);
    rotateMatrixYXZ(pRotation, &m);
    translateMatrix(pThis->m10_translation, &m);

    pThis->m4_worldPosition = m.getTranslation();

    if (pThis->m0_child != nullptr)
    {
        monsterPart_defaultUpdate(pThis->m0_child, &pThis->m4_worldPosition, &pThis->m1C_rotation, &pThis->m1C_rotation);
    }
}

// 06013F24 — walks the part linked list, drawing each part with a
// YXZ-order rotation via `bundle->get3DModel(part.m40_modelIndex)`.
void monsterPart_defaultDraw(s_fileBundle* pBundle, sMonsterBodyPart* pPart)
{
    while (true)
    {
        if (pPart->m40_modelIndex > 0)
        {
            pushCurrentMatrix();
            translateCurrentMatrix(pPart->m4_worldPosition);
            rotateCurrentMatrixYXZ(pPart->m1C_rotation);
            addObjectToDrawList(pBundle->get3DModel(pPart->m40_modelIndex));
            popMatrix();
        }
        if (pPart->m0_child == nullptr)
            break;
        pPart = pPart->m0_child;
    }
}

// 06013FA8 — draws a single part (no chain walk) with a YXZ-order
// rotation. Despite the Saturn labeling the slot `m2C_delete`, the
// default implementation is a one-shot per-part drawer.
void monsterPart_defaultDrawPart(s_fileBundle* pBundle, sMonsterBodyPart* pPart)
{
    if (pPart->m40_modelIndex > 0)
    {
        pushCurrentMatrix();
        translateCurrentMatrix(pPart->m4_worldPosition);
        rotateCurrentMatrixYXZ(pPart->m1C_rotation);
        addObjectToDrawList(pBundle->get3DModel(pPart->m40_modelIndex));
        popMatrix();
    }
}

// 06013D50 / 06013D70 — default head/mid/tail part zero-init helpers.
static void monsterBody_clearBody(sMonsterBody* pData)
{
    pData->m0_translation.zeroize();
    pData->mC_rotation.zeroize();
    pData->m18_rotationTarget.zeroize();
    pData->m24_update    = monsterPart_defaultUpdate;
    pData->m28_draw      = monsterPart_defaultDraw;
    pData->m2C_drawPart  = monsterPart_defaultDrawPart;
    pData->m30_parts.resize(0);
}

static void monsterBody_linkPart(sMonsterBodyPart* pEntry, sMonsterBodyPart* pNextEntry)
{
    pEntry->m0_child = pNextEntry;
    pEntry->m4_worldPosition.zeroize();
    pEntry->m10_translation.zeroize();
    pEntry->m1C_rotation.zeroize();
    pEntry->m28_rotationVelocity.zeroize();
    pEntry->m34_rotationAcceleration.zeroize();
    pEntry->m40_modelIndex = 0;
    pEntry->m44_springStiffness.zeroize();
    pEntry->m50_damping = 0;
}

// 06013C72
sMonsterBody* monsterBody_create(p_workArea /*parent*/, int numEntries)
{
    sMonsterBody* pNewData = new sMonsterBody;
    monsterBody_clearBody(pNewData);

    pNewData->m30_parts.resize(numEntries);

    monsterBody_linkPart(&pNewData->m30_parts[numEntries - 1], nullptr);
    int iVar1 = numEntries - 1;
    while (iVar1 != 0)
    {
        monsterBody_linkPart(&pNewData->m30_parts[iVar1 - 1], &pNewData->m30_parts[iVar1]);
        iVar1--;
    }

    return pNewData;
}

// 06013E02
void monsterBodyPart_loadTemplate(sMonsterBodyPart* dest, sSaturnPtr source)
{
    dest->m40_modelIndex     = readSaturnS16(source);
    dest->m44_springStiffness = readSaturnVec3(source + 4);
    dest->m50_damping        = readSaturnS32(source + 0x10);
}

// Forward-declarations of the baldor-specific physics variants provided
// by battle/BTL_A3/baldor.cpp — loaded when arg2 == 1.
extern void baldorPart_update(sMonsterBodyPart* pThis, const sVec3_FP* pTranslation, const sVec3_FP* pRotation, const sVec3_FP* param4);
extern void baldorPart_draw(s_fileBundle* pBundle, sMonsterBodyPart* pBodyPart);
extern void baldorPart_drawPart(s_fileBundle* pBundle, sMonsterBodyPart* pBodyPart);

// 06013D80
void monsterBody_loadPartData(sMonsterBody* pThis, int arg2, sSaturnPtr arg3)
{
    if (arg2 == 1)
    {
        pThis->m24_update   = baldorPart_update;
        pThis->m28_draw     = baldorPart_draw;
        pThis->m2C_drawPart = baldorPart_drawPart;
    }

    if (!arg3.isNull())
    {
        sMonsterBodyPart* pPart = &pThis->m30_parts[0];
        do
        {
            monsterBodyPart_loadTemplate(pPart, arg3);
            pPart = pPart->m0_child;
            arg3 += 0x14;
        } while (pPart);
    }
}

// 06013C70
void monsterBody_initPose(sMonsterBody* pBody, const sVec3_FP* pPos, const sVec3_FP* pRot)
{
    pBody->m0_translation     = *pPos;
    pBody->mC_rotation        = *pRot;
    pBody->m18_rotationTarget = *pRot;

    sMonsterBodyPart* pPart = &pBody->m30_parts[0];
    while (pPart != nullptr)
    {
        pPart->m1C_rotation = *pRot;
        pPart->m28_rotationVelocity.zeroize();
        pPart->m34_rotationAcceleration.zeroize();
        pPart = pPart->m0_child;
    }

    pBody->m24_update(
        &pBody->m30_parts[0],
        &pBody->m0_translation,
        &pBody->mC_rotation,
        &pBody->m18_rotationTarget);
}
