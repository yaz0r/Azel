#include "PDS.h"
#include "soundDriver.h"

s32 playSystemSoundEffect(s32 soundEffectIndex)
{
    enqueuePlaySoundEffect(soundEffectIndex, 1, 0x7f, 0);
    return 0; // needs to return a s32 as it can ball called directly from scripts
}
