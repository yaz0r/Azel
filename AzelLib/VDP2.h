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
    // --- Display Mode ---
    u16 m0_TVMD;     // TV Screen Mode: display enable, interlace mode, vertical res (224/240), horizontal res (320/352/640/704)
    u16 m2_EXTEN;    // External Signal Enable: external sync input, H/V blank signal input enable
    u16 m4_TVSTAT;   // TV Status (read-only): H-blank, V-blank, even/odd field status flags
    u16 m6_VRSIZE;   // VRAM Size: VRAM capacity select (4Mbit/8Mbit), VDP2 version number
    u16 m8_HCNT;     // H-Counter (read-only): current horizontal dot position
    u16 mA_VCNT;     // V-Counter (read-only): current vertical scan line
    u16 mC_RESERVE;  // Reserved
    u16 mE_RAMCTL;   // RAM Control: VRAM bank split (A0/A1/B0/B1), color RAM mode (RGB555/RGB888), coefficient table bank

    // --- VRAM Cycle Patterns (access timing per bank, 8 slots each) ---
    u32 m10_CYCA0;   // VRAM Cycle Pattern A0: access timing slots for VRAM bank A0 (fetch priority per layer)
    u32 m14_CYCA1;   // VRAM Cycle Pattern A1: access timing slots for VRAM bank A1
    u32 m18_CYCB0;   // VRAM Cycle Pattern B0: access timing slots for VRAM bank B0
    u32 m1C_CYCB1;   // VRAM Cycle Pattern B1: access timing slots for VRAM bank B1

    // --- Layer Enable / Special Effects ---
    u16 m20_BGON;    // Screen Display Enable: enable bits for NBG0, NBG1, NBG2, NBG3, RBG0, and transparent mode
    u16 m22_MZCTL;   // Mosaic Control: mosaic block size (H/V) and enable per layer
    u16 m24_SFSEL;   // Special Function Code Select: which bits of the pattern name form the special function code
    u16 m26_SFCODE;  // Special Function Code: code value used to trigger special priority or color calculation

    // --- Character / Bitmap Control ---
    u16 m28_CHCTLA;  // Character Control A: color depth and bitmap/cell mode for NBG0 and NBG1
    u16 m2A_CHCTLB;  // Character Control B: color depth and bitmap/cell mode for NBG2, NBG3, and RBG0
    u16 m2C_BMPNA;   // Bitmap Palette Number A: palette bank offset for NBG0 and NBG1 in bitmap mode
    u16 m2E_BMPNB;   // Bitmap Palette Number B: palette bank offset for RBG0 in bitmap mode

    // --- Pattern Name Control (cell/tile data format) ---
    u16 m30_PNCN0;   // Pattern Name Control NBG0: word size (1/2 word), character size (1x1/2x2), supplementary data bits
    u16 m32_PNCN1;   // Pattern Name Control NBG1: same fields as PNCN0 but for NBG1
    u16 m34_PNCN2;   // Pattern Name Control NBG2: same fields as PNCN0 but for NBG2
    u16 m36_PNCN3;   // Pattern Name Control NBG3: same fields as PNCN0 but for NBG3
    u16 m38_PNCR;    // Pattern Name Control RBG0: same fields as PNCN0 but for RBG0

    // --- Map / Plane Layout ---
    u16 m3A_PLSZ;    // Plane Size: plane dimensions (1x1, 2x1, 2x2 pages) for NBG0-3 and RBG0
    u16 m3C_MPOFN;   // Map Offset NBG: VRAM page offset applied to NBG0-3 plane addresses
    u16 m3E_MPOFR;   // Map Offset RBG: VRAM page offset applied to RBG0 rotation parameter A/B plane addresses

    // --- NBG Map Plane Addresses ---
    u16 m40_MPABN0;  // Map NBG0 Planes A/B: VRAM addresses for NBG0 scroll map planes A and B
    u16 m42_MPCDN0;  // Map NBG0 Planes C/D: VRAM addresses for NBG0 scroll map planes C and D
    u16 m44_MPABN1;  // Map NBG1 Planes A/B: VRAM addresses for NBG1 scroll map planes A and B
    u16 m46_MPCDN1;  // Map NBG1 Planes C/D: VRAM addresses for NBG1 scroll map planes C and D
    u16 m48_MPABN2;  // Map NBG2 Planes A/B: VRAM addresses for NBG2 scroll map planes A and B
    u16 m4A_MPCDN2;  // Map NBG2 Planes C/D: VRAM addresses for NBG2 scroll map planes C and D
    u16 m4C_MPABN3;  // Map NBG3 Planes A/B: VRAM addresses for NBG3 scroll map planes A and B
    u16 m4E_MPCDN3;  // Map NBG3 Planes C/D: VRAM addresses for NBG3 scroll map planes C and D

    // --- RBG0 Map Plane Addresses (Rotation Parameter A, 16 planes A-P) ---
    u16 m50_MPABRA;  // RBG0 Map RA Planes A/B
    u16 m52_MPCDRA;  // RBG0 Map RA Planes C/D
    u16 m54_MPEFRA;  // RBG0 Map RA Planes E/F
    u16 m56_MPGHRA;  // RBG0 Map RA Planes G/H
    u16 m58_MPIJRA;  // RBG0 Map RA Planes I/J
    u16 m5A_MPKLRA;  // RBG0 Map RA Planes K/L
    u16 m5C_MPMNRA;  // RBG0 Map RA Planes M/N
    u16 m5E_MPOPRA;  // RBG0 Map RA Planes O/P

    // --- RBG0 Map Plane Addresses (Rotation Parameter B, 16 planes A-P) ---
    u16 m60_MPABRB;  // RBG0 Map RB Planes A/B
    u16 m62_MPCDRB;  // RBG0 Map RB Planes C/D
    u16 m64_MPEFRB;  // RBG0 Map RB Planes E/F
    u16 m66_MPGHRB;  // RBG0 Map RB Planes G/H
    u16 m68_MPIJRB;  // RBG0 Map RB Planes I/J
    u16 m6A_MPKLRB;  // RBG0 Map RB Planes K/L
    u16 m6C_MPMNRB;  // RBG0 Map RB Planes M/N
    u16 m6E_MPOPRB;  // RBG0 Map RB Planes O/P

    // --- NBG0 Scroll and Zoom ---
    s32 m70_SCXN0;   // Scroll X NBG0: horizontal scroll amount (16.8 fixed point, signed)
    s32 m74_SCYN0;   // Scroll Y NBG0: vertical scroll amount (16.8 fixed point, signed)
    u32 m78_ZMXN0;   // Zoom X NBG0: horizontal zoom ratio (8.8 fixed point)
    u32 m7C_ZMYN0;   // Zoom Y NBG0: vertical zoom ratio (8.8 fixed point)

    // --- NBG1 Scroll and Zoom ---
    u32 m80_SCXN1;   // Scroll X NBG1: horizontal scroll amount
    u32 m84_SCYN1;   // Scroll Y NBG1: vertical scroll amount
    u32 m88_ZMXN1;   // Zoom X NBG1: horizontal zoom ratio
    u32 m8C_ZMYN1;   // Zoom Y NBG1: vertical zoom ratio

    // --- NBG2/NBG3 Scroll (integer only, no zoom support) ---
    u16 m90_SCXN2;   // Scroll X NBG2: horizontal scroll (integer pixels only)
    u16 m92_SCYN2;   // Scroll Y NBG2: vertical scroll (integer pixels only)
    u16 m94_SCXN3;   // Scroll X NBG3: horizontal scroll (integer pixels only)
    u16 m96_SCYN3;   // Scroll Y NBG3: vertical scroll (integer pixels only)

    // --- Scroll Control ---
    u16 m98_ZMCTL;   // Zoom Control: enables reduced-rate zoom (2x decimation) for NBG0/NBG1
    u16 m9A_SCRCTL;  // Scroll Control: line scroll and vertical cell scroll enable for NBG0/NBG1

    // --- Table Addresses ---
    u8* m9C_VCSTA;   // Vertical Cell Scroll Table Address: VRAM address of vertical cell scroll offset table
    u8* mA0_LSTA0;   // Line Scroll Table Address NBG0: VRAM address of per-line scroll data for NBG0
    u8* mA4_LSTA1;   // Line Scroll Table Address NBG1: VRAM address of per-line scroll data for NBG1
    u32 mA8_LCTA;    // Line Color Screen Table Address: VRAM address of per-line color screen data
    u32 mAC_BKTA;    // Back Screen Table Address: VRAM address of background color data (single color or per-line)

    // --- Rotation Parameters ---
    u16 mB0_RPMD;    // Rotation Parameter Mode: rotation source (A only, B only, A then B, or coefficient table select)
    u16 mB2_RESERVE; // Reserved
    u16 mB4_KTCTL;   // Coefficient Table Control: enables coefficient table, data type (scaling/transparency/priority/color)
    s16 mB6_KTAOF;   // Coefficient Table Address Offset: signed offset into the coefficient VRAM bank
    u16 mB8_OVPNRA;  // Over Pattern Name RA: tile drawn outside the valid RBG0 rotation area for parameter A
    u16 mBA_OVPNRB;  // Over Pattern Name RB: tile drawn outside the valid RBG0 rotation area for parameter B
    u16 mBC_RPTAU;   // Rotation Parameter Table Address Upper: upper word of VRAM address for rotation parameter table
    u16 mBE_RPTAL;   // Rotation Parameter Table Address Lower: lower word of VRAM address for rotation parameter table

    // --- Window Position ---
    u16 mC0_WPSX0;   // Window 0 Start X: left edge of rectangular window 0
    u16 mC2_WPSY0;   // Window 0 Start Y: top edge of rectangular window 0
    u16 mC4_WPEX0;   // Window 0 End X: right edge of rectangular window 0
    u16 mC6_WPEY0;   // Window 0 End Y: bottom edge of rectangular window 0
    u16 mC8_WPSX1;   // Window 1 Start X: left edge of rectangular window 1
    u16 mCA_WPSY1;   // Window 1 Start Y: top edge of rectangular window 1
    u16 mCC_WPEX1;   // Window 1 End X: right edge of rectangular window 1
    u16 mCE_WPEY1;   // Window 1 End Y: bottom edge of rectangular window 1

    // --- Window Control ---
    u16 mD0_WCTLA;   // Window Control A: window enable and inside/outside logic for NBG0 and NBG1
    u16 mD2_WCTLB;   // Window Control B: window enable and inside/outside logic for NBG2 and NBG3
    u16 mD4_WCTLC;   // Window Control C: window enable and inside/outside logic for RBG0 and sprites
    u16 mD6_WCTLD;   // Window Control D: window enable and inside/outside logic for back screen and color screen
    u32 mD8_LWTA0;   // Line Window Table Address 0: VRAM address for per-line horizontal coordinates of window 0
    u32 mDC_LWTA1;   // Line Window Table Address 1: VRAM address for per-line horizontal coordinates of window 1

    // --- Sprite / Shadow / Color RAM ---
    u16 mE0_SPCTL;   // Sprite Control: sprite type, color mode, priority/color calculation attribute source
    u16 mE2_SDCTL;   // Shadow Control: normal shadow and MSB shadow enable per sprite color bank
    u16 mE4_CRAOFA;  // Color RAM Address Offset A: palette bank base offset for NBG0, NBG1, and NBG2
    u16 mE6_CRAOFB;  // Color RAM Address Offset B: palette bank base offset for NBG3, RBG0, and sprites
    s16 mE8_LNCLEN;  // Line Color Screen Enable: enables line color screen insertion per layer
    u16 mEA_SFPRMD;  // Special Priority Mode: priority calculation method when the special function code matches

    // --- Color Calculation (Alpha Blending) ---
    u16 mEC_CCCTL;   // Color Calculation Control: enables color calculation (alpha blending) per layer
    u16 mEE_SFCCMD;  // Special Function Color Calculation Mode: color calculation mode for special function code matches

    // --- Display Priority ---
    u16 mF0_PRISA;   // Priority Sprite 0/1: display priority (0-7) for sprite color banks 0 and 1
    u16 mF2_PRISB;   // Priority Sprite 2/3: display priority for sprite color banks 2 and 3
    u16 mF4_PRISC;   // Priority Sprite 4/5: display priority for sprite color banks 4 and 5
    u16 mF6_PRISD;   // Priority Sprite 6/7: display priority for sprite color banks 6 and 7
    u16 mF8_PRINA;   // Priority NBG0/NBG1: display priority for NBG0 (low byte) and NBG1 (high byte)
    u16 mFA_PRINB;   // Priority NBG2/NBG3: display priority for NBG2 (low byte) and NBG3 (high byte)
    u16 mFC_PRIR;    // Priority RBG0: display priority for RBG0
    u16 mFE_RESERVE; // Reserved

    // --- Color Calculation Ratios ---
    u16 m100_CCRSA;  // Color Calculation Ratio Sprite 0/1: alpha blending ratio for sprite color banks 0 and 1
    u16 m102_CCRSB;  // Color Calculation Ratio Sprite 2/3: alpha blending ratio for sprite color banks 2 and 3
    u16 m104_CCRSC;  // Color Calculation Ratio Sprite 4/5: alpha blending ratio for sprite color banks 4 and 5
    u16 m106_CCRSD;  // Color Calculation Ratio Sprite 6/7: alpha blending ratio for sprite color banks 6 and 7
    u16 m108_CCRNA;  // Color Calculation Ratio NBG0/NBG1: alpha blending ratio for NBG0 and NBG1
    u16 m10A_CCRNB;  // Color Calculation Ratio NBG2/NBG3: alpha blending ratio for NBG2 and NBG3
    u16 m10C_CCRR;   // Color Calculation Ratio RBG0: alpha blending ratio for RBG0
    u16 m10E_CCRLB;  // Color Calculation Ratio Line/Back: alpha blending ratio for line color and back screens

    // --- Color Offset ---
    u16 m110_CLOFEN; // Color Offset Enable: enables additive color offset per layer
    u16 m112_CLOFSL; // Color Offset Select: selects color offset table A or B per layer
    s16 m114_COAR;   // Color Offset A Red: red channel additive offset for group A (-256 to +255)
    s16 m116_COAG;   // Color Offset A Green: green channel additive offset for group A
    s16 m118_COAB;   // Color Offset A Blue: blue channel additive offset for group A
    s16 m11A_COBR;   // Color Offset B Red: red channel additive offset for group B (-256 to +255)
    s16 m11C_COBG;   // Color Offset B Green: green channel additive offset for group B
    s16 m11E_COBB;   // Color Offset B Blue: blue channel additive offset for group B
};
extern s_VDP2Regs VDP2Regs_;

