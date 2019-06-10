#include "PDS.h"
#include "town.h"
#include "town/ruin/twn_ruin.h"

u8 townBuffer[0xB0000];

townDebugTask2Function* townDebugTask2 = nullptr;

void townDebugTask2Function::Update(townDebugTask2Function* pThis)
{
    if (readKeyboardToggle(0x87))
    {
        assert(0);
    }
}

sNpcData npcData0;

void loadTownPrgSub0()
{
    TaskUnimplemented();
}

void setupDragonForTown(u8* r4)
{
    if (READ_BE_U32(r4 + 0x48))
    {
        TaskUnimplemented();
    }
}

void loadTownPrg(s8 r4, s8 r5)
{
    npcData0.m5C = r4;
    npcData0.m5D = r5;

    mainGameState.setPackedBits(0, 2, 2);

    std::string overlayFileName = readSaturnString(readSaturnEA(gCommonFile.getSaturnPtr(0x002165D8 + r4 * 4 * 4)));

    if (overlayFileName == "TWN_RUIN.PRG")
    {
        gFieldOverlayFunction = overlayStart_TWN_RUIN;
    }
    else
    {
        assert(0);
    }

    pauseEngine[2] = 1;
    resetCameraProperties2(&cameraProperties2);
    reset3dEngine();
    resetTempAllocators();
    loadTownPrgSub0();
    setupDragonForTown(gDragonState->m0_pDragonModelRawData);
    gFieldOverlayFunction(townDebugTask2, r5);
}

p_workArea loadTown(p_workArea r4, s32 r5)
{
    townDebugTask2 = createSubTaskFromFunction<townDebugTask2Function>(r4, &townDebugTask2Function::Update);

    loadTownPrg(readSaturnS8(gCommonFile.getSaturnPtr(0x2166E4 + r5 * 2 + 0)), readSaturnS8(gCommonFile.getSaturnPtr(0x2166E5 + r5 * 2 + 1)));

    return townDebugTask2;
}

void startScriptTask(p_workArea r4)
{
    TaskUnimplemented();
}

void mainLogicInitSub0(sMainLogic_74* r4, s32 r5)
{
    r4->m2C = r5;
    r4->m0 = readSaturnS8(gCommonFile.getSaturnPtr(0x201BB8 + 4 * r5));
    r4->m1 = readSaturnS8(gCommonFile.getSaturnPtr(0x201BB8 + 4 * r5 + 1));
    r4->m2 = readSaturnS8(gCommonFile.getSaturnPtr(0x201BB8 + 4 * r5 + 2));
}
void mainLogicInitSub1(sMainLogic_74* r4, sSaturnPtr r5, sSaturnPtr r6)
{
    r4->m20[0] = (readSaturnS32(r5) - readSaturnS32(r6)) / 2;
    r4->m20[1] = (readSaturnS32(r5 + 4) - readSaturnS32(r6 + 4)) / 2;
    r4->m20[2] = (readSaturnS32(r5 + 8) - readSaturnS32(r6 + 8)) / 2;

    if (readSaturnS32(r6) > readSaturnS32(r5))
    {
        r4->m14[0] = r4->m20[0] - readSaturnS32(r5);
    }
    else
    {
        r4->m14[0] = r4->m20[0] - readSaturnS32(r6);
    }

    if (readSaturnS32(r6 + 4) > readSaturnS32(r5 + 4))
    {
        r4->m14[1] = r4->m20[1] - readSaturnS32(r5 + 4);
    }
    else
    {
        r4->m14[1] = r4->m20[1] - readSaturnS32(r6 + 4);
    }

    if (readSaturnS32(r6 + 8) > readSaturnS32(r5 + 8))
    {
        r4->m14[2] = r4->m20[2] - readSaturnS32(r5 + 8);
    }
    else
    {
        r4->m14[2] = r4->m20[2] - readSaturnS32(r6 + 8);
    }

    r4->m4 = sqrt_F(MTH_Product3d_FP(r4->m14, r4->m14));
}

