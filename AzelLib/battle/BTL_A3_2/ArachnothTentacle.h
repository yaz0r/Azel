#pragma once

struct sArachnothTentacleSub {
    sVec3_FP m0;
    sVec3_FP mC;
    sVec3_FP m18;
    // size 0x24
};

struct sArachnothTentacle : public s_workAreaTemplateWithCopy<sArachnothTentacle>
{
    std::array<s16, 6>* m8;
    std::array<sArachnothTentacleSub, 10> mC_segments;
    sVec3_FP* m174_position;
    sVec3_FP* m178_rotation;
    sVec3_FP m17C_offset;
    s32 m188_currentMode;
    fixedPoint m190;
    fixedPoint m194;
    // size 0x198
};

sArachnothTentacle* createArachnothTentacle(s_workAreaCopy* pParent, sVec3_FP* position, sVec3_FP* rotation, sVec3_FP* offset);
void arachnothTentacle_setMode1(sArachnothTentacle* pThis);
void arachnothTentacle_setMode2(sArachnothTentacle* pThis);
