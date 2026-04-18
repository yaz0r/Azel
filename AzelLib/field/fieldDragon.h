#pragma once

struct s_dragonTaskWorkArea_1F0
{
    s32 m_0;
    s32 m_4;
    u32 m_8;
    s32 m_C;
    s16 m_E;
    s16 m_10;
};

struct s_dragonTaskWorkArea_48
{
    sMatrix4x3 m0_matrix; //0

    u32 m30; //30
    u32 m34; //34
    u32 m38; //38
    u8 m3C;
};

struct s_cameraScript;
struct s_cutsceneData;
struct s_cutsceneTask;
struct s_riderAnimTask;
struct s_scriptData3;

struct s_RGB8
{
    s8 m0;
    s8 m1;
    s8 m2;

    u32 toU32()
    {
        return ((m2 & 0xFF) << 16) | ((m1 & 0xFF) << 8) | (m0 & 0xFF);
    }

    static s_RGB8 fromVector(const sVec3_FP& inputColor)
    {
        s_RGB8 temp;
        temp.m0 = inputColor[0].getInteger();
        temp.m1 = inputColor[1].getInteger();
        temp.m2 = inputColor[2].getInteger();
        return temp;
    }
};

s_RGB8 readSaturnRGB8(const sSaturnPtr& ptr);

struct s_dragonTaskWorkArea : s_workAreaTemplateWithArg<s_dragonTaskWorkArea, s32>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &s_dragonTaskWorkArea::Init, &s_dragonTaskWorkArea::Update, &s_dragonTaskWorkArea::Draw, &s_dragonTaskWorkArea::Delete};
        return &taskDefinition;
    }

    static void Init(s_dragonTaskWorkArea*, s32 arg);
    static void Update(s_dragonTaskWorkArea*);
    static void Draw(s_dragonTaskWorkArea*);
    static void Delete(s_dragonTaskWorkArea*)
    {
        PDS_unimplemented("");
    }

    s_memoryAreaOutput m0;

    sVec3_FP m8_pos; // 8
    sVec3_FP m14_oldPos; // 14
    sVec3_FP m20_angle; // 20-24-28
    fixedPoint m2C_savedPitch;
    fixedPoint m30; // saved yaw — updated each frame, used as neutral yaw for roll banking
    fixedPoint m34_savedRoll;
    fixedPoint m38_distanceAccum;
    sVec3_FP m3C_targetAngles; // smoothing targets for pitch[0], yaw[1], roll[2]

    s_dragonTaskWorkArea_48 m48;

    sMatrix4x3 m88_matrix; // 88
    p_workArea mB8_lightWingEffect;
    sVec3_FP* mBC;

    fixedPoint mC0_lightRotationAroundDragon;
    fixedPoint mC4;
    s_RGB8 mC8_normalLightColor;
    s_RGB8 mCB_falloffColor0;
    s_RGB8 mCE_falloffColor1;
    s_RGB8 mD1_falloffColor2;
    s_RGB8 mD4;
    u8 mD7_pad;
    sVec3_FP mD8_sparklePosition; // D8 — light sparkle position in camera space
    fixedPoint mE4_sparkleScale; // E4 — light sparkle intensity scale
    s_RGB8 m_E8_specialColor;
    u8 m_EB_useSpecialColor;
    u8 m_EC;

    void(*mF0)(s_dragonTaskWorkArea*); //F0
    void(*mF4)(s_dragonTaskWorkArea*);
    u32 mF8_Flags; // F8 Flags 0x400 : collisions enabled
    u32 mFC; // FC
    u32 m100_previousDragonType;
    u32 m104_dragonScriptStatus;
    s32 m108;
    sVec3_FP m10C_hotSpot2;
    sVec3_FP m118_hotSpot3;
    sVec3_FP m124_hotSpot4;
    fixedPoint m130_minX;
    fixedPoint m134_minY;
    fixedPoint m138_minZ;
    fixedPoint m13C_maxX;
    fixedPoint m140_maxY;
    fixedPoint m144_maxZ;
    fixedPoint m148_pitchMin;
    fixedPoint m14C_pitchMax;

    u32 m150;
    fixedPoint m154_dragonSpeed;
    fixedPoint m158;
    s32 m15C_dragonSpeedIncrement;

    sVec3_FP m160_deltaTranslation;
    sVec3_FP m16C_deltaRotation;
    fixedPoint m178_turnRate;
    fixedPoint m17C;
    fixedPoint m180;
    fixedPoint m184_animRate;

    sVec3_FP m188;

    sVec3_FP m194;

    sVec3_FP m1A0;
    sVec3_FP m1AC;

    u32 m1B8;
    u32 m1BC;

    u32 m_1C4;

    fixedPoint m1CC_fieldOfView;
    s_cameraScript* m1D0_cameraScript;
    s_cutsceneData* m1D4_cutsceneData;
    s_cutsceneTask* m1D8_cutscene;
    s_riderAnimTask* m1DC_ridersAnimation[2];
    s_scriptData3* m1E4_cutsceneKeyFrame;
    s16 m1E8_cameraScriptDelay;
    s16 m1EA;
    u32 m1EC;
    s16 m1EE;
    s_dragonTaskWorkArea_1F0 m1F0;
    sVec2_S16 m200_LCSCursorScreenSpaceCoordinates;
    fixedPoint m208;
    fixedPoint m20C;
    u32 m210;
    u32 m214;
    fixedPoint m21C_DragonSpeedValues[5];
    u32 m228;
    u32 m22C;
    fixedPoint m230;
    u8 m234;
    s8 m235_dragonSpeedIndex;

    u8 m237;
    u8 m238;
    u8 m239;

    u8 m23A_dragonAnimation;
    u8 m23B;
    u8 m23C;
    u8 m244;
    s8 m245_previousAnalogX;
    s8 m246_previousAnalogY;
    u8 m247;
    u8 m248;
    u8 m249_noCollisionAndHideDragon;
    u8 m24A_runningCameraScript; // has camera script
    u32 m250;
    s32 m254;
    u32 m258;
    u8 m25B;
    u8 m25C;
    s8 m25D;
    s8 m25E;

    //size 0x260?
};

void dragonScriptMovement(s_dragonTaskWorkArea* pTypedWorkArea);
