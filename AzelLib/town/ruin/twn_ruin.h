#pragma once

#include "town/town.h"

extern struct TWN_RUIN_data* gTWN_RUIN;
struct TWN_RUIN_data : public sTownOverlay
{
    TWN_RUIN_data();
    static void makeCurrent()
    {
        if (gTWN_RUIN == NULL)
        {
            gTWN_RUIN = new TWN_RUIN_data();
        }
        gCurrentTownOverlay = gTWN_RUIN;
    }

    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
    sTownObject* createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
};

p_workArea overlayStart_TWN_RUIN(p_workArea pThis, u32 arg);
s32 TWN_RUIN_ExecuteNative(sSaturnPtr ptr);
s32 TWN_RUIN_ExecuteNative(sSaturnPtr ptr, s32 arg0);
s32 TWN_RUIN_ExecuteNative(sSaturnPtr ptr, s32 arg0, s32 arg1);
void EdgeUpdateSub0(struct sCollisionBody* r14_pose);
