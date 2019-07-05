#pragma once

struct s_Vdp2PrintStatus
{
    s32 m0_X; //0
    s32 m4_Y; //4
    s32 m8_oldX; //8
    s32 mC_oldY; //C
    u16 m10_palette; //10
    u16 m14_oldPalette; //14
};
extern s_Vdp2PrintStatus vdp2PrintStatus;

struct s_VDP2Regs
{
    u16 m0_TVMD;
    u16 m2_EXTEN; 
    u16 m4_TVSTAT;
    u16 m6_VRSIZE;
    u16 m8_HCNT;
    u16 mA_VCNT;
    u16 mC_RESERVE;
    u16 mE_RAMCTL;
    u32 m10_CYCA0;
    u32 m14_CYCA1;
    u32 m18_CYCB0;
    u32 m1C_CYCB1;
    u16 m20_BGON;
    u16 m22_MZCTL;
    u16 m24_SFSEL;
    u16 m26_SFCODE;
    u16 m28_CHCTLA;
    u16 m2A_CHCTLB;
    u16 m2C_BMPNA;
    u16 m2E_BMPNB;
    u16 m30_PNCN0;
    u16 m32_PNCN1;
    u16 m34_PNCN2;
    u16 m36_PNCN3;
    u16 m38_PNCR;
    u16 m3A_PLSZ;
    u16 m3C_MPOFN;
    u16 m3E_MPOFR;
    u16 m40_MPABN0;
    u16 m42_MPCDN0;
    u16 m44_MPABN1;
    u16 m46_MPCDN1;
    u16 m48_MPABN2;
    u16 m4A_MPCDN2;
    u16 m4C_MPABN3;
    u16 m4E_MPCDN3;
    u16 m50_MPABRA;
    u16 m52_MPCDRA;
    u16 m54_MPEFRA;
    u16 m56_MPGHRA;
    u16 m58_MPIJRA;
    u16 m5A_MPKLRA;
    u16 m5C_MPMNRA;
    u16 m5E_MPOPRA;
    u16 m60_MPABRB;
    u16 m62_MPCDRB;
    u16 m64_MPEFRB;
    u16 m66_MPGHRB;
    u16 m68_MPIJRB;
    u16 m6A_MPKLRB;
    u16 m6C_MPMNRB;
    u16 m6E_MPOPRB;

    s32 m70_SCXN0;
    s32 m74_SCYN0;
    u32 m78_ZMXN0;
    u32 m7C_ZMYN0;

    u32 m80_SCXN1;
    u32 m84_SCYN1;
    u32 m88_ZMXN1;
    u32 m8C_ZMYN1;

    u16 m90_SCXN2;
    u16 m92_SCYN2;
    u16 m94_SCXN3;
    u16 m96_SCYN3;
    u16 m98_ZMCTL;

    u16 m9A_SCRCTL;

    u32 mAC_BKTA;

    //
    u16 mD0_WCTLA;
    u16 mD2_WCTLB;

    //
    u16 mE0_SPCTL;
    //

    u16 mE4_CRAOFA;
    u16 mE6_CRAOFB; // unset?
    s16 mE8_LNCLEN;

    u16 mEC_CCCTL;
    u16 mEE_SFCCMD;

    u16 mF0_PRISA;
    u16 mF2_PRISB;
    u16 mF4_PRISC;
    u16 mF6_PRISD;
    u16 mF8_PRINA;
    u16 mFA_PRINB;
    u16 mFC_PRIR;
    u16 m10A_CCRNB;
    u16 m110_CLOFEN;
    u16 m112_CLOFSL;
    s16 m114_COAR;
    s16 m116_COAG;
    s16 m118_COAB;
    s16 m11A_COBR;
    s16 m11C_COBG;
    s16 m11E_COBB;
};
extern s_VDP2Regs VDP2Regs_;

struct sVdp2Controls
{
    u32 m_0;
    s_VDP2Regs* m4_pendingVdp2Regs; // 4
    u32 m_8;
    u32 m_C;
    u32 m_10;
    u32 m_isDirty;
    u32 m_18;
    u32 m_1C;

    // 0x20
    s_VDP2Regs m20_registers[2];
};
extern sVdp2Controls vdp2Controls;

enum eVdp2LayerConfig
{
    m0_END = 0,
    m2_CHCN = 2,
    m5_CHSZ = 5,
    m6_PNB = 6,
    m7_CNSM = 7,
    m9_SCC = 9,
    m11_SCN = 11,
    m12_PLSZ = 12,
    m21 = 21,
    m34_W0E = 34,
    m37_W0A = 37,
    m40_CAOS = 40,
    m41 = 41,
    m44_CCEN = 44,
    m45 = 45,
    m46_SCCM = 46,
};

