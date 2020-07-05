#include "PDS.h"
#include "battleResultScreenProgressiveText.h"

struct sBattleResultProgressiveText2 : public s_workAreaTemplate<sBattleResultProgressiveText2>
{
    s8 m0_positionInString;
    s8 m1;
    s8 m2;
    sSaturnPtr m4_config;
    // size 0x8
};

void battleResultScreenProgressiveText2_update(sBattleResultProgressiveText2* pThis)
{
    if (!(graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 6))
    {
        if (readSaturnS8(readSaturnEA(pThis->m4_config + 0xC) + pThis->m0_positionInString) == 0)
        {
            battleResultScreen_createProgressiveText(pThis, pThis->m4_config, 2, 0, 1);
            pThis->getTask()->markFinished();
        }
        else
        {
            pThis->m1++;
            if (pThis->m1 > 1)
            {
                FunctionUnimplemented(); // particle generation
                pThis->m2++;
                pThis->m0_positionInString++;
                pThis->m1 = 0;
            }
        }
    }
    else
    {
        battleResultScreenProgressiveText_printString(pThis->m4_config);
        pThis->getTask()->markFinished();
    }
}

void battleResultScreen_createProgressiveText2(p_workArea parent, sSaturnPtr config)
{
    sBattleResultProgressiveText2* pThis = createSiblingTaskFromFunction<sBattleResultProgressiveText2>(parent, battleResultScreenProgressiveText2_update);

    pThis->m4_config = config;
    pThis->m2 = readSaturnS8(config + 3);
    pThis->m1 = 2;
    pThis->m0_positionInString = 0;
}
