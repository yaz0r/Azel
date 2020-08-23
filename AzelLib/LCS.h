#pragma once

struct sLCSTaskDrawSub5Sub1_Data1
{
    s16 m2;
    s16 m4;
    s16 m6_CMDSRCA;
    s16 m8;
    s16 mA_CMDCOLR;
    s32 mC_spriteWidth;
    s32 m10_spriteHeight;
    s32 m14_offsetX;
    s32 m18_offsetY;
};

extern std::vector<sLCSTaskDrawSub5Sub1_Data1> LCSTaskDrawSub5Sub1_Data1;


struct s_LCSTask_14
{
    struct sLCSTarget* m0;
    s_LCSTask_14* m4_next;
};

struct sLCSTarget
{
    enum flags
    {
        e_locationIsWorld = 0x100,
        e_rotationIsInWorld = 0x200,
    };
    s_workArea* m0;
    void (*m4_callback)(p_workArea, sLCSTarget*);
    const sVec3_FP* m8_LCSWorldCoordinates;
    const sVec3_FP* mC_optionalRotation;
    s16 m10_flags;
    s16 m12;
    eItems m14_receivedItemId;
    s8 m16_receivedItemQuantity;
    s8 m17;
    s8 m18_diableFlags; // 1: disabled, 4: not visible/facing away?
    s8 m19;
    s8 m1A;
    s8 m1B;
    s32 m1C;
    s_workArea* m20;
    sVec3_FP m24_worldspaceCoordinates;
    sVec2_S16 m30_screenspaceCoordinates;
    // size 34
};

struct s_LCSTask340SubSub : public s_workAreaTemplate<s_LCSTask340SubSub>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { nullptr, &s_LCSTask340SubSub::Update, nullptr, &s_LCSTask340SubSub::Delete};
        return &taskDefinition;
    }

    static void Update(s_LCSTask340SubSub*)
    {
        PDS_unimplemented("s_LCSTask340SubSub::Update");
    }

    static void Delete(s_LCSTask340SubSub*)
    {
        PDS_unimplemented("s_LCSTask340SubSub::Delete");
    }

    s32 m0;
    s32 m4;
};

struct sLaserArgs;

typedef std::array<quadColor, 8> s_LCSTask_gradientData; // size 0x40

struct s_LCSTask340Sub : public s_workAreaTemplateWithArg<s_LCSTask340Sub, sLaserArgs*>
{
    struct s_LCSTask340Sub_m58
    {
        sSaturnPtr m0;
        s16 m4;
        s8 m6;
        s8 m7;
    };

    static const std::array<TypedTaskDefinition, 4> constructionTable;

    static void Init0(s_LCSTask340Sub*, sLaserArgs*);
    static void Laser0Init(s_LCSTask340Sub*);
    static void Laser0Update(s_LCSTask340Sub*);
    static void Laser0Draw(s_LCSTask340Sub*);

    static void Init1(s_LCSTask340Sub*, sLaserArgs*);
    static void Laser1Init(s_LCSTask340Sub*);
    static void Laser1Update(s_LCSTask340Sub*);
    void Init1Sub1Sub0();
    static void Laser1Draw(s_LCSTask340Sub*);
    void Laser1DrawSub0(std::array<sVec3_FP, 8>& r5, s32 r6, sSaturnPtr r7, s_LCSTask_gradientData* arg0);

    static void Init2(s_LCSTask340Sub*, sLaserArgs*);
    static void Laser2Init(s_LCSTask340Sub*);
    static void Laser2Update(s_LCSTask340Sub*);
    static void Laser2Draw(s_LCSTask340Sub*);

    static void Init3(s_LCSTask340Sub*, sLaserArgs*);

    static void Laser3Init(s_LCSTask340Sub*);
    static void Laser3Update(s_LCSTask340Sub*);
    static void Laser3Draw(s_LCSTask340Sub*);
    static void Init3Sub3(s_LCSTask340Sub_m58* r4, s32 s5, sSaturnPtr r6);

    static void Update0(s_LCSTask340Sub*);

    static void Update3(s_LCSTask340Sub*);

    static void Delete3(s_LCSTask340Sub*);

    static void Draw(s_LCSTask340Sub* pThis)
    {
        pThis->m30_laserDraw(pThis);
    }

    static void fieldScriptTaskUpdateSub2Sub1Sub1Sub1Sub2Sub(s_LCSTask340Sub* pThis)
    {
        switch (pThis->m15C)
        {
        case 0:
            pThis->m15C++;
            break;
        case 1:
            pThis->getTask()->markFinished();
            break;
        }
    }

