#pragma once

class sCommonOverlay_data : public sSaturnMemoryFile
{
public:
    void init();

    struct sDragonLevelStat
    {
        std::array<s8, 3> m0;
        std::array<s8, 3> m3;
        std::array<s8, 3> m6;
        std::array<s8, 3> m9;
        std::array<s8, 3> mC;
        std::array<s8, 3> mF;
        std::array<s8, 3> m12;
        std::array<s8, 3> m15;
        std::array<s8, 3> m18;
        std::array<s8, 3> m1B;
    };
    std::vector<sDragonLevelStat> dragonLevelStats;

    struct sBattleOverlaySetup
    {
        std::string m0_name;
        std::string m4_prg;
        std::string m8_fnt;
        u32 mC_numSubBattles;
        std::vector<std::string> m10_subBattles;
    };
    std::vector<sBattleOverlaySetup> battleOverlaySetup;
    std::vector<s8> battleActivationList;
};

void initCommonFile();

extern sCommonOverlay_data gCommonFile;
