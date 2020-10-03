#include "PDS.h"
#include "fieldEncounterTask.h"
#include "commonOverlay.h"
#include "audio/soundDriver.h"
#include "battle/battleOverlay.h"

extern s32 battleIndex; // todo: cleanup

struct sEncouterTask : public s_workAreaTemplateWithArg<sEncouterTask, s_workArea*>
{
    s8 m0_status;
    u8 m1;
    s_workArea* m4;
    s_workArea* m8_battleOverlay;
    s8 mC;
    s8 mD;
    s8 mE;
    s8 mF;
    s32 m10;
    s16 m14;
    s16 m16;
    sSaturnPtr m18;
    sSaturnPtr m1C;
    void* m20;
    // size 0x24
};

void encounterTaskInit(sEncouterTask* pThis, s_workArea* pArg)
{
    pThis->m4 = pArg;
    pThis->mD = -1;
    hasEncounterData = 0;
    encounterTaskVar0 = 0;
}

void noEncounterData()
{
    if (enableDebugTask)
    {
        vdp2PrintStatus.m10_palette = 0xB000;
        vdp2DebugPrintSetPosition(1, 0xB);
        vdp2DebugPrintNewLine("NO ENCOUNT AREA");
    }
    fieldTaskPtr->m34_radarDangerLevel = 0;
}

s32 getEncounterArea(sEncouterTask* pThis)
{
    if (fieldTaskPtr->m8_pSubFieldData == nullptr)
    {
        pThis->m1C = sSaturnPtr::getNull();
        pThis->mD = -1;
        return 0;
    }

    // Do we need to update the area?
    if ((pThis->m1C.isNull()) || (pThis->mD != fieldTaskPtr->m2E_currentSubFieldIndex))
    {
        pThis->mD = fieldTaskPtr->m2E_currentSubFieldIndex;

        if (readSaturnEA(pThis->m18).isNull() || readSaturnEA(readSaturnEA(pThis->m18) + pThis->mD * 4).isNull())
        {
            pThis->m1C = sSaturnPtr::getNull();
            pThis->mD = -1;
            return 0;
        }

        pThis->m1C = readSaturnEA(readSaturnEA(pThis->m18) + pThis->mD * 4);
        pThis->mE = 0;
        pThis->mF = readSaturnS8(pThis->m18 + 7);
        pThis->m10 = pThis->m14 = readSaturnS8(pThis->m18 + 6);
        pThis->m16 = readSaturnS16(pThis->m18 + 4);

        s32 piVar2;
        sSaturnPtr local_14 = pThis->m1C;
        do 
        {
            *getSaturnPtr(local_14 + 3);
            piVar2 = readSaturnS32(local_14 + 0x1C);
            local_14 += 0x10;
        } while (piVar2 > -1);
    }

    return 1;
}

s32 evaluateRandomEncounterRateSub(sSaturnPtr param)
{
    int randomValue = performModulo2(100, randomNumber());
    sSaturnPtr pcVar5 = readSaturnEA(param + 0xC);
    int counter = 0;
    (*getSaturnPtr(param + 3))++;
    while (counter += readSaturnS8(pcVar5 + 1), counter <= randomValue)
    {
        pcVar5 += 2;
    }
    return readSaturnS8(pcVar5);
}

