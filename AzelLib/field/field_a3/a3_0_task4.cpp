#include "PDS.h"
#include "a3_0_task4.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup

void fieldA3_0_task4_updateSub0(s_dragonTaskWorkArea* pDragon)
{
    sVec3_FP var8_dragonPosition;
    getFieldDragonPosition(&var8_dragonPosition);

    if (mainGameState.getBit(0xA2 * 8 + 3) || mainGameState.getBit(0x91 *8 + 3))
    {
        if (!mainGameState.getBit(0x6D * 8 + 0))
        {
            return;
        }

        //6057FE8
        if (var8_dragonPosition[0] < 0x3A2000)
            return;

        if (var8_dragonPosition[2] > -0x122A000)
            return;

        startFieldScript(11, 1448);
    }
    else
    {
        //6058012
        switch (pDragon->m108)
        {
        case 0:
            if (mainGameState.getBit(0x6E * 8 +0))
            {
                pDragon->m108 = 5;
            }
            else if(mainGameState.getBit(0x6D * 8 +0))
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
            startCutscene(loadCutsceneData({ 0x60831C0, gFLD_A3 }));
            pDragon->m108++;
            break;
        case 2:
            if ((var8_dragonPosition[0] >= 0x322000) && (var8_dragonPosition[2] <= -0x122A000))
                return;
            pDragon->m108++;
            break;
        case 3:
            if ((var8_dragonPosition[0] < 0x3A2000) || (var8_dragonPosition[2] > -0x122A000))
                return;
            if (!startFieldScript(9, 0x5A6))
                return;
            pDragon->m108++;
            break;
        case 4:
            if (var8_dragonPosition[2] <= -0x122A000)
                return;
            pDragon->m108++;
            break;
        case 5:
            if ((var8_dragonPosition[0] < 0x3A2000) || (var8_dragonPosition[2] > -0x122A000))
                return;
            if (!startFieldScript(10, -1))
                return;
            pDragon->m108++;
            break;
        case 6:
            if (var8_dragonPosition[2] <= -0x122A000)
                return;
            pDragon->m108--;
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
        if (mainGameState.getBit(0xA2 * 8 + 3) || mainGameState.getBit(0x91 * 8 + 3))
        {
            soundFunc(0x67);
        }
        else
        {
            Unimplemented();
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

