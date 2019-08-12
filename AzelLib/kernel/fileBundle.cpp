#include "PDS.h"
#include "fileBundle.h"
#include "animation.h"

s_fileBundle::s_fileBundle(u8* rawBundle):
    mRawBundle(rawBundle)
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
        sProcessed3dModel* pCachedModel = nullptr;
        u32 offsetInBundle = getRawFileOffset(offset);
        if (offsetInBundle)
        {
            pCachedModel = new sProcessed3dModel(mRawBundle, getRawFileOffset(offset));
        }
        mModels[offset] = pCachedModel;
        return pCachedModel;
    }

    return it->second;
}

sAnimationData* s_fileBundle::getAnimation(u32 offset)
{
    std::map<u32, sAnimationData*>::iterator it = mAnimations.find(offset);
    if (it == mAnimations.end())
    {
        sAnimationData* pCachedAnimation = nullptr;
        u32 offsetInBundle = getRawFileOffset(offset);
        if (offsetInBundle)
        {
            pCachedAnimation = new sAnimationData(mRawBundle, offsetInBundle);
        }
        mAnimations[offset] = pCachedAnimation;
        return pCachedAnimation;
    }

    return it->second;
}

