#include "PDS.h"
#include "BTL_A3_map6.h"
#include "battle/battleManager.h"
#include "battle/battleOverlay.h"
#include "battle/battleGrid.h"
#include "field/field_a3/o_fld_a3.h" // TODO: remove
#include "kernel/graphicalObject.h"
#include "kernel/fileBundle.h"
#include "kernel/loadSavegameScreen.h" //TODO: remove
#include "BTL_A3_data.h"
#include "kernel/grid.h"
#include "kernel/cinematicBarsTask.h"
#include "town/town.h" // TODO: cleanup

void s_BTL_A3_Env_InitVdp2Sub4(sSaturnPtr param_1)
{
    // Setup clip distances from Saturn data
    auto& clip = graphicEngineStatus.m405C;
    clip.m14_farClipDistance  = readSaturnS32(param_1 + 0x38);
    clip.m38_oneOverFarClip   = FP_Div(0x8000, clip.m14_farClipDistance);
    clip.m34_oneOverFarClip256 = clip.m38_oneOverFarClip << 8;
    clip.m10_nearClipDistance = readSaturnS32(param_1 + 0x3C);
    clip.m30_oneOverNearClip  = FP_Div(0x10000, clip.m10_nearClipDistance);

    // Setup light parameters in the battle grid
    s_battleGrid* grid = gBattleManager->m10_battleOverlay->m8_gridTask;

    grid->m1CC_lightColor    = readSaturnVec3(param_1 + 0x00);
    grid->m1D8_newLightColor = grid->m1CC_lightColor;

    grid->m1E4_lightFalloff0 = readSaturnVec3(param_1 + 0x0C);
    grid->m1F0               = grid->m1E4_lightFalloff0;

    grid->m1FC_lightFalloff1 = readSaturnVec3(param_1 + 0x18);
    grid->m208_lightFalloff2 = readSaturnVec3(param_1 + 0x24);

    grid->m280_lightAngle1       = readSaturnS32(param_1 + 0x30);
    grid->m284_lightAngle2       = readSaturnS32(param_1 + 0x34);
    grid->m28C_lightAngle1Initial = grid->m280_lightAngle1;
    grid->m290_lightAngle2Initial = grid->m284_lightAngle2;
}

