#include "PDS.h"
#include "battleDebug.h"
#include "battleManager.h"
#include "battleOverlay.h"
#include "battleEngine.h"
#include "battleDragon.h"
#include "battleOverlay_C.h"
#include "battleTargetable.h"

static void s_battleDebug_Init(s_battleDebug* pThis)
{
    gBattleManager->m10_battleOverlay->m10_inBattleDebug = pThis;
    for (int i = 0; i < 0x1D; i++)
    {
        // This init the debug flags based on the gamestate internal flags
        int bitIndex = i + 1000;
        if (999 < bitIndex)
        {
            bitIndex = i + 0x1026;
        }
        if (mainGameState.getBit(bitIndex))
        {
            pThis->mFlags[i] = 1;
        }
        else
        {
            pThis->mFlags[i] = 0;
        }
    }
    pThis->mFlags[4] = 0;
    pThis->mFlags[1] = 6;
}

// 0605d874
static void debugCheat_addAllItems()
{
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x17] == 0)
        return;
    if (!readKeyboardToggle(0x8A))
        return;

    auto& c = mainGameState.consumables;
    c[1]+=5; c[2]+=5; c[3]+=5; c[5]+=5; c[4]+=5; c[6]+=5;
    c[0x26]+=5; c[0x27]+=5; c[0x28]+=5; c[0x29]+=5; c[0x2a]+=5; c[0x2b]+=5;
    c[0x2c]+=5; c[0x2d]+=5; c[0x2e]+=5; c[0x2f]+=5; c[0x30]+=5; c[0x31]+=5;
    c[0x32]+=5; c[0x33]+=5; c[0x34]+=5; c[0x35]+=5; c[0x36]+=5; c[0x37]+=5;
    c[0x3b]=1; c[0x3a]=1; c[0x3e]=1; c[0x3f]=1; c[0x40]=1; c[0x41]=1; c[0x3c]=1; c[0x3d]=1;
    mainGameState.bitField[0x33] |= 0x87;
    mainGameState.bitField[0x30] |= 0x0F;
    mainGameState.bitField[0x34] |= 0xFC;
    mainGameState.bitField[0x31] = 0xFF;
    mainGameState.bitField[0x32] = 0xFF;
}

// 0605db40
static void debugTest_randomDamageOnDragon()
{
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x17] == 0)
        return;

    u32 kind = 0;
    if (readKeyboardToggle(0x96)) kind = 1;
    if (readKeyboardToggle(0x9E)) kind = 2;
    if (readKeyboardToggle(0xA6)) kind = 3;
    if (readKeyboardToggle(0xA5)) kind = 4;
    if (readKeyboardToggle(0xAE)) kind = 5;
    if (readKeyboardToggle(0xB6)) kind = 6;
    if (keyboardIsKeyDown(0xBD)) kind = 7;

    if (kind != 0)
    {
        sVec3_FP dir;
        if (kind == 7)
        {
            dir[0] = 0x1000; dir[1] = 0x1000; dir[2] = 0x1000;
        }
        else
        {
            dir[0] = (randomNumber() & 0x1FFF) - 0x1000;
            dir[1] = (randomNumber() & 0xFFF) - 0x800;
            dir[2] = (randomNumber() & 0x1FFF) - 0x1000;
        }
        sVec2_FP angles;
        computeLookAt(dir, angles);
        s32 dmgType = performModulo2(3, randomNumber()) + 1;
        u32 dmgBits = 1 << (kind - 1);
        s_battleDragon* pDragon = gBattleManager->m10_battleOverlay->m18_dragon;
        applyDamageToDragon(pDragon->m8C, 1,
            pDragon->m8_position, dmgType, dir, dmgBits);

        vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
        vdp2PrintStatus.m10_palette = 0xC000;
        vdp2DebugPrintSetPosition(0xC, 4);
        vdp2PrintfSmallFont("KIND:%1d", kind - 1);
        vdp2DebugPrintSetPosition(0xC, 5);
        vdp2PrintfSmallFont("DMG :%1d", dmgType);
        vdp2DebugPrintSetPosition(0xC, 6);
        vdp2PrintfSmallFont("ANG :%4d :%4d", (s16)angles[0] * 0x168 >> 12, (s16)angles[1] * 0x168 >> 12);
        vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
    }
}

