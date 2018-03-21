#pragma once

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
};

sVec3_FP readSaturnVec3(sSaturnPtr& ptr);

struct sMatrix4x3
{
    fixedPoint matrix[4 * 3];
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
    u8 level;
    e_dragonLevel dragonLevel;
    u8 rider1;
    u8 rider2;

    u16 currentHP; // 0x10
    u16 classMaxHP; // 0x12
    u16 currentBP; // 0x14
    u16 classMaxBP; // 0x16
    u16 field_18; // 0x18
    s16 dragonCursorX; //1A
    s16 dragonCursorY; //1C

    u32 frameCounter; //3C

    u32 field_70; //70

    char playerName[17];
    char dragonName[17];
    u8 dragonArchetype;//B6

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
    u16 m_field_0[4];
    s_RiderDefinitionSub* m_field_8;
};

struct sDragonData3
{
    u32 m_field_0;
    u32 m_field_4;
    sDragonData3Sub m_field_8[7];
};

extern const sDragonData3 dragonData3[DR_LEVEL_MAX];

struct s_dragonData2
{
    const u16* m_data;
    u32 m_count;
};

extern const s_dragonData2 dragonData2[DR_LEVEL_MAX];

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
    sAnimTrackStatus field_48[9];
};

struct s_3dModel
{
    s_workArea* pOwnerTask; //0
    u8* pModelFile; //4

    u16 field_8; //8
    u16 field_A; //A
    u16 modelIndexOffset; //C

    u16 currentAnimationFrame; //10
    u16 numBones; //12
    u16 field_14; //14
    u16 field_16; //16

    void(*drawFunction)(s_3dModel*); // 18
    void (*addToDisplayListFunction)(u8*, u32); // 1C
    void(*positionUpdateFunction)(s_3dModel*); // 20
    void(*rotationUpdateFunction)(s_3dModel*); // 24
    void(*scaleUpdateFunction)(s_3dModel*); // 28
    sPoseData* poseData; //2C

    u8* pCurrentAnimation; //30

    u8* defaultPose; //34
    u32 field_38; //38

    sMatrix4x3* boneMatrices; //3C

    const s_RiderDefinitionSub* field_40; //40
    sVec3_FP** field_44; //44 one entry per bone, and each bone get an array of vertex (hot-spots on the model?)
    u32 field_48; //48

    u16 field_4C; //4C
    u16 field_4E; //4E
};

struct s_runtimeAnimData
{
    sVec3_FP m_vec_0; // 0 Sometimes used as a matrix
    sVec3_FP m_vec_C; // C
    sVec3_FP m_vec_18; // 18
    sVec3_FP m_vec_24; // 24

    sMatrix4x3 m_matrix; // 30
    const struct sDragonAnimDataSub* dataSource; // 60
};

struct s3DModelAnimData
{
    const struct sDragonAnimData* animData; // 0
    sMatrix4x3* boneMatrices; // 4;
    s_runtimeAnimData* runtimeAnimData; // 8 one per countAnims
    u8 countAnims; // C
    u8 count0; // D
    u8 count1; // E
    u8 count2; // F
};

struct s_dragonState : public s_workArea
{
    u8* pDragonModelRawData; //0
    u32 dragonType;//C
    s16 cursorX;//10
    s16 cursorY;//12
    u32 field_14;//14
    u32 field_18;//18
    u32 dragonArchetype; //1C
    const u16* dragonData2; //20
    u32 dragonData2Count; //24
    s_3dModel dragon3dModel; //28

    s3DModelAnimData animData; // 78
    u32 field_88;//88
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
    u8 field_0; // what menu to open
    u8 field_1;
    u8 field_2;
    u8 field_3;
    u8 field_4;
    u8 field_5;
    u8 field_6;
    u8 field_7;
    u8 field_8;
    u8 field_9;
    u16 field_A;
    p_workArea field_C;
};

struct s_graphicEngineStatus_4514
{
    u16 field_6;
    u16 field_8; // gamepad input bitmask
    u16 field_E;
};

struct s_graphicEngineStatus_405C
{
    u16 field_0;
    u16 field_2;
    u16 field_4;
    u16 field_6;
    u16 field_8;
    u16 field_A;
    u16 field_C;
    u16 field_E;

    fixedPoint field_18;
    fixedPoint field_1C;
    fixedPoint field_20;
    fixedPoint field_24;
    fixedPoint field_28;
    fixedPoint field_2C;

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
    s_graphicEngineStatus_405C field_0;
    sVdp2Controls field_50;
    sMenuUnk0 field_2B0;
    u8 field_300[512];
    u16 field_400;
    u16 field_402;
};

struct s_graphicEngineStatus_40BC
{
    u16 field_0;
    u16 field_2;

    u16 field_8;
};

struct s_graphicEngineStatus_14_2024
{
    u32 field_0;
    u32 field_4;
    u32 field_8;
    u32 field_C;
    u32 field_10;
    u32 field_14;
    u32 field_18;
    u32 field_1C;
    u8* field_20;
    u8 buffer[0x2000];
    // size should be 2024
};

struct s_graphicEngineStatus_14
{
    s_graphicEngineStatus_14_2024 field_0;
    s_graphicEngineStatus_14_2024 field_2024;
};

struct s_graphicEngineStatus
{
    u16 field_0;
    u8 field_2;
    u8 field_3;
    u8 field_4;
    u8 field_5;
    u16 field_6; // vdp1 write offset
    u32 field_8; // vdp1 write EA for user clipping parameters
    u32 field_C; // vdp1 write EA of background sprite

    s_graphicEngineStatus_14 field_14;
    s_graphicEngineStatus_405C field_405C;

    u32 field_406C;

    u16 field_4070;
    s32 field_408C;
    u32 field_4090;
    s32 field_4094;
    
    s_graphicEngineStatus_40E4* field_40E4;
    s_graphicEngineStatus_40AC field_40AC;
    s_graphicEngineStatus_40BC field_40BC[4]; // not sure about size, at least 4
    s_graphicEngineStatus_4514 field_4514;
};

extern s_graphicEngineStatus graphicEngineStatus;

void memcpy_dma(void* src, void* dst, u32 size);

void initDramAllocator(s_workArea* pWorkArea, u8* dest, u32 size, const char** assetList);

fixedPoint getCos(u32 value);
fixedPoint getSin(u32 value);

