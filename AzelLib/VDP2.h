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

    s32 SCXN0;
    s32 SCYN0;
    u32 ZMXN0;
    u32 ZMYN0;

    u32 SCXN1;
    u32 SCYN1;
    u32 ZMXN1;
    u32 ZMYN1;

    u16 SCXN2;
    u16 SCYN2;
    u16 SCXN3;
    u16 SCYN3;
    u16 ZMCTL;

    u16 SCRCTL;

    u32 BKTA;

    //
    u16 WCTLA;
    u16 WCTLB;

    //
    u16 SPCTL;
    //

    u16 CRAOFA;
    u16 CRAOFB; // unset?

    u16 CCCTL;

    u16 PRISA;
    u16 PRISB;
    u16 PRISC;
    u16 PRISD;
    u16 PRINA;
    u16 PRINB;
    u16 PRIR;
    u16 CCRNB;
    u16 N1COEN;
    u16 N1COSL;
    s16 COAR;
    s16 COAG;
    s16 COAB;
    s16 COBR;
    s16 COBG;
    s16 COBB;
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
    END = 0,
    CHCN = 2,
    CHSZ = 5,
    PNB = 6,
    CNSM = 7,
    SCN = 11,
    PLSZ = 12,
    CAOS = 40,
    W0E = 34,
    W0A = 37,
    CCEN = 44,
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

int loadFile(const char* fileName, u8* destination, u8* characterAreaPointer);
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