// 0605e020
static void debugAdjustEnvDistance()
{
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x10] == 0)
        return;

    s32* pEnvScale = (s32*)((u8*)gBattleManager->m10_battleOverlay->m1C_envTask + 0x3C);
    if (graphicEngineStatus.m4514.mD8_buttonConfig[0][4] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2)
    {
        *pEnvScale += 0x1999;
        if (*pEnvScale > 0xA0000) *pEnvScale = 0xA0000;
    }
    if (graphicEngineStatus.m4514.mD8_buttonConfig[0][5] & graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2)
    {
        *pEnvScale -= 0x1999;
        if (*pEnvScale < 0x1999) *pEnvScale = 0x1999;
    }
    vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
    vdp2PrintStatus.m10_palette = 0xC000;
    vdp2DebugPrintSetPosition(1, 0x10);
    vdp2PrintfSmallFont("DIST:%3d", intDivide(0x1999, fixedPoint(*pEnvScale & 0xFFFF)).getInteger());
    vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
}

// 0605dd54
static void debugAdjustAltitude()
{
    s_battleEngine* pEngine = gBattleManager->m10_battleOverlay->m4_battleEngine;
    if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[0x20] == 0)
        return;

    if (battleEngine_isPlayerTurnActive() && pEngine->m38C_battleMode == eBattleModes::mC_commandMenuOpen)
    {
        u8 quad = pEngine->m22C_dragonCurrentQuadrant;

        if (readKeyboardToggle(0x106))
            gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[4] = 0;
        if (readKeyboardToggle(0x10D))
            gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[4] = 1;

        if (gBattleManager->m10_battleOverlay->m10_inBattleDebug->mFlags[4] == 0)
        {
            if (readKeyboardTable1(0x109))
                pEngine->m364_perQuadrantDragonAltitude[quad] += 0x1000;
            if (readKeyboardTable1(0x10A))
                pEngine->m364_perQuadrantDragonAltitude[quad] -= 0x1000;

            pEngine->m104_dragonPosition.m4_Y = pEngine->m364_perQuadrantDragonAltitude[quad] +
                gBattleManager->m10_battleOverlay->mC_targetSystem->m204_cameraMaxAltitude;
        }
        else
        {
            if (readKeyboardTable1(0x109))
                pEngine->m374_perQuadrantEnemyAltitude[quad] += 0x1000;
            if (readKeyboardTable1(0x10A))
                pEngine->m374_perQuadrantEnemyAltitude[quad] -= 0x1000;

            pEngine->m270_enemyAltitude.m4_Y = pEngine->m374_perQuadrantEnemyAltitude[quad];
        }
    }

    vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
    vdp2PrintStatus.m10_palette = 0xC000;
    vdp2DebugPrintSetPosition(0xC, 0x16);
    vdp2PrintfSmallFont("Height G:%3d",
        gBattleManager->m10_battleOverlay->mC_targetSystem->m204_cameraMaxAltitude >> 12);
    vdp2DebugPrintSetPosition(0xC, 0x17);
    s32 dragonY = pEngine->m104_dragonPosition.m4_Y;
    vdp2PrintfSmallFont("Height P:%3d :%3d",
        (dragonY - gBattleManager->m10_battleOverlay->mC_targetSystem->m204_cameraMaxAltitude) >> 12,
        dragonY >> 12);
    vdp2DebugPrintSetPosition(0xC, 0x18);
    vdp2PrintfSmallFont("Height E:%3d :%3d",
        pEngine->m270_enemyAltitude.m4_Y >> 12, pEngine->m234.m4_Y >> 12);
    vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
}

