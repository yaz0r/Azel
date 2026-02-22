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
    {
        return;
    }

    if (isCurrentBattleID(0, 8))
    {
        mainGameState.setBit(0x11 * 8 + 4); // Baldor queen mini boss
    }
    else if (isCurrentBattleID(0, 9))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0x10, 0)) // Arachnoth boss fight
    {
        mainGameState.setBit(0x11 * 8 + 6);
    }
    else if (isCurrentBattleID(0x12, 0))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0x11, 0))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0x13, 0))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xe, 0))
    {
        assert(0);
    }
    else if (isCurrentBattleID(4, 0))
    {
        assert(0);
    }
    else if (isCurrentBattleID(5, 4))
    {
        assert(0);
    }
    else if (isCurrentBattleID(5, 5))
    {
        assert(0);
    }
    else if (isCurrentBattleID(5, 8))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0x14, 0))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xe, 1))
    {
        assert(0);
    }
    else if (isCurrentBattleID(6, 0xc))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0x16, 0))
    {
        assert(0);
    }
    else if (isCurrentBattleID(7, 5))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0x17, 0))
    {
        assert(0);
    }
    else if (isCurrentBattleID(8, 5))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xe, 2))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0x18, 0))
    {
        assert(0);
    }
    else if (isCurrentBattleID(9, 4))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0x19, 0))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xb, 0))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xb, 1))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xb, 2))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xb, 3))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xb, 4))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xb, 5))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xb, 6))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xb, 7))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0x1A, 0))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0x6, 0xD))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xC, 6))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xC, 7))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xD, 0))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xD, 1))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xD, 2))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xD, 3))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xD, 4))
    {
        assert(0);
    }
    else if (isCurrentBattleID(0xD, 5))
    {
        assert(0);
    }
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