struct sLayerConfig
{
    eVdp2LayerConfig m_configType;
    u32 m_value;
};

extern u8* vdp2Palette;

void updateVDP2CoordinatesIncrement(u32, u32);
void setupVDP2CoordinatesIncrement2(u32, u32);

void initVDP2();
void reinitVdp2();

void setupNBG0(sLayerConfig* setup);
void setupNBG1(sLayerConfig* setup);
void setupNBG2(sLayerConfig* setup);
void setupNBG3(sLayerConfig* setup);

void setupVDP2StringRendering(s32 x, s32 y, s32 width, s32 height);
void vdp2DebugPrintSetPosition(s32 x, s32 y);
void clearVdp2StringFieldDebugList();
void clearVdp2TextSmallFont();
void clearVdp2TextLargeFont();
void clearVdp2TextMemory();
void vdp2DebugPrintNewLine(const std::string&);
int drawStringLargeFont(const char* text);
int drawStringSmallFont(const char* text);
int drawLineLargeFont(const char* text);
int drawLineSmallFont(const char* text);
void vdp2PrintfSmallFont(const char* format, ...);
void vdp2PrintfLargeFont(const char* format, ...);

extern u8* MENU_SCB;

struct sVdp2StringContext {
    u32 m0;
    s32 m4_cursorX; // 4
    s32 m8_cursorY; // 8
    s32 mC_X; // C
    s32 m10_Y; // 10
    s32 m14_Width; // 14
    s32 m18_Height; // 18
    u32 m1C; // 1C
    u32 m20; // 20
    u32 m24; // 24
    u32 m28; // 28
    u32 m2C; // 2C
    u32 m30; // 30
    u32 m34; // 34
    u32 m38; // 38
    u32 m3C; // 3C
};

extern sVdp2StringContext vdp2StringContext;

struct sVdp2StringControl
{
    u32 f0_index;
    u32 m4;
    u16* m8;
    u16* mC;
    const char* m10;
    u8 m14;
    u8 m15;
    sVdp2StringControl* pPrevious; // 18?
    sVdp2StringControl* pNext; // 1C?
};

extern u32 characterMap1[0x80];
extern u16 characterMap2[0x1000];
//extern u16* vdp2TextMemory;
extern u32 vdp2TextMemoryOffset;

extern sVdp2StringControl* pVdp2StringControl;

s32 setActiveFont(u16 r4);

void unpackGraphicsToVDP2(u8* compressedData, u8* destination);

void resetVdp2Strings();

u16 getVdp2VramU16(u32 offset);
u8 getVdp2VramU8(u32 offset);

u16 getVdp2CramU16(u32 offset);

u8* getVdp2Vram(u32 offset);
void setVdp2VramU16(u32 offset, u16 value);
void setVdp2VramU32(u32 offset, u32 value);
u8* getVdp2Cram(u32 offset);

int loadFile(const char* fileName, u8* destination, u16 vdp1Pointer);
void addToMemoryLayout(u8* pointer, u32 unk0);
void asyncDmaCopy(void* source, void* target, u32 size, u32 unk);
void asyncDmaCopy(sSaturnPtr EA, void* target, u32 size, u32 unk);

void initLayerMap(u32 layer, u32 planeA, u32 planeB, u32 planeC, u32 planeD);
s32 resetVdp2StringsSub1(u16* pData);

struct s_vdp2StringTask* createDisplayStringBorromScreenTask(p_workArea pTask, struct s_vdp2StringTask** r5, s16 duration, sSaturnPtr pString);

struct s_stringStatusQuery
{
    s32 m0_cursorX;
    s32 m4_cursorY;
    s32 m8_windowWidth;
    s32 mC_windowHeight;
    s32 m10_windowX1;
    s32 m14_windowY1;
    s32 m18_windowX2;
    s32 m1C_windowY2;
    const char* m20_string;
    u32 m24_vdp2MemoryOffset;
    u32 m28;
    u32 m2C;
};

void addStringToVdp2(const char* string, s_stringStatusQuery* vars);
void moveVdp2TextCursor(s_stringStatusQuery* vars);
void printVdp2String(s_stringStatusQuery* vars);

void VDP2DrawString(const char*);
s32 computeStringLength(sSaturnPtr pString, s32 r5);

void drawBlueBox(s32 x, s32 y, s32 width, s32 height);
void clearBlueBox(s32 x, s32 y, s32 width, s32 height);
void displayObjectIcon(s32 r4, s32 r5_x, s32 r6_y, s32 r7_iconId);
