#pragma once

struct sFormationData
{
    std::array<sVec3_FP, 3> m0;
    std::array<sVec3_FP, 3> m24;
    s8 m48;
    s8 m49;
    // size: 0x4C
};

std::vector<sFormationData> allocateFormationData(p_workArea pParent, u32 formationSize);