struct sVdp2Controls
{
    u32 m0_doubleBufferIndex;
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
    m21_LCSY = 21,
    m27_RPMD = 27,
    m31_RxKTE = 31,
    m34_W0E = 34,
    m37_W0A = 37,
    m40_CAOS = 40,
    m41 = 41,
    m44_CCEN = 44,
    m45_COEN = 45,
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

void setupNBG0(const sLayerConfig* setup);
void setupNBG1(const sLayerConfig* setup);
void setupNBG2(const sLayerConfig* setup);
void setupNBG3(const sLayerConfig* setup);
void setupRGB0(const sLayerConfig* setup);
void setupRotationParams(const sLayerConfig* setup);
void setupRotationParams2(const sLayerConfig* setup);

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
    u32 m0_index;
    u32 m4;
    u16* m8;
    u16* mC;
    const char* m10;
    u8 m14;
    u8 m15;
    sVdp2StringControl* pPrevious; // 18?
    sVdp2StringControl* pNext; // 1C?
};

extern u16 characterMap2[0x1000];
//extern u16* vdp2TextMemory;
extern u32 vdp2TextMemoryOffset;

extern sVdp2StringControl* pVdp2StringControl;

typedef std::vector<fixedPoint> tCoefficientTable;

struct sVdpVar1
{
    void* m0_source[2];
    u8* m8_destination;
    s16 mC_size;
    s8 mE_isDoubleBuffered;
    s8 mF_isPending;
    sVdpVar1* m10_nextTransfert;
    // size 0x14
};

