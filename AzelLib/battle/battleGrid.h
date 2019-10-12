#pragma once

struct s_battleGrid : public s_workAreaTemplate<s_battleGrid>
{
    s8 m0;
    s8 m1;
    s8 m2;

    s32 m4;
    s32 m8;
    s32 mC;
    sVec3_FP m10;
    sVec3_FP m1C;
    sVec3_FP m28;
    sVec3_FP m34_cameraPosition;
    sVec3_FP m40;
    sVec3_FP m64_cameraRotationTarget;
    sVec3_FP m70;
    sVec3_FP m84_cameraRotationDelta;
    sVec3_FP m90;
    sVec3_FP mB4_cameraRotation;
    sVec3_FP mC0_cameraRotationInterpolation;
    sVec3_FP mCC_cameraInterpolationRotationFactor;
    sVec3_FP mD8_cameraInterpolationRotationFactor2;
    sVec3_FP mE4_currentCameraReferenceCenter;
    sVec3_FP mF0_currentCameraReferenceForward;
    sVec3_FP mFC_cameraReferenceUp;
    sVec3_FP m108_deltaCameraPosition;
    sVec3_FP m114_deltaCameraTarget;
    s32 m12C_cameraInterpolationSpeed;
    s32 m130_cameraInterpolationSpeed2;
    sVec3_FP m134_desiredCameraPosition;
    sVec3_FP m140_desiredCameraTarget;
    s32 m14C;
    sMatrix4x3 m150_cameraMatrix;
    sVec3_FP m180_cameraTranslation;
    sVec3_FP m18C;
    sVec3_FP m198;
    sVec3_FP m1A4;
    s32 m1B0;
    s32 m1B4;
    sVec3_FP* m1B8_pCameraTranslationSource;
    sVec3_FP m1BC_cameraRotationStep;
    u32 m1C8_flags;
    sVec3_FP m1CC_lightColor;
    sVec3_FP m1D8_newLightColor;
    sVec3_FP m1E4_lightFalloff0;
    sVec3_FP m1FC_lightFalloff1;
    sVec3_FP m208_lightFalloff2;
    s8 m214;
    s32 m218_fov;
    fixedPoint m280_lightAngle1;
    fixedPoint m284_lightAngle2;
    std::array<sVec3_FP, 4> m298;
    std::array<sVec3_FP, 4> m2C8;
    s32 m2F8;
    s32 m2FC;
    s32 m300;
    s32 m304;
    s32 m308;
    s32 m30C;
    s8 m314;
    s32 m318;
    s32 m31C;
    s32 m320;
    //size: 0x324

#ifndef SHIPPING_BUILD
    void Inspect() override;
#endif
};

p_workArea createBattleEnvironmentGridTask(p_workArea parent);