void setupRotationMapPlanes(int rotationMapIndex, sSaturnPtr inPlanes)
{
    u32 characterSize = vdp2Controls.m4_pendingVdp2Regs->m2A_CHCTLB & 0x100;
    u32 patternNameDataSize = vdp2Controls.m4_pendingVdp2Regs->m38_PNCR & 0x8000;

    u32 shiftValue;
    if (patternNameDataSize)
    {
        // 1 word
        if (characterSize)
        {
            // 2x2
            shiftValue = 11; // 0x800
        }
        else
        {
            // 1x1
            shiftValue = 13; // 0x2000
        }
    }
    else
    {
        // 2 words
        if (characterSize)
        {
            // 2x2
            shiftValue = 12; // 0x1000
        }
        else
        {
            // 1x1
            shiftValue = 14; // 0x4000
        }
    }

    std::array<u32, 16> planes;
    for (int i = 0; i < 16; i++)
    {
        planes[i] = readSaturnU32(inPlanes + i * 4);
    }

    u32 mapOffset = (rotateRightR0ByR1(planes[0] + 0xDA200000, shiftValue + 6)) & 7;

    switch (rotationMapIndex)
    {
    case 0:
        vdp2Controls.m4_pendingVdp2Regs->m3E_MPOFR = (vdp2Controls.m4_pendingVdp2Regs->m3E_MPOFR & 0xFFF0) | (mapOffset);
        vdp2Controls.m4_pendingVdp2Regs->m50_MPABRA = ((rotateRightR0ByR1(planes[1], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[0], shiftValue) & 0x3F);
        vdp2Controls.m4_pendingVdp2Regs->m52_MPCDRA = ((rotateRightR0ByR1(planes[3], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[2], shiftValue) & 0x3F);
        vdp2Controls.m4_pendingVdp2Regs->m54_MPEFRA = ((rotateRightR0ByR1(planes[5], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[4], shiftValue) & 0x3F);
        vdp2Controls.m4_pendingVdp2Regs->m56_MPGHRA = ((rotateRightR0ByR1(planes[7], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[6], shiftValue) & 0x3F);
        vdp2Controls.m4_pendingVdp2Regs->m58_MPIJRA = ((rotateRightR0ByR1(planes[9], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[8], shiftValue) & 0x3F);
        vdp2Controls.m4_pendingVdp2Regs->m5A_MPKLRA = ((rotateRightR0ByR1(planes[11], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[10], shiftValue) & 0x3F);
        vdp2Controls.m4_pendingVdp2Regs->m5C_MPMNRA = ((rotateRightR0ByR1(planes[13], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[12], shiftValue) & 0x3F);
        vdp2Controls.m4_pendingVdp2Regs->m5E_MPOPRA = ((rotateRightR0ByR1(planes[15], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[14], shiftValue) & 0x3F);
        break;
    case 1:
        vdp2Controls.m4_pendingVdp2Regs->m3E_MPOFR = (vdp2Controls.m4_pendingVdp2Regs->m3E_MPOFR & 0xFF0F) | (mapOffset << 4);
        vdp2Controls.m4_pendingVdp2Regs->m60_MPABRB = ((rotateRightR0ByR1(planes[1], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[0], shiftValue) & 0x3F);
        vdp2Controls.m4_pendingVdp2Regs->m62_MPCDRB = ((rotateRightR0ByR1(planes[3], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[2], shiftValue) & 0x3F);
        vdp2Controls.m4_pendingVdp2Regs->m64_MPEFRB = ((rotateRightR0ByR1(planes[5], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[4], shiftValue) & 0x3F);
        vdp2Controls.m4_pendingVdp2Regs->m66_MPGHRB = ((rotateRightR0ByR1(planes[7], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[6], shiftValue) & 0x3F);
        vdp2Controls.m4_pendingVdp2Regs->m68_MPIJRB = ((rotateRightR0ByR1(planes[9], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[8], shiftValue) & 0x3F);
        vdp2Controls.m4_pendingVdp2Regs->m6A_MPKLRB = ((rotateRightR0ByR1(planes[11], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[10], shiftValue) & 0x3F);
        vdp2Controls.m4_pendingVdp2Regs->m6C_MPMNRB = ((rotateRightR0ByR1(planes[13], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[12], shiftValue) & 0x3F);
        vdp2Controls.m4_pendingVdp2Regs->m6E_MPOPRB = ((rotateRightR0ByR1(planes[15], shiftValue) & 0x3F) << 8) | (rotateRightR0ByR1(planes[14], shiftValue) & 0x3F);
        break;
    default:
        assert(0);
    }

    vdp2Controls.m_isDirty = 1;
}

tCoefficientTable coefficientA0(0x80 * 4);
tCoefficientTable coefficientA1(0x80 * 4);
tCoefficientTable coefficientB0(0x80 * 4);
tCoefficientTable coefficientB1(0x80 * 4);

std::array<std::array<sCoefficientTableData, 2>, 2> gCoefficientTables;

// TODO: figure out why the disassembly of r3 craps out
void setVdp2TableAddress(int p1, u8* vdp2Dest)
{
    int var5 = (getVdp2VramOffset(vdp2Dest) - 0x25E00000) / 2;
    int bankIndex = performModulo(0x40000, var5 * 2);

    int r3 = var5;
    if (r3 >= 0)
    {
        r3 += 0x1FFFF;
    }

    r3 /= 2;
    r3 >>= 16;

    switch (p1)
    {
    case 1: // Line scroll table address (NBG0)
        vdp2Controls.m4_pendingVdp2Regs->mA0_LSTA0 = vdp2Dest;
        break;
    case 2: // Line scroll table address (NBG1)
        vdp2Controls.m4_pendingVdp2Regs->mA4_LSTA1 = vdp2Dest;
        break;
    case 3: // vertical cell scroll table address (NBG0 & NBG1)
        vdp2Controls.m4_pendingVdp2Regs->m9C_VCSTA = vdp2Dest;
        break;
    case 4:
    case 5:
        break;
    case 6: // RAKTAOS Coefficient Table Address Offset A
        vdp2Controls.m4_pendingVdp2Regs->mB6_KTAOF &= 0xFFF8;
        vdp2Controls.m4_pendingVdp2Regs->mB6_KTAOF |= r3;
        vdp2Controls.m_isDirty = 1;
        vdp2Controls.m_C = bankIndex >> 2;
        break;
    case 7: // RAKTAOS Coefficient Table Address Offset B
        vdp2Controls.m4_pendingVdp2Regs->mB6_KTAOF &= 0xF8FF;
        vdp2Controls.m4_pendingVdp2Regs->mB6_KTAOF |= r3 << 8;
        vdp2Controls.m_isDirty = 1;
        vdp2Controls.m_10 = bankIndex >> 2;
        break;
    case 8: // Line color screen table address
        vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA &= 0xfff80000;
        vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA |= var5;
        break;
    case 9: // back screen table address
        vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA &= 0xfff80000;
        vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA |= var5;
        break;
    case 10: // line window table address window 0
        vdp2Controls.m4_pendingVdp2Regs->mD8_LWTA0 = var5;
        break;
    case 11: // line window table address window 1
        vdp2Controls.m4_pendingVdp2Regs->mDC_LWTA1 = var5;
        break;
    default:
        assert(0);
    }

    vdp2Controls.m_isDirty = 1;
}

void setupScrollAndRotation(int p1, void* p2, void* p3, u8* coefficientTableAddress, u8 p5)
{
    sVdpVar1& dest = vdpVar1[p1];
    dest.mE_isDoubleBuffered = 1;
    dest.m0_source[0] = &p2;
    dest.m0_source[1] = &p3;
    dest.m8_destination = coefficientTableAddress;
    dest.mC_size = p5;
    dest.m10_nextTransfert = 0;

    setVdp2TableAddress(p1, coefficientTableAddress);
}

void initCoefficientTable(std::vector<fixedPoint>& p2, std::vector<fixedPoint>& p3, int size)
{
    for (int i=0; i<size; i++)
    {
        p2[i] = fixedPoint(0x10000);
        p3[i] = fixedPoint(0x10000);
    }
}

std::array<std::array<std::vector<fixedPoint>*, 2>, 2> gVdp2CoefficientTables;

// TODO: kernel
void setupVdp2Table(int p1, std::vector<fixedPoint>& p2, std::vector<fixedPoint>& p3, u8* coefficientTableAddress, u8 numCoefficients)
{
    setupScrollAndRotation(p1, p2.data(), p3.data(), coefficientTableAddress, numCoefficients);
    initCoefficientTable(p2, p3, numCoefficients * 4);

    // TODO: this is super broken in ghidra, why?
    switch (p1)
    {
    case 6:
        gVdp2CoefficientTables[0][0] = &p2;
        gVdp2CoefficientTables[0][1] = &p3;
        break;
    case 7:
        gVdp2CoefficientTables[1][0] = &p2;
        gVdp2CoefficientTables[1][1] = &p3;
        break;
    default:
        break;
    }
}

void s_BTL_A3_Env_InitVdp2Sub3Sub1(sCoefficientTableData& table)
{
    table.m0 = 0;
    table.m4 = 0;
    table.m8_Zst = 0;
    table.mC = 0;
    table.m10 = 0x10000;
    table.m14 = 0x10000;
    table.m18 = 0;
    table.m1C = 0x10000;
    table.m20 = 0;
    table.m24 = 0;
    table.m28 = 0;
    table.m2C = 0x10000;
    table.m30 = 0;
    table.m34 = 0xB0;
    table.m36 = 0x70;
    table.m38 = 0xFB;
    table.m3A = 0;
    table.m3C = 0xB0;
    table.m3E = 0x70;
    table.m40 = 0;
    table.m42 = 0;
    table.m44 = 0;
    table.m48 = 0;
    table.m4C = 0x10000;
    table.m50 = 0x10000;
    table.m54 = 0;
    table.m58 = 0;
    table.m5C = 0;
    table.m60 = 0;
    table.m64 = 0;
    table.m68 = 0;
    table.m6C = 0x10000;
}

void s_BTL_A3_Env_InitVdp2Sub3(int layerIndex, u8* table)
{
    if (layerIndex != 4)
    {
        if (layerIndex != 5)
        {
            return;
        }

        s_BTL_A3_Env_InitVdp2Sub3Sub1(gCoefficientTables[1][0]);
        s_BTL_A3_Env_InitVdp2Sub3Sub1(gCoefficientTables[1][1]);
        setupScrollAndRotation(5, &gCoefficientTables[1][0], &gCoefficientTables[1][1], table + 0x80, 6);
    }

    s_BTL_A3_Env_InitVdp2Sub3Sub1(gCoefficientTables[0][0]);
    s_BTL_A3_Env_InitVdp2Sub3Sub1(gCoefficientTables[0][1]);
    setupScrollAndRotation(5, &gCoefficientTables[0][0], &gCoefficientTables[0][1], table + 0x80, 6);
}

void BTL_A3_Env_InitVdp2(s_BTL_A3_Env* pThis)
{
    gBattleManager->m10_battleOverlay->m1C_envTask = pThis;
    reinitVdp2();

    initNBG1Layer();

    asyncDmaCopy(g_BTL_A3->getSaturnPtr(0x60A68BC), getVdp2Cram(0x400), 0x200, 0);
    asyncDmaCopy(g_BTL_A3->getSaturnPtr(0x60A66BC), getVdp2Cram(0xA00), 0x200, 0);
    asyncDmaCopy(g_BTL_A3->getSaturnPtr(0x60A6ABC), getVdp2Cram(0xC00), 0x200, 0);

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

    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL = (vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL & 0xFF00) | 0xb4;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x310F754F;
    vdp2Controls.m_isDirty = 1;

    switch (gBattleManager->m6_subBattleId)
    {
    case 0:
    case 1:
    case 3:
    case 5:
    case 6:
    case 7:
    case 8:
        setupRotationMapPlanes(0, g_BTL_A3->getSaturnPtr(0x60a6cfc));
        break;
    case 2:
    case 4:
    case 9:
        setupRotationMapPlanes(0, g_BTL_A3->getSaturnPtr(0x60a6d80));
        break;
    default:
        assert(0);
    }

    setupRotationMapPlanes(1, g_BTL_A3->getSaturnPtr(0x60a6d3c));

    setupVdp2Table(6, coefficientA0, coefficientA1, getVdp2Vram(0x20000), 0x80); // setup coefficients table A
    setupVdp2Table(7, coefficientB0, coefficientB1, getVdp2Vram(0x24000), 0x80); // setup coefficients table B
    s_BTL_A3_Env_InitVdp2Sub3(5, getVdp2Vram(0x2A000));

    // setup line color screen
    *(u16*)getVdp2Vram(0x2A400) = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA = (vdp2Controls.m4_pendingVdp2Regs->mA8_LCTA & 0xFFF80000) | 0x15200;

    // setup back screen color
    *(u16*)getVdp2Vram(0x2A600) = 0x38E5;
    vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA = (vdp2Controls.m4_pendingVdp2Regs->mAC_BKTA & 0xFFF80000) | 0x15300;

    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x405;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x507;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x505;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x600;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0x700;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 0x3;
    vdp2Controls.m_isDirty = 1;

    pThis->m3C = 0x10000;

    static const std::vector<std::array<s32, 2>> config = {
        {
            {0x2C, 0x1},
        }
    };
    applyLayerDisplayConfig(config);

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

    s_BTL_A3_Env_InitVdp2Sub4(g_BTL_A3->getSaturnPtr(0x60a6dd8));

    pThis->m4C = 0;

    pThis->m40 = g_BTL_A3->getSaturnPtr(0x60A6DD8);
    pThis->m44 = 0x4D5E540;
    pThis->m48 = 0xF5A;
}

void sBattleEnvironmentGridCell_Init(sTownCellTask* pThis, sSaturnPtr arg)
{
    int iVar1 = readSaturnS32(arg + 0xC);
    if ((readSaturnS32(arg + 0xC) == 0) && (readSaturnS32(arg + 0x10) == 0) && (readSaturnS32(arg + 0x14) == 0)) {
        pThis->m_DrawMethod = nullptr;
    }

    pThis->m8_cellPtr = arg;
    pThis->mC_position = readSaturnVec3(arg + 0x0);
}

void sBattleEnvironmentGridCell_Draw(sTownCellTask* pThis)
{
    pushCurrentMatrix();
    translateCurrentMatrix(pThis->mC_position);

    if ((graphicEngineStatus.m405C.m10_nearClipDistance - gTownGrid.m2C) <= pCurrentMatrix->m[2][3])
    {
        s32 iVar1 = MTH_Mul(pCurrentMatrix->m[2][3], graphicEngineStatus.m405C.m2C_widthRatio);
        s32 iVar2 = MTH_Mul(gTownGrid.m2C, graphicEngineStatus.m405C.m28_widthRatio2);

        if (((-(iVar1 + iVar2) <= pCurrentMatrix->m[0][3]) && (pCurrentMatrix->m[0][3] <= iVar1 + iVar2)))
        {
            sSaturnPtr r14 = readSaturnEA(pThis->m8_cellPtr + 0xC);
            if (r14.m_offset)
            {
                while (readSaturnS32(r14))
                {
                    s32 r5 = generateObjectMatrix(r14 + 4, r14 + 0x10);
                    int r4 = 0;
                    while (r5 > gTownGrid.m3C[r4])
                    {
                        r4++;
                    }

                    u16 offset = readSaturnU16(readSaturnEA(r14) + r4 * 2);
                    if (offset)
                    {
                        addObjectToDrawList(pThis->m0_fileBundle->get3DModel(offset));
                    }

                    popMatrix();
                    r14 += 0x18;
                }
            }

            r14 = readSaturnEA(pThis->m8_cellPtr + 0x10);
            if (r14.m_offset)
            {
                Unimplemented();
            }
        }
    }
    popMatrix();
}

void battleGrid_createCellSub0(sTownCellTask* r4_newCellTask, s32 r5, s32 r6, s32 r7, s32 r8)
{
    Unimplemented();
}

void battleGrid_createCell(s32 index, sTownGrid* pGrid)
{
    int iVar2 = pGrid->mC;

    int sizeXMinusOne = pGrid->m0_sizeX - 1;
    int sizeYMinusOne = pGrid->m4_sizeY - 1;

    int sizeXMask = ~sizeXMinusOne;
    int sizeYMask = ~sizeYMinusOne;

    int uVar5 = pGrid->m14_currentY + index & sizeYMinusOne;
    int offsetY = (sizeYMask & (pGrid->m14_currentY + index)) * pGrid->m28_cellSize;

    for(int i=-2; i<3; i++)
    {
        int uVar4 = pGrid->m10_currentX + i & sizeXMinusOne;
        int offsetX = (sizeXMask & (pGrid->m10_currentX + i)) * pGrid->m28_cellSize;

        sSaturnPtr cellData = pGrid->m38_EnvironmentSetup->cells[uVar4][uVar5];

        static const sTownCellTask::TypedTaskDefinition definition = { sBattleEnvironmentGridCell_Init , nullptr, sBattleEnvironmentGridCell_Draw, nullptr};
        sTownCellTask* newCellTask = createSubTaskWithArgWithCopy<sTownCellTask>(pGrid->m34_dataBuffer, cellData, &definition);
        pGrid->m40_cellTasks[(pGrid->mC + index) & 7][(pGrid->m8 + i) & 7] = newCellTask;

        newCellTask->mC_position[0] += offsetX;
        newCellTask->mC_position[2] += offsetY;

        battleGrid_createCellSub0(newCellTask, uVar4, uVar5, offsetX, offsetY);
    }
}

void battleGrid_cellFunc1(s32, sTownGrid*)
{
    Unimplemented();
}

void battleGrid_cellFunc2(s32, sTownGrid*)
{
    Unimplemented();
}

void battleGrid_deleteCell(s32 index, sTownGrid* pGrid)
{
    for (int i = -2; i < 3; i++)
    {
        sTownCellTask** cellTask = &pGrid->m40_cellTasks[(pGrid->mC + index) & 7][(pGrid->m8 + i) & 7];
        if (*cellTask)
        {
            (*cellTask)->getTask()->markFinished();
            *cellTask = nullptr;
        }
    }
}

void initGridForBattle(npcFileDeleter* pFile, const struct sGrid* pGrid, s32 r6_sizeX, s32 r7_sizeY, s32 r8_cellSize)
{
    gTownGrid.m18_createCell = battleGrid_createCell;
    gTownGrid.m1C_createCellColumn = battleGrid_cellFunc1;
    gTownGrid.m20_deleteCell = battleGrid_deleteCell;
    gTownGrid.m24_deleteCellColumn = battleGrid_cellFunc2;

    initNPCSub0Sub2(pFile, pGrid, r6_sizeX, r7_sizeY, r8_cellSize);
}

struct sBTL_A3_map6_sub : public s_workAreaTemplate<sBTL_A3_map6_sub>
{
    // size 0x20
};

void sBTL_A3_map6_sub_Init(sBTL_A3_map6_sub* pThis)
{
    Unimplemented();
}

void sBTL_A3_map6_sub_Update(sBTL_A3_map6_sub* pThis)
{
    Unimplemented();
}

void sBTL_A3_map6_sub_Draw(sBTL_A3_map6_sub* pThis)
{
    Unimplemented();
}

static void BTL_A3_map6_Init(s_BTL_A3_Env* pThis)
{
    loadFile("SCBTLA31.SCB", getVdp2Vram(0x40000), 0);
    loadFile("SCBTL_A3.PNB", getVdp2Vram(0x62800), 0);
    loadFile("SPACE.PNB", getVdp2Vram(0x64000), 0);

    pThis->m38 = -0x64000;

    BTL_A3_Env_InitVdp2(pThis);

    allocateNPC(pThis, 8);
    initGridForBattle(dramAllocatorEnd[8].mC_fileBundle, g_BTL_A3->m_map6, 2, 2, 0x400000);
    pThis->m58 = dramAllocatorEnd[8].mC_fileBundle;

    gBattleManager->m10_battleOverlay->m8_gridTask->m1C8_flags |= 0x10;

    static const sBTL_A3_map6_sub::TypedTaskDefinition definition = {
        &sBTL_A3_map6_sub_Init,
        &sBTL_A3_map6_sub_Update,
        &sBTL_A3_map6_sub_Draw,
        nullptr,
    };

    createSubTask<sBTL_A3_map6_sub>(pThis->m58, &definition);
}

void BTL_A3_Env_Update(s_BTL_A3_Env* pThis)
{
    // Empty
}

sRotationPassState gRotationPassState;
sVdp2RotationData gVdp2RotationMatrix;

void beginRotationPass_resetMatrix()
{
    gVdp2RotationMatrix.m[0][0] = 0x10000;
    gVdp2RotationMatrix.m[0][1] = 0;
    gVdp2RotationMatrix.m[0][2] = 0;
    gVdp2RotationMatrix.m[1][0] = 0;

    gVdp2RotationMatrix.m[1][1] = 0x10000;
    gVdp2RotationMatrix.m[1][2] = 0;
    gVdp2RotationMatrix.m[2][0] = 0;
    gVdp2RotationMatrix.m[2][1] = 0;

    gVdp2RotationMatrix.m[2][2] = 0x10000;
    gVdp2RotationMatrix.Mx = 0;
    gVdp2RotationMatrix.My = 0;
    gVdp2RotationMatrix.Mz = 0;
}

void beginRotationPass(int passIndex, fixedPoint focalLength)
{
    gRotationPassState.m1_scrollMode = 0;
    gRotationPassState.m2_matrixMode = eRMM_roll;
    gRotationPassState.m0_planeIndex = passIndex;
    gRotationPassState.m4_focalLength = focalLength;

    beginRotationPass_resetMatrix();

    if (gRotationPassState.m0_planeIndex == 0)
    {
        vdp2Controls.m_8 = fixedPoint::fromInteger(vdp2Controls.m_10);
    }
    else
    {
        vdp2Controls.m_8 = fixedPoint::fromInteger(vdp2Controls.m_C);
    }
}

// Copies the gVdp2RotationMatrix rotation matrix into the current coefficient table entry (FUN_0602915e)
static void writeRotationMatrixToEntry()
{
    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    t.m1C = gVdp2RotationMatrix.m[0][0];
    t.m20 = gVdp2RotationMatrix.m[0][1];
    t.m24 = gVdp2RotationMatrix.m[0][2];
    t.m28 = gVdp2RotationMatrix.m[1][0];
    t.m2C = gVdp2RotationMatrix.m[1][1];
    t.m30 = gVdp2RotationMatrix.m[1][2];
    t.m44 = gVdp2RotationMatrix.Mx;
    t.m48 = gVdp2RotationMatrix.My;
}

// FUN_060061f0: Fills coefficient table entries by repeated fixed-point division
// For each entry: table[i] = FP_Div(dividend, divisor), divisor -= step
static void generatePerspectiveCoefficients(s32 dividend, s32 divisor, fixedPoint step, int count, std::vector<fixedPoint>* pTable)
{
    std::vector<fixedPoint>& table = *pTable;
    for (int i = 0; i < count; i++) {
        table[i] = FP_Div(dividend, divisor);
        divisor -= (s32)step;
    }
}

// Fills a range of coefficient table entries for the rotation pass (FUN_06029668)
static void fillRotationCoefficientTable(int startY, int endY)
{
    int count = endY - startY;
    if (count == 0) {
        return;
    }

    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    s32 diffX = (s32)t.m34 - (s32)t.m3C;
    s32 diffY = (s32)t.m36 - (s32)t.m3E;
    s32 diffZ = (s32)t.m38 - (s32)t.m40;

    s32 dividend = (s32)gVdp2RotationMatrix.m[2][0] * diffX
                 + (s32)gVdp2RotationMatrix.m[2][1] * diffY
                 + (s32)gVdp2RotationMatrix.m[2][2] * diffZ
                 + (s32)t.m40 * 0x10000
                 + (s32)gVdp2RotationMatrix.Mz;

    s32 divisor;
    fixedPoint step;
    if (gRotationPassState.m1_scrollMode == 0) {
        divisor = (s32)gVdp2RotationMatrix.m[2][0] * (s32)t.m34
                + (s32)gVdp2RotationMatrix.m[2][1] * ((s32)t.m36 - startY);
        step = gVdp2RotationMatrix.m[2][1];
    } else {
        divisor = (s32)gVdp2RotationMatrix.m[2][0] * ((s32)t.m34 - startY)
                + (s32)gVdp2RotationMatrix.m[2][1] * (s32)t.m36;
        step = gVdp2RotationMatrix.m[2][0];
    }

    s32 zTerm = MTH_Mul(gVdp2RotationMatrix.m[2][2], (s32)t.m38 * 0x10000 - t.m8_Zst);
    divisor += (s32)zTerm;

    generatePerspectiveCoefficients(dividend, divisor, step, count,
        gVdp2CoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex]);
}

void commitRotationPass()
{
    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    if (gRotationPassState.m2_matrixMode == eRMM_roll) {
        t.m0  = (s32)t.m3C * (0x10000 - (s32)gRotationPassState.m4_focalLength);
        t.m4  = 0;
        t.m8_Zst  = 0;
        t.mC  = 0;
        t.m10 = 0x10000;
        t.m14 = (s32)gRotationPassState.m4_focalLength;
        t.m18 = 0;
    }
    else if (gRotationPassState.m2_matrixMode == eRMM_pitchYaw) {
        t.m0  = (s32)t.m3C * (0x10000 - (s32)gRotationPassState.m4_focalLength);
        t.m4  = 0;
        t.m8_Zst  = 0;
        t.mC  = 0;
        t.m10 = 0x10000;
        t.m14 = (s32)gRotationPassState.m4_focalLength;
        t.m18 = 0;
        t.m54 = (s32)vdp2Controls.m_8;
        s16 sVar1;
        if (gRotationPassState.m1_scrollMode == 0) {
            t.m58 = 0x10000;
            sVar1 = 0xE2;
            t.m5C = 0;
        } else {
            t.m58 = 0;
            t.m5C = (s32)gRotationPassState.m4_focalLength;
            sVar1 = 0x142;
        }
        fillRotationCoefficientTable(0, (int)sVar1);
    }
    else if (gRotationPassState.m2_matrixMode == eRMM_params) {
        int firstParam;
        s16 sVar1;
        if (gRotationPassState.m1_scrollMode == 0) {
            firstParam = 0xffffff9c; // -100
            sVar1 = 0x144;
        } else {
            firstParam = 0xffffffdd; // -35
            sVar1 = 0x185;
        }
        fillRotationCoefficientTable(firstParam, (int)sVar1);
    }

    writeRotationMatrixToEntry();

    if (gRotationPassState.m0_planeIndex == 0) {
        drawCinematicBar(4);
    } else if (gRotationPassState.m0_planeIndex == 1) {
        drawCinematicBar(5);
    }
}

void buildRotationMatrixRoll(fixedPoint rollAngle)
{
    fixedPoint cosZ = getCos(fixedPoint::toInteger(rollAngle));
    fixedPoint sinZ = getSin(fixedPoint::toInteger(rollAngle));

    gVdp2RotationMatrix.m[0][0] = cosZ;
    gVdp2RotationMatrix.m[0][1] = -sinZ;
    gVdp2RotationMatrix.m[0][2] = 0;
    gVdp2RotationMatrix.m[1][0] = sinZ;
    gVdp2RotationMatrix.m[1][1] = cosZ;
    gVdp2RotationMatrix.m[1][2] = 0;
    gVdp2RotationMatrix.m[2][0] = 0;
    gVdp2RotationMatrix.m[2][1] = 0;
    gVdp2RotationMatrix.m[2][2] = 0x10000;
    gRotationPassState.m2_matrixMode = eRMM_roll;
}

void setRotationScrollOffset(s32 scrollX, s32 scrollY)
{
    gVdp2RotationMatrix.Mx = scrollX;
    gVdp2RotationMatrix.My = scrollY;
}

void writeRotationParams(fixedPoint rollAngle)
{
    fixedPoint cosRoll = getCos(fixedPoint::toInteger(rollAngle));
    fixedPoint sinRoll = getSin(fixedPoint::toInteger(rollAngle));

    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    // Xst: rotate screen center into world space
    t.m0 = MTH_Mul((s32)sinRoll * (s32)t.m3E - (s32)cosRoll * (s32)t.m3C, gRotationPassState.m4_focalLength) + (s32)t.m3C * 0x10000;
    // Yst
    t.m4 = -(s32)cosRoll * (s32)t.m3E - (s32)sinRoll * (s32)t.m3C + (s32)t.m3E * 0x10000;
    // DXx, DXy
    t.mC  = -(s32)sinRoll;
    t.m10 = (s32)cosRoll;
    // DYx, DYy
    t.m14 = MTH_Mul(cosRoll, gRotationPassState.m4_focalLength);
    t.m18 = MTH_Mul(sinRoll, gRotationPassState.m4_focalLength);

    if (gRotationPassState.m1_scrollMode == 0) {
        s32 kastArg = MTH_Mul(-(s32)sinRoll * (s32)t.m3C, gRotationPassState.m4_focalLength);
        t.m54 = (kastArg - (s32)cosRoll * (s32)t.m3E) + (s32)t.m3E * 0x10000 + (s32)vdp2Controls.m_8 + 0x640000;
        t.m58 = (s32)cosRoll;
        t.m5C = MTH_Mul(sinRoll, gRotationPassState.m4_focalLength);
    } else {
        t.m54 = t.m0 + (s32)vdp2Controls.m_8 + 0x230000;
        t.m58 = -(s32)sinRoll;
        t.m5C = MTH_Mul(cosRoll, gRotationPassState.m4_focalLength);
    }

    gRotationPassState.m2_matrixMode = eRMM_params;
}

void buildRotationMatrixPitchYaw(fixedPoint pitchAngle, fixedPoint yawAngle)
{
    fixedPoint sin1 = getCos(fixedPoint::toInteger(pitchAngle));
    fixedPoint cos1 = getSin(fixedPoint::toInteger(pitchAngle));
    fixedPoint sin2 = getCos(fixedPoint::toInteger(yawAngle));
    fixedPoint cos2 = getSin(fixedPoint::toInteger(yawAngle));

    gVdp2RotationMatrix.m[0][0] = sin2;
    gVdp2RotationMatrix.m[0][1] = MTH_Mul(-sin1, cos2);
    gVdp2RotationMatrix.m[0][2] = MTH_Mul(cos1, cos2);

    gVdp2RotationMatrix.m[1][0] = cos2;
    gVdp2RotationMatrix.m[1][1] = MTH_Mul(sin1, sin2);
    gVdp2RotationMatrix.m[1][2] = MTH_Mul(-cos1, sin2);

    gVdp2RotationMatrix.m[2][0] = 0;
    gVdp2RotationMatrix.m[2][1] = cos1;
    gVdp2RotationMatrix.m[2][2] = sin1;

    gRotationPassState.m2_matrixMode = eRMM_pitchYaw;
}

void scaleRotationMatrix(fixedPoint scale)
{
    gVdp2RotationMatrix.m[0][0] = MTH_Mul(gVdp2RotationMatrix.m[0][0], scale);
    gVdp2RotationMatrix.m[0][1] = MTH_Mul(gVdp2RotationMatrix.m[0][1], scale);
    gVdp2RotationMatrix.m[0][2] = MTH_Mul(gVdp2RotationMatrix.m[0][2], scale);
    gVdp2RotationMatrix.m[1][0] = MTH_Mul(gVdp2RotationMatrix.m[1][0], scale);
    gVdp2RotationMatrix.m[1][1] = MTH_Mul(gVdp2RotationMatrix.m[1][1], scale);
    gVdp2RotationMatrix.m[1][2] = MTH_Mul(gVdp2RotationMatrix.m[1][2], scale);
}

void buildGroundRotation(s_BTL_A3_Env* pThis)
{
    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    s32 rotX = pThis->m18_cameraRotation.m0_X;
    if (rotX == 0) {
        rotX = -0x12345; // debug guard against zero pitch
    }
    s32 rotY = pThis->m18_cameraRotation.m4_Y;
    s32 rotZ = pThis->m18_cameraRotation.m8_Z;

    s32 sumX = (s32)pThis->m24_vdp1Clipping[0] + (s32)pThis->m24_vdp1Clipping[2];
    t.m34 = (s16)((sumX + (int)(sumX < 0)) >> 1);
    s32 sumY = (s32)pThis->m24_vdp1Clipping[1] + (s32)pThis->m24_vdp1Clipping[3];
    t.m36 = (s16)((sumY + (int)(sumY < 0)) >> 1);
    t.m38 = pThis->m30_vdp1ProjectionParam[1];
    t.m3C = t.m34;
    t.m3E = t.m36;
    t.m40 = 0;

    buildRotationMatrixPitchYaw(-0x4000000 - rotX, -rotY);
    scaleRotationMatrix(pThis->m3C);
    writeRotationParams(-rotZ);

    s32 diffX = (s32)t.m34 - (s32)t.m3C;
    s32 diffY = (s32)t.m36 - (s32)t.m3E;
    s32 diffZ = (s32)t.m38 - (s32)t.m40;

    gVdp2RotationMatrix.Mx = MTH_Mul(pThis->m3C, (s32)pThis->mC_cameraPosition.m0_X << 4)
                    - gVdp2RotationMatrix.m[0][0] * diffX - gVdp2RotationMatrix.m[0][1] * diffY - gVdp2RotationMatrix.m[0][2] * diffZ
                    + (s32)(s16)t.m3C * -0x10000;
    gVdp2RotationMatrix.My = MTH_Mul(pThis->m3C, (s32)pThis->mC_cameraPosition.m8_Z << 4)
                    - gVdp2RotationMatrix.m[1][0] * diffX - gVdp2RotationMatrix.m[1][1] * diffY - gVdp2RotationMatrix.m[1][2] * diffZ
                    + (s32)(s16)t.m3E * -0x10000;
    gVdp2RotationMatrix.Mz = (pThis->mC_cameraPosition.m4_Y - pThis->m38) * 0x10
                    - gVdp2RotationMatrix.m[2][0] * diffX - gVdp2RotationMatrix.m[2][1] * diffY - gVdp2RotationMatrix.m[2][2] * diffZ
                    + (s32)(s16)t.m40 * -0x10000;
}

s32 computeRotationScrollOffset()
{
    sCoefficientTableData& t = gCoefficientTables[gRotationPassState.m0_planeIndex][(s32)vdp2Controls.m0_doubleBufferIndex];

    s32 pivotX = (s32)gVdp2RotationMatrix.m[2][0] * (s32)t.m34;
    s32 pivotY = (s32)gVdp2RotationMatrix.m[2][1] * (s32)t.m36;
    s32 depthTerm = MTH_Mul(gVdp2RotationMatrix.m[2][2], (s32)t.m38 * 0x10000 - t.m8_Zst);

    return FP_Div(pivotX + pivotY + depthTerm, gVdp2RotationMatrix.m[2][1]).getInteger();
}

static void BTL_A3_Env_DrawSub4(s_BTL_A3_Env* pThis)
{
    auto& table = *gVdp2CoefficientTables[gRotationPassState.m0_planeIndex][vdp2Controls.m0_doubleBufferIndex];
    for (int i = 0; i < 424; i++) {
        table[1] = MTH_Mul(table[1], MTH_Mul(pThis->m48, getSin(pThis->m4C)) + 0x10000);
    }
    pThis->m4C += pThis->m40.m_offset; // TODO: this doesn't really make sense!
}

void BTL_A3_Env_Draw(s_BTL_A3_Env* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();
    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_vdp1LocalCoordinates);
    getVdp1ProjectionParams(&pThis->m30_vdp1ProjectionParam[0], &pThis->m30_vdp1ProjectionParam[1]);

    beginRotationPass(0, performDivision(pThis->m30_vdp1ProjectionParam[0], fixedPoint::fromInteger(pThis->m30_vdp1ProjectionParam[1])));
    buildGroundRotation(pThis);
    drawCinematicBar(6);
    commitRotationPass();

    pThis->m34 = computeRotationScrollOffset();
    BTL_A3_Env_DrawSub4(pThis);

    s32 scrollX = (pThis->m18_cameraRotation.m4_Y >> 0xC) * -0x400;
    s32 scrollY = (0x1EF - pThis->m34) * 0x10000;

    beginRotationPass(1, performDivision(pThis->m30_vdp1ProjectionParam[0], fixedPoint::fromInteger(pThis->m30_vdp1ProjectionParam[1])));

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
    performDivision(pThis->m24_vdp1Clipping[3] - pThis->m24_vdp1Clipping[1], 0x00E00000);
    scaleRotationMatrix(performDivision(pThis->m24_vdp1Clipping[2] - pThis->m24_vdp1Clipping[0], 0x01600000));
    setRotationScrollOffset(scrollX, scrollY);
    commitRotationPass();

    pauseEngine[4] = 0;
    setupVDP2CoordinatesIncrement2(scrollX, 0);
    pauseEngine[4] = 4;

    auto* regs = vdp2Controls.m4_pendingVdp2Regs;
    regs->mC0_WPSX0 = pThis->m24_vdp1Clipping[0] << 1;
    regs->mC2_WPSY0 = pThis->m24_vdp1Clipping[1];
    regs->mC4_WPEX0 = pThis->m24_vdp1Clipping[2] << 1;
    regs->mC6_WPEY0 = pThis->m24_vdp1Clipping[3];
}

p_workArea Create_BTL_A3_map6(p_workArea parent)
{
    static const s_BTL_A3_Env::TypedTaskDefinition definition = {
        &BTL_A3_map6_Init,
        &BTL_A3_Env_Update,
        &BTL_A3_Env_Draw,
        nullptr,
    };

    return createSubTask<s_BTL_A3_Env>(parent, &definition);
}