extern sVdpVar1* vdpVar3;
extern std::array<sVdpVar1, 14> vdpVar1;

s32 setActiveFont(u16 r4);

void unpackGraphicsToVDP2(u8* compressedData, u8* destination);

void resetVdp2Strings();

u32 getVdp2VramU32(u32 offset);
u16 getVdp2VramU16(u32 offset);
u8 getVdp2VramU8(u32 offset);

u16 getVdp2CramU16(u32 offset);

u8* getVdp2Vram(u32 offset);
u32 getVdp2VramOffset(u8* ptr);
void setVdp2VramU16(u32 offset, u16 value);
void setVdp2VramU8(u32 offset, u8 value);
void setVdp2VramU32(u32 offset, u32 value);
u8* getVdp2Cram(u32 offset);

void clearVdp2TextAreaSub1Sub1(u16 r4);
u32 clearVdp2TextAreaSub1(u16 r4, s32 x, s32 y);

int loadFile(const char* fileName, u8* destination, u16 vdp1Pointer);
int loadFile2(const char* fileName, u8* destination, u16 vdp1Pointer);
int loadFile(const char* fileName, s_fileBundle** destination, u16 vdp1Pointer);
void addToMemoryLayout(u8* pointer, u32 unk0);
void asyncDmaCopy(void* source, void* target, u32 size, u32 unk);
void asyncDmaCopy(sSaturnPtr EA, void* target, u32 size, u32 unk);

void initLayerMap(u32 layer, u32 planeA, u32 planeB, u32 planeC, u32 planeD);
s32 resetVdp2StringsSub1(u16* pData);

struct s_vdp2StringTask* createDisplayStringBorromScreenTask(p_workArea pTask, s_vdp2StringTask** r5, s16 duration, sSaturnPtr pString);

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

void getVdp2StringContext(const char* string, s_stringStatusQuery* vars);
void moveVdp2TextCursor(s_stringStatusQuery* vars);
void printVdp2String(s_stringStatusQuery* vars);

void VDP2DrawString(const char*);
s32 computeStringLength(sSaturnPtr pString, s32 r5);
s32 computeStringLength(const char*, s32 r5);

void drawBlueBox(s32 x, s32 y, s32 width, s32 height, u32);
void clearBlueBox(s32 x, s32 y, s32 width, s32 height);
void displayObjectIcon(s32 r4, s32 r5_x, s32 r6_y, s32 r7_iconId);

u32 rotateRightR0ByR1(u32 r0, u32 r1);

void interruptVDP2Update();

void printVdp2Number2(int value, int length);
void printVdp2String2(const char* string);

