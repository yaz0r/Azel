#pragma once

#include <array>

enum eItems : s16;

extern const u8 bitMasks[];
extern const u8 reverseBitMasks[];

struct sVec2_S16
{
    std::array<s16, 2> m_value;

    s16 operator[](int i) const
    {
        return m_value[i];
    }

    s16& operator[](int i)
    {
        return m_value[i];
    }
};

struct sVec2_S32
{
    std::array<s32, 2> m_value;

    s32 operator[](int i) const
    {
        return m_value[i];
    }

    s32& operator[](int i)
    {
        return m_value[i];
    }
};

typedef std::array<u16, 3> sVec3_U16;

struct sVec3_FP
{
    sVec3_FP()
    {
    }

    sVec3_FP(fixedPoint x, fixedPoint y, fixedPoint z)
    {
        m_value[0] = x;
        m_value[1] = y;
        m_value[2] = z;
    }
    std::array<fixedPoint,3> m_value;

    void zeroize()
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

    bool operator==(const sVec3_FP otherVec) const
    {
        return m_value == otherVec.m_value;
    }

    sVec3_FP operator-() const
    {
        sVec3_FP result;
        result[0] = -m_value[0];
        result[1] = -m_value[1];
        result[2] = -m_value[2];

        return result;

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

    sVec3_FP& operator-=(const sVec3_FP otherVec)
    {
        m_value[0] -= otherVec[0];
        m_value[1] -= otherVec[1];
        m_value[2] -= otherVec[2];

        return *this;
    }

    sVec3_FP& operator*=(const sVec3_FP otherVec)
    {
        m_value[0] *= otherVec[0];
        m_value[1] *= otherVec[1];
        m_value[2] *= otherVec[2];

        return *this;
    }

    sVec3_FP  operator *(const sVec3_FP otherVec) const
    {
        sVec3_FP newValue = *this;
        newValue *= otherVec;
        return newValue;
    }

    sVec3_FP operator + (const sVec3_FP otherVec) const
    {
        sVec3_FP newValue = *this;
        newValue += otherVec;
        return newValue;
    }

    sVec3_FP operator * (const s32 value) const
    {
        sVec3_FP newValue = *this;
        newValue[0] *= value;
        newValue[1] *= value;
        newValue[2] *= value;
        return newValue;
    }

    sVec3_FP operator / (const s32 value) const
    {
        sVec3_FP newValue = *this;
        newValue[0] /= value;
        newValue[1] /= value;
        newValue[2] /= value;
        return newValue;
    }

    sVec3_FP operator >> (const int amount) const
    {
        sVec3_FP newValue = *this;
        newValue[0].m_value >>= amount;
        newValue[1].m_value >>= amount;
        newValue[2].m_value >>= amount;
        return newValue;
    }

    sVec3_FP normalized() const
    {
        sVec3_FP newValue;
        newValue[0] = m_value[0].normalized();
        newValue[1] = m_value[1].normalized();
        newValue[2] = m_value[2].normalized();
        return newValue;
    }
};

sVec3_FP FP_Div(sVec3_FP divident, fixedPoint divisor);

struct sVec3_S16_12_4
{
    std::array<s16, 3> m_value;

    s16 operator[](int i) const
    {
        return m_value[i];
    }

    s16& operator[](int i)
    {
        return m_value[i];
    }

    sVec3_FP toSVec3_FP() const
    {
        sVec3_FP convertedVector;
        convertedVector[0] = ((s32)m_value[0]) * 0x10;
        convertedVector[1] = ((s32)m_value[1]) * 0x10;
        convertedVector[2] = ((s32)m_value[2]) * 0x10;

        return convertedVector;
    }
};

struct sVec3_S16
{
    std::array<fp16, 3> m_value;

    fp16 operator[](int i) const
    {
        return m_value[i];
    }

    fp16& operator[](int i)
    {
        return m_value[i];
    }

    sVec3_FP toSVec3_FP() const
    {
        sVec3_FP convertedVector;
        convertedVector[0] = ((s32)m_value[0].m_value) * 0x10000;
        convertedVector[1] = ((s32)m_value[1].m_value) * 0x10000;
        convertedVector[2] = ((s32)m_value[2].m_value) * 0x10000;

        return convertedVector;
    }
};

struct sVec2_FP
{
    fixedPoint m_value[2];

