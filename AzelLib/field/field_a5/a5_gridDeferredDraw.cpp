#include "PDS.h"
#include "o_fld_a5.h"
#include "a5_gridDeferredDraw.h"
#include "field/fieldVisibilityGrid.h"
#include "field/fieldDragon.h"
#include "kernel/fileBundle.h"
#include "kernel/animation.h"
#include "3dEngine.h"
#include "3dModels.h"
#include "processModel.h"

// 06077074 — variant of gridCellDraw_normalSub2 that takes an explicit
// world-space (pos, rot) pair. Culls against the dragon's m8_pos
// directly (axis-aligned), builds a local world matrix, copies it to
// the grid's per-object cache, and enqueues the draw with depth 0x10000.
void a5GridDraw_cullAndEnqueueWithTransform_06077074(
    s_fileBundle* pBundle, s32 bundleKey, const sVec3_FP* pPos, const sVec3_FP* pRot)
{
    if (bundleKey == 0)
        return;

    sProcessed3dModel* pModel = pBundle->get3DModel(bundleKey);
    if (pModel == nullptr)
        return;

    s32 cullRadius = pModel->m0_radius + 0x8000;

    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    s_visibilityGridWorkArea* pGrid = pFieldTask->m8_pSubFieldData->m348_pFieldCameraTask1;
    s_dragonTaskWorkArea* pDragon   = pFieldTask->m8_pSubFieldData->m338_pDragonTask;

    s32 dx = pPos->m0_X.m_value - pDragon->m8_pos.m0_X.m_value;
    if (dx < 0) dx = -dx;
    s32 dy = pPos->m4_Y.m_value - pDragon->m8_pos.m4_Y.m_value;
    if (dy < 0) dy = -dy;
    s32 dz = pPos->m8_Z.m_value - pDragon->m8_pos.m8_Z.m_value;
    if (dz < 0) dz = -dz;

    if (dx <= cullRadius && dy <= cullRadius && dz <= cullRadius)
    {
        sMatrix4x3 local;
        initMatrixToIdentity(&local);
        translateMatrix(*pPos, &local);
        rotateMatrixZYX(pRot, &local);

        copyMatrix(&local, &pGrid->m44->m4_matrix);
        allocateLCSEntry(pGrid, pModel, fixedPoint(0x10000));
    }
}
