#pragma once

p_workArea createDragonMenuMorhTask(p_workArea pWorkArea);
void updateAnimationMatrices(s3DModelAnimData* r4, s_3dModel* r5);
void generateCameraMatrixSub1(sVec3_FP& r4, sVec2_FP& r5);
void generateCameraMatrixSub1(sVec3_FP& r4, sVec3_FP& r5_output);
void updateDragonMovementFromControllerType1Sub2Sub1(s3DModelAnimData* r4, fixedPoint r5);
void updateDragonMovementFromControllerType1Sub2Sub2(s3DModelAnimData* r4, fixedPoint r5);
void updateDragonMovementFromControllerType1Sub2Sub3(s3DModelAnimData* r4, fixedPoint r5);
void playAnimation(s_3dModel* pDragonStateData1, struct sAnimationData* pAnimation, u32 r6);
void resetCameraProperties2(s_cameraProperties2* r4);
void transformVec(const sVec3_FP& r4, sVec3_FP& r5, const sMatrix4x3& r6);
void generateCameraMatrix(s_cameraProperties2* r4, const sVec3_FP& r13, const sVec3_FP& r6, const sVec3_FP& r7);
