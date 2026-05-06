#include "PDS.h"
#include "dragonLightWingEvolution.h"
#include "field/fieldDragonInput.h"
#include "kernel/fade.h"
#include "audio/systemSounds.h"

// shared field functions (local to each overlay)
u32 isDragonInputAllowed(s_dragonTaskWorkArea* r4);
void initDragonMovementMode();
// 060071a8
static void vdp2WriteModelChangeString(s32 x, s32 y, s32 palette, const char* str)
{
    vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
    vdp2PrintStatus.m10_palette = (palette & 0xF) << 12;
    vdp2DebugPrintSetPosition(x, y);
    vdp2PrintfLargeFont("%s", str);
    vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
}

// 060071e4
static void vdp2WriteModelChangeValue(s32 x, s32 y, s32 palette, s32 format, s32 value)
{
    vdp2PrintStatus.m14_oldPalette = vdp2PrintStatus.m10_palette;
    vdp2PrintStatus.m10_palette = (palette & 0xF) << 12;
    vdp2DebugPrintSetPosition(x, y);
    if (format == 8)
        vdp2PrintfLargeFont("%8d", value);
    else if (format == 3)
        vdp2PrintfLargeFont("%2d", value);
    else if (format == 2)
        vdp2PrintfLargeFont("+%3d", value);
    else if (format == 1)
        vdp2PrintfLargeFont("+%2d", value);
    vdp2PrintStatus.m10_palette = vdp2PrintStatus.m14_oldPalette;
}

static const char* s_dragonNames[] = {
    "DRAGON VALIANT WING",
    "DRAGON STRIPE WING",
    "DRAGON PANZER WING",
    "DRAGON EYE WING",
    "DRAGON ARM WING",
    "DRAGON LIGHT WING",
    "DRAGON SOLO WING",
};

// 06007280
void modelChange(s32 dragonType)
{
    static const struct { s16 hp; s8 bp; s8 laser; s8 gun; s8 pad; } modelChangeTable[] = {
        { 40, 10,  8,  6, 0},  // type 1
        { 60, 15, 12,  9, 0},  // type 2
        { 90, 23, 18, 14, 0},  // type 3
        {135, 34, 27, 20, 0},  // type 4
        {203, 51, 41, 30, 0},  // type 5
        {304, 76, 61, 46, 0},  // type 6
    };

    s32 idx = dragonType - 1;
    if (idx >= 6) idx = 5;
    s16 hpDelta = modelChangeTable[idx].hp;
    s8 bpDelta = modelChangeTable[idx].bp;

    mainGameState.gameStats.mC_laserPower += modelChangeTable[idx].laser;
    mainGameState.gameStats.mE_gunPower += modelChangeTable[idx].gun;

    if ((s32)(s16)mainGameState.gameStats.mB8_maxHP + (s32)hpDelta > 9999)
        hpDelta = 9999 - mainGameState.gameStats.mB8_maxHP;
    mainGameState.gameStats.m12_classMaxHP += hpDelta;

    if ((s32)(s16)mainGameState.gameStats.mBA_maxBP + (s32)bpDelta > 999)
        bpDelta = 999 - mainGameState.gameStats.mBA_maxBP;
    mainGameState.gameStats.m16_classMaxBP += bpDelta;

    // Display model change info
    vdp2WriteModelChangeString(0x10, 0x06, 0x0D, "MODEL CHANGE");
    vdp2WriteModelChangeString(0x0D, 0x0C, 0x0C, "HP Max");
    vdp2WriteModelChangeString(0x0D, 0x0E, 0x0C, "BP Max");
    vdp2WriteModelChangeString(0x0D, 0x10, 0x0C, "LASER Power");
    vdp2WriteModelChangeString(0x0D, 0x12, 0x0C, "SHOT  Power");
    if (dragonType != 7)
        vdp2WriteModelChangeString(0x0F, 0x15, 0x0D, "LASER RANK UP");
    vdp2WriteModelChangeString(0x0D, 0x09, 0x08, s_dragonNames[dragonType - 1]);

    vdp2WriteModelChangeValue(0x1A, 0x0C, 0x0C, 2, (s32)hpDelta);
    vdp2WriteModelChangeValue(0x1A, 0x0E, 0x0C, 2, (s32)bpDelta);
    vdp2WriteModelChangeValue(0x1A, 0x10, 0x0C, 2, (s32)modelChangeTable[idx].laser);
    vdp2WriteModelChangeValue(0x1A, 0x12, 0x0C, 2, (s32)modelChangeTable[idx].gun);
}
p_workArea createLightWingEffect(p_workArea parent, s16 p1, s16 p2, s16 p3, s16 p4, s16 p5, s16 p6);

// 06068300
bool areAllDUnitsCollected()
{
    for (int i = 0; i < 12; i++)
    {
        if (!mainGameState.getBit(0x56 + 0xF3 + i))
        {
            return false;
        }
    }
    return true;
}

