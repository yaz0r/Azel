#pragma once

struct sLaserData
{
    s16 m4;
    s16 m6;
    s16 m8;
    s16 mA;
    s16 mC;
    s16 mE;
    s16 m10;
    s16 m12;
    s16 m14;
    std::vector<fixedPoint> m18_vertices;
    std::vector<quadColor> m1C_colors;
    s32 m20_numLaserNodes;
    // size?
};

struct BTL_GenericData : public sSaturnMemoryFile
{
    static void staticInit();

    // Colors used for gun shots
    static const std::vector<quadColor> m60AE424;
    static const std::vector<quadColor> m60AE42C;
    static const std::vector<quadColor> m60AE434;
    static const std::vector<quadColor> m60AE43C;

    // laser data
    static sLaserData mLaserData;

private:
    BTL_GenericData();
};

extern struct BTL_GenericData* g_BTL_GenericData;

