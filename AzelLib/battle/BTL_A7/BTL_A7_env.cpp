#include "PDS.h"
#include "BTL_A7_env.h"
#include "BTL_A7_data.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/BTL_A3/BTL_A3_map6.h"
#include "shared/vdp2PlaneTask.h"
#include "field/field_a3/o_fld_a3.h"
#include "kernel/cinematicBarsTask.h"

void setupCinematicBarData(int param1, std::array<u32, 256>& dataArray, u32 vdpOffset, int numEntries);

// 06056d8c
static void BTL_A7_env0_Init(sVdp2PlaneTask* pThis)
{
    gBattleManager->m10_battleOverlay->m1C_envTask = (sVdp2PlaneTask*)pThis;

    reinitVdp2();
    initNBG1Layer();

    pThis->m78_ptr = (void*)allocateHeapForTask(pThis, 0xC00);

    asyncDmaCopy(g_BTL_A7->getSaturnPtr(0x060a9d38), getVdp2Cram(0x200), 0x20, 0);
    asyncDmaCopy(g_BTL_A7->getSaturnPtr(0x060a9868), getVdp2Cram(0x80), 0x20, 0);
    asyncDmaCopy(g_BTL_A7->getSaturnPtr(0x060a9888), getVdp2Cram(0xA0), 0x20, 0);
    asyncDmaCopy(g_BTL_A7->getSaturnPtr(0x060a98a8), getVdp2Cram(0xA00), 0x200, 0);
    asyncDmaCopy(g_BTL_A7->getSaturnPtr(0x060a9aa8), vdp2Palette, 0x200, 0);

    static const sLayerConfig nbg0Setup[] = {
        {m1_TPEN, 0}, {m2_CHCN, 0}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m34_W0E, 1}, {m37_W0A, 1}, {m44_CCEN, 1}, {m45_COEN, 0x10}, {m10_SPN, 5},
        {m22_N0LSCX, 1}, {m21_LCSY, 1}, {m20_N0LSS, 1},
        {m0_END},
    };
    setupNBG0(nbg0Setup);

    static const sLayerConfig rgb0Setup[] = {
        {m2_CHCN, 1}, {m5_CHSZ, 1}, {m6_PNB, 1}, {m7_CNSM, 0},
        {m27_RPMD, 2}, {m11_SCN, 8}, {m34_W0E, 1}, {m37_W0A, 1},
        {m0_END},
    };
    setupRGB0(rgb0Setup);

    static const sLayerConfig rotParamsSetup[] = { {m31_RxKTE, 1}, {m0_END} };
    setupRotationParams(rotParamsSetup);
    static const sLayerConfig rotParams2Setup[] = { {m0_END} };
    setupRotationParams2(rotParams2Setup);

    loadFile("FNS_A7_1.SCB", getVdp2Vram(0x10000), 0);
    loadFile("FNS_A7_1.PNB", getVdp2Vram(0x1E000), 0);
    loadFile("FRS_A7_1.SCB", getVdp2Vram(0x40000), 0);
    loadFile("FRS_A7_1.PNB", getVdp2Vram(0x60000), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    initLayerMap(0, 0x25E1E000, 0x25E1E000, 0x25E1E000, 0x25E1E000);

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mE_RAMCTL = (regs->mE_RAMCTL & 0xFF00) | 0xB4;
    regs->m10_CYCA0 = 0x310F754F;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, g_BTL_A7->getSaturnPtr(0x060a9db4));
    setupRotationMapPlanes(1, g_BTL_A7->getSaturnPtr(0x060a9df4));
    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80);
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x22000), 0x80);
    initRotationCoefficientTables(5, getVdp2Vram(0x2A000));
    setupCinematicBarData(1, *(std::array<u32, 256>*)pThis->m78_ptr, 0x25E24000, 0xC0);

    *(u16*)getVdp2Vram(0x2A400) = 0x0700;
    regs->mA8_LCTA = (regs->mA8_LCTA & 0xFFF80000) | 0x15200;
    *(u16*)getVdp2Vram(0x2A600) = 0xB4E2;
    regs->mAC_BKTA = (regs->mAC_BKTA & 0xFFF80000) | 0x15300;
    regs->mEC_CCCTL = regs->mEC_CCCTL & 0xFEFF;
    regs->mF0_PRISA = 0x405;
    regs->mF2_PRISB = 0x507;
    regs->mF4_PRISC = 0x505;
    regs->mF6_PRISD = 0x505;
    regs->mF8_PRINA = 0x604;
    regs->mFA_PRINB = 0x700;
    regs->mFC_PRIR = 3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C_scale = 0x10000;
    regs->mEC_CCCTL = regs->mEC_CCCTL & 0xFEFF;
    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xF8FF) | 0x400;
    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xFFF0) | 3;
    regs->mE0_SPCTL = (regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m50 = 0x0D;
    pThis->m54 = 0x17;
    pThis->m51 = 0x18;

    pThis->m4C_wavePhase = 0;
    pThis->m40_waveSpeed = 0xD5A6;
    pThis->m48 = 0xA3A;
    pThis->m44_waveFreq = 0x6E5D4D;

    Unimplemented(); // cinematic bar table init loop + NPC sub-task creation + OVPNRA setup
}

