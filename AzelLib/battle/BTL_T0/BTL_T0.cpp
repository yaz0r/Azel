#include "PDS.h"
#include "BTL_T0.h"
#include "battle/battleMainTask.h"
#include "battle/battleOverlay.h"
#include "battle/battleManager.h"
#include "battle/battleEngine.h"
#include "battle/battleTextDisplay.h"
#include "battle/battleHud.h"
#include "battle/battleCommandMenu.h"
#include "battle/battleEngineSub0.h"
#include "battle/BTL_A3/BTL_A3.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "battle/BTL_A3/BTL_A3_UrchinFormation.h"
#include "battle/BTL_A3/BTL_A3_data.h"
#include "battle/BTL_A3/baldor.h"
#include "battle/battleFormation.h"
#include "battle/battleGrid.h"
#include "kernel/grid.h"
#include "field/field_a3/o_fld_a3.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "kernel/graphicalObject.h"
#include "kernel/fileBundle.h"
#include "battle/battleDebug.h"
#include "battle/battleDragon.h"
#include "battle/battleTargetable.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/loadSavegameScreen.h"

void BattleCommandMenu_UpdateSub0();
void fieldPaletteTaskInitSub0Sub0();

struct BTL_T0_data;
static BTL_T0_data* g_BTL_T0 = nullptr;

static p_workArea Create_BTL_T0_env(p_workArea parent);
static p_workArea Create_BTL_T0_Formation(p_workArea parent, u32 existingFormation);

// Forward declarations
static int BTL_T0_isBattleModeReady();

// 0605d56c
static int BTL_T0_isAttackSeqActive()
{
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m80)
        return 1;
    return 0;
}

// 0606df92
static void BTL_T0_restoreAutoScrollDelta(s_battleEngine* pEngine)
{
    pEngine->m1A0_battleAutoScrollDelta.m0_X = pEngine->m1AC_battleAutoScrollDeltaBackup.m0_X;
    pEngine->m1A0_battleAutoScrollDelta.m4_Y = pEngine->m1AC_battleAutoScrollDeltaBackup.m4_Y;
    pEngine->m1A0_battleAutoScrollDelta.m8_Z = pEngine->m1AC_battleAutoScrollDeltaBackup.m8_Z;
}

// 0605d3c0
static void BTL_T0_beginAttackSequence(u8 param_1)
{
    s_battleEngine* psVar2 = gBattleManager->m10_battleOverlay->m4_battleEngine;
    psVar2->m38C_battleMode = (eBattleModes)param_1;
    s_battleEngine* psVar1 = gBattleManager->m10_battleOverlay->m4_battleEngine;
    psVar1->m188_flags.m80 = 1;
    psVar1->m188_flags.m100_attackAnimationFinished = 0;
    psVar1->m188_flags.m2000 = 0;
    psVar1->m188_flags.m10 = 1;
    psVar1->m188_flags.m20_battleIntroRunning = 1;
    psVar1->m188_flags.m200_suppressBattleInputs = 1;
    psVar2->m38D_battleSubMode = 0;
    psVar2->m384_battleModeDelay = 0;
    psVar2->m386 = 0;
    psVar2->m184 = 0;
    psVar2->m3E8.m0_X = 0;
    psVar2->m3E8.m4_Y = 0;
    psVar2->m3E8.m8_Z = 0;
    BTL_T0_restoreAutoScrollDelta(psVar2);
}

// 0605d39e
static void BTL_T0_triggerAttackCamera(s8 param_1)
{
    BTL_T0_beginAttackSequence(8);
    gBattleManager->m10_battleOverlay->m4_battleEngine->m433_attackCameraIndex = param_1;
}

// 0606c472
static void BTL_T0_displayBottomScreenString(u16 param_1)
{
    sBattleTextDisplayTask* piVar1 = gBattleManager->m10_battleOverlay->m14_textDisplay;
    if ((piVar1 != nullptr) && (piVar1->m0_texts.m_offset != 0))
    {
        piVar1->m14 = 0x2d;
        piVar1->m10 = (s16)param_1;
        createDisplayStringBorromScreenTask(
            piVar1,
            &piVar1->m8,
            -(s16)piVar1->m14,
            readSaturnEA(piVar1->m0_texts + (u32)(u16)piVar1->m10 * 4));
    }
}

// 0606c4ec
static void FUN_BTL_T0__0606c4ec(s16 param_1, s8 param_2, s8 param_3)
{
    displayFormationName(param_1, param_2, param_3);
}

// 0605d58e
static int BTL_T0_isBattleModeReady()
{
    if (!gBattleManager->m10_battleOverlay->m4_battleEngine->m188_flags.m20_battleIntroRunning)
        return 1;
    return 0;
}

// 0605ddb8
static int BTL_T0_hasEnemiesPresent()
{
    u8 bVar1 = gBattleManager->m10_battleOverlay->m4_battleEngine->m230;
    if (bVar1 == 1 || bVar1 == 3 || bVar1 == 5 || bVar1 == 7 || bVar1 == 8 || bVar1 == 9 || bVar1 == 10)
        return 1;
    return 0;
}

// ============================================================
// Tutorial controller task (size 0x24)
// def at 060aae54: {null, null, draw=0605a9ec, delete=0605ac0e}
// ============================================================

struct sTutorialControllerTask : public s_workAreaTemplate<sTutorialControllerTask>
{
    p_workArea m0_textInputChild;    // 0x00
    p_workArea m4_displayChild;      // 0x04
    s32        m8_frameCounter;      // 0x08
    s16        mC;                   // 0x0C
    s16        mE;                   // 0x0E
    s16        m10_timer;            // 0x10
    u8         m12_pad[6];           // 0x12..0x17
    u16        m18_textSet;          // 0x18
    u8         m1a_phase;            // 0x1A
    u8         m1b_nextPhase;        // 0x1B
    u8         m1c_nextPhaseTarget;  // 0x1C
    u8         m1d_off;              // 0x1D
    u8         m1e_mes;              // 0x1E
    u8         m1f_textIndex;        // 0x1F
    u8         m20_flags;            // 0x20
    u8         m21_savedConsumable5; // 0x21
    u8         m22_pad[2];           // 0x22..0x23
}; // size 0x24

// ============================================================
// Tutorial text-input sub-task (size 0xC)
// def at 060aadf0: {null, null, draw=0605a5ba, delete=0605a606}
// ============================================================

struct sTutorialTextInputTask : public s_workAreaTemplate<sTutorialTextInputTask>
{
    p_workArea m0_parent;  // 0x00 — pointer to controller
    u8         m4_pad[4];  // 0x04..0x07
    u8         m8_subState;// 0x08
    u8         m9_pad[3];  // 0x09..0x0B
}; // size 0xC

// ============================================================
// Tutorial sub-functions
// ============================================================

// 0605a66c
static void tutorialController_setupTextArea()
{
    setupVDP2StringRendering(6, 5, 0x20, 10);
    clearVdp2TextArea();
}

// 0605a1ec
static void tutorialController_clearTextArea()
{
    setupVDP2StringRendering(6, 2, 0x20, 2);
    clearVdp2TextArea();
}

// 0606c5de
static void tutorialController_setFont(char param_1)
{
    sBattleTextDisplayTask* pText = gBattleManager->m10_battleOverlay->m14_textDisplay;
    s32 idx = (param_1 == '\0') ? 0x18 : 0x17;
    setActiveFont((s32)(s8)((u8*)pText)[idx]);
}

// 0605a61a
static void tutorialController_drawNextString(sTutorialControllerTask* pThis)
{
    tutorialController_setFont('\x01');
    vdp2StringContext.m1C = vdp2StringContext.m0;
    vdp2StringContext.m0 = 0;
    tutorialController_setupTextArea();
    sBattleTextDisplayTask* pText = gBattleManager->m10_battleOverlay->m14_textDisplay;
    u8 mesIdx = pThis->m1e_mes;
    pThis->m1e_mes++;
    drawObjectName(readSaturnString(readSaturnEA(pText->m0_texts + (u32)mesIdx * 4)).c_str());
    vdp2StringContext.m0 = vdp2StringContext.m1C;
}

