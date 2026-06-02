#include "PDS.h"
#include "o_fld_a5.h"
#include "field/field_a3/o_fld_a3.h"

// Proximity alert task (Saturn size 0x8). Two variants — day and night —
// detect game-state bit transitions and trigger NPC encounters + cutscene
// cameras + field scripts.
struct sA5ProximityAlertTask : public s_workAreaTemplate<sA5ProximityAlertTask>
{
    s32 m0;
    s32 m4;
    // Saturn size 0x8
};

// 060666e8 — create a 0x114-byte NPC entity sub-task from Saturn data.
// The entity has its own init/update/draw chain (task def at 0609d7bc).
static p_workArea a5_createNpcEntity_060666e8(sA5ProximityAlertTask* pParent, u32 dataAddr)
{
    Unimplemented();
    (void)pParent; (void)dataAddr;
    return nullptr;
}

// 06067ee2 — create a 0x14-byte NPC tracking sub-task that waits for
// the NPC to reach a Y threshold, then sets the matching game-state bit.
static void a5_createNpcTracker_06067ee2(p_workArea pNpc, u32 nameTableAddr, s32 yThreshold, s16 bitIndex)
{
    Unimplemented();
    (void)pNpc; (void)nameTableAddr; (void)yThreshold; (void)bitIndex;
}

// 0606f9e4 — wrapper around a5_startCutsceneCameraType1 with flags=0
static void a5_startProximityCamera_0606f9e4(sA5ProximityAlertTask* pTask, u32 cameraDataAddr, u32 callbackAddr, u32 arg)
{
    Unimplemented();
    (void)pTask; (void)cameraDataAddr; (void)callbackAddr; (void)arg;
}

// 06067FDC — day proximity alert update. Checks game-state bits and
// triggers NPC encounters when conditions are met.
static void a5ProximityAlertUpdate_day(sA5ProximityAlertTask* pThis)
{
    if ((mainGameState.bitField[0x12] & 0x80) == 0)
    {
        // Early game: reset worm discovery counter if > 2
        s32 count = mainGameState.readPackedBits(0x578, 8);
        if (count > 2)
        {
            mainGameState.setPackedBits(0x578, 8, 0);
        }
    }
    else if ((mainGameState.bitField[0x95] & 0x20) == 0)
    {
        // Worm discovery complete: spawn the first NPC encounter
        p_workArea pNpc = a5_createNpcEntity_060666e8(pThis, 0x0609E790);
        a5_createNpcTracker_06067ee2(pNpc, 0x0609DBC6, -0x1DE000, 0x74B);
        mainGameState.bitField[0x95] |= 0x20;
        Unimplemented();
        //a5_startProximityCamera_0606f9e4(pThis, 0x0608C9D8, 0x06067F12, (u32)((u8*)pNpc + 0x14));
        startFieldScript(2, 0x5B7);
    }

    if ((mainGameState.bitField[0x95] & 0x10) == 0 && (mainGameState.bitField[0xA2] & 8) != 0)
    {
        // All worm holes discovered: spawn the second NPC encounter
        p_workArea pNpc = a5_createNpcEntity_060666e8(pThis, 0x0609E7AC);
        a5_createNpcTracker_06067ee2(pNpc, 0x0609E090, -0x11E000, 0x74C);
        mainGameState.bitField[0x95] |= 0x10;
        Unimplemented();
        //a5_startProximityCamera_0606f9e4(pThis, 0x0608CB04, 0x06067F7C, (u32)((u8*)pNpc + 0x14));

        if ((mainGameState.bitField[0xB] & 0x80) == 0)
            startFieldScript(0xB, 0x5C0);
        else
            startFieldScript(0xF, 0x5C4);
    }
}

// 0606811E — night proximity alert update. Triggers NPC encounter
// when specific story progression bits are set.
static void a5ProximityAlertUpdate_night(sA5ProximityAlertTask* pThis)
{
    if ((mainGameState.bitField[0x96] & 0x80) != 0)
        return;
    if ((mainGameState.bitField[0xA3] & 0x80) == 0)
        return;

    // Night encounter trigger
    p_workArea pNpc = a5_createNpcEntity_060666e8(pThis, 0x0609E7C8);
    a5_createNpcTracker_06067ee2(pNpc, 0x0609D7DC, -0x21E000, 0x74D);
    mainGameState.bitField[0x96] |= 0x80;
    Unimplemented();
    //a5_startProximityCamera_0606f9e4(pThis, 0x0608CC30, 0x06067FAC, (u32)((u8*)pNpc + 0x14));
}

// 06068182 — create proximity alert task for day subfields (0, 7)
void createA5_proximityAlert_day(p_workArea parent)
{
    static sA5ProximityAlertTask::TypedTaskDefinition td = { nullptr, &a5ProximityAlertUpdate_day, nullptr, nullptr };
    createSubTask<sA5ProximityAlertTask>(parent, &td);
}

// 0606818a — create proximity alert task for night subfields (2, 8)
void createA5_proximityAlert_night(p_workArea parent)
{
    static sA5ProximityAlertTask::TypedTaskDefinition td = { nullptr, &a5ProximityAlertUpdate_night, nullptr, nullptr };
    createSubTask<sA5ProximityAlertTask>(parent, &td);
}
