#include "PDS.h"
#include "BTL_A5_3_env.h"
#include "BTL_A5_3_data.h"
#include "BTL_A5_env.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "kernel/loadSavegameScreen.h"
#include "field/field_a3/o_fld_a3.h"

// 0605794c
static void BTL_A5_3_env_Init(s_BTL_A3_Env* pThis)
{
    gBattleManager->m10_battleOverlay->m1C_envTask = pThis;
    reinitVdp2();
    initNBG1Layer();

    asyncDmaCopy(g_BTL_A5_3->getSaturnPtr(0x060a4370), getVdp2Cram(0x800), 0x200, 0);
    asyncDmaCopy(g_BTL_A5_3->getSaturnPtr(0x060a4170), getVdp2Cram(0xA00), 0x200, 0);

    static const sLayerConfig rgb0Setup[] = {
        m2_CHCN,  1,
        m5_CHSZ,  1,
        m6_PNB,   1,
        m7_CNSM,  0,
        m27_RPMD, 2,
        m11_SCN,  8,
        m34_W0E,  1,
        m37_W0A,  1,
        m0_END,
    };
    setupRGB0(rgb0Setup);

    static const sLayerConfig rotationParams[] = {
        m31_RxKTE, 1,
        m0_END,
    };
    setupRotationParams(rotationParams);

    static const sLayerConfig rotationParams2[] = {
        m0_END,
    };
    setupRotationParams2(rotationParams2);

    loadFile("FNS_A5_0.SCB", getVdp2Vram(0x10000), 0);
    loadFile("FNS_A5_0.PNB", getVdp2Vram(0x1C000), 0);
    loadFile("FRS_A5_0.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_A5_0.PNB", getVdp2Vram(0x60000), 0);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xb4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F754F;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, g_BTL_A5_3->getSaturnPtr(0x060a45e0));
    setupRotationMapPlanes(1, g_BTL_A5_3->getSaturnPtr(0x060a4620));

    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x24000), 0x80);
    initRotationCoefficientTables(5, getVdp2Vram(0x2A000));

    *(u16*)getVdp2Vram(0x2A400) = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    *(u16*)getVdp2Vram(0x2A600) = 0xBF3B;
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x405;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x507;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x600;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 0x3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C = 0x40000;

    static const std::vector<std::array<s32, 2>> layerDisplayConfig = {
        { {0x2C, 0x1} }
    };
    applyLayerDisplayConfig(layerDisplayConfig);

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x400;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 0x3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m50 = 0x0D;
    pThis->m51 = 0x18;
    vdp2Controls.m4_pendingVdp2Regs->m100_CCRSA = pThis->m50 | (pThis->m51 << 8);
    vdp2Controls.m4_pendingVdp2Regs->m102_CCRSB = 0;
    vdp2Controls.m4_pendingVdp2Regs->m104_CCRSC = 0;
    vdp2Controls.m4_pendingVdp2Regs->m106_CCRSD = 0;

    pThis->m55 = 8;
    vdp2Controls.m4_pendingVdp2Regs->m10C_CCRR = pThis->m55;
    vdp2Controls.m_isDirty = 1;

    createPaletteAnimTask(pThis);

    s_BTL_A3_Env_InitVdp2Sub4(g_BTL_A5_3->getSaturnPtr(0x060a46f0));
}

// 06057d10
p_workArea Create_BTL_A5_3_env(p_workArea parent)
{
    static const s_BTL_A3_Env::TypedTaskDefinition definition = {
        &BTL_A5_3_env_Init,
        &BTL_A3_Env_Update,
        &BTL_A5_env_Draw,
        nullptr,
    };

    return createSubTask<s_BTL_A3_Env>(parent, &definition);
}
