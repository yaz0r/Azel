#include "PDS.h"
#include "fieldRadar.h"


void fieldRadar_hide()
{
    getFieldTaskPtr()->m8_pSubFieldData->m33C_fieldRadar->m5A_isHidden = 1;
}

void fieldRadar_show()
{
    getFieldTaskPtr()->m8_pSubFieldData->m33C_fieldRadar->m5A_isHidden = 0;
}

void fieldRadar_enableAltitudeGauge()
{
    getFieldTaskPtr()->m8_pSubFieldData->m33C_fieldRadar->m58_altitudeGaugeEnabled = 1;
}

void fieldRadar_disableAltitudeGauge()
{
    getFieldTaskPtr()->m8_pSubFieldData->m33C_fieldRadar->m58_altitudeGaugeEnabled = 0;
}

void dragonFieldSubTask2InitSub1(s32 r4)
{
    s_FieldRadar* r14 = getFieldTaskPtr()->m8_pSubFieldData->m33C_fieldRadar;
    r14->m64_encounterDistance = r4;
    r14->m68_encounterDistanceSq = MTH_Mul(r4, r4);
}

void fieldRadar_init(s_FieldRadar* pTypedWorkArea)
{
    getFieldTaskPtr()->m8_pSubFieldData->m33C_fieldRadar = pTypedWorkArea;

    getMemoryArea(&pTypedWorkArea->m0, 0);
    u32 r2 = pTypedWorkArea->m0.m4_characterArea - (0x25C00000);
    pTypedWorkArea->m3C = (0x25C00000) + ((0x1748 + (r2 >> 3)) << 3);
    pTypedWorkArea->m8 = 0x50;
    pTypedWorkArea->mC_X = pTypedWorkArea->m8 + 0x30;
    pTypedWorkArea->mE_Y = -pTypedWorkArea->mA + 0x60;
    pTypedWorkArea->m30 = 0x18;
    pTypedWorkArea->m34 = 0x18;
    pTypedWorkArea->m10 = 6;
    pTypedWorkArea->m12 = -14;
    pTypedWorkArea->m28 = -0x1D;
    pTypedWorkArea->m2A = -0x21;

    pTypedWorkArea->m50 = 0x1C;
    pTypedWorkArea->m54 = 0x18;
    pTypedWorkArea->m5C = 0x40000;
    pTypedWorkArea->m60 = 0x40000;

    dragonFieldSubTask2InitSub1(0x200000);
}

// FLD_A3::06093e98 — radar danger level palettes (VDP1 color data, 16 RGB555 entries each)
u8 fieldPalettes[3][0x20] =
{
    { // 0: safe (green)
        0xA0, 0xC4, 0x9C, 0x00, 0xA8, 0x00, 0xB8, 0x00,
        0xC8, 0x00, 0xCC, 0x00, 0xD4, 0x00, 0xDC, 0x00,
        0xE4, 0x00, 0xF0, 0x00, 0xFC, 0x00, 0xFC, 0x20,
        0xFC, 0x61, 0xFC, 0xA2, 0xFC, 0xE3, 0xFD, 0x04,
    },
    { // 1: warning (yellow)
        0xA0, 0xC4, 0x80, 0x87, 0x80, 0xC9, 0x81, 0x2C,
        0x81, 0x6E, 0x81, 0xD1, 0x82, 0x14, 0x82, 0x76,
        0x82, 0xB9, 0x83, 0x1C, 0x83, 0x1C, 0x87, 0x3D,
        0x87, 0x3E, 0x8B, 0x5F, 0x8F, 0x9F, 0x93, 0xFF,
    },
    { // 2: danger (red)
        0xA0, 0xC4, 0x80, 0x07, 0x80, 0x0A, 0x80, 0x0E,
        0x80, 0x12, 0x80, 0x13, 0x80, 0x15, 0x80, 0x17,
        0x80, 0x19, 0x80, 0x1C, 0x80, 0x1F, 0x80, 0x3F,
        0x84, 0x7F, 0x88, 0xBF, 0x8C, 0xFF, 0x91, 0x1F,
    },
};

s8 paletteIndexTable[4] = {
    0,1,2,2
};

void createPaletteFadeTask(p_workArea parent, void* pal1, void* pal2, s32 duration)
{
    Unimplemented();
}

u32 isDragonBoostAvailable();
u32 isDragonBoostLocked();

