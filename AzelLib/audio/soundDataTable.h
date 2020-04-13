#pragma once


struct sSequenceConfig
{
    sSaturnPtr m0;
    sSaturnPtr m4_soundConfigs;
    s16 m8;
    s16 mA;
    u8 mC;
};

void soundDataTableInit();
extern std::vector<sSequenceConfig> SoundDataTable;
