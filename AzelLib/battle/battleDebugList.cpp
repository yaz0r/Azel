#include "PDS.h"
#include "battleDebugList.h"
#include "battle/battleManager.h"
#include "audio/systemSounds.h"

u32 s_battlePrgTask_var0 = 0;

struct s_battlePrgTask : public s_workAreaTemplate<s_battlePrgTask>
{
    u32 m0;
    //size 4
};

static void s_battlePrgTask_Init(s_battlePrgTask*)
{
    sBattleManager* pLoadingTask = gBattleManager;
    if (pLoadingTask->mA_pendingBattleOverlayId < 0)
    {
        pLoadingTask->mA_pendingBattleOverlayId = 0;
    }

    sSaturnPtr battleListData = gCommonFile.getSaturnPtr(0x2002bc);
    while (readSaturnU8(battleListData + pLoadingTask->mA_pendingBattleOverlayId) == 0)
    {
        pLoadingTask->mA_pendingBattleOverlayId++;
    }
    reinitVdp2();
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x3FFF7FFF;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB &= ~0x700; // that seems useless
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB |= 0x700;
    vdp2Controls.m_isDirty = true;

    g_fadeControls.m_4D = 6;
    if (g_fadeControls.m_4C < g_fadeControls.m_4D)
    {
        vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
        vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
    }

    fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0xC210, 1);
    s_battlePrgTask_var0 = 0;
    g_fadeControls.m_4D = 5;
}

static void s_battlePrgTask_Update(s_battlePrgTask* pThis)
{
    pThis->m0++;
    sBattleManager* pBattleManager = gBattleManager;
    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x80)
    {
        s_battlePrgTask_var0 = 1;
        playSystemSoundEffect(7);
    }
    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0x40)
    {
        s_battlePrgTask_var0 = 0;
        playSystemSoundEffect(7);
    }

    sSaturnPtr battleActivationList = gCommonFile.getSaturnPtr(0x2002bc);
    sSaturnPtr battleOverlaySetup = gCommonFile.getSaturnPtr(0x2005dc);

    if (s_battlePrgTask_var0 == 0)
    {
        // Select battle module
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x20)
        {
            FunctionUnimplemented();
        }
        if(graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x10)
        {
            FunctionUnimplemented();
        }
    }
    else
    {
        // Select sub battle
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x20)
        {
            pBattleManager->m6_subBattleId++;
            if (readSaturnU32(battleOverlaySetup + pBattleManager->mA_pendingBattleOverlayId * 0x14 + 0xC) <= pBattleManager->m6_subBattleId)
            {
                pBattleManager->m6_subBattleId = 0;
            }
        }
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x10)
        {

        }

        FunctionUnimplemented();
    }

    int printY = 0x4;
    for (int i = 0; i < 0x1B; i++)
    {
        vdp2DebugPrintSetPosition(10, printY);
        if (pBattleManager->mA_pendingBattleOverlayId == i)
        {
            vdp2PrintStatus.m10_palette = 0xD000; // This was programmatic, but can only be a single value
        }
        else
        {
            vdp2PrintStatus.m10_palette = 0x8000;
        }

        if (readSaturnU8(battleActivationList + i) != 0)
        {
            vdp2DebugPrintNewLine(readSaturnString(readSaturnEA(battleOverlaySetup + i * 0x14)));
            if ((s_battlePrgTask_var0 == 0) && (pBattleManager->mA_pendingBattleOverlayId == i))
            {
                vdp2PrintStatus.m10_palette = 0xD000;
                vdp2DebugPrintSetPosition(10, printY);
                vdp2DebugPrintNewLine("\x7F");
            }
            printY++;
        }
    }

    int numSubBattle = readSaturnS32(battleOverlaySetup + (pBattleManager->mA_pendingBattleOverlayId * 0x14) + 0xC);
    sSaturnPtr subBattleTable = readSaturnEA(battleOverlaySetup + (pBattleManager->mA_pendingBattleOverlayId * 0x14) + 0x10);
    for (int i = 0; i < numSubBattle; i++)
    {
        vdp2DebugPrintSetPosition(0x12, i + 4);
        vdp2PrintStatus.m10_palette = 0x9000;
        if (pBattleManager->m6_subBattleId != i)
        {
            vdp2PrintStatus.m10_palette = 0x8000;
        }

        vdp2DebugPrintNewLine(readSaturnString(readSaturnEA(subBattleTable + i * 4)));
    }

    if (s_battlePrgTask_var0 != 0)
    {
        vdp2PrintStatus.m10_palette = 0x9000;
        vdp2DebugPrintSetPosition(0x12, pBattleManager->m6_subBattleId + 4);
        vdp2DebugPrintNewLine("\x7F");
    }

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0xE)
    {
        playSystemSoundEffect(0xC);
        startDebugBattle(pBattleManager->mA_pendingBattleOverlayId);
    }
}

void terminateCurrentBattle(p_workArea r4)
{
    if (r4)
    {
        r4->getTask()->markFinished();
    }
}

p_workArea startBattlePrgTask(p_workArea parentTask)
{
    static const s_battlePrgTask::TypedTaskDefinition definition = {
        &s_battlePrgTask_Init,
        &s_battlePrgTask_Update,
        nullptr,
        nullptr,
    };
    return createSubTask<s_battlePrgTask>(parentTask, &definition);
}
