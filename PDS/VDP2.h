#pragma once

struct s_Vdp2PrintStatus
{
    s32 X; //0
    s32 Y; //4
    s32 oldX; //8
    s32 oldY; //C
    u16 palette; //10
    u16 oldPalette; //14
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

    u16 PRINA;
    u16 PRINB;
    u16 PRIR;

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

struct sVdp2Controls
{
    u32 m_0;
    s_VDP2Regs* m_pendingVdp2Regs;
    u32 m_8;
    u32 m_C;
    u32 m_10;
    u32 m_isDirty;
    u32 m_18;
    u32 m_1C;

    // 0x20
    s_VDP2Regs m_registers[2];
};
extern sVdp2Controls vdp2Controls;

enum eVdp2LayerConfig
{
    END = 0,
    CHCN = 2,
    CHSZ = 5,
    PNB = 6,
    CNSM = 7,
    PLSZ = 12,
    CAOS = 40,
};

struct sLayerConfig
{
    eVdp2LayerConfig m_configType;
    u32 m_value;
};

extern u8 incrementVar;
void updateVDP2CoordinatesIncrement(u32, u32);
void updateVDP2CoordinatesIncrement2(u32, u32);

void initVDP2();
void reinitVdp2();

void setupNBG0(sLayerConfig* setup);
void setupNBG1(sLayerConfig* setup);
void setupNBG2(sLayerConfig* setup);
void setupNBG3(sLayerConfig* setup);

void vdp2DebugPrintSetPosition(s32 x, s32 y);
void clearVdp2StringFieldDebugList();
void clearVdp2TextSmallFont();
void clearVdp2TextLargeFont();
void clearVdp2TextMemory();
int drawStringLargeFont(const char* text);
int drawStringSmallFont(const char* text);
int drawLineLargeFont(const char* text);
int drawLineSmallFont(const char* text);
void vdp2PrintfSmallFont(const char* format, ...);
void vdp2PrintfLargeFont(const char* format, ...);

u16 getVdp2VramU16(u32 offset);
u8 getVdp2VramU8(u32 offset);

u16 getVdp2CramU16(u32 offset);

u8* getVdp2Vram(u32 offset);
void setVdp2VramU16(u32 offset, u16 value);
u8* getVdp2Cram(u32 offset);

void loadFile(const char* fileName, u8* destination, u32 unk0);
void addToMemoryLayout(u8* pointer, u32 unk0);
void asyncDmaCopy(void* source, void* target, u32 size, u32 unk);

void initLayerMap(u32 layer, u32 planeA, u32 planeB, u32 planeC, u32 planeD);