// 0605ae94
static void tutorialController_showPanel(sTutorialControllerTask* pThis, s16 param_2, s16 param_3)
{
    pThis->m20_flags |= 1;
    pThis->mC = param_2;
    pThis->mE = param_3;
}

// 0605aea8
static void tutorialController_hidePanel(sTutorialControllerTask* pThis)
{
    pThis->m20_flags &= ~1u;
    pThis->mC = 0;
    pThis->mE = 0;
}

// 0605a200
static void tutorialController_setTextOverlay(sTutorialControllerTask* pThis, u8 param_2, u16 param_3)
{
    pThis->m1f_textIndex = param_2;
    pThis->m18_textSet = param_3;
    pThis->m20_flags |= 8;
}

// 0605a5ba — text input sub-task draw
static void tutorialTextInput_draw(sTutorialTextInputTask* pThis)
{
    u8 subState = pThis->m8_subState;
    if (subState == 0)
    {
        BattleCommandMenu_UpdateSub0();
    }
    else if (subState == 1)
    {
        drawBlueBox(3, 4, 0x28, 0xc, 0x1000);
        tutorialController_drawNextString((sTutorialControllerTask*)pThis->m0_parent);
        playSystemSoundEffect(3);
    }
    else
    {
        return;
    }
    pThis->m8_subState++;
}

// 0605a606 — text input sub-task delete
static void tutorialTextInput_delete(sTutorialTextInputTask* pThis)
{
    clearBlueBox(3, 4, 0x28, 0xc);
    fieldPaletteTaskInitSub0Sub0();
}

// 0605a680
static void tutorialController_hideTextInput(sTutorialControllerTask* pThis)
{
    tutorialController_setupTextArea();
    if (pThis->m0_textInputChild)
    {
        ((s_workArea*)pThis->m0_textInputChild)->getTask()->markFinished();
        pThis->m0_textInputChild = nullptr;
        playSystemSoundEffect(4);
    }
}

// 0605a5a0
static void tutorialController_createTextInput(sTutorialControllerTask* pThis)
{
    static const sTutorialTextInputTask::TypedTaskDefinition def = {
        nullptr,
        nullptr,
        &tutorialTextInput_draw,
        &tutorialTextInput_delete,
    };
    sTutorialTextInputTask* pChild = createSubTask<sTutorialTextInputTask>(pThis, &def);
    pChild->m0_parent = pThis;
    pThis->m0_textInputChild = pChild;
}

// 0605a79e
static void tutorialController_setupTextInput(sTutorialControllerTask* pThis, u8 mode)
{
    pThis->m1d_off = mode;
    tutorialController_createTextInput(pThis);
    BTL_T0_beginAttackSequence(0xf);
}

// 0605a7ae
static int tutorialController_checkInputConfirm(sTutorialControllerTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    s8 mode = (s8)pThis->m1d_off;
    u16 buttons = 0;

    if (mode == 0)
    {
        buttons = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 1;
    }
    else if (mode == 1)
    {
        u16 cfg = graphicEngineStatus.m4514.mD8_buttonConfig[2][1];
        if (cfg & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown)
        {
            tutorialController_hideTextInput(pThis);
            pEngine->m188_flags.m100_attackAnimationFinished = 1;
            pThis->m1b_nextPhase = pThis->m1c_nextPhaseTarget;
            pEngine->m188_flags.m10 = 0;
            return 1;
        }
        buttons = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 6;
    }
    else if (mode == 2)
    {
        buttons = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 6;
    }
    else
    {
        return 0;
    }

    if (buttons == 0)
        return 0;

    tutorialController_hideTextInput(pThis);
    pEngine->m188_flags.m100_attackAnimationFinished = 1;
    pThis->m1b_nextPhase = pThis->m1c_nextPhaseTarget;
    return 1;
}

// 0605a72a
static int tutorialController_scrollAndAdvance(sTutorialControllerTask* pThis)
{
    pThis->m10_timer--;
    if (pThis->m10_timer < 1)
    {
        pThis->m10_timer = 0;
        if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 6)
        {
            pThis->m10_timer = 10;
            tutorialController_drawNextString(pThis);
            pThis->m1b_nextPhase++;
            pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
            playSystemSoundEffect(6);
            return 1;
        }
    }
    return 0;
}

// 0605a776
static int tutorialController_timerAndConfirm(sTutorialControllerTask* pThis)
{
    pThis->m10_timer--;
    if (pThis->m10_timer < 1)
    {
        pThis->m10_timer = 0;
        if (tutorialController_checkInputConfirm(pThis))
            return 1;
    }
    return 0;
}

// 0605a6e8
static int tutorialController_checkD56cAndSetMode2(sTutorialControllerTask* pThis)
{
    if (BTL_T0_isAttackSeqActive() != 0)
        return 0;
    pThis->m1d_off = 2;
    tutorialController_createTextInput(pThis);
    BTL_T0_beginAttackSequence(0xf);
    pThis->m1b_nextPhase++;
    pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
    pThis->m10_timer = 10;
    return 1;
}

// 0605a98e
static int tutorialController_checkQuadrantAndSetMode(sTutorialControllerTask* pThis, char quadrant, u8 mode)
{
    if (BTL_T0_isAttackSeqActive() != 0)
        return 0;
    if (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant != quadrant)
        return 0;
    pThis->m1d_off = mode;
    tutorialController_createTextInput(pThis);
    BTL_T0_beginAttackSequence(0xf);
    pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
    pThis->m1b_nextPhase = 99;
    return 1;
}

// 0605a938
static int tutorialController_checkComboAndSetMode(sTutorialControllerTask* pThis, u8 comboThreshold, u8 mode)
{
    if ((int)(u32)comboThreshold > (int)(s8)gBattleManager->m10_battleOverlay->m4_battleEngine->m3B4.m16_combo)
        return 0;
    pThis->m1d_off = mode;
    tutorialController_createTextInput(pThis);
    BTL_T0_beginAttackSequence(0xf);
    pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
    pThis->m1b_nextPhase = 99;
    return 1;
}

// 0605a214
static void tutorialController_checkTextCondition(sTutorialControllerTask* pThis)
{
    sBattleCommandMenu* pMenu = gBattleManager->m10_battleOverlay->m20_battleHud->m28_battleCommandMenu;
    if (pMenu == nullptr)
    {
        pThis->m20_flags &= ~0x10u;
    }
    else if ((pMenu->m20 & 8) == 0)
    {
        pThis->m20_flags &= ~0x10u;
    }
    else if ((pThis->m20_flags & 0x10) == 0)
    {
        pThis->m20_flags |= 0x10;
        tutorialController_clearTextArea();
    }

    u16 textSet = pThis->m18_textSet;
    if (textSet == 0)
    {
        if (gBattleManager->m10_battleOverlay->m4_battleEngine->m22C_dragonCurrentQuadrant == '\0')
        {
            pThis->m20_flags &= ~0x10u;
            pThis->m20_flags &= ~0x08u;
            tutorialController_clearTextArea();
        }
    }
    else
    {
        Unimplemented();
    }
}

// 0605a134
static void tutorialController_periodicTextDisplay(sTutorialControllerTask* pThis)
{
    if ((pThis->m20_flags & 8) == 0)
        return;

    tutorialController_checkTextCondition(pThis);
    if (pThis->m20_flags & 0x10)
        return;

    int mod = performModulo(0x14, pThis->m8_frameCounter);
    if (mod == 0)
    {
        if (pThis->m20_flags & 0x10)
        {
            tutorialController_setFont('\0');
            return;
        }
        if ((pThis->m20_flags & 8) == 0)
        {
            tutorialController_clearTextArea();
            return;
        }
        tutorialController_setFont('\x01');
        vdp2StringContext.m1C = vdp2StringContext.m0;
        vdp2StringContext.m0 = 0;
        tutorialController_clearTextArea();
        sBattleTextDisplayTask* pText = gBattleManager->m10_battleOverlay->m14_textDisplay;
        VDP2DrawString(readSaturnString(readSaturnEA(pText->m0_texts + (u32)pThis->m1f_textIndex * 4)).c_str());
        vdp2StringContext.m0 = vdp2StringContext.m1C;
    }
    else if (mod == 10)
    {
        tutorialController_clearTextArea();
    }
}