// 06057224
static void BTL_A7_env0_Update(sVdp2PlaneTask* pThis)
{
    pThis->m34 = 0;
}

// 060572c0
static void BTL_A7_env0_Draw(sVdp2PlaneTask* pThis)
{
    Unimplemented(); // VDP2 rotation scroll draw — same pattern as exca/camp/e011
}

// 06057788
void Create_BTL_A7_env0(s_workAreaCopy* parent)
{
    static const sVdp2PlaneTask::TypedTaskDefinition def = {
        &BTL_A7_env0_Init,
        &BTL_A7_env0_Update,
        &BTL_A7_env0_Draw,
        nullptr,
    };
    createSubTask<sVdp2PlaneTask>((s_workArea*)parent, &def);
}

// 060565b0
static void BTL_A7_env1_Init(sVdp2PlaneTask* pThis)
{
    gBattleManager->m10_battleOverlay->m1C_envTask = (sVdp2PlaneTask*)pThis;
    Unimplemented(); // same VDP2 init pattern as env0 but with different palette/file data (FNS_A7_2/FRS_A7_2)
}

// 06056136
static void BTL_A7_env12_Update(sVdp2PlaneTask* pThis)
{
    pThis->m34 = 0;
}

// 06056a24
static void BTL_A7_env1_Draw(sVdp2PlaneTask* pThis)
{
    Unimplemented(); // VDP2 rotation scroll draw
}

// 06056c54
void Create_BTL_A7_env1(s_workAreaCopy* parent)
{
    static const sVdp2PlaneTask::TypedTaskDefinition def = {
        &BTL_A7_env1_Init,
        &BTL_A7_env12_Update,
        &BTL_A7_env1_Draw,
        nullptr,
    };
    createSubTask<sVdp2PlaneTask>((s_workArea*)parent, &def);
}

// 06055d00
static void BTL_A7_env2_Init(sVdp2PlaneTask* pThis)
{
    gBattleManager->m10_battleOverlay->m1C_envTask = (sVdp2PlaneTask*)pThis;
    Unimplemented(); // same VDP2 init pattern as env0 but with different palette/file data (FNS_A7_3/FRS_A7_3)
}

// 060561ac
static void BTL_A7_env2_Draw(sVdp2PlaneTask* pThis)
{
    Unimplemented(); // VDP2 rotation scroll draw
}

// 06056588
void Create_BTL_A7_env2(s_workAreaCopy* parent)
{
    static const sVdp2PlaneTask::TypedTaskDefinition def = {
        &BTL_A7_env2_Init,
        &BTL_A7_env12_Update,
        &BTL_A7_env2_Draw,
        nullptr,
    };
    createSubTask<sVdp2PlaneTask>((s_workArea*)parent, &def);
}
