#include "PDS.h"
#include "battleOverlay.h"
#include "battleManager.h"
#include "BTL_A3/BTL_A3.h"
#include "BTL_A3_2/BTL_A3_2.h"
#include "audio/systemSounds.h"
#include "commonOverlay.h"

void unloadFnt(); // TODO: clean

std::string sBattleOverlayName = "";

battleOverlay* gCurrentBattleOverlay = nullptr;

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
        sBattleOverlayName = gCommonFile->battleOverlaySetup[gBattleManager->m2_currentBattleOverlayId].m4_prg;
        loadFnt("ITEM.FNT");
        pThis->m2_numLoadedFnt++;
        {
            std::string customFontName = gCommonFile->battleOverlaySetup[gBattleManager->m2_currentBattleOverlayId].m8_fnt;
            loadFnt(customFontName.c_str());
            pThis->m2_numLoadedFnt++;
            if (sBattleOverlayName == std::string("BTL_A3.PRG"))
            {
                overlayStart_BTL_A3(pThis);
            }
            else if (sBattleOverlayName == std::string("BTL_A3_2.PRG"))
            {
                overlayStart_BTL_A3_2(pThis);
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
    while (pThis->m2_numLoadedFnt--)
    {
        unloadFnt();
    }
    if (pThis->m3)
    {
        titleScreenDrawSub3(1);
    }

    freeRamResources(gBattleManager->m10_battleOverlay);
    vdp1FreeLastAllocation(gBattleManager->m10_battleOverlay);
    gBattleManager->m10_battleOverlay = nullptr;
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

p_workArea loadBattleOverlay(s32 battleId, s32 subBattleId)
{
    gBattleManager->m2_currentBattleOverlayId = battleId;
    gBattleManager->m6_subBattleId = subBattleId;
    loadBattleOverlaySub0(gBattleManager);
    if (gCommonFile->battleOverlaySetup[gBattleManager->m2_currentBattleOverlayId].m4_prg == "")
    {
        return nullptr;
    }

    return createBattleOverlayTask(gBattleManager);
}

