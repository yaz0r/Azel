#pragma once

p_workArea createDragonMenuMorhTask(p_workArea pWorkArea);
void updateAnimationMatrices(s3DModelAnimData* r4, s_3dModel* r5);
void generateCameraMatrixSub1(sVec3_FP& r4, u32(&r5)[2]);
void updateDragonMovementFromControllerType1Sub2Sub1(s3DModelAnimData* r4, fixedPoint r5);
void updateDragonMovementFromControllerType1Sub2Sub2(s3DModelAnimData* r4, fixedPoint r5);
void updateDragonMovementFromControllerType1Sub2Sub3(s3DModelAnimData* r4, fixedPoint r5);
