#include "PDS.h"
#include "menuSprite.h"

void drawMenuSprite(const s_menuSprite* r4, s16 r5, s16 r6, u32 r7)
{
    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;

    vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0, normal sprite + JUMP
    vdp1WriteEA.m4_CMDPMOD = 0x80; // CMDPMOD
    vdp1WriteEA.m6_CMDCOLR = r7; // CMDCOLR
    vdp1WriteEA.m8_CMDSRCA = r4->SRCA; // CMDSRCA
    vdp1WriteEA.mA_CMDSIZE = r4->SIZE; // CMDSIZE
    vdp1WriteEA.mC_CMDXA = r4->X + r5 - 0xB0; // CMDXA
    vdp1WriteEA.mE_CMDYA = r4->Y + r6 - 0x70; // CMDYA

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

void drawMenuSprite2(const s_menuSprite* r4, s16 r5, s16 r6, u32 r7)
{
    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;

    vdp1WriteEA.m0_CMDCTRL = 0x1000; // command 0, normal sprite + JUMP
    vdp1WriteEA.m4_CMDPMOD = 0xA0; // CMDPMOD
    vdp1WriteEA.m6_CMDCOLR = r7; // CMDCOLR
    vdp1WriteEA.m8_CMDSRCA = r4->SRCA; // CMDSRCA
    vdp1WriteEA.mA_CMDSIZE = r4->SIZE; // CMDSIZE
    vdp1WriteEA.mC_CMDXA = r4->X + r5 - 0xB0; // CMDXA
    vdp1WriteEA.mE_CMDYA = r4->Y + r6 - 0x70; // CMDYA

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}
