#pragma once

p_workArea createDragonMenuMorhTask(p_workArea pWorkArea);
void updateAnimationMatrices(s3DModelAnimData* r4, s_3dModel* r5);
void generateCameraMatrixSub1(sVec3_FP& r4, fixedPoint(&r5)[2]);
void updateDragonMovementFromControllerType1Sub2Sub1(s3DModelAnimData* r4, fixedPoint r5);
void updateDragonMovementFromControllerType1Sub2Sub2(s3DModelAnimData* r4, fixedPoint r5);
void updateDragonMovementFromControllerType1Sub2Sub3(s3DModelAnimData* r4, fixedPoint r5);
void playAnimation(s_3dModel* pDragonStateData1, u8* r5, u32 r6);
u32 dragonMenuDragonInitSub2Sub1(s_3dModel* pDragonStateData1, u32 interpolationLength);
void resetCameraProperties2(s_cameraProperties2* r4);
void transformVec(const sVec3_FP& r4, sVec3_FP& r5, const sMatrix4x3& r6);
