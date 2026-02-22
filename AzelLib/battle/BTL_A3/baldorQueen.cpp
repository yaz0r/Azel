#include "PDS.h"
#include "baldorQueen.h"
#include "baldor.h"
#include "BTL_A3_data.h"

void Baldor_initSub0(sBaldorBase* pThis, sSaturnPtr dataPtr, sFormationData* pFormationEntry, s32 arg); // TODO: cleanup

struct sBaldorQueen : public sBaldorBase
{
    // 0x48
};

void BaldorQueen_init(sBaldorBase* pThisBase, sFormationData* pFormationData) {
    sBaldorQueen* pThis = (sBaldorQueen*)pThisBase;
    Baldor_initSub0(pThis, g_BTL_A3->getSaturnPtr(0x60A75CC), pFormationData, 0);
}

void BaldorQueen_update(sBaldorBase* pThisBase) {
    sBaldorQueen* pThis = (sBaldorQueen*)pThisBase;
}

void BaldorQueen_draw(sBaldorBase* pThisBase) {
    sBaldorQueen* pThis = (sBaldorQueen*)pThisBase;
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m1C_translation.m0_current);
    rotateCurrentMatrixYXZ(pThis->m28_rotation.m0_current);
    pThis->m38_3dModel->m18_drawFunction(pThis->m38_3dModel);
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
