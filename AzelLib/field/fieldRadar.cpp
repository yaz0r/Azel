#include "PDS.h"
#include "fieldRadar.h"

struct s_FieldRadar : public s_workAreaTemplate<s_FieldRadar>
{
    s_memoryAreaOutput m0;
    s16 m8;
    s16 mA;
    s16 mC_X;
    s16 mE_Y;
    s16 m10;
    s16 m12;
    s16 m28;
    s16 m2A;
    s32 m30;
    s32 m34;
    fixedPoint m38_dragonDirection;
    u32 m3C;
    s32 m40_paletteFadeDuration;
    s8 m44;
    s8 m48;
    s8 m49;
    s8 m4A_newPaletteIndex;
    s8 m4B;
    s8 m4C_dangerLevel;
    s32 m50;
    s32 m54;
    s32 m58_altitudeGaugeEnabled;
    s8 m59;
    s8 m5A_isHidden;
    s8 m5B;
    s32 m5C;
    s32 m60;
    s32 m64;
    fixedPoint m68;
};


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
    r14->m64 = r4;
    r14->m68 = MTH_Mul(r4, r4);
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

u8 fieldPalettes[1][0x20] =
{
    {
        0xA0,
        0xC4,
        0x9C,
        0x00,
        0xA8,
        0x00,
        0xB8,
        0x00,
        0xC8,
        0x00,
        0xCC,
        0x00,
        0xD4,
        0x00,
        0xDC,
        0x00,
        0xE4,
        0x00,
        0xF0,
        0x00,
        0xFC,
        0x00,
        0xFC,
        0x20,
        0xFC,
        0x61,
        0xFC,
        0xA2,
        0xFC,
        0xE3,
        0xFD,
        0x04,
    },
};

s8 paletteIndexTable[4] = {
    0,1,2,2
};

void createPaletteFadeTask(p_workArea parent, void* pal1, void* pal2, s32 duration)
{
    FunctionUnimplemented();
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

    FunctionUnimplemented(); // Radar line draw

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