// ============================================================
// Phase functions
// ============================================================

// 06058c3c — phase 0: basic combat tutorial
static void tutorialPhase0(sTutorialControllerTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    switch (pThis->m1b_nextPhase)
    {
    case 0:
        pEngine->m388 = 0x3ff;
        mainGameState.consumables[5] += 2;
        pThis->m1e_mes = 0x1c;
        pThis->m1b_nextPhase++;
        mainGameState.gameStats.m10_currentHP = 0x4b;
        break;
    case 1:
    case 0xe:
        if (BTL_T0_isAttackSeqActive() != 0) break;
        tutorialController_setupTextInput(pThis, 2);
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m10_timer = 10;
        tutorialController_showPanel(pThis, 0xd4, 0xc1);
        break;
    case 2:
        pThis->m10_timer--;
        if (pThis->m10_timer > 0) break;
        pThis->m10_timer = 0;
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 6) == 0) break;
        pThis->m10_timer = 10;
        tutorialController_drawNextString(pThis);
        playSystemSoundEffect(6);
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        tutorialController_showPanel(pThis, 0xd4, 0xc1);
        break;
    case 3:
    case 0xf:
        pThis->m10_timer--;
        if (pThis->m10_timer < 1)
        {
            pThis->m10_timer = 0;
            tutorialController_checkInputConfirm(pThis);
        }
        if (pThis->m1b_nextPhase == pThis->m1c_nextPhaseTarget) break;
        tutorialController_hidePanel(pThis);
        break;
    case 4:
    case 6:
    case 8:
    case 10:
    {
        int comboNeeded = (pThis->m1b_nextPhase == 10) ? 2 : 1;
        if (!tutorialController_checkComboAndSetMode(pThis, (u8)comboNeeded, 1)) break;
        pThis->m10_timer = 10;
        tutorialController_showPanel(pThis, 0xd4, 0xc1);
        break;
    }
    case 5:
    case 7:
    case 9:
    case 0xb:
    case 0xd:
    {
        u32 m38C = (u32)pEngine->m38C_battleMode;
        if (m38C != 0 && m38C != 1 && m38C != 3 && m38C != 4) break;
        if (!BTL_T0_isBattleModeReady()) break;
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pEngine->m388 = 0x3ff;
        break;
    }
    case 0xc:
        if (pEngine->m3B4.m16_combo < 2) break;
        if ((u32)pEngine->m3B4.mC < 0x8000) break;
        tutorialController_setupTextInput(pThis, 1);
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m1b_nextPhase = 99;
        tutorialController_showPanel(pThis, 0xd4, 0xc1);
        break;
    case 0x10:
        pThis->m10_timer = 0x1e;
        pThis->m1b_nextPhase++;
        break;
    case 0x11:
        pThis->m10_timer--;
        if (pThis->m10_timer >= 0) break;
        pEngine->m188_flags.m4 = 1;
        break;
    case 99:
    {
        s8 target = (s8)pThis->m1c_nextPhaseTarget;
        u32 packVal = 0;
        if (target == 5)       { pEngine->m388 = (u16)-0x385; packVal = 1; }
        else if (target == 7)  { pEngine->m388 = (u16)-0x385; packVal = 0; }
        else if (target == 9)  { pEngine->m388 = (u16)-0x385; packVal = 2; }
        else if (target == 11) { pEngine->m388 = (u16)-0x385; packVal = 3; }
        else if (target == 13) { pEngine->m388 = (u16)-0x39d; }
        else                   { pEngine->m388 = 0x3ff; goto phase0_99_after_pack; }
        mainGameState.setPackedBits(0x1043, 4, packVal);
phase0_99_after_pack:
        pThis->m10_timer--;
        if (pThis->m10_timer < 1)
        {
            pThis->m10_timer = 0;
            tutorialController_checkInputConfirm(pThis);
        }
        u8 nextPhase = pThis->m1b_nextPhase;
        if (nextPhase == 5)       { tutorialController_setTextOverlay(pThis, 0x49, 5); tutorialController_hidePanel(pThis); }
        else if (nextPhase == 7)  { tutorialController_setTextOverlay(pThis, 0x4a, 4); tutorialController_hidePanel(pThis); }
        else if (nextPhase == 9)  { tutorialController_setTextOverlay(pThis, 0x4b, 6); tutorialController_hidePanel(pThis); }
        else if (nextPhase == 11) { tutorialController_setTextOverlay(pThis, 0x4c, 7); tutorialController_hidePanel(pThis); }
        else if (nextPhase == 13) { tutorialController_setTextOverlay(pThis, 0x4d, 8); tutorialController_hidePanel(pThis); }
        break;
    }
    default:
        break;
    }
}

// 06059044 — phase 1: movement/position tutorial
static void tutorialPhase1(sTutorialControllerTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    switch (pThis->m1b_nextPhase)
    {
    case 0:
        pEngine->m388 = 0x3ff;
        pThis->m1e_mes = 0x24;
        pThis->m1b_nextPhase++;
        break;
    case 1:
    case 0x13:
        tutorialController_checkD56cAndSetMode2(pThis);
        break;
    case 2:
    case 0x14:
        tutorialController_scrollAndAdvance(pThis);
        break;
    case 3:
        if (BTL_T0_isAttackSeqActive() != 0) break;
        if (pEngine->m3CC->m4 < 2) break;
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        break;
    case 4:
    case 9:
    case 0xa:
    case 0x15:
        tutorialController_timerAndConfirm(pThis);
        break;
    case 0x12:
    {
        u8 mode = (u8)pEngine->m38C_battleMode;
        if (mode != 7 && mode != 8) break;
        if (!BTL_T0_isBattleModeReady()) break;
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        break;
    }
    case 5:
        if (tutorialController_scrollAndAdvance(pThis))
            tutorialController_showPanel(pThis, 0xb1, 0xa1);
        break;
    case 6:
        tutorialController_checkD56cAndSetMode2(pThis);
        break;
    case 7:
        if (tutorialController_scrollAndAdvance(pThis))
            tutorialController_showPanel(pThis, 0xba, 0xb0);
        break;
    case 8:
        if (tutorialController_scrollAndAdvance(pThis))
            tutorialController_hidePanel(pThis);
        break;
    case 0xb:
        if (BTL_T0_isAttackSeqActive() != 0) break;
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m10_timer = 10;
        pEngine->m388 = (u16)-0x421;
        tutorialController_setTextOverlay(pThis, 0x4e, 0);
        break;
    case 0xc:
        if (BTL_T0_isAttackSeqActive() != 0) break;
        if (pEngine->m22C_dragonCurrentQuadrant != '\0') break;
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m10_timer = 10;
        pEngine->m388 = 0x3ff;
        break;
    case 0xd:
        if (tutorialController_checkD56cAndSetMode2(pThis))
            tutorialController_showPanel(pThis, 0xd4, 0xc1);
        break;
    case 0xe:
        if (tutorialController_scrollAndAdvance(pThis))
            tutorialController_showPanel(pThis, 0xb1, 0xb8);
        break;
    case 0xf:
        if (tutorialController_timerAndConfirm(pThis))
            tutorialController_hidePanel(pThis);
        break;
    case 0x10:
        if (BTL_T0_isAttackSeqActive() != 0) break;
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m10_timer = 10;
        pEngine->m388 = (u16)-0x479;
        tutorialController_setTextOverlay(pThis, 0x4f, 2);
        break;
    case 0x11:
        if (tutorialController_checkQuadrantAndSetMode(pThis, '\x02', 2))
            tutorialController_showPanel(pThis, 0xb1, 0xa9);
        break;
    case 0x16:
        pThis->m10_timer = 0x1e;
        pThis->m1b_nextPhase++;
        break;
    case 0x17:
        pThis->m10_timer--;
        if (pThis->m10_timer >= 0) break;
        pEngine->m188_flags.m4 = 1;
        break;
    case 99:
        pEngine->m388 = 0x3ff;
        pThis->m10_timer--;
        if (pThis->m10_timer < 1)
        {
            pThis->m10_timer = 0;
            tutorialController_checkInputConfirm(pThis);
        }
        if (pThis->m1b_nextPhase == 0x12)
            tutorialController_hidePanel(pThis);
        break;
    default:
        break;
    }
}

