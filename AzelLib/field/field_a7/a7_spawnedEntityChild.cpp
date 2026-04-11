#include "PDS.h"
#include "a7_spawnedEntityChild.h"
#include "kernel/fade.h"

extern s32 battleIndex; // todo: cleanup

// 06066e4e — battle trigger helper used by spawnedEntityChild state 3.
// Two-arm dispatch: if encounters are disabled (m28_status & 1 == 0) it stamps
// battleIndex once encounter data is available; otherwise it kicks a sub-task at
// FLD_A7::060870a4 (init=06066a98, draw=06066b06, delete=06066c90).
static void a7TriggerBattle_06066e4e(s32 enemyId, s32 param2)
{
    s_fieldTaskWorkArea* pField = getFieldTaskPtr();
    if ((pField->m28_status & 1) == 0)
    {
        if (hasEncounterData != 0)
        {
            battleIndex = (s8)enemyId;
        }
    }
    else if ((pField->m28_status & 0xFFFE) == 0)
    {
        Unimplemented();
        (void)param2;
    }
}

// 06054b54
void a7SpawnedEntityChild_Init_06054b54(sA7SpawnedEntityChild* pThis)
{
    graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
    u32 color = convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color);
    fadePalette(&g_fadeControls.m0_fade0, color, 0xC218, 10);
    pThis->m0_state = 0;
}

// 06054b82
void a7SpawnedEntityChild_Update_06054b82(sA7SpawnedEntityChild* pThis)
{
    u32 state = pThis->m0_state;

    if (state == 0)
    {
        if (g_fadeControls.m0_fade0.m20_stopped != 0)
        {
            pThis->m0_state++;
        }
        return;
    }

    if (state == 1)
    {
        u32 color = convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color);
        fadePalette(&g_fadeControls.m0_fade0, color, (s16)g_fadeControls.m_48, 0x1E);
        pThis->m0_state++;
    }
    else if (state != 2)
    {
        if (state == 3)
        {
            a7TriggerBattle_06066e4e(6, -1);
            pThis->m0_state++;
            return;
        }
        if (state != 4)
        {
            return;
        }
        pThis->getTask()->markFinished();
        return;
    }

    // Shared tail for states 1 (after fade trigger) and 2.
    if (g_fadeControls.m0_fade0.m20_stopped != 0)
    {
        graphicEngineStatus.m40AC.m1_isMenuAllowed = 1;
        pThis->m0_state++;
    }
}
