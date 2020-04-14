#pragma once


struct sSequenceConfig
{
    sSaturnPtr m0;
    sSaturnPtr m4_soundConfigs;
    s16 m8_areaMapIndex;
    s16 mA;
    u8 mC_numMapEntries;
    u8 mD_playerSoundTypes;
};

void soundDataTableInit();
extern std::vector<sSequenceConfig> SoundDataTable;
