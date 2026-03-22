#include "PDS.h"
#include "BTL_A3_2_map.h"
#include "BTL_A3_2_data.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleGrid.h"
#include "kernel/graphicalObject.h"

#include "battle/BTL_A3/BTL_A3_map6.h" //TODO: cleanup
#include "town/town.h" //TODO: cleanup
#include "field/field_a3/o_fld_a3.h" //TODO: cleanup
#include "kernel/loadSavegameScreen.h"
#include "kernel/cinematicBarsTask.h"

void BTL_A3_2_Env_InitVdp2(s_BTL_A3_Env* pThis)
{
    gBattleManager->m10_battleOverlay->m1C_envTask = pThis;
    reinitVdp2();
    initNBG1Layer();
    asyncDmaCopy(g_BTL_A3_2->getSaturnPtr(0x60a86e0), getVdp2Cram(0xA00), 0x200, 0);
    asyncDmaCopy(g_BTL_A3_2->getSaturnPtr(0x60a8ae0), getVdp2Cram(0xC00), 0x200, 0);

    static const sLayerConfig setup[] =
    {
        m2_CHCN,  1,
        m5_CHSZ,  1, // character size is 2 cells x 2 cells (16*16)
        m6_PNB,   1, // pattern data size is 1 word
        m7_CNSM,  0,
        m27_RPMD, 2, // rotation parameter mode: Use both A&B
        m11_SCN,  8,
        m34_W0E,  1,
        m37_W0A,  1,
        m0_END,
    };
    setupRGB0(setup);

    static const sLayerConfig rotationPrams[] =
    {
        m31_RxKTE, 1, // use coefficient table
        m0_END,
    };
    setupRotationParams(rotationPrams);

    static const sLayerConfig rotationPrams2[] =
    {
        m0_END,
    };
    setupRotationParams2(rotationPrams2);

    loadFile("SCBTLA31.SCB", getVdp2Vram(0x40000), 0);
    loadFile("SCBTL_A3.PNB", getVdp2Vram(0x62800), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xb4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F754F;
    vdp2Controls.m_isDirty = 1;

    setupRotationMapPlanes(0, g_BTL_A3_2->getSaturnPtr(0x60a8d20));
    setupRotationMapPlanes(1, g_BTL_A3_2->getSaturnPtr(0x60a8ce0));

    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80); // setup coefficients table A
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x24000), 0x80); // setup coefficients table B
    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x2A000));

    // setup line color screen
    *(u16*)getVdp2Vram(0x2A400) = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    // setup back screen color
    *(u16*)getVdp2Vram(0x2A600) = 0x38E5;
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;
    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = (vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF);
    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x405;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x507;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x600;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 0x3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C = 0x10000;
    pThis->m38 = -0x6C000;

    static const std::vector<std::array<s32, 2>> config = {
        {
            {0x2C, 0x1},
        }
    };
    applyLayerDisplayConfig(config);

    vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL = (vdp2Controls.m4_pendingVdp2Regs->mEC_CCCTL & 0xFEFF);
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xF8FF) | 0x400;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xFFF0) | 0x3;
    vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL = (vdp2Controls.m4_pendingVdp2Regs->mE0_SPCTL & 0xCFFF) | 0x1000;

    pThis->m50 = 0x10;
    pThis->m51 = 0x12;
    pThis->m52 = 0x14;
    vdp2Controls.m4_pendingVdp2Regs->m100_CCRSA = pThis->m50 | (pThis->m51 << 8);
    vdp2Controls.m4_pendingVdp2Regs->m102_CCRSB = pThis->m52;
    vdp2Controls.m4_pendingVdp2Regs->m104_CCRSC = 0;
    vdp2Controls.m4_pendingVdp2Regs->m106_CCRSD = 0;

    pThis->m55 = 8;
    vdp2Controls.m4_pendingVdp2Regs->m10C_CCRR = pThis->m55;
    vdp2Controls.m_isDirty = 1;

    s_BTL_A3_Env_InitVdp2Sub4(g_BTL_A3_2->getSaturnPtr(0x60a8db4));
}

void BTL_A3_2_map_init(s_BTL_A3_Env* pThis)
{
    BTL_A3_2_Env_InitVdp2(pThis);
    allocateNPC(pThis, 10);
    initGridForBattle(dramAllocatorEnd[10].mC_fileBundle, g_BTL_A3_2->m_map, 2, 2, 0x200000);
    pThis->m58 = dramAllocatorEnd[10].mC_fileBundle;
    gBattleManager->m10_battleOverlay->m8_gridTask->m1C8_flags |= 0x10;
}

void BTL_A3_2_map_update(s_BTL_A3_Env* pThis)
{
    updateWorldGrid(gBattleManager->m10_battleOverlay->m8_gridTask->m180_cameraTranslation[0], gBattleManager->m10_battleOverlay->m8_gridTask->m180_cameraTranslation[2]);
}

// 0605445c
void BTL_A3_2_Env_Draw(s_BTL_A3_Env* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();
    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_vdp1LocalCoordinates);
    getVdp1ProjectionParams(&pThis->m30_vdp1ProjectionParam[0], &pThis->m30_vdp1ProjectionParam[1]);

    beginRotationPass(0, intDivide(pThis->m30_vdp1ProjectionParam[0], fixedPoint::fromInteger(pThis->m30_vdp1ProjectionParam[1])));
    buildGroundRotation(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34 = computeRotationScrollOffset();

    s32 scrollX = (pThis->m18_cameraRotation.m4_Y >> 0xC) * -0x400;
    s32 scrollY = (0x1EF - pThis->m34) * 0x10000;

    beginRotationPass(1, intDivide(pThis->m30_vdp1ProjectionParam[0], fixedPoint::fromInteger(pThis->m30_vdp1ProjectionParam[1])));

    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];
    s32 iX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    t.m34 = (s16)((iX + (int)(iX < 0)) >> 1);
    s32 iY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    t.m36 = (s16)((iY + (int)(iY < 0)) >> 1);
    t.m38 = pThis->m30_vdp1ProjectionParam[1];
    t.m3C = t.m34;
    t.m3E = t.m36;
    t.m40 = 0;

    buildRotationMatrixRoll(-pThis->m18_cameraRotation.m8_Z);
    intDivide(pThis->m24_vdp1Clipping[3] - pThis->m24_vdp1Clipping[1], 0x00E00000);
    scaleRotationMatrix(intDivide(pThis->m24_vdp1Clipping[2] - pThis->m24_vdp1Clipping[0], 0x01600000));
    setRotationScrollOffset(scrollX, scrollY);
    commitRotationPass();

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
}

void BTL_A3_2_createMap(p_workArea parent)
{
    s_BTL_A3_Env::TypedTaskDefinition definition = {
        BTL_A3_2_map_init,
        BTL_A3_2_map_update,
        BTL_A3_2_Env_Draw,
        nullptr,
    };
    createSubTask<s_BTL_A3_Env>(parent, &definition);
}
