#pragma once

#include <array>

extern const u8 bitMasks[];
extern const u8 reverseBitMasks[];

struct sVec3_FP
{
    fixedPoint m_value[3];

    void zero()
    {
        m_value[0] = 0;
        m_value[1] = 0;
        m_value[2] = 0;
    }

    fixedPoint operator[](int i) const
    {
        return m_value[i];
    }

    fixedPoint& operator[](int i)
    {
        return m_value[i];
    }

    sVec3_FP operator-(sVec3_FP otherVec) const
    {
        sVec3_FP result;
        result[0] = m_value[0] - otherVec[0];
        result[1] = m_value[1] - otherVec[1];
        result[2] = m_value[2] - otherVec[2];

        return result;
    }

    sVec3_FP& operator+=(const sVec3_FP otherVec)
    {
        m_value[0] += otherVec[0];
        m_value[1] += otherVec[1];
        m_value[2] += otherVec[2];

        return *this;
    }

    sVec3_FP operator + (const sVec3_FP otherVec) const
    {
        sVec3_FP newValue = *this;
        newValue += otherVec;
        return newValue;
    }
};

fixedPoint dot3_FP(sVec3_FP* r4, sVec3_FP* r5);

u8* getSaturnPtr(sSaturnPtr& ptr);
s8 readSaturnS8(sSaturnPtr& ptr);
u8 readSaturnU8(sSaturnPtr& ptr);
s16 readSaturnS16(sSaturnPtr& ptr);
u16 readSaturnU16(sSaturnPtr& ptr);
s32 readSaturnS32(sSaturnPtr& ptr);
u32 readSaturnU32(sSaturnPtr& ptr);
sVec3_FP readSaturnVec3(sSaturnPtr& ptr);
sSaturnPtr readSaturnEA(sSaturnPtr& ptr);

struct sMatrix4x3
{
    std::array<fixedPoint, 4*3> matrix;
};

enum e_dragonLevel : unsigned char {
    DR_LEVEL_0_BASIC_WING = 0,
    DR_LEVEL_1_VALIANT_WING,
    DR_LEVEL_2_STRIPE_WING,
    DR_LEVEL_3_PANZER_WING,
    DR_LEVEL_4_EYE_WING,
    DR_LEVEL_5_ARM_WING,
    DR_LEVEL_6_LIGHT_WING,
    DR_LEVEL_7_SOLO_WING,
    DR_LEVEL_8_FLOATER,

    DR_LEVEL_MAX
};

enum e_dragonArchetype : unsigned char {
    DR_ARCHETYPE_0_NORMAL = 0,
    DR_ARCHETYPE_1_DEFENSE,
    DR_ARCHETYPE_2_ATTACK,
    DR_ARCHETYPE_3_AGILITY,
    DR_ARCHETYPE_4_SPIRITUAL,

    DR_ARCHETYPE_MAX
};

enum e_dragonAnim : unsigned char {
    DR_ANIM_0_BASIC_WING = 0,
    DR_ANIM_1_VALIANT_WING,
    DR_ANIM_2_STRIPE_WING,
    DR_ANIM_3_PANZER_WING,
    DR_ANIM_4_EYE_WING,
    DR_ANIM_5_ARM_WING,
    DR_ANIM_6_LIGHT_WING,
    DR_ANIM_7_SOLO_WING,
    DR_ANIM_8_FLOATER,
    DR_ANIM_9_PUP,

    DR_ANIM_MAX
};

struct s_gameStats
{
    u8 m0_level; // 0
    e_dragonLevel m1_dragonLevel; // 1
    u8 m2_rider1; // 2
    u8 m3_rider2; // 3

    u16 currentHP; // 0x10
    u16 classMaxHP; // 0x12
    u16 currentBP; // 0x14
    u16 classMaxBP; // 0x16
    u16 m18; // 0x18
    s16 dragonCursorX; //1A
    s16 dragonCursorY; //1C
    u32 XP; //20
    u32 dyne; // 38
    u32 frameCounter; //3C

    u32 m70; //70

