#pragma once

struct pvecControl
{
    sVec3_FP* m0_current;
    sVec3_FP* m4_target;
    sVec3_FP* m8;
};

struct vecControl
{
    sVec3_FP m0_current;
    sVec3_FP mC_target;
    sVec3_FP m18;
};

struct sFormationData
{
    vecControl m0_translation;
    vecControl m24_rotation;
    s8 m48;
    s8 m49;
    // size: 0x4C
};

std::vector<sFormationData> allocateFormationData(p_workArea pParent, u32 formationSize);


