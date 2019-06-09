#include "PDS.h"
#include "town.h"
#include "town/ruin/twn_ruin.h"

townDebugTask2Function* townDebugTask2 = nullptr;

void townDebugTask2Function::Update(townDebugTask2Function* pThis)
{
    if (readKeyboardToggle(0x87))
    {
        assert(0);
    }
}

struct sNpcData
{
    s8 m5C;
    s8 m5D;
};

sNpcData npcData0;

void loadTownPrgSub0()
{
    TaskUnimplemented();
}

void setupDragonForTown(s_dragonState* r4)
{
    TaskUnimplemented();
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
    setupDragonForTown(gDragonState);
    gFieldOverlayFunction(townDebugTask2, r5);
}

p_workArea loadTown(p_workArea r4, s32 r5)
{
    townDebugTask2 = createSubTaskFromFunction<townDebugTask2Function>(r4, &townDebugTask2Function::Update);

    loadTownPrg(readSaturnS8(gCommonFile.getSaturnPtr(0x2166E4 + r5 * 2 + 0)), readSaturnS8(gCommonFile.getSaturnPtr(0x2166E5 + r5 * 2 + 1)));

    return townDebugTask2;
}


