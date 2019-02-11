#include "PDS.h"
#include "a3_0_task4.h"

void fieldA3_0_task4_updateSub0(s_dragonTaskWorkArea* pDragon)
{
    sVec3_FP var8_dragonPosition;
    getFieldDragonPosition(&var8_dragonPosition);

    if (mainGameState.getBit(0xA2, 3) || mainGameState.getBit(0x91, 3))
    {
        if (!mainGameState.getBit(0x6D, 0))
        {
            return;
        }

        //6057FE8
        assert(0);
    }
    else
    {
        //6058012
        switch (pDragon->m108)
        {
        case 0:
            if (mainGameState.getBit(0x6E, 0))
            {
                pDragon->m108 = 5;
            }
            else if(mainGameState.getBit(0x6D, 0))
            {
                pDragon->m108 = 3;
            }
            else
            {
                pDragon->m108 = 1;
            }
            // fall
        case 1:
            if (var8_dragonPosition[2] > -0x1194000)
                return;
            if (!startFieldScript(1, 0x59E))
                return;
            startCutscene(loadCutsceneData({ 0x60831C0, gFLD_A3 }, 0x24));
            pDragon->m108++;
            break;
        case 2:
            if ((var8_dragonPosition[0] >= 0x322000) && (var8_dragonPosition[2] <= -0x122A000))
                return;
            pDragon->m108++;
            break;
        default:
            assert(0);
            break;
        }
    }
}

struct s_A3_0_task4 : public s_workAreaTemplate<s_A3_0_task4>
{
    static void update(s_A3_0_task4* pThis)
    {
        if (mainGameState.getBit(0xA2, 3) || mainGameState.getBit(0x91, 3))
        {
            soundFunc(0x67);
        }
        else
        {
            TaskUnimplemented();
        }

        //6058552
        fieldA3_0_task4_updateSub0(getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask);
    }

    //size 0
};

void create_fieldA3_0_task4(p_workArea workArea)
{
    createSubTaskFromFunction<s_A3_0_task4>(workArea, &s_A3_0_task4::update);
}