    char playerName[17];
    char dragonName[17];
    e_dragonArchetype dragonArchetype;//B6

    u16 maxHP; // B8
    u16 maxBP; // BA;
    u16 dragonDef; // BC
    u16 dragonAtt; // BE
    u16 dragonAgl; // C0
    u16 dragonSpr; // C2
};

struct sBitfieldMapEntry
{
    u32 m_bitOffset;
    u32 m_bitSize;
    const char* m_name;
};

struct s_mainGameState
{
private:
    u8 bitField[630];
    std::vector<sBitfieldMapEntry> m_bitFieldMap;

public:
    s_gameStats gameStats;

    void reset()
    {
        m_bitFieldMap.clear();
        memset(bitField, 0, sizeof(bitField));
        memset(&gameStats, 0, sizeof(gameStats));
    }

    void setPackedBits(u32 firstBitOffset, u32 numBits, u32 value)
    {
        void setPackedBits(u8* bitField, u32 firstBitOffset, u32 numBits, u32 value);

        setPackedBits(bitField, firstBitOffset, numBits, value);
    }

    void setBit(u32 bitIndex)
    {
        bitField[bitIndex / 8] |= 0x80 >> (bitIndex % 8);
    }

    void setBit(u32 byteIndex, u32 bitIndex)
    {
        bitField[byteIndex] |= 0x80 >> bitIndex;
    }

    void clearBit(u32 bitIndex)
    {
        bitField[bitIndex / 8] &= ~(0x80 >> (bitIndex % 8));
    }

    bool getBit(u32 bitIndex)
    {
        if (bitField[bitIndex / 8] &= 0x80 >> (bitIndex % 8))
            return true;
        return false;
    }

    bool getBit(u32 byteIndex, u32 bitIndex)
    {
        if (bitField[byteIndex] &= 0x80 >> (bitIndex))
            return true;
        return false;
    }
};

extern s_mainGameState mainGameState;

struct s_dragonPerLevelMaxHPBP
{
    u16 maxHP;
    u16 maxBP;
};

extern const s_dragonPerLevelMaxHPBP dragonPerLevelMaxHPBP[DR_LEVEL_MAX];

extern const u32 longBitMask[];

extern const u32 fieldEnabledTable[23];

struct s_fieldDefinition
{
    const char* m_name;
    const char* m_prg;
    const char* m_fnt;
    u32 m_numSubFields;
    const char** m_subFields;
};

extern const s_fieldDefinition fieldDefinitions[];

struct s_MCB_CGB {
    const char* MCB;
    const char* CGB;
};

struct s_dragonFileConfig {
    s_MCB_CGB m_base;
    s_MCB_CGB m_M;
    s_MCB_CGB m_C;
};

extern const s_dragonFileConfig dragonFilenameTable[DR_LEVEL_MAX];

struct s_RiderDefinitionSub
{
    sSaturnPtr m_ptr;
    u32 m_count;
};

struct s_RiderDefinition
{
    const char* m_MCBName; //0
    const char* m_CGBName; //4
    u16 m_flags; //8
    u16 m_flags2; //A
    const s_RiderDefinitionSub* m_pExtraData; //C
};

struct sDragonData3Sub
{
    u16 m_m0[4];
    s_RiderDefinitionSub* m_m8;
};

struct sDragonData3
{
    u32 m_m0;
    u32 m_m4;
    sDragonData3Sub m_m8[7];
};

extern const sDragonData3 dragonData3[DR_LEVEL_MAX];

struct s_dragonData2
{
    const u16* m_data;
    u32 m_count;
};

extern const s_dragonData2 dragonAnimOffsets[DR_LEVEL_MAX];

struct sAnimTrackStatus
{
    s32 currentStep; //0
    s32 delay; //4
    s32 value; //8
};

struct sPoseData
{
    sVec3_FP m_translation; // 0
    sVec3_FP m_rotation; // C
    sVec3_FP m_scale; // 18
    sVec3_FP halfTranslation; // 24
    sVec3_FP halfRotation; // 30
    sAnimTrackStatus m48[9];
    //size B4
};