// 0606802a
struct s_dragonLightWingEvolutionTask : public s_workAreaTemplateWithArg<s_dragonLightWingEvolutionTask, u8>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, &Update, NULL, NULL };
        return &taskDefinition;
    }

    static void Init(s_dragonLightWingEvolutionTask* pThis, u8 arg)
    {
        pThis->m4_dragonType = arg;
    }

    // 06068030
    static void Update(s_dragonLightWingEvolutionTask* pThis)
    {
        s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
        s8 state = pThis->m5_state;

        switch (state)
        {
        case 0:
            if (!isDragonInputAllowed(pDragon))
                return;
            initDragonMovementMode();
            if ((s8)g_fadeControls.m_4C <= (s8)g_fadeControls.m_4D)
            {
                vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
                vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
            }
            {
                u32 color = convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color);
                fadePalette(&g_fadeControls.m0_fade0, color, 0xFFFF, 0x1E);
            }
            playSystemSoundEffect(0x1B);
            pThis->m5_state++;
            break;
        case 1:
            if (g_fadeControls.m0_fade0.m20_stopped == 0)
                return;
            pThis->m5_state++;
            break;
        case 2:
        {
            updateDragonIfCursorChanged(pThis->m4_dragonType);
            if (pThis->m4_dragonType == 6)
            {
                s16 vdp1Base = (s16)(((s32)pDragon->m0.m4_characterArea + (s32)0xDA400000) >> 3);
                pDragon->mB8_lightWingEffect = createLightWingEffect(pDragon,
                    vdp1Base + 0x11FC, vdp1Base + 0x1790, 0x20C,
                    vdp1Base + 0x1208, vdp1Base + 0x1790, 0x20C);
            }
            else if (pDragon->mB8_lightWingEffect)
            {
                pDragon->mB8_lightWingEffect->getTask()->markFinished();
                pDragon->mB8_lightWingEffect = nullptr;
            }
            pThis->m5_state++;
            break;
        }
        case 3:
            modelChange(pThis->m4_dragonType);
            updateDragonStatsFromLevel();
            mainGameState.gameStats.m10_currentHP = mainGameState.gameStats.mB8_maxHP;
            mainGameState.gameStats.m14_currentBP = mainGameState.gameStats.mBA_maxBP;
            pThis->m0_countdown = 0x5A;
            if ((s8)g_fadeControls.m_4C <= (s8)g_fadeControls.m_4D)
            {
                vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
                vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
            }
            {
                u32 color = convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color);
                fadePalette(&g_fadeControls.m0_fade0, color, (s32)(s16)g_fadeControls.m_48, 0x1E);
                u32 color2 = convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color);
                fadePalette(&g_fadeControls.m24_fade1, color2, (s32)(s16)g_fadeControls.m_4A, 0x1E);
            }
            pThis->m5_state++;
            break;
        case 4:
            pThis->m0_countdown--;
            if (pThis->m0_countdown < 1)
                pThis->m0_countdown = 0;
            if (pThis->m0_countdown != 0)
                return;
            if (g_fadeControls.m0_fade0.m20_stopped == 0)
                return;
            pThis->m5_state++;
            pThis->m6 = 1;
            break;
        case 5:
            clearVdp2TextMemory();
            Unimplemented(); // overlay-specific cutscene setup + script dispatch
            pThis->m5_state++;
            break;
        case 6:
            if (!getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE->m4_currentScript.isNull())
                return;
            pThis->m5_state++;
            break;
        case 7:
            dragonTransitionFromScript();
            pThis->getTask()->markFinished();
            break;
        default:
            break;
        }
    }

    s32 m0_countdown;
    u8 m4_dragonType;
    s8 m5_state;
    u8 m6;
    // size 0x8
};

// 060682de
void createDragonLightWingEvolutionTask(u32 param)
{
    createSubTaskWithArg<s_dragonLightWingEvolutionTask>(getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask, (u8)param);
}

// Polling task — checks each frame if all D-units have been collected
struct s_dragonLightWingEvolutionCheckTask : public s_workAreaTemplate<s_dragonLightWingEvolutionCheckTask>
{
};

// 06068340
static void dragonLightWingEvolutionCheckTask_Update(s_dragonLightWingEvolutionCheckTask* pThis)
{
    if (areAllDUnitsCollected())
    {
        createDragonLightWingEvolutionTask(6);
        pThis->getTask()->m14_flags |= 1;
    }
}

// 06068394
void initDragonLightWingEvolutionCheck(s_dragonTaskWorkArea* pDragon)
{
    if (!areAllDUnitsCollected())
    {
        createSubTaskFromFunction<s_dragonLightWingEvolutionCheckTask>(pDragon, &dragonLightWingEvolutionCheckTask_Update);
    }
}