// 06059464 — phase 2: berserk/laser tutorial
static void tutorialPhase2(sTutorialControllerTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    switch (pThis->m1b_nextPhase)
    {
    case 0:
        pEngine->m388 = 0x3ff;
        pThis->m1e_mes = 0x30;
        pThis->m1b_nextPhase++;
        break;
    case 1:
    case 0xf:
        if (BTL_T0_isAttackSeqActive() != 0) break;
        tutorialController_setupTextInput(pThis, 2);
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m10_timer = 10;
        break;
    case 2:
    case 0x11:
    {
        pThis->m10_timer--;
        if (pThis->m10_timer < 1)
        {
            pThis->m10_timer = 0;
            tutorialController_checkInputConfirm(pThis);
        }
        break;
    }
    case 3:
        if (BTL_T0_isAttackSeqActive() != 0) break;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m1b_nextPhase = 99;
        tutorialController_setTextOverlay(pThis, 0x50, 5);
        break;
    case 4:
    {
        u32 m38C = (u32)pEngine->m38C_battleMode;
        if (m38C != 3) break;
        if (!BTL_T0_isBattleModeReady()) break;
        tutorialController_setupTextInput(pThis, 2);
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m1b_nextPhase = 99;
        pThis->m10_timer = 10;
        break;
    }
    case 5:
        if (BTL_T0_isAttackSeqActive() != 0) break;
        if ((u8)pEngine->m22C_dragonCurrentQuadrant != 1) break;
        pEngine->m388 = 0x3ff;
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        break;
    case 6:
        if (pEngine->m3B4.m16_combo < 1) break;
        pEngine->m388 = (u16)-0x385;
        mainGameState.setPackedBits(0x1043, 4, 1);
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        tutorialController_setTextOverlay(pThis, 0x52, 5);
        break;
    case 7:
    {
        u32 m38C = (u32)pEngine->m38C_battleMode;
        if (m38C != 3) break;
        if (!BTL_T0_isBattleModeReady()) break;
        tutorialController_setupTextInput(pThis, 2);
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m10_timer = 10;
        pEngine->m388 = 0x3ff;
        break;
    }
    case 8:
        pThis->m10_timer--;
        if (pThis->m10_timer > 0) break;
        pThis->m10_timer = 0;
        tutorialController_checkInputConfirm(pThis);
        break;
    case 9:
        if (BTL_T0_isAttackSeqActive() != 0) break;
        pEngine->m388 = (u16)-0x479;
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m10_timer = 10;
        tutorialController_setTextOverlay(pThis, 0x53, 0);
        break;
    case 0xa:
        if (BTL_T0_isAttackSeqActive() != 0) break;
        if (pEngine->m22C_dragonCurrentQuadrant != '\0') break;
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m10_timer = 0x2d;
        pEngine->m388 = 0x3ff;
        break;
    case 0xb:
        pThis->m10_timer--;
        if (pThis->m10_timer > 0) break;
        pThis->m10_timer = 10;
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        tutorialController_setupTextInput(pThis, 2);
        break;
    case 0xc:
        pThis->m10_timer--;
        if (pThis->m10_timer > 0) break;
        pThis->m10_timer = 0;
        tutorialController_checkInputConfirm(pThis);
        break;
    case 0xd:
        if (pEngine->m3B4.m16_combo < 1) break;
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m10_timer = 10;
        pEngine->m388 = (u16)-0x385;
        mainGameState.setPackedBits(0x1043, 4, 1);
        tutorialController_setTextOverlay(pThis, 0x54, 5);
        break;
    case 0xe:
    {
        u32 m38C = (u32)pEngine->m38C_battleMode;
        if (m38C != 3) break;
        if (!BTL_T0_isBattleModeReady()) break;
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m10_timer = 10;
        pEngine->m388 = 0x3ff;
        break;
    }
    case 0x10:
        pThis->m10_timer--;
        if (pThis->m10_timer > 0) break;
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 6) == 0) break;
        pThis->m10_timer = 10;
        tutorialController_drawNextString(pThis);
        playSystemSoundEffect(6);
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        break;
    case 0x12:
        pThis->m10_timer = 0x1e;
        pThis->m1b_nextPhase++;
        break;
    case 0x13:
        pThis->m10_timer--;
        if (pThis->m10_timer >= 0) break;
        pEngine->m188_flags.m4 = 1;
        break;
    case 99:
    {
        s8 target = (s8)pThis->m1c_nextPhaseTarget;
        if (target == 4)
        {
            pEngine->m388 = (u16)-0x385;
            mainGameState.setPackedBits(0x1043, 4, 1);
        }
        else if (target == 5 || target == 10)
        {
            pEngine->m388 = (u16)-0x479;
        }
        else
        {
            pEngine->m388 = 0x3ff;
        }
        pThis->m10_timer--;
        if (pThis->m10_timer < 1)
        {
            pThis->m10_timer = 0;
            tutorialController_checkInputConfirm(pThis);
        }
        u8 nextPhase = pThis->m1b_nextPhase;
        if (nextPhase == 4)
        {
            // nothing
        }
        else if (nextPhase == 5)
        {
            tutorialController_setTextOverlay(pThis, 0x51, 1);
        }
        break;
    }
    default:
        break;
    }
}