// VDP1 line draw command
static void lineDraw(u16 mode, s16* coords, u16 color, fixedPoint depth)
{
    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    vdp1WriteEA.m0_CMDCTRL = 0x1006;
    vdp1WriteEA.m4_CMDPMOD = mode | 0x400;
    vdp1WriteEA.m6_CMDCOLR = color;
    vdp1WriteEA.mC_CMDXA = coords[0];
    vdp1WriteEA.mE_CMDYA = -coords[1];
    vdp1WriteEA.m10_CMDXB = coords[2];
    vdp1WriteEA.m12_CMDYB = -coords[3];

    fixedPoint computedDepth = depth * graphicEngineStatus.m405C.m38_oneOverFarClip;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = computedDepth.getInteger();
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

// Speed dot sprite params: {CMDSRCA offset, CMDSIZE, CMDCOLR offset}
struct sSpeedDotParams {
    s16 m_cmdsrca;
    s16 m_cmdsize;
    s16 m_cmdcolr;
};

// FLD_A3::06093f0c — normal speed dots
static const sSpeedDotParams speedDotParamsNormal = { 0x0418, 0x0108, 0x174C };
// FLD_A3::06093f18 — boost active speed dots
static const sSpeedDotParams speedDotParamsBoostActive = { 0x041C, 0x0103, 0x172C };
// FLD_A3::06093f24 — boost locked speed dots
static const sSpeedDotParams speedDotParamsBoostLocked = { 0x0420, 0x0103, 0x1750 };

// FLD_A3::06093f3c — speed dot positions (X, Y offsets from radar center)
static const s16 speedDotPositions[4][2] = {
    { 0x30, -0x33 },
    { 0x2A, -0x37 },
    { 0x23, -0x39 },
    { 0x1C, -0x3A },
};

// 06093e34
static const sSpeedDotParams* getSpeedDotParams()
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    if (isDragonBoostAvailable() && (pDragon->m25C & 1))
    {
        return &speedDotParamsBoostActive;
    }
    if (isDragonBoostLocked())
    {
        return &speedDotParamsBoostLocked;
    }
    return &speedDotParamsNormal;
}

// 06071ec4 — draw destination indicator on radar
static void fieldRadar_drawDestinationIndicator(s_FieldRadar* pThis, s32* pDestPosition)
{
    Unimplemented();
}

void fieldRadar_udpate(s_FieldRadar* pThis)
{
    if (mainGameState.consumables[56] == 0) // telepathy shard
    {
        pThis->m4C_dangerLevel = 0;
    }
    else
    {
        pThis->m4C_dangerLevel = getFieldTaskPtr()->m34_radarDangerLevel;
    }

    if (graphicEngineStatus.m40AC.m8 != 2)
    {
        if (pThis->m5A_isHidden == 0)
        {
            if (pThis->m8 > 0)
            {
                pThis->m8 -= 10;
            }
        }
        else
        {
            if (pThis->m8 < 0x50)
            {
                pThis->m8 += 10;
            }
        }

        if (pThis->m5B == 0)
        {
            pThis->mC_X = pThis->m8 + 0x60;
            pThis->mE_Y = 0x60 - pThis->mA;
            pThis->m59 = 0;
        }
        else
        {
            pThis->mC_X = -0x1D;
            pThis->mE_Y = 0x21;
            pThis->m59 = 1;
        }
        pThis->m38_dragonDirection = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask->m20_angle[1];

        switch (pThis->m48)
        {
        case 0:
            if (fieldPalettes[pThis->m4A_newPaletteIndex] == fieldPalettes[pThis->m4C_dangerLevel])
            {
                if (pThis->m4A_newPaletteIndex != pThis->m4C_dangerLevel)
                {
                    pThis->m4A_newPaletteIndex = pThis->m4C_dangerLevel;
                }
            }
            else
            {
                pThis->m40_paletteFadeDuration = 0xF;
                createPaletteFadeTask(pThis, fieldPalettes[pThis->m4A_newPaletteIndex], fieldPalettes[pThis->m4C_dangerLevel], 0xF);
                pThis->m4B = pThis->m4C_dangerLevel;
                pThis->m49++;
                pThis->m48++;
            }
            break;
        case 1:
            if (--pThis->m40_paletteFadeDuration < 1)
            {
                pThis->m48 = 0;
                pThis->m4A_newPaletteIndex = pThis->m4B;
            }
            break;
        default:
            assert(0);
        }

    }

    asyncDmaCopy(fieldPalettes[pThis->m4C_dangerLevel], getVdp1Pointer(pThis->m3C), 0x20, 0);
}

void drawGaugeVdp1(u16 mode, std::array<sVec2_S16, 4>& params, u16 color, fixedPoint depth);

