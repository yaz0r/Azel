#include "PDS.h"
#include "baldorQueen.h"
#include "baldor.h"
#include "BTL_A3_data.h"
#include "battle/battleGenericData.h"
#include "battle/battleManager.h"
#include "battle/battleDebug.h"
#include "battle/battleEngine.h"
#include "kernel/animation.h"
#include "battle/particleEffect.h"

void Baldor_initSub0(sBaldorBase* pThis, sSaturnPtr dataPtr, sFormationData* pFormationEntry, s32 arg); // TODO: cleanup
void urchinUpdateSub3(s_3dModel* pModel, std::vector<sVec3_FP>& pPosition); // TODO: cleanup

struct sBaldorQueen : public sBaldorBase
{
    // 0x48
};

void BaldorQueen_init(sBaldorBase* pThisBase, sFormationData* pFormationData) {
    sBaldorQueen* pThis = (sBaldorQueen*)pThisBase;
    Baldor_initSub0(pThis, g_BTL_A3->getSaturnPtr(0x60A75CC), pFormationData, 0);
}

void BaldorQueen_updateSub0(sBaldorQueen* pThis) {
    Unimplemented();
}

void BaldorQueen_updateMode0(sBaldorQueen* pThis) {
    stepAnimation(pThis->m38_3dModel);
    if (pThis->m10_HP > 415)
    {
        stepAnimation(pThis->m38_3dModel); // double animation speed when not damaged?
    }
    if ((randomNumber() & 0x1F) == 0) {
        sVec3_FP tempVec;
        transformAndAddVec(pThis->m38_3dModel->m44_hotpointData[1][3], tempVec, cameraProperties2.m28[1]);
        for (int i = 0; i < 10; i++) {
            sVec3_FP spawnLocation;
            spawnLocation[0] = (randomNumber() & 0xFFF) - 0x7FF;
            spawnLocation[1] = (randomNumber() & 0xFFF) - 0x17FF;
            spawnLocation[2] = (randomNumber() & 0xFFF) - 0x7FF;

            sVec3_FP tempVector2 = {0, -0x2C, 0};
            createParticleEffect(dramAllocatorEnd[readSaturnU8(pThis->m3C_dataPtr)].mC_fileBundle, &g_BTL_GenericData->m_0x60a8c24_animatedQuad, &tempVec, &spawnLocation, &tempVector2, 0x10000, 0, (randomNumber() & 0x1F) + 0x18);
        }
    }
    if (pThis->m34_formationEntry->m49 == 2) {
        pThis->m8_mode = 1;
        pThis->m34_formationEntry->m48 |= 1;
        pThis->m34_formationEntry->m49 = 0;
    }
}

void BaldorQueen_update(sBaldorBase* pThisBase) {
    sBaldorQueen* pThis = (sBaldorQueen*)pThisBase;
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1B]) {
        assert(0);
    }
    *pThis->m1C_translation.m0_current = gBattleManager->m10_battleOverlay->m4_battleEngine->mC_battleCenter + *pThis->m1C_translation.m4_target;
    urchinUpdateSub3(pThis->m38_3dModel, pThis->m18_position);
    BaldorQueen_updateSub0(pThis);
    switch (pThis->m8_mode) {
    case 0:
        BaldorQueen_updateMode0(pThis);
        break;
    default:
        assert(0);
    }
}

void BaldorQueen_draw(sBaldorBase* pThisBase) {
    sBaldorQueen* pThis = (sBaldorQueen*)pThisBase;
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m1C_translation.m0_current);
    rotateCurrentMatrixYXZ(pThis->m28_rotation.m0_current);
    pThis->m38_3dModel->m18_drawFunction(pThis->m38_3dModel);
    popMatrix();
}

p_workArea baldorQueenCreateQueenTask(s_workAreaCopy* pParent, sFormationData* param_2) {

    static const sBaldorQueen::TypedTaskDefinition definition = {
        BaldorQueen_init,
        BaldorQueen_update,
        BaldorQueen_draw,
        nullptr,
    };
    return createSubTaskWithArgWithCopy<sBaldorQueen, sFormationData*>(pParent, param_2, &definition);
}
