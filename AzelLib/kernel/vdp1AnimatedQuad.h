#pragma once

struct sVdp1Quad {
    u8 m0_isLast;
    u8 m1;
    u16 m2_CMDCTRL;
    u16 m4_CMDPMOD;
    u16 m6_CMDSRCA;
    u16 m8_CMDSIZE;
    u16 mA_CMDCOLR;
    fixedPoint mC_width;
    fixedPoint m10_height;
    fixedPoint m14_X;
    fixedPoint m18_Y;
};


struct sAnimatedQuad {
    const std::vector<sVdp1Quad>* m0_quad{};
    u16 m4_vdp1Memory{};
    u8 m6{};
    u8 m7_currentFrame{};
    // size about 0x8?
};

void particleInitSub(sAnimatedQuad* pThis, u16 vdp1Memory, const std::vector<sVdp1Quad>* param_3);
int sGunShotTask_UpdateSub4(sAnimatedQuad* pThis);
int drawProjectedParticle(sAnimatedQuad* pThis, sVec3_FP* position);
int vdp1DrawQuadScaled(sAnimatedQuad* pThis, sVec3_FP* position, fixedPoint scale);