void fieldRadar_draw(s_FieldRadar* pThis)
{
    s_dragonTaskWorkArea* pDragon = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;
    s16 charAreaOffset = (s16)((pThis->m0.m4_characterArea - 0x25C00000) >> 3);

    // Radar sphere
    if ((pThis->m4A_newPaletteIndex != 3) || (pThis->m44 & 4))
    {
        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000;
        vdp1WriteEA.m4_CMDPMOD = 0x88;
        vdp1WriteEA.m6_CMDCOLR = charAreaOffset + 0x1748;
        vdp1WriteEA.m8_CMDSRCA = charAreaOffset + 0x388;
        vdp1WriteEA.mA_CMDSIZE = 0x630;
        vdp1WriteEA.mC_CMDXA = pThis->mC_X + pThis->m10;
        vdp1WriteEA.mE_CMDYA = -(pThis->mE_Y + pThis->m12);

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    // Direction line
    {
        s16 centerX = pThis->mC_X + pThis->m28;
        s16 centerY = pThis->mE_Y + pThis->m2A;
        s16 dirIndex = (s16)((u16)(pThis->m38_dragonDirection.getInteger()) & 0xFFF);
        s32 sinVal = (s32)getSin(dirIndex);
        s32 cosVal = (s32)getCos(dirIndex);

        s16 lineCoords[4];
        lineCoords[2] = (s16)(((s32)centerX * 0x10000 + pThis->m30 * sinVal) >> 16);
        lineCoords[3] = (s16)(((s32)centerY * 0x10000 + pThis->m30 * cosVal) >> 16);
        lineCoords[0] = (s16)(((s32)centerX * 0x10000 + pThis->m34 * sinVal) >> 16);
        lineCoords[1] = (s16)(((s32)centerY * 0x10000 + pThis->m34 * cosVal) >> 16);

        lineDraw(0xC0, lineCoords, 0xF39C, 0);
    }

    // Radar frame overlay
    if (gDragonState->mC_dragonType == 8)
    {
        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000;
        vdp1WriteEA.m4_CMDPMOD = 0x88;
        vdp1WriteEA.m6_CMDCOLR = charAreaOffset + 0x1744;
        vdp1WriteEA.m8_CMDSRCA = charAreaOffset + 0x248;
        vdp1WriteEA.mA_CMDSIZE = 0xA40;
        vdp1WriteEA.mC_CMDXA = pThis->mC_X;
        vdp1WriteEA.mE_CMDYA = -pThis->mE_Y;

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }
    else
    {
        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000;
        vdp1WriteEA.m4_CMDPMOD = 0x88;
        vdp1WriteEA.m6_CMDCOLR = charAreaOffset + 0x1740;
        vdp1WriteEA.m8_CMDSRCA = charAreaOffset + 0x108;
        vdp1WriteEA.mA_CMDSIZE = 0xA40;
        vdp1WriteEA.mC_CMDXA = pThis->mC_X;
        vdp1WriteEA.mE_CMDYA = -pThis->mE_Y;

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    // Speed dot count — how many speed thresholds has the dragon exceeded?
    s32 numSpeedDot = 0;
    do {
        s32 sum = (s32)pDragon->m21C_DragonSpeedValues[numSpeedDot] + (s32)pDragon->m21C_DragonSpeedValues[numSpeedDot + 1];
        s32 avg = (sum + (u32)(sum < 0)) >> 1;
        if ((s32)pDragon->m154_dragonSpeed <= avg) break;
        numSpeedDot++;
    } while (numSpeedDot < 4);

    // Boost indicator
    if (!isDragonBoostLocked())
    {
        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000;
        vdp1WriteEA.m4_CMDPMOD = 0x88;
        vdp1WriteEA.m8_CMDSRCA = charAreaOffset + 0x424;
        vdp1WriteEA.mA_CMDSIZE = 0x103;
        vdp1WriteEA.m6_CMDCOLR = charAreaOffset + 0x1750;
        vdp1WriteEA.mC_CMDXA = pThis->mC_X + 0x1C;
        vdp1WriteEA.mE_CMDYA = -(pThis->mE_Y + -0x3A);

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }

    // Speed dots
    {
        const sSpeedDotParams* pParams = getSpeedDotParams();
        s32 i = numSpeedDot;
        while (i != 0)
        {
            s16 dotX = speedDotPositions[i - 1][0];
            s16 dotY = speedDotPositions[i - 1][1];

            s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            vdp1WriteEA.m0_CMDCTRL = 0x1000;
            vdp1WriteEA.m4_CMDPMOD = 0x88;
            vdp1WriteEA.m8_CMDSRCA = pParams->m_cmdsrca + charAreaOffset;
            vdp1WriteEA.mA_CMDSIZE = pParams->m_cmdsize;
            vdp1WriteEA.m6_CMDCOLR = pParams->m_cmdcolr + charAreaOffset;
            vdp1WriteEA.mC_CMDXA = pThis->mC_X + dotX;
            vdp1WriteEA.mE_CMDYA = -(pThis->mE_Y + dotY);

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;

            i--;
        }
    }

    // Altitude gauge
    if (pThis->m58_altitudeGaugeEnabled)
    {
        fixedPoint altRatio = FP_Div(
            ((s32)pDragon->m8_pos[1] - (s32)pDragon->m134_minY) >> 12,
            ((s32)pDragon->m140_maxY - (s32)pDragon->m134_minY) >> 12);

        if ((s32)altRatio > 0x10000)
        {
            altRatio = 0x10000;
        }
        if ((s32)altRatio > 0)
        {
            if ((s32)altRatio > 0x10000)
            {
                altRatio = 0x10000;
            }

            std::array<sVec2_S16, 4> gaugeQuad;
            gaugeQuad[0][0] = pThis->mC_X + 0x38;
            gaugeQuad[1][0] = pThis->mC_X + 0x3C;
            gaugeQuad[2][0] = gaugeQuad[1][0];
            gaugeQuad[3][0] = gaugeQuad[0][0];

            s16 gaugeHeight = (s16)(s32)MTH_Mul(altRatio, 0x1D);
            gaugeQuad[0][1] = gaugeHeight + pThis->mE_Y - 0x39;
            gaugeQuad[1][1] = gaugeQuad[0][1];
            gaugeQuad[2][1] = pThis->mE_Y - 0x39;
            gaugeQuad[3][1] = gaugeQuad[2][1];

            drawGaugeVdp1(0xC0, gaugeQuad, 0x801C, 0);
        }
    }

    // Destination indicator
    if (pThis->m18_currentSelection > 0)
    {
        s32* pDestPosition = (s32*)((u8*)pThis->m1C_encounterList + pThis->m18_currentSelection * 0x14 + 8);
        fieldRadar_drawDestinationIndicator(pThis, pDestPosition);
    }

    // Frame counter
    pThis->m44++;
}

void createFieldRadar(s_workArea* pWorkArea)
{
    static const s_FieldRadar::TypedTaskDefinition definition = {
        fieldRadar_init,
        fieldRadar_udpate,
        fieldRadar_draw,
        nullptr
    };

    createSubTask<s_FieldRadar>(pWorkArea, &definition);
}

// 06068f0c
void fieldRadar_setEncounterDistance(fixedPoint distance)
{
    s_FieldRadar* pRadar = getFieldTaskPtr()->m8_pSubFieldData->m33C_fieldRadar;
    pRadar->m64_encounterDistance = distance;
    pRadar->m68_encounterDistanceSq = MTH_Mul(distance, distance);
}

// 0607bab4 — clear radar battle state
static void fieldRadar_clearBattleState()
{
    s_FieldRadar* pRadar = getFieldTaskPtr()->m8_pSubFieldData->m33C_fieldRadar;
    if (pRadar->m1C_encounterList != nullptr)
    {
        pRadar->m1C_encounterList = nullptr;
    }
    pRadar->m14_encounterCount = 0;
    pRadar->m18_currentSelection = 0;
}

// 06071e94 — parse random battle encounter list from script data
static void fieldRadar_parseEncounterList(s32 scriptIndex)
{
    s_fieldScriptWorkArea* pScripts = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    s_FieldRadar* pRadar = getFieldTaskPtr()->m8_pSubFieldData->m33C_fieldRadar;

    if (pScripts->m0_pScripts == nullptr)
        return;

    // Count entries in the script data
    sSaturnPtr pData = pScripts->m0_pScripts[scriptIndex];
    pRadar->m14_encounterCount = 1;
    while (readSaturnU8(pData) != 0x01)
    {
        if (readSaturnU8(pData) == 0x7F)
        {
            pRadar->m14_encounterCount++;
            pData = sSaturnPtr(((pData.m_offset + 4) & ~3) + 0x14, pData.m_file);
        }
        else
        {
            pData = pData + 1;
        }
    }

    // Allocate and populate entry table
    u32* pAlloc = (u32*)allocateHeapForTask(pRadar, pRadar->m14_encounterCount * 0x14);
    pRadar->m1C_encounterList = (sRadarDestEntry*)pAlloc;
    if (pAlloc)
    {
        // First entry: default "deactivate" option
        pAlloc[0] = 0; // placeholder for string pointer
        pAlloc += 5;

        pData = pScripts->m0_pScripts[scriptIndex];
        while (readSaturnU8(pData) != 0x01)
        {
            if (readSaturnU8(pData) == 0x7F)
            {
                sSaturnPtr pSrc = sSaturnPtr(((pData.m_offset + 4) & ~3), pData.m_file);
                for (int j = 0; j < 5; j++)
                {
                    pAlloc[j] = readSaturnU32(pSrc + j * 4);
                }
                pAlloc += 5;
                pData = pSrc + 0x14;
            }
            else
            {
                pData = pData + 1;
            }
        }
    }
}

// 0607baa0
void fieldRadar_initRandomBattle(s32 scriptIndex)
{
    fieldRadar_clearBattleState();
    fieldRadar_parseEncounterList(scriptIndex);
}

