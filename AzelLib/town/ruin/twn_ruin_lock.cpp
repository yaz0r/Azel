#include "PDS.h"
#include "town/town.h"
#include "town/townScript.h"
#include "twn_ruin.h"
#include "twn_ruin_lock.h"
#include "kernel/fileBundle.h"
#include "audio/systemSounds.h"
#include "town/collisionRegistry.h"

void sLockTask::Init(sLockTask* pThis, sSaturnPtr r5)
{
    pThis->mC = r5;
    pThis->m74_translation = readSaturnVec3(r5 + 8);
    pThis->m80_rotation = readSaturnVec3(r5 + 0x14);
    pThis->m8E_translationLength = 0x32;
    npcData0.m70_npcPointerArray[readSaturnU16(r5 + 0x28)].workArea = pThis;
}

void sLockTask::Update(sLockTask* pThis)
{
    if (!isDataLoaded(readSaturnU32(pThis->mC)))
        return;

    pThis->m10.m30_pPosition = &pThis->m74_translation;
    pThis->m10.m34_pRotation = &pThis->m80_rotation;
    pThis->m10.m38_pOwner = pThis;

    pThis->m10.m3C_scriptEA = readSaturnEA(readSaturnEA(pThis->mC + 0x24) + 4);

    if (s16 r4 = readSaturnU16(readSaturnEA(pThis->mC + 0x24) + 2))
    {
        pThis->m10.m40 = pThis->m0_fileBundle->getRawFileAtOffset(r4);
    }
    else
    {
        pThis->m10.m40 = 0;
    }

    sSaturnPtr configEA = readSaturnEA(pThis->mC + 0x24);

    setCollisionSetup(&pThis->m10, readSaturnU8(configEA));
    setCollisionBounds(&pThis->m10, readSaturnVec3(configEA + 8), readSaturnVec3(configEA + 0x14));
    pThis->m_UpdateMethod = &sLockTask::UpdateAlternate;
    pThis->m_DrawMethod = &sLockTask::Draw;
}

void sLockTask::UpdateAlternate(sLockTask* pThis)
{
    switch (pThis->m8C_status)
    {
    case 0: // noting
        break;
    case 1: // moving down
        pThis->m74_translation[1] -= 0x7A;
        pThis->m8E_translationLength--;
        if (pThis->m8E_translationLength == 0)
        {
            pThis->m8C_status = 2;
        }
        break;
    case 2: // unlocked
        removeNPC(pThis, pThis, pThis->mC);
        break;
    default:
        assert(0);
        break;
    }
    registerCollisionBody(&pThis->m10);
}

void sLockTask::Draw(sLockTask* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m74_translation);
    rotateCurrentMatrixZYX(pThis->m80_rotation);
    addObjectToDrawList(pThis->m0_fileBundle->get3DModel(readSaturnU16(pThis->mC + 0x20)));
    popMatrix();
}

void sLockTask::Delete(sLockTask* pThis)
{
    Unimplemented();
}

s32 scriptFunction_6054334_disableLock(s32 arg0, s32 arg1)
{
    p_workArea pTask = getNpcDataByIndexAsTask(arg0);
    if (pTask)
    {
        sLockTask* pLockTask = pTask->castTo<sLockTask>();
        pLockTask->m8C_status = arg1;
        if (arg1 == 1)
        {
            playSystemSoundEffect(101);
        }
    }
    return 0;
}

s32 scriptFunction_6054364_waitForLockDisableCompletion(s32 arg0)
{
    p_workArea pTask = getNpcDataByIndexAsTask(arg0);
    if (pTask)
    {
        sLockTask* pLockTask = pTask->castTo<sLockTask>();
        if (pLockTask->m8C_status != 2)
        {
            return 0;
        }
    }
    return 1;
}

