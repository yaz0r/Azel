#include "PDS.h"
#include "battleIntro.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDragon.h"
#include "kernel/fileBundle.h"

#include "town/town.h" // TODO: remove

struct sBattleIntroSubTask : public s_workAreaTemplate<sBattleIntroSubTask>
{
    s16 m0_frameIndex;
    sVec3_FP m4;
    sVec3_FP m10;
    u8* m1C_pCameraData;
    u8* m20;
    s8 m24_interpolationType;
    s8 m25;
    s32 m28;
    npcFileDeleter* m2C;
    // size 0x30
};

static void sBattleIntroSubTask_delete(sBattleIntroSubTask* pThis)
{
    FunctionUnimplemented();
}

void createBattleIntroTaskSub0()
{
    getBattleManager()->m10_battleOverlay->m4_battleEngine->m188_flags |= 0x800;
}

s32 createBattleIntroTaskSub1()
{
    s8 bVar1 = getBattleManager()->m10_battleOverlay->m4_battleEngine->m230;

    if ((((bVar1 != 1) && (bVar1 != 3)) && (bVar1 != 5)) &&
        (((bVar1 != 7 && (bVar1 != 8)) && ((bVar1 != 9 && (bVar1 != 10)))))) {
        return 0;
    }
    return 1;
}

void createBattleIntroTaskSub2(sBattleIntroSubTask* pThis)
{
    sVec3_FP localVector;

    u8* cameraData = pThis->m1C_pCameraData + (s64)pThis->m0_frameIndex * 6;

    s16 rawCameraData[3];
    rawCameraData[0] = READ_BE_S16(cameraData + 0) * 32;
    rawCameraData[1] = READ_BE_S16(cameraData + 2) * 32;
    rawCameraData[2] = READ_BE_S16(cameraData + 4) * 32;

    switch (pThis->m25)
    {
    case 0:
        localVector[0] = rawCameraData[2];
        localVector[1] = rawCameraData[1];
        localVector[2] = -rawCameraData[0];
        break;
    case 1:
        localVector[2] = rawCameraData[2];
        localVector[1] = rawCameraData[1];
        localVector[0] = -rawCameraData[0];
        break;
    case 2:
        localVector[0] = rawCameraData[2];
        localVector[1] = rawCameraData[1];
        localVector[2] = rawCameraData[0];
        break;
    case 3:
        localVector[2] = rawCameraData[2];
        localVector[1] = rawCameraData[1];
        localVector[0] = rawCameraData[0];
        break;
    default:
        assert(0);
    }

    switch (pThis->m28)
    {
    case 0:
    case 2:
        localVector[0] = -localVector[0];
        break;
    case 1:
    case 3:
        localVector[2] = -localVector[2];
        break;
    default:
        assert(0);
    }

    pThis->m4 = getBattleManager()->m10_battleOverlay->m18_dragon->m8_position + getBattleManager()->m10_battleOverlay->m4_battleEngine->m1A0 + localVector;
}

void createBattleIntroTaskSub3(sBattleIntroSubTask* pThis)
{
    sVec3_FP temp = getBattleManager()->m10_battleOverlay->m18_dragon->m8_position + getBattleManager()->m10_battleOverlay->m4_battleEngine->m1A0;

    switch (pThis->m24_interpolationType)
    {
    case 0:
        pThis->m10[0] = getBattleManager()->m10_battleOverlay->m4_battleEngine->mC[0] + MTH_Mul(READ_BE_S16(pThis->m20 + pThis->m0_frameIndex * 6 + 4) * 256, temp[0] - getBattleManager()->m10_battleOverlay->m4_battleEngine->mC[0]);
        pThis->m10[1] = getBattleManager()->m10_battleOverlay->m4_battleEngine->mC[1] + MTH_Mul(READ_BE_S16(pThis->m20 + pThis->m0_frameIndex * 6 + 4) * 256, temp[1] - getBattleManager()->m10_battleOverlay->m4_battleEngine->mC[1]);
        pThis->m10[2] = getBattleManager()->m10_battleOverlay->m4_battleEngine->mC[2] + MTH_Mul(READ_BE_S16(pThis->m20 + pThis->m0_frameIndex * 6 + 4) * 256, temp[1] - getBattleManager()->m10_battleOverlay->m4_battleEngine->mC[2]);
        break;
    case 1:
        pThis->m10 = temp + MTH_Mul(FP_Div(pThis->m0_frameIndex * 512, 0x960000), getBattleManager()->m10_battleOverlay->m4_battleEngine->mC - temp);
        break;
    case 2:
        pThis->m10 = getBattleManager()->m10_battleOverlay->m4_battleEngine->mC;
        break;
    default:
        assert(0);
    }
}

static void sBattleIntroSubTask_update(sBattleIntroSubTask* pThis)
{
    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m6_buttonDown & 0x1000)
    {
        // skip
        assert(0);
    }

    createBattleIntroTaskSub2(pThis);
    createBattleIntroTaskSub3(pThis);
    if (keyboardIsKeyDown(0xBA))
    {
        assert(0);
    }

    pThis->m0_frameIndex++;
    if (pThis->m0_frameIndex > 0x95)
    {
        getBattleManager()->m10_battleOverlay->m4_battleEngine->m3D0 = nullptr;
        pThis->getTask()->markFinished();
    }
}

p_workArea createBattleIntroTask(p_workArea parent)
{
    static const sBattleIntroSubTask::TypedTaskDefinition definition =
    {
        nullptr,
        &sBattleIntroSubTask_update,
        nullptr,
        &sBattleIntroSubTask_delete,
    };

    sBattleIntroSubTask* pNewTask = createSubTask<sBattleIntroSubTask>(parent, &definition);

    pNewTask->m2C = dramAllocatorEnd[4].mC_buffer;
    createBattleIntroTaskSub0();

    int randomIntroId = performModulo2(3, randomNumber());

    pNewTask->m1C_pCameraData = pNewTask->m2C->m0_dramAllocation->getRawBuffer() + 0x384 * randomIntroId;
    pNewTask->m20 = pNewTask->m2C->m0_dramAllocation->getRawBuffer() + 0x384 * 3;

    if (createBattleIntroTaskSub1() == 0)
    {
        pNewTask->m24_interpolationType = randomNumber() & 1;
    }
    else
    {
        pNewTask->m24_interpolationType = 1;
    }

    pNewTask->m25 = getBattleManager()->m10_battleOverlay->m4_battleEngine->m22C_battleDirection;
    pNewTask->m28 = (randomNumber() & 1) != 0;
    pNewTask->m0_frameIndex = 0;

    createBattleIntroTaskSub2(pNewTask);
    createBattleIntroTaskSub3(pNewTask);
    battleEngine_InitSub6(&pNewTask->m4);
    battleEngine_InitSub7(&pNewTask->m10);
    battleEngine_InitSub8();

    return pNewTask;
}
