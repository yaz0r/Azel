#include "PDS.h"
#include "battleFormation.h"

std::vector<sFormationData> allocateFormationData(p_workArea pParent, u32 formationSize)
{
    std::vector<sFormationData> formation;
    formation.resize(formationSize);

    for (int i = 0; i < formationSize; i++)
    {
        sFormationData& formationEntry = formation[i];
        for (int j = 0; j < 3; j++)
        {
            formationEntry.m0[j].zeroize();
            formationEntry.m24[j].zeroize();
        }
        formationEntry.m24[0][1] = 0x8000000;
    }

    return formation;
}

