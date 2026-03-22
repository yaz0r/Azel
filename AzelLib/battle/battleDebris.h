#pragma once

void createBoneDebris(s_workAreaCopy* parent, npcFileDeleter* fileBundle, s_3dModel* model,
    sVec3_FP* position, sVec3_FP* rotation,
    sVec3_FP* velocity, sVec3_FP* acceleration,
    s32 gravity, s32 velocityRandomMask,
    s32 param10, s32 param11, s16 bounceCountMax,
    s32 scale, s16 lifetime);
