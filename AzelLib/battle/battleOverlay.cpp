#include "PDS.h"
#include "battleOverlay.h"
#include "battleManager.h"
#include "BTL_A3/BTL_A3.h"
#include "audio/systemSounds.h"

std::string sBattleOverlayName = "";

battleOverlay* gCurrentBattleOverlay = nullptr;

const std::vector<sF0Color> battleOverlay::m60AE424 = {{0xFE00, 0xC208, 0xC208, 0xFE00}};
const std::vector<sF0Color> battleOverlay::m60AE42C;
const std::vector<sF0Color> battleOverlay::m60AE434;
const std::vector<sF0Color> battleOverlay::m60AE43C;


static void battleOverlayTask_Init(sBattleOverlayTask* pThis)
{
    gBattleManager->m10_battleOverlay = pThis;
    mainGameState.setPackedBits(0, 2, 1);

    fadePalette(&g_fadeControls.m0_fade0, convertColorToU32(g_fadeControls.m0_fade0.m0_color), -1, 0x3C);
    fadePalette(&g_fadeControls.m24_fade1, convertColorToU32(g_fadeControls.m24_fade1.m0_color), -1, 0x3C);

    playSystemSoundEffect(0x10);
}

extern s32 e006Task0Var0;

static void battleOverlayTask_Update(sBattleOverlayTask* pThis)
{
    pThis->m24++;
    if (readKeyboardToggle(0x87) && (e006Task0Var0 & 4))
    {
        assert(0);
    }
}

static void battleOverlayTask_Draw(sBattleOverlayTask* pThis)
{
    sSaturnPtr battleOverlaySetup = gCommonFile.getSaturnPtr(0x2005dc);

    switch (pThis->m0)
    {
    case 0:
        graphicEngineStatus.m4 = 1;
        pThis->m0++;
        break;
    case 1:
        sBattleOverlayName = readSaturnString(readSaturnEA(battleOverlaySetup + gBattleManager->m2_currentBattleOverlayId * 0x14 + 4));
        loadFnt("ITEM.FNT");
        pThis->m2++;
        {
            std::string customFontName = readSaturnString(readSaturnEA(battleOverlaySetup + gBattleManager->m2_currentBattleOverlayId * 0x14 + 8));
            loadFnt(customFontName.c_str());
            pThis->m2++;
            if (sBattleOverlayName == std::string("BTL_A3.PRG"))
            {
                overlayStart_BTL_A3(pThis);
            }
            else
            {
                assert(0);
            }
            pThis->m0++;
        }
    case 2: // running
        break;
    default:
        assert(0);
        break;
    }
}

static void battleOverlayTask_Delete(sBattleOverlayTask* pThis)
{
    FunctionUnimplemented();
}

p_workArea createBattleOverlayTask(sBattleManager* pParent)
{
    pauseEngine[2] = 1;
    graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
    
    static const sBattleOverlayTask::TypedTaskDefinition definition = {
        &battleOverlayTask_Init,
        &battleOverlayTask_Update,
        &battleOverlayTask_Draw,
        &battleOverlayTask_Delete,
    };

    sBattleOverlayTask* pBattleOverlayTask = createSubTask<sBattleOverlayTask>(pParent, &definition);

    if (pBattleOverlayTask)
    {
        pParent->m0_status = 3;
    }

    return pBattleOverlayTask;
}