// 060599fc — phase 3: magic (berserk) tutorial
static void tutorialPhase3(sTutorialControllerTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    switch (pThis->m1b_nextPhase)
    {
    case 0:
        pEngine->m388 = 0x3ff;
        pThis->m1b_nextPhase++;
        mainGameState.bitField[0x32] |= 0x20;
        mainGameState.gameStats.m14_currentBP = 0x1c;
        pThis->m1e_mes = 0x36;
        break;
    case 1:
        if (BTL_T0_isAttackSeqActive() != 0) break;
        tutorialController_checkComboAndSetMode(pThis, 1, 1);
        pThis->m10_timer = 10;
        break;
    case 2:
    {
        u32 m38C = (u32)pEngine->m38C_battleMode;
        if (m38C != 3) break;
        if (!BTL_T0_isBattleModeReady()) break;
        tutorialController_setupTextInput(pThis, 2);
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m10_timer = 10;
        pEngine->m388 = 0x3ff;
        break;
    }
    case 3:
    case 6:
    case 7:
    case 0xb:
    case 0xc:
    case 0x11:
        tutorialController_scrollAndAdvance(pThis);
        break;
    case 4:
    case 0xd:
        tutorialController_timerAndConfirm(pThis);
        break;
    case 5:
        if (BTL_T0_isAttackSeqActive() != 0) break;
        if (pEngine->m3B4.m16_combo < 1) break;
        tutorialController_setupTextInput(pThis, 1);
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m10_timer = 10;
        break;
    case 8:
        if (tutorialController_timerAndConfirm(pThis))
        {
            pEngine->m388 = (u16)-0x385;
            mainGameState.setPackedBits(0x1043, 4, 0);
            tutorialController_setTextOverlay(pThis, 0x56, 4);
        }
        break;
    case 9:
        if (pEngine->m38C_battleMode != (eBattleModes)0) break;
        if (!BTL_T0_isBattleModeReady()) break;
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pEngine->m388 = 0x3ff;
        break;
    case 0xa:
        if (pEngine->m3B4.m16_combo < 2) break;
        tutorialController_setupTextInput(pThis, 1);
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m10_timer = 10;
        break;
    case 0xe:
        if (BTL_T0_isAttackSeqActive() != 0) break;
        pEngine->m388 = (u16)-0x385;
        mainGameState.setPackedBits(0x1043, 4, 3);
        pThis->m1b_nextPhase++;
        tutorialController_setTextOverlay(pThis, 0x58, 7);
        break;
    case 0xf:
    {
        u32 m38C = (u32)pEngine->m38C_battleMode;
        if (m38C != 4) break;
        if (!BTL_T0_isBattleModeReady()) break;
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pEngine->m388 = 0x3ff;
        pThis->m10_timer = 10;
        break;
    }
    case 0x10:
        tutorialController_checkD56cAndSetMode2(pThis);
        break;
    case 0x12:
        pThis->m10_timer--;
        if (pThis->m10_timer > 0) break;
        pThis->m10_timer = 0;
        tutorialController_checkInputConfirm(pThis);
        break;
    case 0x13:
        pThis->m10_timer = 0x1e;
        pThis->m1b_nextPhase++;
        break;
    case 0x14:
        pThis->m10_timer--;
        if (pThis->m10_timer >= 0) break;
        pEngine->m188_flags.m4 = 1;
        break;
    case 99:
    {
        s8 target = (s8)pThis->m1c_nextPhaseTarget;
        u32 packVal = 0;
        bool doPack = false;
        if (target == 2) { pEngine->m388 = (u16)-0x385; packVal = 1; doPack = true; }
        else if (target == 11) { pEngine->m388 = (u16)-0x385; packVal = 3; doPack = true; }
        else { pEngine->m388 = 0x3ff; }
        if (doPack)
            mainGameState.setPackedBits(0x1043, 4, packVal);
        pThis->m10_timer--;
        if (pThis->m10_timer < 1)
        {
            pThis->m10_timer = 0;
            tutorialController_checkInputConfirm(pThis);
        }
        if (pThis->m1b_nextPhase == 2)
            tutorialController_setTextOverlay(pThis, 0x55, 5);
        break;
    }
    default:
        break;
    }
}

// 06059e88 — phase 4: item tutorial
static void tutorialPhase4(sTutorialControllerTask* pThis)
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;

    switch (pThis->m1b_nextPhase)
    {
    case 0:
        pEngine->m388 = 0x3ff;
        mainGameState.consumables[0x2d]++;
        pThis->m1e_mes = 0x41;
        pThis->m1b_nextPhase++;
        break;
    case 1:
    case 5:
        tutorialController_checkD56cAndSetMode2(pThis);
        break;
    case 2:
    case 8:
        tutorialController_scrollAndAdvance(pThis);
        break;
    case 3:
    case 9:
        tutorialController_timerAndConfirm(pThis);
        break;
    case 4:
    {
        u32 m38C = (u32)pEngine->m38C_battleMode;
        if (m38C != 8) break;
        if (!BTL_T0_isBattleModeReady()) break;
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        pThis->m10_timer = 10;
        break;
    }
    case 6:
        if (tutorialController_scrollAndAdvance(pThis))
            tutorialController_showPanel(pThis, 0x113, 0xb2);
        break;
    case 7:
        if (tutorialController_scrollAndAdvance(pThis))
            tutorialController_hidePanel(pThis);
        break;
    case 0xa:
        if (pEngine->m3B4.m16_combo < 1) break;
        pEngine->m388 = (u16)-0x285;
        mainGameState.setPackedBits(0x1043, 4, 2);
        pThis->m1b_nextPhase++;
        pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
        tutorialController_setTextOverlay(pThis, 0x5a, 6);
        pThis->m10_timer = 10;
        break;
    case 0xb:
        if (*(s16*)((u8*)gBattleManager->m10_battleOverlay->m18_dragon + 0x1d8) == 2)
            *(u16*)((u8*)gBattleManager->m10_battleOverlay->m18_dragon + 0x1d8) = 0xb4;
        {
            u32 m38C = (u32)pEngine->m38C_battleMode;
            if (m38C != 1) break;
            if (!BTL_T0_isBattleModeReady()) break;
            pThis->m1b_nextPhase++;
            pThis->m1c_nextPhaseTarget = pThis->m1b_nextPhase + 1;
            pEngine->m388 = 0x3ff;
        }
        break;
    case 0xc:
        if (tutorialController_checkD56cAndSetMode2(pThis))
            tutorialController_showPanel(pThis, 0x113, 0xb2);
        break;
    case 0xd:
        tutorialController_scrollAndAdvance(pThis);
        break;
    case 0xe:
        if (tutorialController_timerAndConfirm(pThis))
            tutorialController_hidePanel(pThis);
        break;
    case 0xf:
        pThis->m10_timer--;
        if (pThis->m10_timer >= 0) break;
        pEngine->m188_flags.m4 = 1;
        break;
    default:
        break;
    }
}

// 0605ac0e
static void tutorialController_delete(sTutorialControllerTask* pThis)
{
    mainGameState.gameStats.m10_currentHP = mainGameState.gameStats.mB8_maxHP;
    mainGameState.gameStats.m14_currentBP = mainGameState.gameStats.mBA_maxBP;
    mainGameState.bitField[0x32] &= 0xd7;
    mainGameState.bitField[0x34] |= 4;
    mainGameState.consumables[5] = pThis->m21_savedConsumable5;
    mainGameState.consumables[0x2d] = 0;
}

// 0605a9ec
static void tutorialController_draw(sTutorialControllerTask* pThis)
{
    pThis->m8_frameCounter++;

    switch (pThis->m1a_phase)
    {
    case 0: tutorialPhase0(pThis); break;
    case 1: tutorialPhase1(pThis); break;
    case 2: tutorialPhase2(pThis); break;
    case 3: tutorialPhase3(pThis); break;
    case 4: tutorialPhase4(pThis); break;
    }
    tutorialController_periodicTextDisplay(pThis);

    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x17])
    {
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x10) && pThis->mE > 0)
            pThis->mE--;
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x20) && pThis->mE < 0xe0)
            pThis->mE++;
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x80) && pThis->mC < 0x160)
            pThis->mC++;
        if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2 & 0x40) && pThis->mC > 0)
            pThis->mC--;

        vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
        vdp2PrintStatus.m10_palette = 0xc000;
        vdp2DebugPrintSetPosition(0x19, 0x10);
        vdp2PrintfSmallFont("PHASE:%2d NEXT:%2d", (u32)pThis->m1b_nextPhase, (u32)pThis->m1c_nextPhaseTarget);
        vdp2DebugPrintSetPosition(0x19, 0x11);
        vdp2PrintfSmallFont("MES# :%2d ALX=%4d", (u32)pThis->m1e_mes, (int)pThis->mC);
        vdp2DebugPrintSetPosition(0x19, 0x12);
        vdp2PrintfSmallFont("EXE# :%2d ALY=%4d", (u32)pThis->m1a_phase, (int)pThis->mE);
        vdp2DebugPrintSetPosition(0x19, 0x13);
        vdp2PrintfSmallFont("OFF# :%2d %2x", (u32)pThis->m1d_off, (u32)pThis->m20_flags);
        vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
    }
}

// ============================================================
// Tutorial display sub-task (size 0xC0)
// def at 060aae64: {null, null, draw=0605ad40, delete=null}
// ============================================================

struct sTutorialDisplayTask : public s_workAreaTemplate<sTutorialDisplayTask>
{
    sTutorialControllerTask* m0_parent;  // 0x00
    u8 m4_pad[0xB4];                     // 0x04..0xB7
    s16 mB8_frame;                       // 0xB8
    u8 mBA_pad[2];                       // 0xBA..0xBB
    u8 mBC_subState;                     // 0xBC
    u8 mBD_pad[3];                       // 0xBD..0xBF
}; // size 0xC0