    fixedPoint operator[](int i) const
    {
        return m_value[i];
    }

    fixedPoint& operator[](int i)
    {
        return m_value[i];
    }

    sVec2_FP& operator-=(const sVec2_FP otherVec)
    {
        m_value[0] -= otherVec[0];
        m_value[1] -= otherVec[1];

        return *this;
    }
};

sVec3_FP MTH_Mul(const fixedPoint& a, const sVec3_FP& b);

#define xstr(a) str(a)
#define str(a) #a

#ifndef SHIPPING_BUILD
void Imgui_FP(const char* label, fixedPoint* pFP);
void Imgui_FP_Angle(const char* name, fixedPoint* pFP);
void Imgui_Vec3FP(sVec3_FP* pVector);
void Imgui_Vec3FP(const char* name, sVec3_FP* pVector);

struct s_memberDefinitions {
    const std::string m_name;
    const u32 m_ptr;
    const size_t m_type;
};

#define MEMBER_TYPE(classname, name) {#name, offsetof(classname, name), typeid(((classname*)nullptr)->name).hash_code()}

void InspectTask(const char* className, void* pBase, const std::vector<s_memberDefinitions>& members);

#endif

#include "kernel/fade.h"

struct sInterpolator_FP
{
    fixedPoint m0_step;
    fixedPoint m4_currentValue;
    fixedPoint m8_targetValue;
};

fixedPoint dot3_FP(const sVec3_FP* r4, const sVec3_FP* r5);
fixedPoint MTH_Product3d_FP(const sVec3_FP& r4, const sVec3_FP& r5);

u8* getSaturnPtr(sSaturnPtr ptr);
s8 readSaturnS8(sSaturnPtr ptr);
u8 readSaturnU8(sSaturnPtr ptr);
s16 readSaturnS16(sSaturnPtr ptr);
u16 readSaturnU16(sSaturnPtr ptr);
s32 readSaturnS32(sSaturnPtr ptr);
u32 readSaturnU32(sSaturnPtr ptr);
fixedPoint readSaturnFP(sSaturnPtr ptr);
sVec3_FP readSaturnVec3(sSaturnPtr ptr);
sSaturnPtr readSaturnEA(sSaturnPtr ptr);
std::string readSaturnString(sSaturnPtr ptr);

struct sMatrix4x3
{
    std::array<fixedPoint, 4*3> matrix;

    sVec3_FP getTranslation()
    {
        sVec3_FP value;
        value[0] = matrix[3];
        value[1] = matrix[7];
        value[2] = matrix[11];

        return value;
    }

    fixedPoint& operator[](int i)
    {
        return matrix[i];
    }
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

    u16 mA_weaponType;
    u16 mC_laserPower;
    u16 mE_gunPower;

    u16 m10_currentHP; // 0x10
    u16 m12_classMaxHP; // 0x12
    u16 m14_currentBP; // 0x14
    u16 m16_classMaxBP; // 0x16
    u16 m18_statAxisScale; // 0x18
    s16 m1A_dragonCursorX; //1A
    s16 m1C_dragonCursorY; //1C
    u32 XP; //20
    u32 dyne; // 38
    u32 frameCounter; //3C

    s32 m5C_gunShotFired;
    s32 m60_homingLaserFired;

    u32 m70; //70

    char m94_playerName[17];
    char mA5_dragonName[17];
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

    s8 getItemCount(eItems itemIndex)
    {
        if (itemIndex < 0x4D)
        {
            return consumables[itemIndex];
        }
        else
        {
            return getBit(0xF3 + itemIndex);
        }
    }

    void setItemCount(eItems itemIndex, s8 count)
    {
        if (itemIndex < 0x4D)
        {
            consumables[itemIndex] = count;
        }
        else
        {
            if (count)
            {
                setBit(0xF3 + itemIndex);
            }
            else
            {
                clearBit(0xF3 + itemIndex);
            }
        }
    }