s32 evaluateRandomEncounterRate(sEncouterTask* pThis)
{
    if (!getEncounterArea(pThis))
    {
        noEncounterData();
        return -1;
    }

    s_dragonTaskWorkArea* pDragon = fieldTaskPtr->m8_pSubFieldData->m338_pDragonTask;

    int dragonX = pDragon->m8_pos[0] >> 0xC;
    int dragonZ = pDragon->m8_pos[2] >> 0xC;

    sSaturnPtr pcVar5 = pThis->m1C;
    sSaturnPtr local_8;
    int iVar6 = 0;
    do
    {
        int X1 = readSaturnS16(pcVar5 + 4);
        int Z1 = readSaturnS16(pcVar5 + 6);
        int X2 = readSaturnS16(pcVar5 + 8);
        int Z2 = readSaturnS16(pcVar5 + 10);

        if ((X1 <= dragonX) && (dragonX <= X2) && (Z1 <= dragonZ) && (dragonZ <= Z2))
        {
            local_8 = pcVar5;
            if (readSaturnS8(pcVar5 + 2) == '\0' || (readSaturnS8(pcVar5 + 3) < readSaturnS8(pcVar5 + 2)))
            {
                break;
            }
        }

        local_8 = pcVar5 + 0x10;
        s32 piVar3 = readSaturnS32(pcVar5 + 0x1C);
        iVar6++;
        pcVar5 = local_8;
        if (piVar3 <= -1)
        {
            break;
        }
    } while (1);

    if (readSaturnS32(local_8 + 0xC) < 0)
    {
        noEncounterData();
        return -1;
    }

    // we have an encounter area
    if (enableDebugTask)
    {
        vdp2PrintStatus.m10_palette = 0xb000;
        vdp2DebugPrintSetPosition(1, 0xb);
        vdp2PrintfSmallFont("EA-%02d:%2d %2d %2d ", iVar6, readSaturnS8(local_8 + 0), readSaturnS8(local_8 + 2), readSaturnS8(local_8 + 3));
    }

    // setup the radar danger
    if (readSaturnS8(local_8 + 1) > -1)
    {
        fieldTaskPtr->m34_radarDangerLevel = readSaturnS8(local_8 + 1);
    }

    // if moving and there is a valid entry
    if ((pDragon->m235_dragonSpeedIndex) && (readSaturnS8(local_8 + 0) != 0))
    {
        if (readSaturnS8(local_8 + 0) < 0)
        {
            return evaluateRandomEncounterRateSub(local_8);
        }
        else
        {
            pThis->m10--;
            if (pThis->m10 < 1)
            {
                int cVar1 = readSaturnS8(local_8);
                int randomValue = performModulo2(100, randomNumber());
                if (randomValue < cVar1)
                {
                    return evaluateRandomEncounterRateSub(local_8);
                }
                pThis->mE++;
                if (pThis->mF <= pThis->mE)
                {
                    return evaluateRandomEncounterRateSub(local_8);
                }
                pThis->m10 = pThis->m16;
            }
        }
    }

    return -1;
}

void* backupGraphicsToEnterBattle(sEncouterTask* pThis, s32 param_1)
{
    Unimplemented();

    return nullptr;
}

void encounterTaskUpdate(sEncouterTask* pThis)
{
    switch (pThis->m0_status)
    {
    case 0:
    {
        sSaturnPtr pRandomEncounterData = gCommonFile->getSaturnPtr(0x208710 + fieldTaskPtr->m2C_currentFieldIndex * 0x14);
        u16 bitIndex = readSaturnU16(pRandomEncounterData + 2);
        pThis->m18 = pRandomEncounterData + 4 + (mainGameState.getBit(bitIndex) ^ 1) * 8; // contains entries controlled by the bitIndex
        pThis->mC = readSaturnS8(pRandomEncounterData + 1);
        pThis->m0_status = 1;
    }
        [[fallthrough]];
    case 1:
        battleIndex = -1;
        encounterTaskVar0 = 0;
        pThis->m0_status = 2;
        break;
    case 2:
        if (hasEncounterData == 0)
        {
            battleIndex = -1;
            noEncounterData();
        }
        else
        {
            battleIndex = evaluateRandomEncounterRate(pThis);

            if (battleIndex < 0)
                return;

            pThis->m1 = graphicEngineStatus.m40AC.m1_isMenuAllowed;
            graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;

            pThis->m4->getTask()->markPaused();

            pThis->m20 = backupGraphicsToEnterBattle(pThis, 0);

            battleLoading_InitSub0();

            pThis->mE = 0;
            pThis->m10 = pThis->m14;
            encounterTaskVar0 = 1;

            pThis->m0_status = 3;
        }
        break;
    case 3:
        pThis->m8_battleOverlay = loadBattleOverlay(pThis->mC, battleIndex);
        pThis->m0_status = 4;
        break;
    case 4: // wait for battle to finish
        if ((pThis->m8_battleOverlay) && (!pThis->m8_battleOverlay->getTask()->isFinished()))
        {
            return;
        }
        pThis->m0_status = 4;
        break;
    default:
        assert(0);
        break;
    }

}

void encounterTaskDraw(sEncouterTask* pThis)
{
    Unimplemented();
}

void encounterTaskDelete(sEncouterTask* pThis)
{
    Unimplemented();
}

void createEncounterTask(s_workArea* pWorkArea)
{
    static const sEncouterTask::TypedTaskDefinition definition =
    {
        encounterTaskInit,
        encounterTaskUpdate,
        encounterTaskDraw,
        encounterTaskDelete,
    };

    createSiblingTaskWithArg<sEncouterTask, s_workArea*>(pWorkArea, pWorkArea , &definition);
}