struct sPoseDataInterpolation
{
    sVec3_FP m0_translation; // 0
    sVec3_FP mC_rotation; // C
    sVec3_FP m18_scale; // 18
    sVec3_FP m24_halfTranslation; // 24
    sVec3_FP m30_halfRotation; // 30
    sVec3_FP m3C_halfScale;
    //size 0x48
};

struct s_3dModel
{
    s_workArea* m0_pOwnerTask; //0
    u8* m4_pModelFile; //4

    u16 m8; //8
    u16 mA; //A
    u16 mC_modelIndexOffset; //C

    u16 m10_currentAnimationFrame; //10
    u16 m12_numBones; //12
    u16 m14; //14
    u16 m16; //16

    void (*m18_drawFunction)(s_3dModel*); // 18
    void (*m1C_addToDisplayListFunction)(u8*, u32); // 1C
    void (*m20_positionUpdateFunction)(s_3dModel*); // 20
    void (*m24_rotationUpdateFunction)(s_3dModel*); // 24
    void (*m28_scaleUpdateFunction)(s_3dModel*); // 28
    sPoseData* m2C_poseData; //2C

    u8* m30_pCurrentAnimation; //30

    u8* m34_pDefaultPose; //34
    u32 m38; //38

    sMatrix4x3* m3C_boneMatrices; //3C

    const s_RiderDefinitionSub* m40; //40
    sVec3_FP** m44; //44 one entry per bone, and each bone get an array of vertex (hot-spots on the model?)
    std::vector<sPoseDataInterpolation> m48_poseDataInterpolation; //48

    u16 m4C_interpolationStep; //4C
    u16 m4E_interpolationLength; //4E
};

struct sDragonAnimDataSubRanges
{
    sVec3_FP m_vec0;
    sVec3_FP m_vecC;
    sVec3_FP m_max;
    sVec3_FP m_min;
};

struct s_runtimeAnimData
{
    sVec3_FP m0_root; // 0
    sVec3_FP m_vec_C; // C
    sVec3_FP m_vec_18; // 18
    sVec3_FP m24_rootDelta; // 24

    sDragonAnimDataSubRanges m_factors; // 30 //6-8 min 9-11 max
    const struct sDragonAnimDataSub* dataSource; // 60
};

struct s3DModelAnimData
{
    const struct sDragonAnimData* m0_animData; // 0
    sMatrix4x3* boneMatrices; // 4;
    s_runtimeAnimData* m8_runtimeAnimData; // 8 one per countAnims
    u8 countAnims; // C
    u8 count0; // D
    u8 count1; // E
    u8 count2; // F
};

struct s_dragonState : public s_workArea
{
    u8* m0_pDragonModelRawData; //0
    u32 mC_dragonType;//C F can contain some other info
    s16 m10_cursorX;//10
    s16 m12_cursorY;//12
    u32 m14_modelIndex;//14
    u32 m18_shadowModelIndex;//18
    u32 m1C_dragonArchetype; //1C
    const u16* m20_dragonAnimOffsets; //20
    u32 m24_dragonAnimCount; //24
    s_3dModel m28_dragon3dModel; //28

    s3DModelAnimData m78_animData; // 78
    u32 m88;//88
};
extern s_dragonState* gDragonState;

extern u8 playerDataMemoryBuffer[0x28000];

struct s_vdp1AllocatorNode
{
    s_vdp1AllocatorNode* m_nextAllocator;
    s_vdp1AllocatorNode* m_nextNode;
};
extern s_vdp1AllocatorNode* vdp1AllocatorHead;

struct s_graphicEngineStatus_40AC
{
    u8 m0_menuId; // 0 what menu to open
    u8 m1_isMenuAllowed; // 1 is menu allowed?
    u8 m2;
    u8 m3;
    u8 m4;
    u8 m5;
    u8 m6;
    u8 m7;
    u8 m8;
    u8 m9;
    u16 fontIndex; // A
    p_workArea mC;
};