// 0605ad40
static void tutorialDisplay_draw(sTutorialDisplayTask* pThis)
{
    sTutorialControllerTask* pParent = pThis->m0_parent;
    pThis->mB8_frame++;

    if (pThis->mBC_subState == 0)
    {
        if (pParent->m20_flags & 2)
        {
            pParent->m20_flags &= ~4u;
            pThis->mBC_subState++;
        }
    }
    else if (pThis->mBC_subState == 1 && !(pParent->m20_flags & 2))
    {
        pParent->m20_flags |= 4;
        pThis->mBC_subState--;
    }

    if (!(pParent->m20_flags & 1))
        return;

    // TODO: render tutorial display panel sprite from table at 060aade8
    Unimplemented();
}

// 0605ace6
static void createTutorialDisplayTask(sTutorialControllerTask* pParent)
{
    static const sTutorialDisplayTask::TypedTaskDefinition def = {
        nullptr,
        nullptr,
        &tutorialDisplay_draw,
        nullptr,
    };
    sTutorialDisplayTask* pChild = createSubTask<sTutorialDisplayTask>(pParent, &def);
    pChild->m0_parent = pParent;
    pParent->m4_displayChild = pChild;
}

// 0605ac56
static void createTutorialControllerTask(p_workArea parent)
{
    if ((s16)gBattleManager->m6_subBattleId >= 6)
        return;

    static const sTutorialControllerTask::TypedTaskDefinition def = {
        nullptr,
        nullptr,
        &tutorialController_draw,
        &tutorialController_delete,
    };
    sTutorialControllerTask* pTask = createSubTask<sTutorialControllerTask>(
        gBattleManager->m10_battleOverlay->m18_dragon, &def);

    pTask->m1a_phase = (u8)(s8)gBattleManager->m6_subBattleId;
    gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x15] = 0;
    gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x21] = 0;
    mainGameState.bitField[0x32] |= 8;
    mainGameState.bitField[0x34] &= ~4u;
    pTask->m21_savedConsumable5 = mainGameState.consumables[5];
    createTutorialDisplayTask(pTask);
    pTask->mC = 0;
    pTask->mE = 0;
    pTask->m20_flags |= 1;
    pTask->m20_flags |= 4;
}

// ============================================================
// Per-slot in the secondary formation heap (0x4C bytes each)
// ============================================================

// T0-specific Baldor init (same struct as BTL_A3's sBaldor, different overlay data addresses)
static void BTL_T0_Baldor_init(sBaldorBase* pThisBase, sFormationData* pFormationEntry);

// 0606d698
static void* BTL_T0_allocFormationSlots(p_workArea pTask, u8 numEntries)
{
    sFormationData* psVar1 = (sFormationData*)allocateHeapForTask(pTask, numEntries * 0x4C);
    for (int iVar5 = 0; iVar5 < numEntries; iVar5++)
    {
        sFormationData& s = psVar1[iVar5];
        s.m0_translation.m0_current.zeroize();
        s.m0_translation.mC_target.zeroize();
        s.m0_translation.m18.zeroize();
        s.m24_rotation.m0_current.zeroize();
        s.m24_rotation.mC_target.zeroize();
        s.m24_rotation.m18.zeroize();
        s.m24_rotation.m0_current.m4_Y.m_value = 0x8000000;
    }
    return psVar1;
}

// 06055804
static sBaldor* BTL_T0_createBaldorSubEntity(s_workAreaCopy* parent, sFormationData* slot)
{
    // Update and Draw are identical to BTL_A3's Baldor_update/Baldor_draw.
    // Only Init differs due to T0-specific overlay data addresses.
    static const sBaldor::TypedTaskDefinition def = {
        BTL_T0_Baldor_init,
        Baldor_update,
        Baldor_draw,
        nullptr,
    };
    return createSubTaskWithArgWithCopy<sBaldor, sFormationData*>(parent, slot, &def);
}

// ============================================================
// Secondary formation entity task (Saturn size 0x14)
// def at 060a9a04: {init=060544b8, update=060545e4, draw=null, delete=null}
// ============================================================

struct sBTL_T0_SecondaryFormation : public s_workAreaTemplateWithArgWithCopy<sBTL_T0_SecondaryFormation, u32>
{
    u8    m0;              // Saturn 0x00: state machine state
    u8    m1;              // Saturn 0x01: sub-state
    u8    m2_pad[2];       // Saturn 0x02..0x03
    void* m4_heapPtr;      // Saturn 0x04 (4 bytes Saturn, 8 bytes C++)
    u16   m8_timer;        // Saturn 0x08
    u8    mA_numGroups;    // Saturn 0x0A
    u8    mB;              // Saturn 0x0B: counter for quadrant 1/3 attack
    u8    mC;              // Saturn 0x0C: flag for quadrant 2
    // Saturn size 0x14

    // 060544b8
    static void Init(sBTL_T0_SecondaryFormation* pThis, u32 param_2);

    // 060545e4
    static void Update(sBTL_T0_SecondaryFormation* pThis)
    {
        s_battleEngine* psVar5 = gBattleManager->m10_battleOverlay->m4_battleEngine;
        sFormationData* slots = (sFormationData*)pThis->m4_heapPtr;

        if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x1b])
        {
            vdp2DebugPrintSetPosition(10, 8);
            vdp2PrintfSmallFont("P %d %d   ", (u32)pThis->m0, (u32)pThis->m1);
            for (int i = 0; i < pThis->mA_numGroups; i++)
            {
                vdp2DebugPrintSetPosition(10, i + 9);
                vdp2PrintfSmallFont("sts %02x cmd %02x     ", (u32)slots[i].m48, (u32)slots[i].m49);
            }
        }

        psVar5->m3B2_numBattleFormationRunning++;

        if (pThis->m0 != 0)
        {
            if (pThis->m0 == 1 || pThis->m0 == 2)
            {
                if (pThis->m1 == 0)
                {
                    if (!(psVar5->m188_flags.m2000))
                        return;
                    pThis->m1++;
                }
                else if (pThis->m1 == 1)
                {
                    for (int i = 0; i < pThis->mA_numGroups; i++)
                    {
                        if (slots[i].m48 & 1)
                        {
                            slots[i].m49 = 1;
                            pThis->m1++;
                            return;
                        }
                    }
                    pThis->m1 = 10;
                }
                else if (pThis->m1 == 2)
                {
                    for (int i = 0; i < pThis->mA_numGroups; i++)
                    {
                        if (slots[i].m48 & 2)
                            return;
                    }
                    pThis->m1 = 1;
                }
                else if (pThis->m1 == 10)
                {
                    psVar5->m188_flags.m100_attackAnimationFinished = 1;
                    for (int i = 0; i < pThis->mA_numGroups; i++)
                        slots[i].m49 = 0;
                    pThis->m0 = 0;
                    pThis->m1 = 0;
                }
            }
            return;
        }

        bool bVar3 = false;
        for (int i = 0; i < pThis->mA_numGroups; i++)
        {
            if (!(slots[i].m48 & 4))
            {
                bVar3 = true;
                break;
            }
        }

        if (!bVar3)
        {
            pThis->m8_timer--;
            if ((s16)pThis->m8_timer < 0 && pThis != nullptr)
            {
                pThis->getTask()->markFinished();
            }
        }

        if (psVar5->m3CC->m8 == 0)
            return;

        if (BTL_T0_isAttackSeqActive() != 0)
            return;

        psVar5->m3CC->m8 = 0;
        psVar5->m3CC->m0 = 0;

        if (!bVar3)
            return;

        u32 quadrant = (u32)(u8)psVar5->m22C_dragonCurrentQuadrant;
        if (quadrant == 0)
            return;

        if (quadrant == 2)
        {
            if ((s8)pThis->mC != 0)
                return;
            pThis->mC = 1;

            s32 iVar8 = randomNumber();
            iVar8 = performModulo2(3, iVar8);
            if (iVar8 == 0)
            {
                BTL_T0_beginAttackSequence(7);
            }
            else if (iVar8 == 1)
            {
                BTL_T0_triggerAttackCamera('\b');
            }
            else if (iVar8 == 2)
            {
                BTL_T0_triggerAttackCamera('\x01');
            }

            int count = 0;
            for (int i = 0; i < pThis->mA_numGroups; i++)
            {
                if (!(slots[i].m48 & 4))
                {
                    slots[i].m49 = 2;
                    count++;
                    if (count > 0) break;
                }
            }
            BTL_T0_displayBottomScreenString(4);
            battleEngine_FlagQuadrantForAttack(2);
            pThis->m0 = 1;
            return;
        }

        if (quadrant != 1 && quadrant != 3)
            return;

        if (pThis->mB > 1)
            return;
        pThis->mB++;

        u32 rng = (u32)randomNumber();
        if ((rng & 1) == 0)
            BTL_T0_triggerAttackCamera('\x02');
        else if ((rng & 1) == 1)
            BTL_T0_triggerAttackCamera('\a');

        for (int i = 0; i < pThis->mA_numGroups; i++)
            slots[i].m49 = 3;

        BTL_T0_displayBottomScreenString(5);
        battleEngine_FlagQuadrantForAttack(1);
        battleEngine_FlagQuadrantForAttack(3);
        pThis->m0 = 2;
    }
}; // Saturn size 0x14