// 0605e196
static void s_battleDebug_Update(s_battleDebug* pThis)
{
    debugAdjustAltitude();
    debugAdjustEnvDistance();
    debugTest_randomDamageOnDragon();
    debugCheat_addAllItems();
}

static const char s_debugMenuStrings[][24] = {
    " 1 PCM OFF MODE      ",
    " 2 SHOT EQUIP CHANGE ",
    " 3 LASER EQUIP CHANGE",
    " 4 PRELOCK CHANGE    ",
    " 5 COLLI ENTRY NUMBER",
    " 6 CAMERA SPRING     ",
    " 7 CAMERA FIX        ",
    " 8 CAMERA DISTANCE   ",
    " 9 CAMERA SCRNANG    ",
    "10 PLAYER SPEED      ",
    "11 AXIS ROTATE       ",
    "12 SCROLL DOT RATIO  ",
    "13 CAM DATA DISP     ",
    "14 LIGHT SETTING     ",
    "15 COLLI SPHERE      ",
    "16 COLLI VALUE DISP  ",
    "17 GAUGE INVALID     ",
    "18 BTL DATA DISP     ",
    "19 NO DEATH MODE     ",
    "20 CYLINDER SIZE     ",
    "21 FUTAKAWA MODE     ",
    "22 YAMAKEN MODE      ",
    "23 MITA MODE         ",
    "24 SHINYA MODE       ",
    "25 YASUHARA MODE     ",
    "26 TAKAMA MODE       ",
    "27 MOVE MODE         ",
    "28 HEIGHT SET        ",
    "29 ENEMY GAUGE DISP  ",
};

// 0605e1a8
static void s_battleDebug_Draw(s_battleDebug* pThis)
{
    if (pThis->mFlags[0] == 0)
    {
        if (readKeyboardToggle(0x84)) // F1
        {
            pThis->mFlags[0] = 1;
            pThis->mFlags[2] = 0;
        }
    }
    else
    {
        if (readKeyboardTable1(0x109)) // up
        {
            pThis->mFlags[1]--;
            if (pThis->mFlags[1] < 0) pThis->mFlags[1] = 0x1C;
        }
        if (readKeyboardTable1(0x10A)) // down
        {
            pThis->mFlags[1]++;
            if (pThis->mFlags[1] > 0x1C) pThis->mFlags[1] = 0;
        }
        if (readKeyboardToggle(0x106)) // left — disable flag
        {
            pThis->mFlags[pThis->mFlags[1] + 5] = 0;
        }
        if (readKeyboardToggle(0x10D)) // right — enable flag
        {
            pThis->mFlags[pThis->mFlags[1] + 5] = 1;
        }
        if (readKeyboardToggle(0x84)) // F1 — close menu
        {
            pThis->mFlags[0] = 0;
            clearVdp2TextMemory();
            return;
        }

        vdp2DebugPrintSetPosition(0xE, 1);
        vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
        if (pThis->mFlags[pThis->mFlags[1] + 5] == 0)
            vdp2PrintStatus.m10_palette = 0xC000;
        else
            vdp2PrintStatus.m10_palette = 0xB000;
        vdp2PrintfSmallFont("%s", s_debugMenuStrings[pThis->mFlags[1]]);
        vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
    }
}

// 0605e2b0
static void s_battleDebug_Delete(s_battleDebug* pThis)
{
    for (s32 i = 0; i < 0x1D; i++)
    {
        u32 bitIndex = i + 1000;
        if (bitIndex > 999)
            bitIndex = i + 0x1026;
        if (pThis->mFlags[i + 5] == 0)
            mainGameState.clearBit(bitIndex);
        else
            mainGameState.setBit(bitIndex);
    }
}

void createInBattleDebugTask(p_workArea parent)
{
    static const s_battleDebug::TypedTaskDefinition definition = {
        &s_battleDebug_Init,
        &s_battleDebug_Update,
        &s_battleDebug_Draw,
        &s_battleDebug_Delete,
    };

    createSubTask<s_battleDebug>(parent, &definition);
}
