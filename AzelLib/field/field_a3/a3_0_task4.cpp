#include "PDS.h"
#include "a3_0_task4.h"
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup
#include "audio/soundDriver.h"

s32 playBattleSoundEffect(s32 effectIndex); // TODO: cleanup

// 0602c2d4
static void startSoundWithVolume(s32 soundIndex, s32 volume)
{
    enqueuePlaySoundEffect(soundIndex, 1, volume, 0);
}

// 0602c2e4
static void updateSoundVolume(s32 soundIndex, s32 volume)
{
    enqueuePlaySoundEffect(soundIndex, 5, volume, 0);
}

// 0605849a
static s32 fieldA3_0_task4_computeSoundVolume(sVec3_FP* pPos)
{
    (*pPos)[0] = (*pPos)[0] >> 2;
    (*pPos)[1] = (*pPos)[1] >> 2;
    (*pPos)[2] = (*pPos)[2] >> 2;

    fixedPoint distSq = MTH_Product3d_FP(*pPos, *pPos);
    s32 dist = (s32)sqrt_F(distSq);
    s32 result = dist * -0x1b;
    if (result < 0)
    {
        result = result + 0x7ffff;
    }
    return (s16)(result >> 0x13) + 0x7f;
}

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
            playBattleSoundEffect(0x67);
        }
        else
        {
            // 06058510: 3D positional sound attenuation
            static const sVec3_FP refPos = { fixedPoint(0x39D000), fixedPoint(0), fixedPoint((s32)0xFED80000) };
            sVec3_FP transformedPos;
            transformAndAddVecByCurrentMatrix(&refPos, &transformedPos);
            s32 volume = fieldA3_0_task4_computeSoundVolume(&transformedPos);
            if (volume < 0x51)
            {
                playBattleSoundEffect(0x67);
            }
            else
            {
                s32 found = findSound(0x67);
                if ((s8)found < 0)
                {
                    startSoundWithVolume(0x67, volume);
                }
                else
                {
                    updateSoundVolume(0x67, volume);
                }
            }
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

