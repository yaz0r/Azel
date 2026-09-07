#pragma once

#include "field/fieldModelRender.h"
#include "kernel/monsterPart.h"

struct sA5SandWorm : public s_workAreaTemplate<sA5SandWorm>
{
    s_memoryAreaOutput m0_memoryArea;
    s32* m8_perPartInts;
    u8* mC_perPartBytes;
    sVec3_FP m10_pos;
    s32 m1C_radius;
    s32 m20_verticalAccel;
    s32 m24;
    s32 m28_param10;
    s32 m2C_param2C;
    s32 m30_timer;
    s32 m34;
    s32 m38;
    s32 m3C;
    sVec3_FP m40_renderPosition;
    sVec3_FP m4C_rotationTarget;
    sMonsterBody* m58_body;
    sVec3_FP m5C_velocity;
    s32 m68_partCount;
    s_3dModel m6C_modelHead;
    s_3dModel mBC_modelTail;
    sFieldModelRenderContext m10C_modelCtx;
    u8 m140_state;
    u8 m141_argFlag;
    u8 m142_subState;
    u8 m143_pad;
    // size 0x144
};

void a5SandWorm_Update(sA5SandWorm* pThis);
