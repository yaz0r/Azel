#include "PDS.h"
#include "battleFormation.h"

std::vector<sFormationData> allocateFormationData(p_workArea pParent, u32 formationSize)
{
    std::vector<sFormationData> formation;
    formation.resize(formationSize);

    for (int i = 0; i < formationSize; i++)
    {
        sFormationData& formationEntry = formation[i];
        formationEntry.m0.m0.zeroize();
        formationEntry.m0.mC.zeroize();
        formationEntry.m0.m18.zeroize();

        formationEntry.m24.m0.zeroize();
        formationEntry.m24.mC.zeroize();
        formationEntry.m24.m18.zeroize();

        formationEntry.m24.m0[1] = 0x8000000;
    }

    return formation;
}

