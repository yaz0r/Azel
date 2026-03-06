#pragma once

struct s_lightSetup
{
    fixedPoint m0;
    fixedPoint m4;
    fixedPoint m8;
    fixedPoint mC;
    u32 m10;
    u32 m14;
    u32 m18;
    fixedPoint m1C_pointLightX;
    fixedPoint m20_pointLightY;
    fixedPoint m24_pointLightZ;
    u32 m28_pointLightParam;
};

extern s_lightSetup lightSetup;
extern s32 pointLightParams[5];
