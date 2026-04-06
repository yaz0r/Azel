#include "PDS.h"
#include "dragonLightWingEvolution.h"
#include "kernel/fade.h"
#include "audio/systemSounds.h"

// shared field functions (local to each overlay)
u32 isDragonInputAllowed(s_dragonTaskWorkArea* r4);
void initDragonMovementMode();

// 06068300
bool areAllDUnitsCollected()
{
    for (int i = 0; i < 12; i++)
    {
        if (!mainGameState.getBit(0x56 + 0xF3 + i))
        {
            return false;
        }
    }
    return true;
}

// 0606802a
struct s_dragonLightWingEvolutionTask : public s_workAreaTemplateWithArg<s_dragonLightWingEvolutionTask, u8>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, &Update, NULL, NULL };
        return &taskDefinition;
    }

    static void Init(s_dragonLightWingEvolutionTask* pThis, u8 arg)
    {
        pThis->m4_dragonType = arg;
    }

    // 06068030
    static void Update(s_dragonLightWingEvolutionTask* pThis)
    {
        s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        s8 state = pThis->m5_state;

        switch (state)
        {
        case 0:
            if (!isDragonInputAllowed(pDragon))
                return;
            initDragonMovementMode();
            if ((s8)g_fadeControls.m_4C <= (s8)g_fadeControls.m_4D)
            {
                vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
                vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
            }
            {
                u32 color = convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color);
                fadePalette(&g_fadeControls.m0_fade0, color, 0xFFFF, 0x1E);
            }
            playSystemSoundEffect(0x1B);
            pThis->m5_state++;
            break;
        default:
            assert(0); // TODO: states 1-7
            break;
        }
    }

    s32 m0_countdown;
    u8 m4_dragonType;
    s8 m5_state;
    u8 m6;
    // size 0x8
};

// 060682de
void createDragonLightWingEvolutionTask(u32 param)
{
    createSubTaskWithArg<s_dragonLightWingEvolutionTask>(getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask, (u8)param);
}

// Polling task — checks each frame if all D-units have been collected
struct s_dragonLightWingEvolutionCheckTask : public s_workAreaTemplate<s_dragonLightWingEvolutionCheckTask>
{
};

// 06068340
static void dragonLightWingEvolutionCheckTask_Update(s_dragonLightWingEvolutionCheckTask* pThis)
{
    if (areAllDUnitsCollected())
    {
        createDragonLightWingEvolutionTask(6);
        pThis->getTask()->m14_flags |= 1;
    }
}

// 06068394
void initDragonLightWingEvolutionCheck(s_dragonTaskWorkArea* pDragon)
{
    if (!areAllDUnitsCollected())
    {
        createSubTaskFromFunction<s_dragonLightWingEvolutionCheckTask>(pDragon, &dragonLightWingEvolutionCheckTask_Update);
    }
}
