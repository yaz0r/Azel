#include "PDS.h"
#include "battleFormation.h"

std::vector<sFormationData> allocateFormationData(p_workArea pParent, u32 formationSize)
{
    std::vector<sFormationData> formation;
    formation.resize(formationSize);

    for (int i = 0; i < formationSize; i++)
    {
        sFormationData& formationEntry = formation[i];
        formationEntry.m0_translation.m0_current.zeroize();
        formationEntry.m0_translation.mC_target.zeroize();
        formationEntry.m0_translation.m18.zeroize();

        formationEntry.m24_rotation.m0_current.zeroize();
        formationEntry.m24_rotation.mC_target.zeroize();
        formationEntry.m24_rotation.m18.zeroize();

        formationEntry.m24_rotation.m0_current[1] = 0x8000000;
    }

    return formation;
}

