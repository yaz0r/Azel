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

void fieldRadar_draw(s_FieldRadar* pThis)
{
    if ((pThis->m4A_newPaletteIndex != 3) || (pThis->m44 & 4))
    {
        //radar sphere
        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000;
        vdp1WriteEA.m4_CMDPMOD = 0x88;
        vdp1WriteEA.m6_CMDCOLR = ((pThis->m0.m4_characterArea - (0x25C00000)) >> 3) + 0x1748;
        vdp1WriteEA.m8_CMDSRCA = ((pThis->m0.m4_characterArea - (0x25C00000)) >> 3) + 0x388;
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

    Unimplemented(); // Radar line draw

    if (gDragonState->mC_dragonType == 8)
    {
        s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        vdp1WriteEA.m0_CMDCTRL = 0x1000;
        vdp1WriteEA.m4_CMDPMOD = 0x88;
        vdp1WriteEA.m6_CMDCOLR = ((pThis->m0.m4_characterArea - (0x25C00000)) >> 3) + 0x1744;
        vdp1WriteEA.m8_CMDSRCA = ((pThis->m0.m4_characterArea - (0x25C00000)) >> 3) + 0x248;
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
        vdp1WriteEA.m6_CMDCOLR = ((pThis->m0.m4_characterArea - (0x25C00000)) >> 3) + 0x1740;
        vdp1WriteEA.m8_CMDSRCA = ((pThis->m0.m4_characterArea - (0x25C00000)) >> 3) + 0x108;
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