struct s_graphicEngineStatus_4514_inputStatus
{
    u8 m0;
    u16 m2;
    u16 m4;
    u16 m6_buttonDown; // direction
    u16 m8_newButtonDown; // button mask
    u16 mA;
    u16 mC_newButtonDown2; // 0x10: up
                 // 0x20: down
                 // 0x40: left
                 // 0x80: right
    u16 mE;
    u16 m10;
    u16 m12;
    u16 m14;
};

struct s_graphicEngineStatus_4514_sub2
{
    u8 m0;
    u32 m4;
    u32 mC;
}; // size should be 40

    

struct s_graphicEngineStatus_4514_sub
{
    s_graphicEngineStatus_4514_inputStatus m0_current; //0
    s_graphicEngineStatus_4514_inputStatus m16_pending; //16
    s_graphicEngineStatus_4514_sub2 m2C;//2C
    // size should be 6C
};

struct s_graphicEngineStatus_4514
{
    s_graphicEngineStatus_4514_sub m0[2]; // one per input port
    u16 mD8[3][16];
    u8 m138[3];
};

struct s_graphicEngineStatus_405C
{
    u16 m0;
    u16 m2;
    u16 m4;
    u16 m6;
    u16 m8;
    u16 mA;
    u16 mC;
    u16 mE;

    fixedPoint m18;
    fixedPoint m1C;
    fixedPoint m20;
    fixedPoint m24;
    fixedPoint m28;
    fixedPoint m2C;

    u16 VDP1_X1; // 3C
    u16 VDP1_Y1; // 3E
    u16 VDP1_X2; // 40
    u16 VDP1_Y2; // 42
    u16 localCoordinatesX; // 44
    u16 localCoordinatesY; // 46
    u32 setClippingCoordinatesEA; // 0x48 ptr in VDP1 Memory
    u32 setLocalCoordinatesEA; // 0x4C ptr in VDP1 Memory

    // size should be 50
};

struct s_graphicEngineStatus_40E4
{
    s_graphicEngineStatus_405C m0;
    sVdp2Controls m50;
    sMenuUnk0 m2B0;
    u8 m300[512];
    u16 m400;
    u16 m402;
};

struct s_graphicEngineStatus_40BC
{
    s16 scrollX; //0
    s16 scrollY; //2
    s16 scrollIncX; // 4
    s16 scrollIncY; // 6
    u16 m8_scrollFrameCount;
}; // size A?

struct s_vdp1Packet
{
    s_vdp1Packet* pNext;
    u16 bucketTypes; // 4
    u16 vdp1EA; //6
};

struct s_vdp1Context
{
    u32 currentVdp1WriteEA; //0
    u32 m4[2]; //4
    u32 mC; //C
    u32 m10; //10
    u32 m14; //14
    u32 m18; //18
    u32 m1C; //1C
    s_vdp1Packet* pCurrentVdp1Packet; //20
    s_vdp1Packet vdp1Packets[1024]; // 24
    // size should be 2024
};

struct s_graphicEngineStatus
{
    u16 m0;
    u8 doubleBufferState;
    u8 m3;
    u8 m4;
    u8 m5;
    u16 m6; // vdp1 write offset
    u32 m8; // vdp1 write EA for user clipping parameters
    u32 mC; // vdp1 write EA of background sprite

    s_vdp1Context vdp1Context[2]; // 14
    s_graphicEngineStatus_405C m405C;

    u32 m406C;

    s32 m4070_farClipDistance;
    s32 m408C;
    u32 m4090;
    s32 m4094;
    
    s_graphicEngineStatus_40E4* m40E4;
    s_graphicEngineStatus_40AC m40AC;
    s_graphicEngineStatus_40BC layersConfig[4];
    s_graphicEngineStatus_4514 m4514;
};

extern s_graphicEngineStatus graphicEngineStatus;

void memcpy_dma(void* src, void* dst, u32 size);

void initDramAllocator(s_workArea* pWorkArea, u8* dest, u32 size, const char** assetList);

fixedPoint getCos(u32 value);
fixedPoint getSin(u32 value);

extern u16 atanTable[2049];
extern u16 resetVdp2StringsData[4106];

void registerModelAndCharacter(u8* model, u8* character);
