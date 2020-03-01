#pragma once

struct s_battleEngineSub
{
    s8 m0_isActive;
    struct s_battleDragon_8C* m4;
    s32 m8_distanceToDragonSquare;
    // size C?
};

struct s_battleEngine_3B4
{
    fixedPoint m0_max;
    s32 m4;
    s32 m8;
    s32 mC;
    fixedPoint m10_value;
    s16 m14;
    s8 m16_combo;
};

enum eBattleModes
{
    m0 = 0,
    m6_dragonMoving = 6,
    m7 = 7,
    m8_playAttackCamera = 8,
    mA = 0xA,
    mB_enemyMovingDragon = 0xB,
    mE_battleIntro = 0xE,
    m10 = 0x10,
};

struct s_battleEngine : public s_workAreaTemplateWithArgWithCopy<s_battleEngine, sSaturnPtr>
{
    sVec3_FP mC_battleCenter;
    sVec3_FP m18;
    sVec3_FP m60;
    sVec3_FP m6C_dragonIntegrateStep;
    sVec3_FP m104_dragonPosition;
    struct {
        u32 m1 : 1;
        u32 m2 : 1;
        u32 m4 : 1;
        u32 m8 : 1;
        u32 m10 : 1;
        u32 m20_battleIntroRunning : 1;
        u32 m40 : 1;
        u32 m80 : 1;
        u32 m100_attackAnimationFinished : 1;
        u32 m200_suppressBattleInputs : 1;
        u32 m800 : 1;
        u32 m1000 : 1;
        u32 m2000 : 1;
        u32 m4000 : 1;
        u32 m8000 : 1;
        u32 m10000 : 1;
        u32 m20000 : 1;
        u32 m80000_hideBattleHUD : 1;
        u32 m100000 : 1;
        u32 m200000 : 1;
        u32 m400000 : 1;
        u32 m1000000_dragonMoving : 1;
        u32 m2000000 : 1;
        u32 m4000000 : 1;
    } m188_flags;
    //u32 m188_flags; // 8: setup lights color for dragon
    s8 m18C_status;
    std::array<s32, 2> m190;
    sVec3_FP m164;
    s16 m184;
    sVec3_FP m1A0;
    sVec3_FP m1AC;
    s32 m1B8_dragonPitch;
    s32 m1BC_dragonYaw;
    s32 m1C8;
    s32 m1CC;
    fixedPoint m1D0;
    fixedPoint m1D4;
    u16 m1E0_quadrantBitStatus; // 3 groups of 4 bits
    int m1E8[2];
    sMatrix4x3 m1F0;
    sVec3_FP m220;
    s8 m22C_dragonCurrentQuadrant; // 1: right, 3: left, (0: bottom, 3: top)?
    s8 m22D_dragonPreviousQuadrant;
    s8 m22E_dragonMoveDirection;
    u8 m22F_battleRadarLockIcon;
    s8 m230;
    sVec3_FP m234;
    sVec3_FP m240;
    sVec3_FP m24C;
    sVec3_FP m258;
    sVec3_FP m264;
    sVec3_FP m270_enemyAltitude;

    struct s_27C
    {
        sVec3_FP m0_computedValue;
        sVec3_FP mC_initialValue;
        sVec3_FP m18_middle;
        sVec3_FP m24_targetValue;
        sVec3_FP m30;
        sVec3_FP m3C;
        sVec3_FP m48;
        s8 m50;
        sVec3_FP m54;
        fixedPoint m60_currentStep;
        fixedPoint m64_stepSize;
        s16 m68_rate;
        //size: 0x68
    };

    s_27C m27C_dragonMovementInterpolator1;
    s_27C m2E8_dragonMovementInterpolator2;
    sVec2_FP m354_dragonAltitudeMinMax;
    sVec2_FP m35C_cameraAltitudeMinMax;
    std::array<s32, 4> m364_perQuadrantDragonAltitude;
    std::array<s32, 4> m374_perQuadrantEnemyAltitude;
    s16 m384_battleModeDelay;
    s16 m386;
    u16 m388;
    eBattleModes m38C_battleMode;
    s8 m38D_battleSubMode;
    s8 m38E;
    std::array<s16, 3> m390;
    std::array<s16, 2> m398;
    u8 m3A0_LaserType; // 0: single, else multi
    s16 m3A2;
    std::array<s16, 2> m3A4_prelockMode;
    sSaturnPtr m3A8_overlayBattledata;
    sSaturnPtr m3AC;
    s8 m3B0_subBattleId;
    s8 m3B1;
    s8 m3B2_numBattleFormationRunning;
    s_battleEngine_3B4 m3B4;
    struct s_battleEngineSub0* m3CC;
    p_workArea m3D0;
    sVec3_FP* m3D4;
    sVec3_FP* m3D8;
    sVec3_FP m3DC;
    sVec3_FP m3E8;
    sVec3_FP m3F4;
    sVec3_FP m418;
    sVec3_FP m424;
    s16 m430;
    u8 m432;
    u8 m433_attackCameraIndex;
    s32 m434;
    s32 m43C;
    fixedPoint m440_battleDirectionAngle;
    std::array<fixedPoint, 4> m45C_perQuadrantDragonSpeed;
    sVec2_FP m46C_dragon2dSpeed;
    sVec3_FP m474;
    std::array<s16, 2> m480;
    std::array<std::array<s16, 2>, 5> m484;
    s16 m498;
    std::array<s_battleEngineSub, 0x80> m49C;
    // size: 0xaa8
};

s32 BattleEngineSub0_UpdateSub0();
s32 s_battleDragon_InitSub0();

void battleEngine_InitSub6(sVec3_FP* pData);
void battleEngine_InitSub7(sVec3_FP* pData);
void battleEngine_InitSub8();
void battleEngine_UpdateSub7Sub3();

void battleEngine_SetBattleMode(eBattleModes param);

void battleEngine_FlagQuadrantForSafety(u8 quadrant);
void battleEngine_FlagQuadrantForDanger(u8 quadrant);
void battleEngine_FlagQuadrantForAttack(u8 quadrant);

p_workArea createBattleEngineTask(p_workArea, sSaturnPtr battleData);
