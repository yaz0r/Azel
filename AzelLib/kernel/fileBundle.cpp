#include "PDS.h"
#include "fileBundle.h"

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
    return mRawBundle + READ_BE_U32(mRawBundle + offset);
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
        sProcessed3dModel* pCachedModel = new sProcessed3dModel(mRawBundle, getRawFileOffset(offset));
        mModels[offset] = pCachedModel;
        return pCachedModel;
    }

    return it->second;
}