    void addItemCount(eItems itemIndex, s8 count)
    {
        if (itemIndex < 0x4D)
        {
            //0607A1F6
            if (consumables[itemIndex] + count > 99)
            {
                consumables[itemIndex] = 99;
            }
            else
            {
                consumables[itemIndex] += count;
            }
        }
        else
        {
            //607A2A8
            if (count == 1)
            {
                setBit(0xF3 + itemIndex);
            }
            else if (count == -1)
            {
                clearBit(0xF3 + itemIndex);
            }
            else
            {
                assert(0);
            }
        }

    }

    std::array<s8, 0x4D> consumables;
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

    u32 readPackedBits(u32 firstBitOffset, u32 numBits)
    {
        u32 readPackedBits(u8 * bitField, u32 firstBitOffset, u32 numBits);
        
        return readPackedBits(bitField, firstBitOffset, numBits);
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

    void clearBit(u32 byteIndex, u32 bitIndex)
    {
        bitField[byteIndex] &= ~(0x80 >> bitIndex);
    }

    bool getBit(u32 bitIndex)
    {
        if (bitField[bitIndex / 8] & (0x80 >> (bitIndex % 8)))
            return true;
        return false;
    }

    bool getBit(u32 byteIndex, u32 bitIndex)
    {
        if (bitField[byteIndex] & (0x80 >> (bitIndex)))
            return true;
        return false;
    }

    bool getBit566(u32 bitIndex)
    {
        if (bitIndex < 1000)
        {
            return getBit(bitIndex);
        }
        else
        {
            return getBit(bitIndex - 566);
        }
    }

    void setBit566(u32 bitIndex)
    {
        if (bitIndex < 1000)
        {
            return setBit(bitIndex);
        }
        else
        {
            return setBit(bitIndex - 566);
        }
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
    sSaturnPtr m0_ptr;
    u32 m4_count;
};

struct s_RiderDefinition
{
    const char* m_MCBName; //0
    const char* m_CGBName; //4
    u16 m_flags; //8
    u16 mA_offsetToDefaultPose; //A
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
    sVec3_FP m0_translation; // 0
    sVec3_FP mC_rotation; // C
    sVec3_FP m18_scale; // 18
    sVec3_FP m24_halfTranslation; // 24
    sVec3_FP m30_halfRotation; // 30
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
    struct s_fileBundle* m4_pModelFile; //4

    u16 m8; //8 2 = m3C_boneMatrices is allocated
    u16 mA_animationFlags; //A 0x100: only root has translation
    u16 mC_modelIndexOffset; //C

    u16 m10_currentAnimationFrame; //10
    u16 m12_numBones; //12
    s16 m14; //14
    u16 m16_previousAnimationFrame; //16

    void (*m18_drawFunction)(s_3dModel*); // 18
    void (*m1C_addToDisplayListFunction)(struct sProcessed3dModel*); // 1C
    void (*m20_positionUpdateFunction)(s_3dModel*); // 20
    void (*m24_rotationUpdateFunction)(s_3dModel*); // 24
    void (*m28_scaleUpdateFunction)(s_3dModel*); // 28
    std::vector<sPoseData> m2C_poseData; //2C

    struct sAnimationData* m30_pCurrentAnimation;

    struct sStaticPoseData* m34_pDefaultPose; //34
    u8* m38_pColorAnim; //38

    std::vector<sMatrix4x3> m3C_boneMatrices; //3C

    const s_RiderDefinitionSub* m40; //40
    std::vector<std::vector<sVec3_FP>> m44_hotpointData; //44 one entry per bone, and each bone get an array of vertex (hot-spots on the model?)
    std::vector<sPoseDataInterpolation> m48_poseDataInterpolation; //48

    u16 m4C_interpolationStep; //4C
    u16 m4E_interpolationLength; //4E

    //size 0x4C
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
    std::vector<sMatrix4x3>* m4_boneMatrices; // 4;
    s_runtimeAnimData* m8_runtimeAnimData; // 8 one per countAnims
    u8 countAnims; // C
    u8 count0; // D
    u8 count1; // E
    u8 count2; // F
};

struct s_dragonState : public s_workAreaTemplate<s_dragonState>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, NULL, NULL, NULL};
        return &taskDefinition;
    }

