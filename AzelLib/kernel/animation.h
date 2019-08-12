#pragma once

struct sAnimationData
{
    struct sTrackHeader
    {
        s16 m0_tracksLength[3 * 3];
        u32 m14_trackDataOffset[3 * 3];
        std::vector<s16> m14_trackData[3 * 3];
    };
    sAnimationData(u8* base, u32 offset);

    u16 m0_flags; // 8: position, 0x10: rotation, 0x20: scale
    u16 m2_numBones;
    u16 m4_numFrames;
    u32 m8_offsetToTrackHeader;
    std::vector<sTrackHeader> m8_trackHeader;
};

struct sStaticPoseData
{
    struct sBonePoseData
    {
        sVec3_FP m0_translation;
        sVec3_FP mC_rotation;
        sVec3_FP m18_scale;
    };

    sStaticPoseData(u8* base, u32 offset, u32 numBones);

    std::vector<sBonePoseData> m0_bones;
};

struct sModelHierarchy
{
    //struct sProcessed3dModel* m0_3dModel;
    u32 m0_3dModelOffset;
    sModelHierarchy* m4_subNode;
    sModelHierarchy* m8_nextNode;

    u32 countNumberOfBones();
};

void copyPosePosition(s_3dModel* pModel);
void copyPoseRotation(s_3dModel* pModel);
void resetPoseScale(s_3dModel* pModel);

u32 stepAnimation(s_3dModel* p3DModel);
void interpolateAnimation(s_3dModel* p3dModel);

u32 setupModelAnimation(s_3dModel* pModel, struct sAnimationData* pAnimation);
u32 setupPoseInterpolation(s_3dModel* pModel, u32 interpolationLength);
