#include "PDS.h"
#include "campDragonNameEntry.h"
#include "town/town.h"
#include "town/townCamera.h"
#include "field/field_d5/d5_nameEntry.h"

// 06057d52 — darken light data by subtracting a value, clamped to -31
static void darkenLightData(s8* output, s32 amount) {
    for (int i = 0; i < 12; i++) {
        s32 val = (s8)readSaturnU8(cameraTaskPtr->m8_colorData + i) - amount;
        if (val < -0x1F) val = -0x1F;
        output[i] = (s8)val;
    }
}

// computeModulatedColor — takes 12-byte light data + intensity, returns VDP2 color
static u16 computeModulatedColor(s8* lightData, u32 intensity) {
    s32 r = MTH_Mul((s32)lightData[0], intensity);
    s32 g = MTH_Mul((s32)lightData[1], intensity);
    s32 b = MTH_Mul((s32)lightData[2], intensity);
    s8 baseR = lightData[3];
    s8 baseG = lightData[4];
    s8 baseB = lightData[5];
    return 0x8000 | (((r + baseR) >> 1) + 8) | ((((g + baseG) >> 1) + 8) << 5) | ((((b + baseB) >> 1) + 8) << 10);
}

// 06057d74 — darken and apply light
static void darkenAndApplyLight(s32 amount) {
    s8 darkened[12];
    darkenLightData(darkened, amount);
    cameraTaskPtr->m10.m0 = darkened[0];
    cameraTaskPtr->m10.m1 = darkened[1];
    cameraTaskPtr->m10.m2 = darkened[2];
    u32 f0 = (u32)(u8)darkened[5] << 16 | (u32)(u8)darkened[4] << 8 | (u32)(u8)darkened[3];
    u32 f1 = (u32)(u8)darkened[8] << 16 | (u32)(u8)darkened[7] << 8 | (u32)(u8)darkened[6];
    u32 f2 = (u32)(u8)darkened[11] << 16 | (u32)(u8)darkened[10] << 8 | (u32)(u8)darkened[9];
    generateLightFalloffMap(f0, f1, f2);
}

// 06057d08 — set VDP2 priorities for name entry screen
static void setupNameEntryVdp2Priorities() {
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x23F467F;
    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x204;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x607;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x404;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 5;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x706;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;
}

// Dragon name entry task for camp overlay
struct sCampDragonNameEntryTask : public s_workAreaTemplate<sCampDragonNameEntryTask>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition td = { nullptr, nullptr, &sCampDragonNameEntryTask::Draw, nullptr };
        return &td;
    }

    // 06057df6
    static void Draw(sCampDragonNameEntryTask* pThis)
    {
        switch (pThis->m4_state) {
        case 0:
        {
            // Fade to darkened scene
            pauseEngine[0] = 1;
            s8 darkened[12];
            darkenLightData(darkened, 8);
            u16 fadeColor = computeModulatedColor(darkened, cameraTaskPtr->m30_colorIntensity);
            s32 curColor = convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color);
            fadePalette(&g_fadeControls.m24_fade1, curColor, fadeColor, 0x10);
            pThis->m4_state++;
            break;
        }
        case 1:
        {
            // Darken lighting gradually
            s32 counter = pThis->m8;
            pThis->m8 = counter + 1;
            if (counter + 1 < 9) {
                darkenAndApplyLight(pThis->m8);
            } else {
                pThis->m4_state++;
            }
            break;
        }
        case 2:
        {
            // Create the name entry UI widget (shared code at 06023728)
            pThis->m0_nameEntrySubTask = nameEntry(pThis, mainGameState.gameStats.mA5_dragonName);
            setupNameEntryVdp2Priorities();
            pThis->m4_state++;
            break;
        }
        case 3:
        {
            // Wait for name entry to complete
            if (!isNameEntryComplete(pThis->m0_nameEntrySubTask)) {
                return; // not done yet
            }
            if (pThis->m0_nameEntrySubTask) {
                pThis->m0_nameEntrySubTask->getTask()->markFinished();
            }
            setGlobalStringTableEntry(1, mainGameState.gameStats.mA5_dragonName);
            pThis->m4_state++;
            break;
        }
        case 4:
        {
            // Fade back to normal lighting
            pThis->m8 = 8;
            s8 lightData[12];
            for (int i = 0; i < 12; i++) lightData[i] = (s8)readSaturnU8(cameraTaskPtr->m8_colorData + i);
            u16 fadeColor = computeModulatedColor(lightData, cameraTaskPtr->m30_colorIntensity);
            s32 curColor = convertColorToU32ForFade(g_fadeControls.m24_fade1.m0_color);
            fadePalette(&g_fadeControls.m24_fade1, curColor, fadeColor, 0x10);
            pThis->m4_state++;
            break;
        }
        case 5:
        {
            // Restore lighting gradually
            s32 counter = pThis->m8;
            pThis->m8 = counter - 1;
            if (counter - 1 >= 0) {
                darkenAndApplyLight(pThis->m8);
            } else {
                pThis->m4_state++;
            }
            break;
        }
        case 6:
        {
            // Done — clean up
            pauseEngine[0] = 0;
            pThis->getTask()->markFinished();
            break;
        }
        default:
            break;
        }
    }

    p_workArea m0_nameEntrySubTask;
    s32 m4_state;
    s32 m8;
    // size 0xC
};

// 06057f46
s32 startDragonNameEntryTask() {
    preloadNameEntryResources();
    createSubTask<sCampDragonNameEntryTask>(townDebugTask2);
    return 0;
}