    struct s_fileBundle* m0_pDragonModelBundle;
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
    u8 m0_inputType; // 2 = analog stick
    s8 m2_analogX;
    s8 m3_analogY;
    s8 m4;
    s8 m5;
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
    s_graphicEngineStatus_4514_sub m0_inputDevices[2]; // one per input port
    u16 mD8_buttonConfig[3][16]; //0: on feet, 1: dragon, 2: battle
    u8 m138[3];
};

struct s_graphicEngineStatus_405C
{
    s16 m0;
    s16 m2;
    s16 m4;
    s16 m6;
    s16 m8;
    s16 mA;
    s16 mC;
    s16 mE;

    fixedPoint m10_nearClipDistance;
    fixedPoint m14_farClipDistance; // max distance for drawing laser?
    fixedPoint m18_widthScale;
    fixedPoint m1C_heightScale;
    fixedPoint m20_heightRatio2;
    fixedPoint m24_heightRatio;
    fixedPoint m28_widthRatio2;
    fixedPoint m2C_widthRatio;
    fixedPoint m30;
    fixedPoint m34;
    fixedPoint m38;
    u16 VDP1_X1; // 3C
    u16 VDP1_Y1; // 3E
    u16 VDP1_X2; // 40
    u16 VDP1_Y2; // 42
    u16 m44_localCoordinatesX; // 44
    u16 m46_localCoordinatesY; // 46
    u32 setClippingCoordinatesEA; // 0x48 ptr in VDP1 Memory
    u32 setLocalCoordinatesEA; // 0x4C ptr in VDP1 Memory

    // size should be 50
};

struct s_graphicEngineStatus_40E4
{
    s_graphicEngineStatus_405C m0;
    sVdp2Controls m50;
    sFadeControls m2B0;
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
    s_vdp1Packet* m0_pNext;
    u16 m4_bucketTypes; // 4
    u16 m6_vdp1EA; //6
};

struct s_vdp1Command
{
    u16 CMDCTRL;
    u16 CMDLINK;
    u16 CMDPMOD;
    u16 CMDCOLR;
    u16 CMDSRCA;
    u16 CMDSIZE;
    u16 CMDXA;
    u16 CMDYA;
    u16 CMDXB;
    u16 CMDYB;
    u16 CMDXC;
    u16 CMDYC;
    u16 CMDXD;
    u16 CMDYD;
    u16 CMDGRA;
    u16 _DUMMY;
};

struct s_vd1ExtendedCommand
{
    u32 frameIndex;
    float depth;
};

s_vd1ExtendedCommand* createVdp1ExtendedCommand(u32 vd1PacketStart);
s_vd1ExtendedCommand* fetchVdp1ExtendedCommand(u32 vd1PacketStart);

struct sPerQuadDynamicColor
{
    s16 m0[4];
};

struct s_vdp1Context
{
    u32 m0_currentVdp1WriteEA; //0
    u32 m4[2]; //4
    u32 mC; //C
    std::array<sPerQuadDynamicColor, 1024>::iterator m10; //10
    std::array<sPerQuadDynamicColor, 1024> m14[2]; //14 this used to be allocated inside of VDP1 memory
    u32 m1C; //1C
    s_vdp1Packet* m20_pCurrentVdp1Packet; //20
    s_vdp1Packet m24_vdp1Packets[1024]; // 24
    // size should be 2024

    std::array<s_vd1ExtendedCommand, 1024> m_vd1pExtendedCommand;
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

    s_vdp1Context m14_vdp1Context[2]; // 14
    s_graphicEngineStatus_405C m405C;
    s_graphicEngineStatus_40AC m40AC;
    s_graphicEngineStatus_40BC m40BC_layersConfig[4];
    s_graphicEngineStatus_40E4* m40E4;
    s_graphicEngineStatus_4514 m4514;
};

extern s_graphicEngineStatus graphicEngineStatus;

void memcpy_dma(void* src, void* dst, u32 size);

void initDramAllocator(s_workArea* pWorkArea, u8* dest, u32 size, const char** assetList);

fixedPoint getCos(u32 value);
fixedPoint getSin(u32 value);

extern u16 atanTable[2049];
extern u16 resetVdp2StringsData[4106];

void registerModelAndCharacter(u8* model, u32 character);

void getVdp1ClippingCoordinates(std::array<s16, 4>& r4);
void getVdp1LocalCoordinates(std::array<s16, 2>& r4);

