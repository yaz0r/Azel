#pragma once

struct s_battleEngineSub
{
    s8 m0;
    s32 m4;
    s32 m8;
    // size C?
};

struct s_battleEngine : public s_workAreaTemplateWithArgWithCopy<s_battleEngine, sSaturnPtr>
{
    sVec3_FP mC;
    sVec3_FP m18;
    sVec3_FP m6C;
    sVec3_FP m104_dragonStartPosition;
    u32 m188_flags;
    s8 m18C_status;
    std::array<s32, 2> m190;
    sVec3_FP m164;
    s16 m184;
    sVec3_FP m1A0;
    sVec3_FP m1AC;
    s32 m1B8;
    s32 m1BC;
    s32 m1C8;
    s32 m1CC;
    fixedPoint m1D0;
    fixedPoint m1D4;
    int m1E8[2];
    sMatrix4x3 m1F0;
    sVec3_FP m220;
    s8 m22C;
    s8 m22D;
    s8 m230;
    sVec3_FP m234;
    sVec3_FP m240;
    sVec3_FP m24C;
    sVec3_FP m258;
    sVec3_FP m264;
    sVec3_FP m270;
    s8 m2CC;
    std::array<s32, 4> m354;
    std::array<s32, 4> m364;
    std::array<s32, 4> m374;
    s16 m384_battleIntroDelay;
    s16 m386;
    u16 m388;
    s8 m38C_battleIntroType;
    s8 m38D_battleIntroStatus;
    std::array<s16, 3> m390;
    std::array<s16, 2> m398;
    s16 m3A2;
    std::array<s16, 2> m3A4;
    sSaturnPtr m3A8_overlayBattledata;
    sSaturnPtr m3AC;
    s8 m3B0_subBattleId;
    s8 m3B1;
    s8 m3B2;
    s32 m3B4;
    s32 m3B8;
    s32 m3BC;
    s8 m3CA;
    struct s_battleEngineSub0* m3CC;
    p_workArea m3D0;
    sVec3_FP* m3D4;
    sVec3_FP* m3D8;
    sVec3_FP m3E8;
    s32 m434;
    s32 m43C;
    fixedPoint m440;
    std::array<fixedPoint, 4> m45C;
    fixedPoint m46C;
    fixedPoint m470;
    sVec3_FP m474;
    std::array<s16, 2> m480;
    std::array<std::array<s16, 2>, 5> m484;
    s16 m498;
    std::array<s_battleEngineSub, 0x80> m49C;
    // size: 0xaa8
};

s32 BattleEngineSub0_UpdateSub0();
s32 s_battleDragon_InitSub0();

p_workArea createBattleEngineTask(p_workArea, sSaturnPtr battleData);
