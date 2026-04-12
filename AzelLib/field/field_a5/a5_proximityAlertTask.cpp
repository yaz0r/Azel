#include "PDS.h"
#include "o_fld_a5.h"

// Proximity alert task (Saturn size 0x8). Two instances exist — a day
// variant (0x06067FDC) and a night variant (0x0606811E) — each of which
// detects specific game-state bit transitions and triggers the matching
// message/script + fade via a chain of unreimplemented A5 helpers
// (FUN_FLD_A5__060666e8, 06067ee2, 0606f9e4, 06071708). The task state
// itself is just 2 x s32 scratch fields.
struct sA5ProximityAlertTask : public s_workAreaTemplate<sA5ProximityAlertTask>
{
    s32 m0;
    s32 m4;
    // Saturn size 0x8
};

// 06067FDC
static void a5ProximityAlertUpdate_day(sA5ProximityAlertTask* /*pThis*/)
{
    Unimplemented();
}

// 0606811E
static void a5ProximityAlertUpdate_night(sA5ProximityAlertTask* /*pThis*/)
{
    Unimplemented();
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
