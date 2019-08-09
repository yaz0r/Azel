#pragma once

#include "town/town.h"

struct TWN_RUIN_data : public sTownOverlay
{
    static void create();
    void init() override;
    sTownObject* createObjectTaskFromEA_siblingTaskWithEAArgWithCopy(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
    sTownObject* createObjectTaskFromEA_subTaskWithEAArg(npcFileDeleter* parent, sSaturnPtr definitionEA, s32 size, sSaturnPtr arg) override;
};

extern TWN_RUIN_data* gTWN_RUIN;

p_workArea overlayStart_TWN_RUIN(p_workArea pThis, u32 arg);
s32 TWN_RUIN_ExecuteNative(sSaturnPtr ptr);
s32 TWN_RUIN_ExecuteNative(sSaturnPtr ptr, s32 arg0);
s32 TWN_RUIN_ExecuteNative(sSaturnPtr ptr, s32 arg0, s32 arg1);
void EdgeUpdateSub0(struct sMainLogic_74* r14_pose);
