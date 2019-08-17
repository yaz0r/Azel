#include "PDS.h"
#include "town/town.h"
#include "twn_ruin.h"
#include "twn_ruin_lock.h"
#include "kernel/fileBundle.h"

void sLockTask::Init(sLockTask* pThis, sSaturnPtr r5)
{
    pThis->mC = r5;
    pThis->m74_translation = readSaturnVec3(r5 + 8);
    pThis->m80_rotation = readSaturnVec3(r5 + 0x14);
    pThis->m8E = 0x32;
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
        pThis->m10.m40 = pThis->m0_dramAllocation->getRawFileAtOffset(r4);
    }
    else
    {
        pThis->m10.m40 = 0;
    }

    sSaturnPtr configEA = readSaturnEA(pThis->mC + 0x24);

    mainLogicInitSub0(&pThis->m10, readSaturnU8(configEA));
    mainLogicInitSub1(&pThis->m10, readSaturnVec3(configEA + 8), readSaturnVec3(configEA + 0x14));
    pThis->m_UpdateMethod = &sLockTask::UpdateAlternate;
    pThis->m_DrawMethod = &sLockTask::Draw;
}

void sLockTask::UpdateAlternate(sLockTask* pThis)
{
    switch (pThis->m8C_status)
    {
    case 0:
        break;
    default:
        assert(0);
        break;
    }
    EdgeUpdateSub0(&pThis->m10);
}

void sLockTask::Draw(sLockTask* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->m74_translation);
    rotateCurrentMatrixZYX(pThis->m80_rotation);
    addObjectToDrawList(pThis->m0_dramAllocation->get3DModel(readSaturnU16(pThis->mC + 0x20)));
    popMatrix();
}

void sLockTask::Delete(sLockTask* pThis)
{
    FunctionUnimplemented();
}