// ============================================================
// Water effect sub-task (size 0x20)
// def at 060a9324: {init=0605411c, update=06054156, draw=06054260, delete=null}
// ============================================================

struct sBTL_T0_WaterEffect : public s_workAreaTemplate<sBTL_T0_WaterEffect>
{
    // 0605411c
    static void Init(sBTL_T0_WaterEffect* pThis)
    {
        allocateNPC(pThis, 0xc);
        pThis->m10_vdp1Base  = dramAllocatorEnd[12].mC_fileBundle->m4_vd1Allocation->m4_vdp1Memory;
        pThis->m12           = 0xa37;
        pThis->m14_width     = 0x200;
        pThis->m16_height    = 0xa0;
        pThis->m18_height2   = 0xa0;
        pThis->m1A_height3   = 0x50;
        pThis->m1C_flag      = 1;
    }

    // 06054156
    static void Update(sBTL_T0_WaterEffect* pThis)
    {
        Unimplemented();
    }

    // 06054260
    static void Draw(sBTL_T0_WaterEffect* pThis)
    {
        Unimplemented();
    }

    u8  m0_pad[0x10];   // 0x00..0x0F
    u16 m10_vdp1Base;   // 0x10
    u16 m12;            // 0x12
    u16 m14_width;      // 0x14
    u16 m16_height;     // 0x16
    u16 m18_height2;    // 0x18
    u16 m1A_height3;    // 0x1A
    u8  m1C_flag;       // 0x1C
    u8  m1D_pad[3];     // 0x1D..0x1F
}; // Saturn size 0x20

// 060544a2
static void BTL_T0_createWaterEffect(p_workArea parent)
{
    static const sBTL_T0_WaterEffect::TypedTaskDefinition def = {
        &sBTL_T0_WaterEffect::Init,
        &sBTL_T0_WaterEffect::Update,
        &sBTL_T0_WaterEffect::Draw,
        nullptr,
    };
    createSubTask<sBTL_T0_WaterEffect>(parent, &def);
}

// ============================================================

struct BTL_T0_data : public battleOverlay
{
    BTL_T0_data() : battleOverlay("BTL_T0.PRG")
    {
        m_grid = readGrid(getSaturnPtr(0x060aa548), 2, 2);
    }

    const sGrid* m_grid;

    sSaturnPtr getEncounterDataTable() override
    {
        return getSaturnPtr(0x060ace6c);
    }

    void invoke(sSaturnPtr Func, s_workAreaCopy* pParent) override
    {
        switch (Func.m_offset)
        {
        case 0x060585f0:
            Create_BTL_T0_env(pParent);
            break;
        default:
            Unimplemented();
            break;
        }
    }

    void invoke(sSaturnPtr Func, s_workAreaCopy* pParent, u32 arg0, u32 arg1) override
    {
        switch (Func.m_offset)
        {
        case 0x060540fc:
            Create_BTL_T0_Formation(pParent, arg0);
            break;
        case 0x060549b4:
        {
            static const sBTL_T0_SecondaryFormation::TypedTaskDefinition def = {
                &sBTL_T0_SecondaryFormation::Init,
                &sBTL_T0_SecondaryFormation::Update,
                nullptr,
                nullptr,
            };
            createSubTaskWithArg<sBTL_T0_SecondaryFormation, u32>(pParent, arg0, &def);
            break;
        }
        default:
            Unimplemented();
            break;
        }
    }

    p_workArea invokeCreateEffect(sSaturnPtr Func, s_workAreaCopy* pParent) override
    {
        Unimplemented();
        return nullptr;
    }
};

// 060544b8
void sBTL_T0_SecondaryFormation::Init(sBTL_T0_SecondaryFormation* pThis, u32 param_2)
{
    allocateNPC(pThis, 8);

    int iVar5 = (int)(intptr_t)dramAllocatorEnd[8].mC_fileBundle;

    if (param_2 == 0)
        pThis->mA_numGroups = 1;
    else if (param_2 == 1 || param_2 == 2)
        pThis->mA_numGroups = 2;

    pThis->m4_heapPtr = BTL_T0_allocFormationSlots(pThis, pThis->mA_numGroups);

    sSaturnPtr posDataPtr = readSaturnEA(g_BTL_T0->getSaturnPtr(0x060a99d0 + param_2 * 4));
    sFormationData* slots = (sFormationData*)pThis->m4_heapPtr;
    for (int i = 0; i < pThis->mA_numGroups; i++)
    {
        sVec3_FP pos = readSaturnVec3(posDataPtr + i * 12);
        slots[i].m0_translation.mC_target = pos;
        slots[i].m0_translation.m18 = pos;
        slots[i].m24_rotation.mC_target.m4_Y.m_value = 0x8000000;
        sBaldor* pSubEntity = BTL_T0_createBaldorSubEntity((s_workAreaCopy*)(intptr_t)iVar5, &slots[i]);
        pSubEntity->mA_indexInFormation = (s8)i;
    }

    battleEngine_FlagQuadrantForSafety(2);
    battleEngine_FlagQuadrantForDanger(0);
    pThis->m8_timer = 0x3c;
    FUN_BTL_T0__0606c4ec(3, 1, 9);
    pThis->m0 = 0;
    pThis->m1 = 0;
}

static const char* BTL_T0_fileList[] = {
    "BATTLE.MCB", "BATTLE.CGB",        // 0, 1
    "TUP.MCB", "TUP.CGB",              // 2, 3
    "ENCAM.BDB",                         // 4
    (const char*)-1,                     // 5
    "A3CMN.MCB", "A3CMN.CGB",           // 6, 7
    "HEBI_TUR.MCB", "HEBI_TUR.CGB",    // 8, 9
    "BENITATE.MCB", "BENITATE.CGB",     // 10, 11
    "FLD_A3.MCB", "FLD_A3.CGB",         // 12, 13
    (const char*)-1,                     // 14
    "TUTORIAL.CGB",                      // 15
    "KARAA72.MCB", "KARAA72.CGB",       // 16, 17
    "KEIKOKU.MCB", "KEIKOKU.CGB",       // 18, 19
    "STK01.MCB", "STK01.CGB",           // 20, 21
    nullptr
};

