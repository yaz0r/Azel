#pragma once

p_workArea createDragonMenuMorhTask(p_workArea pWorkArea);
void updateAnimationMatrices(s3DModelAnimData* r4, s_3dModel* r5);
void computeVectorAngles(const sVec3_FP& r4, sVec2_FP& r5);
void computeVectorAngles(const sVec3_FP& r4, sVec3_FP& r5_output);
void incrementAnimationRootY(s3DModelAnimData* r4, fixedPoint r5);
void incrementAnimationRootX(s3DModelAnimData* r4, fixedPoint r5);
void incrementAnimationRootZ(s3DModelAnimData* r4, fixedPoint r5);
void playAnimation(s_3dModel* pDragonStateData1, struct sAnimationData* pAnimation, u32 r6);
void resetCameraProperties2(s_cameraProperties2* r4);
void transformVec(const sVec3_FP& r4, sVec3_FP& r5, const sMatrix4x3& r6);
void generateCameraMatrix(s_cameraProperties2* r4, const sVec3_FP& position, const sVec3_FP& target, const sVec3_FP& up);
void submitModelAndShadowModelToRendering(s_3dModel* p3dModel, u32 modelIndex, u32 shadowModelIndex, sVec3_FP* translation, sVec3_FP* rotation, fixedPoint shadowHeight);
void setupVdp1LocalCoordinatesAndClipping();
