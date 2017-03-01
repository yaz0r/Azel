#pragma once

struct s_Vdp2PrintStatus
{
    s32 X;
    s32 Y;
    s32 oldX;
    s32 oldY;
    u16 field_10;
    u16 field_12;
};
extern s_Vdp2PrintStatus vdp2PrintStatus;

struct s_VDP2Regs
{
    u16 TVMD;
    u16 EXTEN; 
    u16 TVSTAT;
    u16 VRSIZE;
    u16 HCNT;
    u16 VCNT;
    u16 m_dummy_C;
    u16 RAMCTL;
    u32 CYCA0;
    u32 CYCA1;
    u32 CYCB0;
    u32 CYCB1;
    u16 BGON;
    u16 MZCTL;
    u16 SFSEL;
    u16 SFCODE;
    u16 CHCTLA;
    u16 CHCTLB;
    u16 BMPNA;
    u16 BMPNB;
    u16 PNCN0;
    u16 PNCN1;
    u16 PNCN2;
    u16 PNCN3;
    u16 PNCR;
    u16 PLSZ;
    u16 MPOFN;
    u16 MPOFR;
    u16 MPABN0;
    u16 MPCDN0;
    u16 MPABN1;
    u16 MPCDN1;
    u16 MPABN2;
    u16 MPCDN2;
    u16 MPABN3;
    u16 MPCDN3;
    u16 MPABRA;
    u16 MPCDRA;
    u16 MPEFRA;
    u16 MPGHRA;
    u16 MPIJRA;
    u16 MPKLRA;
    u16 MPMNRA;
    u16 MPOPRA;
    u16 MPABRB;
    u16 MPCDRB;
    u16 MPEFRB;
    u16 MPGHRB;
    u16 MPIJRB;
    u16 MPKLRB;
    u16 MPMNRB;
    u16 MPOPRB;

    u16 SCXIN0;
    u16 SCXDN0;
    u16 SCYIN0;
    u16 SCYDN0;
    u16 ZMXIN0;
    u16 ZMXDN0;
    u16 ZMYIN0;
    u16 ZMYDN0;

    u16 SCXIN1;
    u16 SCXDN1;
    u16 SCYIN1;
    u16 SCYDN1;
    u16 ZMXIN1;
    u16 ZMXDN1;
    u16 ZMYIN1;
    u16 ZMYDN1;

    u16 SCXN2;
    u16 SCYN2;
    u16 SCXN3;
    u16 SCYN3;
    u16 ZMCTL;

    u16 SCRCTL;

    //
    u16 SPCTL;
    //

    u16 CRAOFA;

    u16 N1COEN;
    u16 N1COSL;
    u16 COAR;
    u16 COAG;
    u16 COAB;
    u16 COBR;
    u16 COBG;
    u16 COBB;
};
extern s_VDP2Regs VDP2Regs_;

void initVDP2();

void vdp2DebugPrintSetPosition(s32 x, s32 y);
void clearVdp2Text();
int renderVdp2String(char* text);
