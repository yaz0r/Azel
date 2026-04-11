#include "PDS.h"
#include "a7_delayedExitTrigger.h"
#include "field/field_a3/o_fld_a3.h"

// 0605eb88 — a 4-byte counter subtask that fires a field exit after 0x52 frames

struct sA7DelayedExitTrigger : public s_workAreaTemplate<sA7DelayedExitTrigger>
{
    s32 m0_counter;
    // size 0x4
};

// 0605eb5c
static void a7DelayedExitTrigger_Update(sA7DelayedExitTrigger* pThis)
{
    s32 counter = pThis->m0_counter;
    pThis->m0_counter = counter + 1;
    if (counter + 1 > 0x51)
    {
        fieldA3_1_checkExitsTaskUpdate2Sub1(0xe);
        pThis->getTask()->markFinished();
    }
}

// 0605eb88
void a7CreateDelayedExitTrigger(p_workArea parent)
{
    createSubTaskFromFunction<sA7DelayedExitTrigger>(parent, &a7DelayedExitTrigger_Update);
}
