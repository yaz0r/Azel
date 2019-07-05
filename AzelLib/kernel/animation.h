#pragma once

void copyPosePosition(s_3dModel* pModel);
void copyPoseRotation(s_3dModel* pModel);
void resetPoseScale(s_3dModel* pModel);

u32 stepAnimation(s_3dModel* p3DModel);
void interpolateAnimation(s_3dModel* p3dModel);

u32 setupModelAnimation(s_3dModel* pModel, u8* r5);
u32 setupPoseInterpolation(s_3dModel* pModel, u32 interpolationLength);
