#pragma once

struct sCollisionBody;

struct sResCameraProperties
{
    fixedPoint m0_LCS_X;
    fixedPoint m4_LCS_Y;
    s32 m8_LCSWidth;
    fixedPoint mC_LCSHeight;
    s32 m10_activeLCSType; // define the type of m14.
    s32 m14_activeLCSEnvironmentIndex; // used for LCS type 1 (environment LCS)
    sCollisionBody* m14_activeLCS; // used for LCS type 2 (npc LCS)
    fixedPoint m18;
    s16 m1C_LCSHeightMin;
    s16 m1E_LCSHeightMax;
    s16 m20_LCSWidthMin;
    s16 m22_LCSWidthMax;
    fixedPoint m24_LCSDepthMin;
    fixedPoint m28_LCSDepthMax;
    fixedPoint m2C_projectionWidthScale;
    fixedPoint m30_projectionHeightScale;
    fixedPoint m34_boundMinX;
    fixedPoint m38_radiusScaleMinX;
    fixedPoint m3C_boundMaxX;
    fixedPoint m40_radiusScaleMaxX;
    fixedPoint m44_boundMinY;
    fixedPoint m48_radiusScaleMinY;
    fixedPoint m4C_boundMaxY;
    fixedPoint m50_radiusScaleMaxY;
    fixedPoint m54;
    fixedPoint m58;
};
extern sResCameraProperties LCSCollisionData;

void setupDataForLCSCollision();
void findLCSCollision();
void copyCameraPropertiesToRes();
