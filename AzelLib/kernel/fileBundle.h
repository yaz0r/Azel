#pragma once

#include <map>

struct s_fileBundle
{
    s_fileBundle(u8* rawBundle);
    u8* getRawFileAtOffset(u32 offset);
    u8* getRawBuffer();
    u32 getRawFileOffset(u32 offset);
    struct sProcessed3dModel* get3DModel(u32 offset);
    struct sProcessed3dModel* getCollisionModel(u32 offset);
    struct sAnimationData* getAnimation(u32 offset);
    struct sStaticPoseData* getStaticPose(u32 offset, u32 numBones);
    struct sModelHierarchy* getModelHierarchy(u32 offset);

    void setPatchFilePointers(u32 characterArea)
    {
        assert(mVdp1Offset == 0);
        mVdp1Offset = characterArea;
    }
private:
    u8* mRawBundle;
    u32 mVdp1Offset;
    std::map<u32, struct sProcessed3dModel*> mModels;
    std::map<u32, struct sAnimationData*> mAnimations;
    std::map<u32, struct sStaticPoseData*> mPoses;
    std::map<u32, struct sModelHierarchy*> mModelHierarchies;

    struct sModelHierarchy* readNode(u8* pBase, u32 offset);
};
