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
#include "town/town.h" // TODO: cleanup

void s_BTL_A3_Env_InitVdp2Sub4(sSaturnPtr)
{
    Unimplemented();
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

struct sCoefficientTableData
{
    s16 m34;
    s16 m36;
    s16 m38;
    s16 m3C;
    s16 m3E;
    s16 m40;

    // size 0x70
};

std::array<sCoefficientTableData, 4> tableCoefficient4_0;

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

std::array<std::vector<fixedPoint>*, 4> vdp2CoefficientTables;

// TODO: kernel
void setupVdp2Table(int p1, std::vector<fixedPoint>& p2, std::vector<fixedPoint>& p3, u8* coefficientTableAddress, u8 numCoefficients)
{
    setupScrollAndRotation(p1, p2.data(), p3.data(), coefficientTableAddress, numCoefficients);
    initCoefficientTable(p2, p3, numCoefficients * 4);

    // TODO: this is super broken in ghidra, why?
    switch (p1)
    {
    case 6:
        vdp2CoefficientTables[0] = &p2;
        vdp2CoefficientTables[1] = &p3;
        break;
    case 7:
        vdp2CoefficientTables[2] = &p2;
        vdp2CoefficientTables[3] = &p3;
        break;
    default:
        break;
    }
}

void s_BTL_A3_Env_InitVdp2Sub3Sub1(sCoefficientTableData& table)
{
    /*
    table[0] = 0;
    table[1] = 0;
    table[2] = 0;
    table[3] = 0;
    table[4] = 0x10000;
    table[5] = 0x10000;
    table[6] = 0;
    table[7] = 0x10000;
    table[8] = 0;
    table[9] = 0;
    table[0xa] = 0;
    table[0xb] = 0x10000;
    table[0xc] = 0;
    table[0xd] = 0xb00070;
    table[0xe] = 0xfb0000;
    table[0xf] = 0xb00070;
    table[0x10] = 0;
    table[0x11] = 0;
    table[0x12] = 0;
    table[0x13] = 0x10000;
    table[0x14] = 0x10000;
    table[0x15] = 0;
    table[0x16] = 0;
    table[0x17] = 0;
    table[0x18] = 0;
    table[0x19] = 0;
    table[0x1a] = 0;
    table[0x1b] = 0x10000;
    */
    Unimplemented();
}

void s_BTL_A3_Env_InitVdp2Sub3(int layerIndex, u8* table)
{
    if (layerIndex != 4)
    {
        if (layerIndex != 5)
        {
            return;
        }

        s_BTL_A3_Env_InitVdp2Sub3Sub1(tableCoefficient4_0[2]);
        s_BTL_A3_Env_InitVdp2Sub3Sub1(tableCoefficient4_0[3]);
        setupScrollAndRotation(5, &tableCoefficient4_0[2], &tableCoefficient4_0[3], table + 0x80, 6);
    }

    s_BTL_A3_Env_InitVdp2Sub3Sub1(tableCoefficient4_0[0]);
    s_BTL_A3_Env_InitVdp2Sub3Sub1(tableCoefficient4_0[1]);
    setupScrollAndRotation(5, &tableCoefficient4_0[0], &tableCoefficient4_0[1], table + 0x80, 6);
}

void BTL_A3_Env_InitVdp2(s_BTL_A3_Env* pThis)
{
    /* Expected results:
    Parameter A/B switched via coefficients
    8-bit(256 colors)
    Tile(2H x 2V)
    Plane Size = 1H x 1V
    Pattern Name data size = 1 word
    Character Number Supplement bit = 0
    Special Priority bit = 0
    Special Color Calculation bit = 0
    Supplementary Palette number = 0
    Supplementary Color number = 8
    Plane A Address = 00063000
    Plane B Address = 00063000
    Plane C Address = 00063000
    Plane D Address = 00063000
    Plane E Address = 00063000
    Plane F Address = 00063000
    Plane G Address = 00063000
    Plane H Address = 00063000
    Plane I Address = 00063000
    Plane J Address = 00063000
    Plane K Address = 00063000
    Plane L Address = 00063000
    Plane M Address = 00063000
    Plane N Address = 00063000
    Plane O Address = 00063000
    Plane P Address = 00063000
    Window W0 Enabled:
    Horizontal start = 0
    Vertical start = 0
    Horizontal end = 352
    Vertical end = 224
    Display inside of Window
    Window Overlap Logic: OR
    Color Ram Address Offset = 0
    Priority = 3
    Color Offset A Enabled
    R = 0, G = 0, B = 0
    Special Color Calculation 0
*/
    gBattleManager->m10_battleOverlay->m1C_envTask = pThis;
    reinitVdp2();

    fieldPaletteTaskInitSub0();

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
    loadSaveBackgroundSub0(config);

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

    if ((graphicEngineStatus.m405C.m10_nearClipDistance - gTownGrid.m2C) <= pCurrentMatrix->matrix[11])
    {
        s32 iVar1 = MTH_Mul(pCurrentMatrix->matrix[11], graphicEngineStatus.m405C.m2C_widthRatio);
        s32 iVar2 = MTH_Mul(gTownGrid.m2C, graphicEngineStatus.m405C.m28_widthRatio2);

        if (((-(iVar1 + iVar2) <= pCurrentMatrix->matrix[3]) && (pCurrentMatrix->matrix[3] <= iVar1 + iVar2)))
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

void battleGrid_cellFunc1()
{
    Unimplemented();
}

void battleGrid_cellFunc2()
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
    gTownGrid.m1C = battleGrid_cellFunc1;
    gTownGrid.m20_deleteCell = battleGrid_deleteCell;
    gTownGrid.m24 = battleGrid_cellFunc2;

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

std::array<s8, 4> vdpVar6;
fixedPoint vdpVar7;
sMatrix4x3 vdpVar8;

void BTL_A3_Env_DrawSub1ResetMatrix()
{
    vdpVar8[0] = 0x10000;
    vdpVar8[1] = 0;
    vdpVar8[2] = 0;
    vdpVar8[3] = 0;

    vdpVar8[4] = 0x10000;
    vdpVar8[5] = 0;
    vdpVar8[6] = 0;
    vdpVar8[7] = 0;

    vdpVar8[8] = 0x10000;
    vdpVar8[9] = 0;
    vdpVar8[10] = 0;
    vdpVar8[11] = 0;
}

void BTL_A3_Env_DrawSub1(int p1, fixedPoint p2)
{
    vdpVar6[1] = 0;
    vdpVar6[2] = 0;
    vdpVar6[0] = p1;
    vdpVar7 = p2;

    BTL_A3_Env_DrawSub1ResetMatrix();

    if (vdpVar6[0] == 0)
    {
        vdp2Controls.m_8 = fixedPoint::fromInteger(vdp2Controls.m_10);
    }
    else
    {
        vdp2Controls.m_8 = fixedPoint::fromInteger(vdp2Controls.m_C);
    }
}

void BTL_A3_Env_DrawSub8Sub1(fixedPoint p1, fixedPoint p2)
{
    fixedPoint cos1 = getCos(fixedPoint::toInteger(p1));
    fixedPoint sin1 = getSin(fixedPoint::toInteger(p1));
    fixedPoint cos2 = getCos(fixedPoint::toInteger(p2));
    fixedPoint sin2 = getSin(fixedPoint::toInteger(p2));

    vdpVar8[0] = cos2;
    vdpVar8[1] = MTH_Mul(-sin1, cos2);
    vdpVar8[2] = MTH_Mul(cos1, cos2);

    vdpVar8[3] = cos2;
    vdpVar8[4] = MTH_Mul(sin1, sin2);
    vdpVar8[5] = MTH_Mul(-cos1, sin2);

    vdpVar8[6] = 0;
    vdpVar8[7] = cos1;
    vdpVar8[8] = sin1;

    vdpVar6[2] = 1;
}

void BTL_A3_Env_DrawSub6(fixedPoint param1)
{
    vdpVar8[0] = MTH_Mul(vdpVar8[0], param1);
    vdpVar8[1] = MTH_Mul(vdpVar8[1], param1);
    vdpVar8[2] = MTH_Mul(vdpVar8[2], param1);
    vdpVar8[3] = MTH_Mul(vdpVar8[3], param1);
    vdpVar8[4] = MTH_Mul(vdpVar8[4], param1);
    vdpVar8[5] = MTH_Mul(vdpVar8[5], param1);
}

void BTL_A3_Env_DrawSub8(s_BTL_A3_Env* pThis)
{
    sCoefficientTableData& table = tableCoefficient4_0[vdp2Controls.m0_doubleBufferIndex];

    sVec3_FP cameraRotation = pThis->m18_cameraRotation;
    if (pThis->m18_cameraRotation[0] == 0)
    {
        cameraRotation[0] = -0x12345; // was that a debug piece of code left over?
    }
    
    table.m34 = pThis->m24_vdp1Clipping[0] + pThis->m24_vdp1Clipping[2];
    table.m36 = pThis->m24_vdp1Clipping[1] + pThis->m24_vdp1Clipping[3];
    table.m38 = pThis->m30_vdp1ProjectionParam[1];
    table.m3C = table.m34;
    table.m3E = table.m36;
    table.m40 = 0;

    BTL_A3_Env_DrawSub8Sub1(-0x4000000 - cameraRotation[0], cameraRotation[1]);
    BTL_A3_Env_DrawSub6(pThis->m3C);
    Unimplemented();
}

void BTL_A3_Env_Draw(s_BTL_A3_Env* pThis)
{
    pThis->mC_cameraPosition = cameraProperties2.m0_position;
    pThis->m18_cameraRotation = cameraProperties2.mC_rotation.toSVec3_FP();
    getVdp1ClippingCoordinates(pThis->m24_vdp1Clipping);
    getVdp1LocalCoordinates(pThis->m2C_vdp1LocalCoordinates);
    getVdp1ProjectionParams(&pThis->m30_vdp1ProjectionParam[0], &pThis->m30_vdp1ProjectionParam[1]);

    BTL_A3_Env_DrawSub1(0, performDivision(pThis->m30_vdp1ProjectionParam[0], fixedPoint::fromInteger(pThis->m30_vdp1ProjectionParam[1])));
    BTL_A3_Env_DrawSub8(pThis);
    /*
    drawCinematicBar(6);
    BTL_A3_Env_DrawSub2();

    pThis->m34 = BTL_A3_Env_DrawSub3();

    BTL_A3_Env_DrawSub4(pThis);

    pThis->m0 = (pThis->m18_cameraRotation[1] >> 0xC) * -0x400;
    pThis->m4 = fixedPoint::fromInteger(495 - pThis->m34);

    BTL_A3_Env_DrawSub1(1, performDivision(pThis->m30_vdp1ProjectionParam[0], fixedPoint::fromInteger(pThis->m30_vdp1ProjectionParam[1])));

    int iVar3 = vdp2Controls.m_0
    */
    Unimplemented();
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
