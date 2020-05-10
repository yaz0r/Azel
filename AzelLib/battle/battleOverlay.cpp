#include "PDS.h"
#include "battleOverlay.h"
#include "battleManager.h"
#include "BTL_A3/BTL_A3.h"
#include "audio/systemSounds.h"
#include "commonOverlay.h"

std::string sBattleOverlayName = "";

battleOverlay* gCurrentBattleOverlay = nullptr;

const std::vector<quadColor> battleOverlay::m60AE424 = { {0xFE00, 0xC208, 0xC208, 0xFE00} };
const std::vector<quadColor> battleOverlay::m60AE42C = { {0x8E1F, 0x8F1F, 0x8F1F, 0x8F1F} };
const std::vector<quadColor> battleOverlay::m60AE434 = { {0x83EE, 0x93EE, 0x93EE, 0x83EE} };
const std::vector<quadColor> battleOverlay::m60AE43C = { {0x83FF, 0x83FF, 0x83FF, 0x83FF} };
sLaserData battleOverlay::mLaserData;

static void battleOverlayTask_Init(sBattleOverlayTask* pThis)
{
    gBattleManager->m10_battleOverlay = pThis;
    mainGameState.setPackedBits(0, 2, 1);

    fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), -1, 0x3C);
    fadePalette(&g_fadeControls.m24_fade1, convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color), -1, 0x3C);

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
    switch (pThis->m0)
    {
    case 0:
        graphicEngineStatus.m4 = 1;
        pThis->m0++;
        break;
    case 1:
        sBattleOverlayName = gCommonFile.battleOverlaySetup[gBattleManager->m2_currentBattleOverlayId].m4_prg;
        loadFnt("ITEM.FNT");
        pThis->m2++;
        {
            std::string customFontName = gCommonFile.battleOverlaySetup[gBattleManager->m2_currentBattleOverlayId].m8_fnt;
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
