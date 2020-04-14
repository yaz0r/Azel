#include "PDS.h"
#include "soundDataTable.h"

std::vector<sSequenceConfig> SoundDataTable;

void soundDataTableInit()
{
    sSaturnPtr dataTableEA = gCommonFile.getSaturnPtr(0x2152d8);

    for (int i=0; i<79; i++)
    {
        sSequenceConfig entry;
        entry.m0 = readSaturnEA(dataTableEA);
        entry.m4_soundConfigs = readSaturnEA(dataTableEA + 4);
        entry.m8_areaMapIndex = readSaturnS16(dataTableEA + 8);
        entry.mA = readSaturnS16(dataTableEA + 0xA);
        entry.mC_numMapEntries = readSaturnU8(dataTableEA + 0xC);
        entry.mD_playerSoundTypes = readSaturnU8(dataTableEA + 0xD);
        dataTableEA += 0x10;
        SoundDataTable.push_back(entry);
    }
}

