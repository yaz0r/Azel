#pragma once

struct s_LCSTask_828
{
    s_LCSTask_828* m4_next;
};

struct sLCSTarget
{
    enum flags
    {
        e_moveWithParent = 0x100,
        e_200 = 0x200,
    };
    s_workArea* m0;
    void* m4_callback;
    const sVec3_FP* m8_parentWorldCoordinates;
    const sVec3_FP* mC;
    s16 m10_flags;
    s16 m12;
    s16 m14;
    s8 m16;
    s8 m17;
    s8 m18;
    s8 m19;
    s8 m1A;
    s8 m1B;
    s32 m1C;
    s_workArea* m20;
    sVec3_FP m24_worldspaceCoordinates;
    sVec2_S16 m30_screenspaceCoordinates;
    // size 34
};

struct s_LCSTask_14
{
    sLCSTarget* m0;
    s32 m4;
};

struct s_LCSTask : public s_workArea
{
    s_memoryAreaOutput m0;
    u32 m8;
    u32 mC;
    u32 m10;
    std::array<s_LCSTask_14, 0x100> m14;
    fixedPoint m814_LCSTargetMaxHeight;
    sLCSTarget* m818;
    sLCSTarget* m81C_currentLCSTarget;
    s32 m820;
    s32 m824;
    s_LCSTask_828* m828;
    s32 m830;
    s32 m834;
    s32 m838;
    s8 m83C;
    s8 m83D;
    s8 m83E;
    s8 m83F;
    void* m9C0;
    u32 m9C4;
    u32 m9C8;
    u32 m9CC;
    s32 m9D0;

    enum : s8 {
        LCSPhase_0_init = 0,
        LCSPhase_2_targeting = 2,
        LCSPhase_3 = 3,
        LCSPhase_4_executing = 4,
    } m9DA_LCSPhase;
    // size 0x9DC
};

void createLCSTarget(sLCSTarget* r4, s_workArea* r5, void* r6, const sVec3_FP* r7, const sVec3_FP* arg0, s16 flags, s16 argA, s16 argE, s32 arg10, s32 arg14);
void updateLCSTarget(sLCSTarget* r14);
void LCSTaskDrawSub();
void allocateLCSEntry(s_visibilityGridWorkArea* r4, u8* r5, u32 r6);
void fieldScriptTaskUpdateSub2Sub1();
void dragonFieldTaskUpdateSub2(u32 r4);
void createFieldOverlaySubTask2(s_workArea* pWorkArea);