// 0605400a
static void BTL_T0_initMusic(p_workArea pThis)
{
    s16 subBattleId = gBattleManager->m6_subBattleId;

    s8 soundBank;
    switch (subBattleId)
    {
    case 0:
    case 1:
    case 5:
        soundBank = 4;
        break;
    case 2:
        soundBank = 5;
        break;
    case 3:
        soundBank = 8;
        break;
    default:
        soundBank = 0x21;
        break;
    }

    loadSoundBanks(soundBank, 0);
    gBattleManager->m10_battleOverlay->m3 = 1;
    playPCM(pThis, 100);
}

// 0605409c
static void BTL_T0_initBattle(p_workArea pThis)
{
    initMemoryForBattle(pThis, BTL_T0_fileList);
    allocateNPC(pThis, 6);
    allocateNPC(pThis, 0xE);
    createBattleEngineTask(pThis, g_BTL_T0->getSaturnPtr(0x060a989c));
    createTutorialControllerTask(pThis);
}

// 060540fc
static p_workArea Create_BTL_T0_Formation(p_workArea parent, u32 existingFormation)
{
    if (existingFormation != 0)
        return reinterpret_cast<p_workArea>(static_cast<uintptr_t>(existingFormation));
    const sGenericFormationData* config = readUrchinFormation(g_BTL_T0->getSaturnPtr(0x060a76f4));
    return Create_BTL_A3_UrchinFormation(parent, config);
}

// 06057f34
static void BTL_T0_env_InitVdp2(s_BTL_A3_Env* pThis)
{
    gBattleManager->m10_battleOverlay->m1C_envTask = (p_workArea)pThis;
    reinitVdp2();
    initNBG1Layer();
    asyncDmaCopy(g_BTL_T0->getSaturnPtr(0x060aa76c), getVdp2Cram(0x400), 0x200, 0);
    asyncDmaCopy(g_BTL_T0->getSaturnPtr(0x060aa56c), getVdp2Cram(0xA00), 0x200, 0);
    asyncDmaCopy(g_BTL_T0->getSaturnPtr(0x060aa96c), vdp2Palette, 0x200, 0);

    static const sLayerConfig rgb0Setup[] = {
        m2_CHCN, 1, m5_CHSZ, 1, m6_PNB, 1, m7_CNSM, 0,
        m27_RPMD, 2, m11_SCN, 8, m34_W0E, 1, m37_W0A, 1,
        m0_END,
    };
    setupRGB0(rgb0Setup);

    static const sLayerConfig rotParamsSetup[] = { m31_RxKTE, 1, m13, 1, m0_END };
    setupRotationParams(rotParamsSetup);
    static const sLayerConfig rotParams2Setup[] = { m0_END };
    setupRotationParams2(rotParams2Setup);

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mE_RAMCTL = (regs->mE_RAMCTL & 0xFF00) | 0xB4;
    regs->m10_CYCA0 = 0x310F754F;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, g_BTL_T0->getSaturnPtr(0x060aabac));
    setupRotationMapPlanes(1, g_BTL_T0->getSaturnPtr(0x060aab6c));
    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x24000), 0x80);
    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x2A000));

    regs->mA8_LCTA = (regs->mA8_LCTA & 0xFFF80000) | 0x15200;
    *(u16*)getVdp2Vram(0x2A400) = 0x0700;
    *(u16*)getVdp2Vram(0x2A600) = 0x38E5;
    regs->mAC_BKTA = (regs->mAC_BKTA & 0xFFF80000) | 0x15300;
    regs->mF0_PRISA = 0x405;
    regs->mF2_PRISB = 0x507;
    regs->mF4_PRISC = 0x505;
    regs->mF6_PRISD = 0x505;
    regs->mF8_PRINA = 0x600;
    regs->mFA_PRINB = 0x700;
    regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C = 0x10000;

    static const std::vector<std::array<s32, 2>> layerDisplayConfig = { { 0x2C, 0x1 } };
    applyLayerDisplayConfig(layerDisplayConfig);

    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xF8FF) | 0x400;
    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xFFF0) | 3;
    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m50 = 0x10;
    pThis->m51 = 0x12;
    pThis->m52 = 0x14;
    regs->m100_CCRSA = pThis->m50 | (pThis->m51 << 8);
    regs->m102_CCRSB = pThis->m52;
    regs->m104_CCRSC = 0;
    regs->m106_CCRSD = 0;
    pThis->m55 = 8;
    regs->m10C_CCRR = pThis->m55;
    vdp2Controls.m_isDirty = 1;

    s_BTL_A3_Env_InitVdp2Sub4(g_BTL_T0->getSaturnPtr(0x060aac40));
}

// 06058580
static void BTL_T0_env_Init(s_BTL_A3_Env* pThis)
{
    loadFile("SCBTLA31.SCB", getVdp2Vram(0x40000), 0);
    loadFile("SCBTL_A3.PNB", getVdp2Vram(0x62800), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    pThis->m38 = -0x8000;

    BTL_T0_env_InitVdp2(pThis);

    // Grid setup
    npcFileDeleter* envFile = (npcFileDeleter*)allocateNPC(pThis, 0x12);
    pThis->m58 = envFile;
    initGridForBattle(envFile, g_BTL_T0->m_grid, 2, 2, 0x400000);

    // 060544a2 — water/overlay effect sub-task
    BTL_T0_createWaterEffect(envFile);

    gBattleManager->m10_battleOverlay->m8_gridTask->m1C8_flags |= 0x10;
}

// 060585f0
static p_workArea Create_BTL_T0_env(p_workArea parent)
{
    static const s_BTL_A3_Env::TypedTaskDefinition definition = {
        &BTL_T0_env_Init,
        &BTL_A3_Env_Update,
        &BTL_A3_Env_Draw,
        nullptr,
    };

    return createSubTask<s_BTL_A3_Env>(parent, &definition);
}

// 06054000
p_workArea overlayStart_BTL_T0(p_workArea parent)
{
    if (g_BTL_T0 == nullptr)
    {
        g_BTL_T0 = new BTL_T0_data();
    }

    gCurrentBattleOverlay = g_BTL_T0;
    return createBattleMainTask(parent, &BTL_T0_initMusic, BTL_T0_initBattle);
}


// 060549dc — T0-specific Baldor init.
// Same logic as Baldor_init (BTL_A3) but with T0 overlay data addresses.
// BTL_A3's Baldor_init has an additional queen-baldor branch (subBattleId == 8/9)
// with different oscillation frequencies and random initial rotations; T0 is always
// the standalone tutorial battle so that path doesn't apply here.
static void BTL_T0_Baldor_init(sBaldorBase* pThisBase, sFormationData* pFormationEntry)
{
    sBaldor* pThis = (sBaldor*)pThisBase;

    Baldor_initSub0(pThis, g_BTL_T0->getSaturnPtr(0x060a9c54), pFormationEntry, 0);

    pThis->m14_targetable.resize(4);
    pThis->m18_position.resize(4);
    pThis->mC_numTargetables = 4;

    for (int i = 0; i < 4; i++)
    {
        u32 flags = (i == 0) ? 0xf0000000u : 0xf0000002u;
        initTargetable(&pThis->m14_targetable[i], nullptr, &pThis->m18_position[i], 0x1000, flags, 0, 0, 10);
    }

    *pThis->m28_rotation.m0_current = *pThis->m28_rotation.m4_target;

    pThis->m68_body = Baldor_createBody(pThis, 6);
    Baldor_loadBodyPartData(pThis->m68_body, 1, g_BTL_T0->getSaturnPtr(0x060a9c68));

    sSaturnPtr pDataSource = g_BTL_T0->getSaturnPtr(0x060a9ce0);
    for (int i = 0; i < 6; i++)
    {
        sBaldorBodyPart& dest = pThis->m68_body->m30_parts[i];
        dest.m10_translation = readSaturnVec3(pDataSource + 0xC * i);
        dest.m1C_rotation[1] = (*pThis->m28_rotation.m4_target)[1];
    }

    pThis->m6C_oscillationPhase[0].m_value = randomNumber();
    pThis->m6C_oscillationPhase[1].m_value = randomNumber();
    pThis->m6C_oscillationPhase[2].m_value = randomNumber();
}

