#include "PDS.h"
#include "battleEnd.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"

bool isCurrentBattleID(s8 param1, s8 param2); // TODO: cleanup

struct sBattleEnd : public s_workAreaTemplate<sBattleEnd>
{
    //size 0xC
};

void BattleEnd_update(sBattleEnd* pThis)
{
    if (g_fadeControls.m0_fade0.m20_stopped)
    {
        pThis->getTask()->markFinished();
    }
}

void BattleEnd_deleteSub0()
{
    if (gBattleManager->m0_status == 3)
    {
        gBattleManager->m0_status = 4;
    }
}

// BTL_A3::0605fc74
static void FUN_BTL_A3__0605fc74()
{
    if (isCurrentBattleID(0, 8)) {
        mainGameState.setBit(0x11, 4);
    } else if (isCurrentBattleID(0, 9)) {
        mainGameState.setBit(0x11, 2);
    } else if (isCurrentBattleID(0x10, 0)) {
        mainGameState.setBit(0x11, 1);
    } else if (isCurrentBattleID(0x12, 0)) {
        mainGameState.setBit(0x12, 0x80);
    } else if (isCurrentBattleID(0x11, 0)) {
        mainGameState.setBit(0x12, 0x40);
    } else if (isCurrentBattleID(0x13, 0)) {
        mainGameState.setBit(0x12, 0x20);
    } else if (isCurrentBattleID(0xe, 0)) {
        mainGameState.setBit(0x12, 0x10);
    } else if (isCurrentBattleID(4, 0)) {
        mainGameState.setBit(0x12, 8);
        mainGameState.setBit(0x12, 4);
    } else if (isCurrentBattleID(5, 4)) {
        mainGameState.setBit(0x12, 2);
    } else if (isCurrentBattleID(5, 0)) {
        mainGameState.setBit(0x12, 1);
    } else if (isCurrentBattleID(5, 8)) {
        mainGameState.setBit(0x13, 0x80);
    } else if (isCurrentBattleID(0x14, 0)) {
        mainGameState.setBit(0x13, 0x40);
    } else if (isCurrentBattleID(0xe, 1)) {
        mainGameState.setBit(0x13, 0x20);
        gBattleManager->m10_battleOverlay->m3 = 0;
        return;
    } else if (isCurrentBattleID(6, 0xc)) {
        mainGameState.setBit(0x13, 0x10);
    } else if (isCurrentBattleID(0x16, 0)) {
        mainGameState.setBit(0x13, 8);
    } else if (isCurrentBattleID(7, 5)) {
        mainGameState.setBit(0x13, 4);
    } else if (isCurrentBattleID(0x17, 0)) {
        mainGameState.setBit(0x13, 2);
    } else if (isCurrentBattleID(8, 5)) {
        mainGameState.setBit(0x13, 1);
    } else if (isCurrentBattleID(0xe, 2)) {
        mainGameState.setBit(0x14, 0x80);
    } else if (isCurrentBattleID(0x18, 0)) {
        mainGameState.setBit(0x14, 0x40);
    } else if (isCurrentBattleID(9, 4)) {
        mainGameState.setBit(0x14, 0x20);
        gBattleManager->m10_battleOverlay->m3 = 1;
        return;
    } else if (isCurrentBattleID(0x19, 0)) {
        mainGameState.setBit(0x14, 0x10);
    } else if (isCurrentBattleID(0xb, 0)) {
        mainGameState.setBit(0x14, 8);
    } else if (isCurrentBattleID(0xb, 1)) {
        mainGameState.setBit(0x14, 4);
    } else if (isCurrentBattleID(0xb, 2)) {
        mainGameState.setBit(0x14, 2);
    } else if (isCurrentBattleID(0xb, 3)) {
        mainGameState.setBit(0x14, 1);
    } else if (isCurrentBattleID(0xb, 4)) {
        mainGameState.setBit(0x15, 0x80);
    } else if (isCurrentBattleID(0xb, 5)) {
        mainGameState.setBit(0x15, 0x40);
    } else if (isCurrentBattleID(0xb, 6)) {
        mainGameState.setBit(0x15, 0x20);
    } else if (isCurrentBattleID(0xb, 7)) {
        mainGameState.setBit(0x15, 0x10);
    } else if (isCurrentBattleID(0x1a, 0)) {
        mainGameState.setBit(0x15, 8);
    } else if (isCurrentBattleID(6, 0xd)) {
        mainGameState.setBit(0x15, 4);
    } else if (isCurrentBattleID(0xc, 6)) {
        mainGameState.setBit(0x15, 2);
    } else if (isCurrentBattleID(0xc, 7)) {
        mainGameState.setBit(0x15, 1);
    } else if (isCurrentBattleID(0xd, 0)) {
        mainGameState.setBit(0x16, 0x80);
    } else if (isCurrentBattleID(0xd, 1)) {
        mainGameState.setBit(0x16, 0x40);
    } else if (isCurrentBattleID(0xd, 2)) {
        mainGameState.setBit(0x16, 0x20);
    } else if (isCurrentBattleID(0xd, 3)) {
        mainGameState.setBit(0x16, 0x10);
    } else if (isCurrentBattleID(0xd, 4)) {
        mainGameState.setBit(0x16, 8);
    } else if (isCurrentBattleID(0xd, 5)) {
        mainGameState.setBit(0x16, 4);
        mainGameState.setBit(0x16, 2);
    }
}

void BattleEnd_delete(sBattleEnd* pThis)
{
    if (!isCurrentBattleID(9, 0))
    {
        // setup back screen color
        *(u16*)getVdp2Vram(0x2A600) = 0x8000;
        vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;
    }

    BattleEnd_deleteSub0();

    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m10000)
        return;

    FUN_BTL_A3__0605fc74();
}

void createBattleEndTask(p_workArea parent, int param)
{
    sBattleEnd::TypedTaskDefinition definition = {
        nullptr,
        BattleEnd_update,
        nullptr,
        BattleEnd_delete,
    };
    sBattleEnd* pThis = createSiblingTask<sBattleEnd>(parent, &definition);

    if (isCurrentBattleID(9, 0))
    {
        pThis->getTask()->markFinished();
    }

    if (g_fadeControls.m_4C <= g_fadeControls.m_4D)
    {
        vdp2Controls.m20_registers[1].m112_CLOFSL = 8;
        vdp2Controls.m20_registers[0].m112_CLOFSL = 8;
    }

    if (param)
    {
        fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), 0xFFFF, 0x1E);
    }
    else
    {
        fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), 0x8000, 0x1E);
    }
}

