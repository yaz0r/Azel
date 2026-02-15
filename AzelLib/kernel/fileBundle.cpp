#include "PDS.h"
#include "fileBundle.h"
#include "animation.h"
#include "processModel.h"

s_fileBundle::s_fileBundle(u8* rawBundle):
    mRawBundle(rawBundle),
    mVdp1Offset(0)
{

}

u8* s_fileBundle::getRawBuffer()
{
    return mRawBundle;
}

u8* s_fileBundle::getRawFileAtOffset(u32 offset)
{
    u32 offsetInBundle = getRawFileOffset(offset);
    if (offsetInBundle == 0)
    {
        return NULL;
    }
    return mRawBundle + offsetInBundle;
}

u32 s_fileBundle::getRawFileOffset(u32 offset)
{
    return READ_BE_U32(mRawBundle + offset);
}

sProcessed3dModel* s_fileBundle::get3DModel(u32 offset)
{
    std::map<u32, sProcessed3dModel*>::iterator it = mModels.find(offset);
    if (it == mModels.end())
    {
        sProcessed3dModel* pCachedData = nullptr;
        u32 offsetInBundle = getRawFileOffset(offset);
        if (offsetInBundle)
        {
            pCachedData = new sProcessed3dModel(mRawBundle, getRawFileOffset(offset));
            pCachedData->patchFilePointers(mVdp1Offset);
        }
        mModels[offset] = pCachedData;
        return pCachedData;
    }

    return it->second;
}

sProcessed3dModel* s_fileBundle::getCollisionModel(u32 offset)
{
    std::map<u32, sProcessed3dModel*>::iterator it = mModels.find(offset);
    if (it == mModels.end())
    {
        sProcessed3dModel* pCachedData = nullptr;
        u32 offsetInBundle = getRawFileOffset(offset);
        if (offsetInBundle)
        {
            pCachedData = new sProcessed3dModel(mRawBundle, getRawFileOffset(offset));
        }
        mModels[offset] = pCachedData;
        return pCachedData;
    }

    return it->second;
}

sAnimationData* s_fileBundle::getAnimation(u32 offset)
{
    std::map<u32, sAnimationData*>::iterator it = mAnimations.find(offset);
    if (it == mAnimations.end())
    {
        sAnimationData* pCachedData = nullptr;
        u32 offsetInBundle = getRawFileOffset(offset);
        if (offsetInBundle)
        {
            pCachedData = new sAnimationData(mRawBundle, offsetInBundle);
        }
        mAnimations[offset] = pCachedData;
        return pCachedData;
    }

    return it->second;
}

sStaticPoseData* s_fileBundle::getStaticPose(u32 offset, u32 numBones)
{
    std::map<u32, sStaticPoseData*>::iterator it = mPoses.find(offset);
    if (it == mPoses.end())
    {
        sStaticPoseData* pCachedData = nullptr;
        u32 offsetInBundle = getRawFileOffset(offset);
        if (offsetInBundle)
        {
            pCachedData = new sStaticPoseData(mRawBundle, offsetInBundle, numBones);
        }
        mPoses[offset] = pCachedData;
        return pCachedData;
    }

    return it->second;
}

sModelHierarchy* s_fileBundle::readNode(u8* pBase, u32 offset)
{
    sModelHierarchy* pNewNode = new sModelHierarchy;
    pNewNode->m0_3dModel = nullptr;
    pNewNode->m4_subNode = nullptr;
    pNewNode->m8_nextNode = nullptr;

    u32 modelOffset = READ_BE_U32(pBase + offset + 0);
    u32 subNodeOffset = READ_BE_U32(pBase + offset + 4);
    u32 nextNodeOffset = READ_BE_U32(pBase + offset + 8);


    if (modelOffset)
    {
        pNewNode->m0_3dModel = new sProcessed3dModel(pBase, modelOffset);
        pNewNode->m0_3dModel->patchFilePointers(mVdp1Offset);
    }

    if (subNodeOffset)
    {
        pNewNode->m4_subNode = readNode(pBase, subNodeOffset);
    }

    if (nextNodeOffset)
    {
        pNewNode->m8_nextNode = readNode(pBase, nextNodeOffset);
    }

    return pNewNode;
}

sModelHierarchy* s_fileBundle::getModelHierarchy(u32 offset)
{
    std::map<u32, sModelHierarchy*>::iterator it = mModelHierarchies.find(offset);
    if (it == mModelHierarchies.end())
    {
        sModelHierarchy* pCachedData = nullptr;
        u32 offsetInBundle = getRawFileOffset(offset);
        if (offsetInBundle)
        {
            pCachedData = readNode(mRawBundle, offsetInBundle);
        }
        mModelHierarchies[offset] = pCachedData;
        return pCachedData;
    }

    return it->second;
}