    s_memoryAreaOutput m0;
    p_workArea m8;
    const sVec3_FP* mC;
    s32 m10;
    const sVec3_FP* m14;
    sVec3_FP* m18;
    sLCSTarget* m1C;
    struct s_LCSTask340* m20;
    eItems m24_receivedItemId;
    s8 m26_receivedItemQuantity;
    s8 m27;
    void (*m28_laserInit)(s_LCSTask340Sub*);
    void (*m2C_laserUpdate)(s_LCSTask340Sub*);
    void (*m30_laserDraw)(s_LCSTask340Sub*);
    fixedPoint m34;
    fixedPoint m38;
    fixedPoint m3C;
    s32 m40;
    s_LCSTask340Sub_m58 m58;
    sVec3_FP m60;
    std::array<sVec3_FP, 0x10> m6C;
    sVec3_FP m144;
    s32 m154;
    s32 m158;
    s8 m15C;

    //size 160
};

struct s_LCSTask340
{
    s32 m0_index;
    s_LCSTask340* m4_next;
    sLCSTarget* m8;
    s_LCSTask340Sub* mC;
    s_LCSTask340Sub* m10;
    s32 m14;
    // size 0x18
};

struct s_LCSTask : public s_workAreaTemplate<s_LCSTask>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_LCSTask::Init, NULL, NULL, NULL};
        return &taskDefinition;
    }

    static void Init(s_LCSTask*);

    s_memoryAreaOutput m0;
    u32 m8;
    u32 mC;
    u32 m10;
    std::array<s_LCSTask_14, 0x100> m14;
    fixedPoint m814_LCSTargetMaxDistance;
    sLCSTarget* m818_curr;
    sLCSTarget* m81C_curs;
    sLCSTarget* m820_Fewl;
    s32 m824;
    s_LCSTask340* m828_activeStart;
    s_LCSTask340* m82C_activeEnd;
    s_LCSTask340* m830_unactiveStart;
    s_LCSTask340* m834_unactiveEnd;
    s_LCSTask340* m838_Next;
    s8 m83C_time0;
    s8 m83D_time1;
    s8 m83E_LaserNum;
    s8 m83F_activeLaserCount;
    std::array<s_LCSTask340, 0x10> m840;
    s_LCSTask_gradientData* m9C0;
    u32 m9C4;
    s_LCSTask_14* m9C8;
    s_LCSTask_14* m9CC;
    s32 m9D0_mode;

    enum : s8 {
        LCSPhase_0_init = 0,
        LCSPhase_1 = 1,
        LCSPhase_2_targeting = 2,
        LCSPhase_3 = 3,
        LCSPhase_4_executing = 4,
    } m9DA_LCSPhase;
    // size 0x9DC
};

struct sLCSSelectedSub : public s_workAreaTemplate<sLCSSelectedSub>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { nullptr, &sLCSSelectedSub::Update, &sLCSSelectedSub::Draw, nullptr};
        return &taskDefinition;
    }

    static void Update(sLCSSelectedSub*);
    static void Draw(sLCSSelectedSub*);

    s_memoryAreaOutput m0;
    sVec2_S16* m8;
    s32 mC_numFrames;
    // size 0x10
};

struct sLCSSelected : public s_workAreaTemplate<sLCSSelected>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { nullptr, &sLCSSelected::Update, &sLCSSelected::Draw, nullptr};
        return &taskDefinition;
    }

    static void Update(sLCSSelected*);
    void UpdateSub0(sVec2_S16* r5);

    static void Draw(sLCSSelected*);
    void DrawSub0(sLCSTaskDrawSub5Sub1_Data1* r5, const sInterpolator_FP& r6);
    void DrawSub1(s8 r5);

    s_memoryAreaOutput m0;
    sLCSTarget* m8;
    sInterpolator_FP mC_verticalInterpolator;
    sInterpolator_FP m18_secondarySpriteInterpolator;
    sVec2_S16* m24_screenLocation;
    s32 m28;
    s8 m2C;
    s8 m2D;
    s8 m2E;
    //size 0x30
};

void createLCSTarget(sLCSTarget* r4, s_workArea* r5, void (*r6)(p_workArea, sLCSTarget*), const sVec3_FP* r7, const sVec3_FP* arg0, s16 flags, s16 argA, eItems receivedItemId, s32 receivedItemQuantity, s32 arg14);
void updateLCSTarget(sLCSTarget* r14);
void LCSTaskDrawSub();
void allocateLCSEntry(struct s_visibilityGridWorkArea* r4, sProcessed3dModel* r5, fixedPoint r6);
void fieldScriptTaskUpdateSub2Sub1();
void dragonFieldTaskUpdateSub2(u32 r4);
void createFieldOverlaySubTask2(s_workArea* pWorkArea);
