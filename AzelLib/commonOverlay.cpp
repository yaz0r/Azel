#include "PDS.h"
#include "commonOverlay.h"
#include "audio/soundDataTable.h"

sCommonOverlay_data gCommonFile;

void initCommonFile()
{
    gCommonFile.m_name = "COMMON.DAT";
    gCommonFile.m_data = COMMON_DAT;
    gCommonFile.m_dataSize = 0x98000;
    gCommonFile.m_base = 0x00200000;
    gCommonFile.init();
}


void sCommonOverlay_data::init()
{
    // dragonLevelStats
    {
        sSaturnPtr pDataTable = getSaturnPtr(0x206FF8);
        for (int i = 0; i < 9; i++)
        {
            sSaturnPtr pData = readSaturnEA(pDataTable + 4 * i);
            sDragonLevelStat entry;

            for (int j = 0; j < 3; j++)
            {
                entry.m0[j] = readSaturnS8(pData + 0 + j);
            }
            for (int j = 0; j < 3; j++)
            {
                entry.m3[j] = readSaturnS8(pData + 3 + j);
            }
            for (int j = 0; j < 3; j++)
            {
                entry.m6[j] = readSaturnS8(pData + 6 + j);
            }
            for (int j = 0; j < 3; j++)
            {
                entry.m9[j] = readSaturnS8(pData + 9 + j);
            }
            for (int j = 0; j < 3; j++)
            {
                entry.mC[j] = readSaturnS8(pData + 0xC + j);
            }
            for (int j = 0; j < 3; j++)
            {
                entry.mF[j] = readSaturnS8(pData + 0xF + j);
            }
            for (int j = 0; j < 3; j++)
            {
                entry.m12[j] = readSaturnS8(pData + 0x12 + j);
            }
            for (int j = 0; j < 3; j++)
            {
                entry.m15[j] = readSaturnS8(pData + 0x15 + j);
            }
            for (int j = 0; j < 3; j++)
            {
                entry.m18[j] = readSaturnS8(pData + 0x18 + j);
            }
            for (int j = 0; j < 3; j++)
            {
                entry.m1B[j] = readSaturnS8(pData + 0x1B + j);
            }
            dragonLevelStats.push_back(entry);
        }
    }

    soundDataTableInit();

    //battleOverlaySetup
    {
        sSaturnPtr battleOverlaySetupEA = getSaturnPtr(0x2005dc);
        for (int i = 0; i < 27; i++)
        {
            sBattleOverlaySetup entry;
            entry.m0_name = readSaturnString(readSaturnEA(battleOverlaySetupEA + 0x0));
            entry.m4_prg = readSaturnString(readSaturnEA(battleOverlaySetupEA + 0x4));
            entry.m8_fnt = readSaturnString(readSaturnEA(battleOverlaySetupEA + 0x8));
            entry.mC_numSubBattles = readSaturnU32(battleOverlaySetupEA + 0xC);
            sSaturnPtr subBattlesEA = readSaturnEA(battleOverlaySetupEA + 0x10);
            for (int j = 0; j < entry.mC_numSubBattles; j++)
            {
                entry.m10_subBattles.push_back(readSaturnString(readSaturnEA(subBattlesEA)));
                subBattlesEA += 4;
            }
            battleOverlaySetup.push_back(entry);

            battleOverlaySetupEA += 0x14;
        }
    }

    //battleActivationList
    {
        sSaturnPtr battleActivationListEA = getSaturnPtr(0x2002bc);
        for (int i = 0; i < 27; i++)
        {
            battleActivationList.push_back(readSaturnS8(battleActivationListEA + i));
        }
    }
}
