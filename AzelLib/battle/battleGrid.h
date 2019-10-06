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
    sVec3_FP m34;
    sVec3_FP m40;
    sVec3_FP m64;
    sVec3_FP m70;
    sVec3_FP m84;
    sVec3_FP m90;
    sVec3_FP mB4_rotation;
    sVec3_FP mC0;
    sVec3_FP mCC;
    sVec3_FP mD8;
    sVec3_FP mE4;
    sVec3_FP mF0;
    sVec3_FP mFC;
    sVec3_FP m108;
    sVec3_FP m114;
    s32 m12C;
    s32 m130;
    sVec3_FP m134;
    sVec3_FP m140;
    s32 m14C;
    sMatrix4x3 m150_matrix;
    sVec3_FP m180_translation;
    sVec3_FP m198;
    sVec3_FP m1A4;
    s32 m1B4;
    sVec3_FP* m1B8;
    s32 m1B0;
    u32 m1C8;
    sVec3_FP m1CC_lightColor;
    sVec3_FP m1E4_lightFalloff0;
    sVec3_FP m1FC_lightFalloff1;
    sVec3_FP m208_lightFalloff2;
    s8 m214;
    s32 m218;
    fixedPoint m280;
    fixedPoint m284;
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
};

p_workArea createBattleEnvironmentGridTask(p_workArea parent);
