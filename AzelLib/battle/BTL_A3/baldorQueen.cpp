#include "PDS.h"
#include "baldorQueen.h"
#include "baldor.h"
#include "BTL_A3_data.h"
#include "battle/battleManager.h"
#include "battle/battleDebug.h"
#include "battle/battleEngine.h"
#include "kernel/animation.h"

void Baldor_initSub0(sBaldorBase* pThis, sSaturnPtr dataPtr, sFormationData* pFormationEntry, s32 arg); // TODO: cleanup
void urchinUpdateSub3(s_3dModel* pModel, std::vector<sVec3_FP>& pPosition);

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
        stepAnimation(pThis->m38_3dModel);
        if (pThis->m10_HP > 415)
        {
            stepAnimation(pThis->m38_3dModel); // double animation speed when not damaged?
        }
        Unimplemented();
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
