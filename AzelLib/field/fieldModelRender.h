#pragma once

struct sFieldModelRenderContext
{
    void* m0_parentWorkArea;
    void* m4_initCallback;
    sVec3_FP* m8_pPosition;
    sVec3_FP* mC_pNormal;
    u16 m10_flags;
    s16 m12_modelIndex;
    s16 m14_poseIndex;
    u8 m16_param9;
    u8 m17_param10;
    u8 m18_visibilityFlags;
    u8 m19_drawFlags;
    u8 m1A;
    u8 m1B;
    s32 m1C;
    s32 m20;
    u8 m24_pad[0x10];
    // size 0x34
};

void initFieldModelRenderContext(sFieldModelRenderContext* pCtx, void* parent, void* callback,
    sVec3_FP* pPosition, sVec3_FP* pNormal, u16 flags, s16 modelIdx, s16 poseIdx, u8 param9, u8 param10);
void updateFieldModelRenderContext(sFieldModelRenderContext* pCtx);
