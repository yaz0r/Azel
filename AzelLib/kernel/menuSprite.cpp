#include "PDS.h"
#include "menuSprite.h"

void drawMenuSprite(const s_menuSprite* r4, s16 r5, s16 r6, u32 r7)
{
    u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;

    setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0, normal sprite + JUMP
    setVdp1VramU16(vdp1WriteEA + 0x04, 0x80); // CMDPMOD
    setVdp1VramU16(vdp1WriteEA + 0x06, r7); // CMDCOLR
    setVdp1VramU16(vdp1WriteEA + 0x08, r4->SRCA); // CMDSRCA
    setVdp1VramU16(vdp1WriteEA + 0x0A, r4->SIZE); // CMDSIZE
    setVdp1VramU16(vdp1WriteEA + 0x0C, r4->X + r5 - 0xB0); // CMDXA
    setVdp1VramU16(vdp1WriteEA + 0x0E, r4->Y + r6 - 0x70); // CMDYA

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

void drawMenuSprite2(const s_menuSprite* r4, s16 r5, s16 r6, u32 r7)
{
    u32 vdp1WriteEA = graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;

    setVdp1VramU16(vdp1WriteEA + 0x00, 0x1000); // command 0, normal sprite + JUMP
    setVdp1VramU16(vdp1WriteEA + 0x04, 0xA0); // CMDPMOD
    setVdp1VramU16(vdp1WriteEA + 0x06, r7); // CMDCOLR
    setVdp1VramU16(vdp1WriteEA + 0x08, r4->SRCA); // CMDSRCA
    setVdp1VramU16(vdp1WriteEA + 0x0A, r4->SIZE); // CMDSIZE
    setVdp1VramU16(vdp1WriteEA + 0x0C, r4->X + r5 - 0xB0); // CMDXA
    setVdp1VramU16(vdp1WriteEA + 0x0E, r4->Y + r6 - 0x70); // CMDYA

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = vdp1WriteEA >> 3;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA = vdp1WriteEA + 0x20;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}
