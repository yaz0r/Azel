#pragma once

#include <map>

struct s_fileBundle
{
    s_fileBundle(u8* rawBundle);
    u8* getRawFileAtOffset(u32 offset);
    u8* getRawBuffer();
    u32 getRawFileOffset(u32 offset);
    struct sProcessed3dModel* get3DModel(u32 offset);
    struct sAnimationData* getAnimation(u32 offset);

private:
    u8* mRawBundle;
    std::map<u32, struct sProcessed3dModel*> mModels;
    std::map<u32, struct sAnimationData*> mAnimations;
};
