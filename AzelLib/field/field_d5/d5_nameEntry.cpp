#include "PDS.h"
#include "d5_nameEntry.h"
#include "d5_particles.h"
#include "o_fld_d5.h"
#include "field.h"
#include "field/field_a3/o_fld_a3.h"
#include "field/field_a3/o_fld_a3_1.h"
#include "audio/soundDriver.h"
#include "audio/systemSounds.h"
#include "kernel/graphicalObject.h"
#include "kernel/vdp1Allocator.h"
#include "kernel/vdp1AnimatedQuad.h"
#include "kernel/fade.h"
#include "menu_dragon.h"

struct s_nameEntryUITask;
struct s_nameEntryParticleTask;

struct s_nameEntryTask : public s_workAreaTemplate<s_nameEntryTask>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &nameEntryInit, NULL, &nameEntryDraw, NULL };
        return &taskDefinition;
    }

    static void nameEntryInit(s_nameEntryTask* pThis);
    static void nameEntryDraw(s_nameEntryTask* pThis);

    u32 m0_state;
    s_nameEntryUITask* m4_nameEntryTask;
    s_nameEntryParticleTask* m8_particleTask;
    // size 0xC
};

// 060237a8
static void preloadNameEntryResourcesInternal()
{
    // Note: original also preloads NAME_ENT.CGB with destination 0 (Saturn CD cache optimization, skipped)
    loadFile("NAME_ENT.SCB", getVdp2Vram(0x10000), 0);
}

// 060237a8
void preloadNameEntryResources()
{
    preloadNameEntryResourcesInternal();
}

// Keyboard layout tables
// Mode 0: uppercase (4 cols x 8 rows)
static const char nameEntryKeyboard_upper[] =
    "AHOVBIPWCJQXDKRYELSZFMT GNU     ";

// Per-mode layout: {xStart, yStart, cols, rows}
struct s_nameEntryKeyboardLayout
{
    s16 m0_xStart;
    s16 m2_yStart;
    s16 m4_cols;
    s16 m6_rows;
    const char* m8_chars;
    s16 mC_charStride; // number of chars per row
};

static const s_nameEntryKeyboardLayout nameEntryKeyboardLayouts[] =
{
    { 0x30, 0x28, 0x10, 0x04, nameEntryKeyboard_upper, 4 }, // mode 0
};

// Action bar layout: {xStart, yStart, cols, rows}
static const s_nameEntryKeyboardLayout nameEntryActionBarLayout =
    { 0xB7, 0x6B, 0x03, 0x01, nullptr, 0 };

// VDP2 scroll positions per keyboard mode (X, Y as fixed-point)
static const s16 nameEntryScrollPositions[][2] =
{
    { 0x0000, 0x00E0 },
    { 0x0158, 0x0000 },
    { 0x02B0, 0x0000 },
};

// Saved graphics state for name entry UI (replaces Saturn's raw memcpy with sized buffer)
struct s_nameEntryGraphicsState
{
    s_graphicEngineStatus_405C m0_graphicState;
    sVdp2Controls m1_vdp2Controls;
    u8 m2_cramBackup[0x40];
};

struct s_nameEntryUITask : public s_workAreaTemplate<s_nameEntryUITask>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { NULL, NULL, &Draw, &Delete };
        return &taskDefinition;
    }

    static void Draw(s_nameEntryUITask* pThis);
    static void Delete(s_nameEntryUITask* pThis);

    u8 m0_cursorMode; // 0x00: 0=normal, 1=confirmed
    u8 m1_keyboardMode; // 0x01: keyboard page (0=upper, 1=lower, 2=katakana)
    u8 m2_pad[2];
    s32 m4_state; // 0x04: 0=init, 1=input, 2=transition animation
    u8 m8_inActionBar; // 0x08: 0=char grid, 1=action bar
    u8 m9_pad;
    s16 mA_cursorX; // 0x0A
    s16 mC_cursorY; // 0x0C
    u8 mE_pad[2];
    npcFileDeleter* m10_npcHandle; // 0x10
    s32 m14_animCounter; // 0x14
    s16 m18_startX; // 0x18
    s16 m1A_startY; // 0x1A
    s16 m1C_endX; // 0x1C
    s16 m1E_endY; // 0x1E
    char m20_nameBuffer[16]; // 0x20: 15 chars + null
    s32 m34_nameLength; // 0x34
    char* m38_destBuffer; // 0x38: destination for confirmed name
    u8 m3C_menuAllowedFlag; // 0x3C
    u8 m3D_pad[3];
    s_nameEntryGraphicsState m40_savedGraphicsState; // 0x40: saved graphicEngineStatus.m405C + vdp2Controls + CRAM
};

// 06023112: save current graphics state to buffer
static void nameEntryUI_saveGraphicsState(s_nameEntryGraphicsState* dest)
{
    dest->m0_graphicState = graphicEngineStatus.m405C;
    dest->m1_vdp2Controls = vdp2Controls;
    asyncDmaCopy(getVdp2Cram(0x800), dest->m2_cramBackup, 0x40, 0);
}

// 060230e4: restore graphics state from buffer
static void nameEntryUI_restoreGraphicsState(s_nameEntryGraphicsState* src)
{
    graphicEngineStatus.m405C = src->m0_graphicState;
    vdp2Controls = src->m1_vdp2Controls;
    asyncDmaCopy(src->m2_cramBackup, getVdp2Cram(0x800), 0x40, 0);
}

// Sprite definitions from PDS_Base at 0x0021076a (0x1C bytes each, big-endian values)
struct s_nameEntrySpriteData
{
    u16 m0_CMDCTRL;
    u16 m2_CMDPMOD;
    u16 m4_CMDSRCAbase;
    u16 m6_CMDSIZE;
    u16 m8_CMDCOLR;
    u16 mA_pad;
    s32 mC_widthFP;  // fixed point, >> 12 for pixels
    s32 m10_heightFP; // fixed point, >> 12 for pixels
    s32 m14_xOffsetFP; // fixed point, >> 12 for pixels
    s32 m18_yOffsetFP; // fixed point, >> 12 for pixels
};

static const s_nameEntrySpriteData nameEntrySpriteData[] =
{
    // Entry 0: text underline cursor (16x1)
    { 0x1001, 0x0080, 0x0000, 0x0201, 0x6410, 0x0001, 0x00010000, 0x00001000, 0x00000000, (s32)0xFFFF8000 },
    // Entry 1: keyboard grid cursor (16x16)
    { 0x1001, 0x0080, 0x0004, 0x0210, 0x6410, 0x0001, 0x00010000, 0x00010000, 0x00000000, 0x00000000 },
};

// 060231a8
static void nameEntryUI_drawCursorSprite(npcFileDeleter* npcHandle, s16* position, u32 spriteIndex)
{
    const s_nameEntrySpriteData& sprite = nameEntrySpriteData[spriteIndex & 1];

    s16 posX = position[0] - (s16)graphicEngineStatus.m405C.m44_localCoordinatesX;
    s16 posY = (s16)graphicEngineStatus.m405C.m46_localCoordinatesY - position[1];

    s16 width = (s16)(sprite.mC_widthFP >> 12);
    s16 height = (s16)(sprite.m10_heightFP >> 12);
    s16 xOffset = (s16)(sprite.m14_xOffsetFP >> 12);
    s16 yOffset = (s16)(sprite.m18_yOffsetFP >> 12);

    s_vdp1Command& vdp1WriteEA = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
    vdp1WriteEA.m0_CMDCTRL = sprite.m0_CMDCTRL;
    vdp1WriteEA.m4_CMDPMOD = sprite.m2_CMDPMOD;
    vdp1WriteEA.m6_CMDCOLR = sprite.m8_CMDCOLR;

    s16 vdp1MemoryOffset = 0;
    if (npcHandle && npcHandle->m4_vd1Allocation)
    {
        vdp1MemoryOffset = npcHandle->m4_vd1Allocation->m4_vdp1Memory;
    }
    vdp1WriteEA.m8_CMDSRCA = sprite.m4_CMDSRCAbase + vdp1MemoryOffset;
    vdp1WriteEA.mA_CMDSIZE = sprite.m6_CMDSIZE;

    vdp1WriteEA.mC_CMDXA = posX + xOffset;
    vdp1WriteEA.mE_CMDYA = -(posY + yOffset);
    vdp1WriteEA.m14_CMDXC = posX + xOffset + width;
    vdp1WriteEA.m16_CMDYC = -(posY - height + yOffset);

    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &vdp1WriteEA;
    graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;

    graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
    graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
    graphicEngineStatus.m14_vdp1Context[0].mC += 1;
}

// 0602313e
static void nameEntryUI_addChar(s_nameEntryUITask* pThis)
{
    s32 newLen = pThis->m34_nameLength + 1;
    pThis->m34_nameLength = newLen;
    pThis->m20_nameBuffer[newLen] = '\0';
    if (pThis->m34_nameLength > 0xE)
    {
        pThis->m8_inActionBar = 1;
        pThis->mA_cursorX = 2;
        pThis->mC_cursorY = 0;
    }
    playSystemSoundEffect(0);
}

// 06023168
static void nameEntryUI_deleteChar(s_nameEntryUITask* pThis)
{
    if (pThis->m34_nameLength < 1)
    {
        playSystemSoundEffect(5);
    }
    else
    {
        s32 newLen = pThis->m34_nameLength - 1;
        pThis->m34_nameLength = newLen;
        pThis->m20_nameBuffer[newLen] = '\0';
        playSystemSoundEffect(1);
    }
}

// 06023292
static void nameEntryUI_renderName(s_nameEntryUITask* pThis)
{
    setupVDP2StringRendering(0xE, 2, 0x10, 2);
    clearVdp2TextArea();
    vdp2DebugPrintSetPosition(0xE, 2);
    drawLineLargeFont(pThis->m20_nameBuffer);
    if (pThis->m34_nameLength < 0xF)
    {
        s16 cursorPos[2];
        cursorPos[0] = (s16)(pThis->m34_nameLength + 0xE) * 8;
        cursorPos[1] = 0x10;
        nameEntryUI_drawCursorSprite(pThis->m10_npcHandle, cursorPos, 0);
    }
}

// 060232fc
static s32 nameEntryUI_getKeyboardMode(s_nameEntryUITask* pThis)
{
    return (s32)(s8)pThis->m1_keyboardMode;
}

// 06023348
static void nameEntryUI_handleCursorMovement(s_nameEntryUITask* pThis)
{
    const s_nameEntryKeyboardLayout* layout;
    if (pThis->m8_inActionBar == 0)
    {
        layout = &nameEntryKeyboardLayouts[(s8)pThis->m1_keyboardMode];
    }
    else
    {
        layout = &nameEntryActionBarLayout;
    }

    if (pThis->m0_cursorMode == 0)
    {
        u16 buttons = graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.mC_newButtonDown2;
        if (buttons & 0x40) // left
        {
            playSystemSoundEffect(10);
            pThis->mA_cursorX--;
        }
        if (buttons & 0x80) // right
        {
            playSystemSoundEffect(10);
            pThis->mA_cursorX++;
        }
        if (buttons & 0x10) // up
        {
            playSystemSoundEffect(10);
            pThis->mC_cursorY--;
        }
        if (buttons & 0x20) // down
        {
            playSystemSoundEffect(10);
            pThis->mC_cursorY++;
        }
    }

    // Wrap cursor X
    while (pThis->mA_cursorX < 0)
    {
        pThis->mA_cursorX += layout->m4_cols;
    }
    while (pThis->mA_cursorX >= layout->m4_cols)
    {
        pThis->mA_cursorX -= layout->m4_cols;
    }

    // Handle max name length -> force action bar
    if (pThis->m34_nameLength > 0xF)
    {
        pThis->mC_cursorY = 0;
    }

    // Handle transition between keyboard grid and action bar
    if (pThis->mC_cursorY < 0 || pThis->mC_cursorY >= layout->m6_rows)
    {
        pThis->m8_inActionBar ^= 1;

        if (pThis->m8_inActionBar == 0)
        {
            // Transitioning from action bar to keyboard
            if (pThis->mA_cursorX < 1)
            {
                pThis->mA_cursorX = 9;
            }
            else if (pThis->mA_cursorX < 2)
            {
                pThis->mA_cursorX = 0xC;
            }
            else
            {
                pThis->mA_cursorX = 0xF;
            }
            layout = &nameEntryKeyboardLayouts[(s8)pThis->m1_keyboardMode];
        }
        else
        {
            // Transitioning from keyboard to action bar
            if (pThis->mA_cursorX < 0xC)
            {
                pThis->mA_cursorX = 0;
            }
            else if (pThis->mA_cursorX < 0xF)
            {
                pThis->mA_cursorX = 1;
            }
            else
            {
                pThis->mA_cursorX = 2;
            }
            layout = &nameEntryActionBarLayout;
        }

        if (pThis->mC_cursorY < 0)
        {
            pThis->mC_cursorY = layout->m6_rows - 1;
        }
        else
        {
            pThis->mC_cursorY = 0;
        }
    }
}

// 0602347a
static void nameEntryUI_handleCharInput(s_nameEntryUITask* pThis)
{
    s32 mode = (s8)pThis->m1_keyboardMode;
    const s_nameEntryKeyboardLayout* layout = &nameEntryKeyboardLayouts[mode];

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 6) // A or B
    {
        if (pThis->m34_nameLength < 0xF)
        {
            char c = layout->m8_chars[pThis->mA_cursorX * layout->mC_charStride + pThis->mC_cursorY];
            pThis->m20_nameBuffer[pThis->m34_nameLength] = c;
            nameEntryUI_addChar(pThis);
        }
        else
        {
            playSystemSoundEffect(5);
        }
    }

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 1) // C
    {
        nameEntryUI_deleteChar(pThis);
    }
}

// 060234f4
static void nameEntryUI_handleActionBarInput(s_nameEntryUITask* pThis)
{
    if (pThis->m0_cursorMode != 0)
    {
        return;
    }

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 1) // C = backspace
    {
        nameEntryUI_deleteChar(pThis);
        return;
    }

    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 6) // A or B
    {
        s16 action = pThis->mA_cursorX;
        if (action == 0)
        {
            // Back to keyboard (delete last char)
            nameEntryUI_deleteChar(pThis);
        }
        else if (action == 1)
        {
            // Insert space
            if (pThis->m34_nameLength < 0xF)
            {
                pThis->m20_nameBuffer[pThis->m34_nameLength] = ' ';
                nameEntryUI_addChar(pThis);
            }
            else
            {
                playSystemSoundEffect(5);
            }
        }
        else if (action == 2)
        {
            // Confirm name
            if (pThis->m34_nameLength < 2)
            {
                playSystemSoundEffect(5);
            }
            else
            {
                if (pThis->m38_destBuffer != nullptr)
                {
                    memcpy(pThis->m38_destBuffer, pThis->m20_nameBuffer, pThis->m34_nameLength + 1);
                }
                pThis->m0_cursorMode = 1; // signal completion
                playSystemSoundEffect(0);
            }
        }
    }
}

// 06023586
void s_nameEntryUITask::Draw(s_nameEntryUITask* pThis)
{
    s32 scrollX = 0;
    s32 scrollY = 0;

    switch (pThis->m4_state)
    {
    case 0: // init
        pThis->m4_state = 1;
        break;

    case 1: // main input state
    {
        s32 mode = (s8)pThis->m1_keyboardMode;
        scrollX = (s32)nameEntryScrollPositions[mode][0] << 16;
        scrollY = (s32)nameEntryScrollPositions[mode][1] << 16;

        nameEntryUI_handleCursorMovement(pThis);

        if (pThis->m8_inActionBar == 0)
        {
            nameEntryUI_handleCharInput(pThis);
        }
        else
        {
            nameEntryUI_handleActionBarInput(pThis);
        }

        if (pThis->m8_inActionBar != 0)
        {
            // In action bar: render cursor at action bar position
            s16 pos[2];
            pos[0] = pThis->mA_cursorX * 0x30 + 0xB7;
            pos[1] = pThis->mC_cursorY * 0x30 + 0x6B;
            nameEntryUI_drawCursorSprite(pThis->m10_npcHandle, pos, 0);
        }
        else
        {
            // In keyboard grid: render cursor
            s32 layoutOffset = (s8)pThis->m1_keyboardMode * 12;
            s16 pos[2];
            pos[0] = pThis->mA_cursorX * 0x10 + nameEntryKeyboardLayouts[(s8)pThis->m1_keyboardMode].m0_xStart;
            pos[1] = nameEntryKeyboardLayouts[(s8)pThis->m1_keyboardMode].m2_yStart + pThis->mC_cursorY * 0x10;
            nameEntryUI_drawCursorSprite(pThis->m10_npcHandle, pos, 1);
        }

        nameEntryUI_getKeyboardMode(pThis);
        nameEntryUI_renderName(pThis);
        break;
    }

    case 2: // transition animation
    {
        fixedPoint t = FP_Div(pThis->m14_animCounter, 8);
        scrollX = ((s32)pThis->m1C_endX - (s32)pThis->m18_startX) * (s32)t + (s32)pThis->m18_startX * 0x10000;
        scrollY = ((s32)pThis->m1E_endY - (s32)pThis->m1A_startY) * (s32)t + (s32)pThis->m1A_startY * 0x10000;
        pThis->m14_animCounter++;
        if (pThis->m14_animCounter > 7)
        {
            pThis->m4_state = 1;
        }
        setupVDP2StringRendering(0xE, 2, 0x10, 2);
        clearVdp2TextArea();
        break;
    }

    default:
        assert(0);
        break;
    }

    pauseEngine[4] = 0;
    setupVDP2CoordinatesIncrement2(scrollX, scrollY);
    pauseEngine[4] = 4;
}

// 060236be
void s_nameEntryUITask::Delete(s_nameEntryUITask* pThis)
{
    // npcFileDeleter cleanup is handled by the task system when this task is destroyed
    setupVDP2StringRendering(0xE, 2, 0x10, 2);
    clearVdp2TextArea();
    nameEntryUI_restoreGraphicsState(&pThis->m40_savedGraphicsState);
    if (pThis->m3C_menuAllowedFlag != 0)
    {
        graphicEngineStatus.m40AC.m1_isMenuAllowed = 1;
    }
}

// NBG0 config for name entry keyboard (from PDS_Base at 0x002106f4)
static const sLayerConfig nameEntryNBG0Config[] = {
    { m2_CHCN, 0 },    // 16 colors
    { m5_CHSZ, 1 },    // 2x2 cell (16x16)
    { m6_PNB, 1 },     // 1 word pattern
    { m7_CNSM, 0 },    // 10-bit char number, flip
    { m12_PLSZ, 1 },   // plane 2H x 1V
    { m40_CAOS, 4 },   // palette offset 4 * 0x200 = 0x800
    { m0_END, 0 },
};

// Compressed keyboard tile graphics from PDS_Base at 0x002100d8 (LZ-like format, terminates on 0x0000)
static u8 nameEntryTileData[] = {
    0x7B, 0x06, 0x00, 0xF0, 0xFF, 0x3F, 0x02, 0x01, 0x02, 0xF8, 0xFF, 0x03, 0x02, 0x04, 0x02, 0x05,
    0xFB, 0x02, 0x06, 0x20, 0xFF, 0x10, 0x07, 0x02, 0x08, 0x02, 0x7F, 0x09, 0x02, 0x0A, 0x02, 0x0B,
    0x02, 0x0C, 0xFE, 0xF0, 0x0D, 0x02, 0x0E, 0x02, 0x0F, 0xBF, 0x02, 0x10, 0x02, 0x11, 0x02, 0x12,
    0xE7, 0xFE, 0xFF, 0x13, 0x02, 0x14, 0x02, 0x15, 0x02, 0x16, 0x02, 0xFF, 0x17, 0x02, 0x18, 0x02,
    0x19, 0x02, 0x1A, 0x02, 0xFF, 0x1B, 0x02, 0x1C, 0x02, 0x1D, 0x02, 0x1E, 0x02, 0xFF, 0x1F, 0x02,
    0x20, 0x02, 0x21, 0x02, 0x22, 0x02, 0x7F, 0x23, 0x02, 0x24, 0x02, 0x25, 0x02, 0x26, 0xFE, 0xD4,
    0x27, 0x02, 0x28, 0x02, 0x29, 0xFF, 0x02, 0x2A, 0x02, 0x2B, 0x02, 0x2C, 0x02, 0x2D, 0x8F, 0x02,
    0x2E, 0x02, 0x2F, 0xEC, 0xFF, 0x30, 0x02, 0x31, 0x02, 0x32, 0x02, 0x33, 0x02, 0xFF, 0x34, 0x02,
    0x35, 0x02, 0x36, 0x02, 0x37, 0x02, 0xFF, 0x38, 0x02, 0x39, 0x02, 0x3A, 0x02, 0x3B, 0x02, 0xFF,
    0x3C, 0x02, 0x3D, 0x02, 0x3E, 0x02, 0x3F, 0x02, 0x7F, 0x40, 0x02, 0x41, 0x02, 0x42, 0x02, 0x43,
    0xFE, 0xC0, 0x44, 0x02, 0x45, 0x02, 0x46, 0xFF, 0x02, 0x47, 0x02, 0x48, 0x02, 0x49, 0x02, 0x4A,
    0x0F, 0x02, 0x4B, 0x02, 0x4C, 0xEC, 0xE1, 0x0A, 0xC0, 0x4D, 0x02, 0x4E, 0xFF, 0x02, 0x4F, 0x02,
    0x50, 0x02, 0x51, 0x02, 0x52, 0xFF, 0x02, 0x53, 0x02, 0x54, 0x02, 0x55, 0x02, 0x56, 0xFF, 0x02,
    0x57, 0x02, 0x58, 0x02, 0x59, 0x02, 0x5A, 0xFF, 0x02, 0x5B, 0x02, 0x5C, 0x02, 0x5D, 0x02, 0x5E,
    0xFD, 0x0A, 0x04, 0xFE, 0x5F, 0x02, 0x60, 0x02, 0x61, 0xFF, 0x02, 0x62, 0x02, 0x63, 0x02, 0x64,
    0x02, 0x65, 0xCF, 0x02, 0x66, 0x02, 0x67, 0xC0, 0xFF, 0x68, 0x02, 0x69, 0x02, 0x6A, 0x02, 0x6B,
    // 0x100
    0x02, 0xFF, 0x6C, 0x02, 0x6D, 0x02, 0x6E, 0x02, 0x6F, 0x02, 0xFF, 0x70, 0x02, 0x71, 0x02, 0x72,
    0x02, 0x73, 0x02, 0xFF, 0x74, 0x02, 0x75, 0x02, 0x76, 0x02, 0x77, 0x02, 0xD1, 0x78, 0xDA, 0x04,
    0xFE, 0x79, 0xFF, 0x02, 0x7A, 0x02, 0x7B, 0x02, 0x7C, 0x02, 0x7D, 0xFF, 0x02, 0x7E, 0x02, 0x7F,
    0x02, 0x80, 0x02, 0x81, 0xFC, 0xC0, 0x82, 0x02, 0x83, 0x02, 0xFF, 0x84, 0x02, 0x85, 0x02, 0x86,
    0x02, 0x87, 0x02, 0xFF, 0x88, 0x02, 0x89, 0x02, 0x8A, 0x02, 0x8B, 0x02, 0xFF, 0x8C, 0x02, 0x8D,
    0x02, 0x8E, 0x02, 0x8F, 0x02, 0xDF, 0x90, 0x02, 0x91, 0x02, 0x92, 0x07, 0xFE, 0x93, 0xFF, 0x02,
    0x94, 0x02, 0x95, 0x02, 0x96, 0x02, 0x97, 0xFF, 0x02, 0x98, 0x02, 0x99, 0x02, 0x9A, 0x02, 0x9B,
    0xF8, 0xEC, 0x9C, 0x02, 0x9D, 0x02, 0xFF, 0x9E, 0x02, 0x9F, 0x02, 0xA0, 0x02, 0xA1, 0x02, 0xFF,
    0xA2, 0x02, 0xA3, 0x02, 0xA4, 0x02, 0xA5, 0x02, 0xFF, 0xA6, 0x02, 0xA7, 0x02, 0xA8, 0x02, 0xA9,
    0x02, 0xFF, 0xAA, 0x02, 0xAB, 0x02, 0xAC, 0x02, 0xAD, 0x02, 0xE7, 0xAE, 0x02, 0xAF, 0xC0, 0xB0,
    0xFF, 0x02, 0xB1, 0x02, 0xB2, 0x02, 0xB3, 0x02, 0xB4, 0xFF, 0x02, 0xB5, 0x02, 0xB6, 0x02, 0xB7,
    0x02, 0xB8, 0xFE, 0x30, 0xF1, 0x1C, 0xB9, 0x02, 0xBA, 0x02, 0xBB, 0x02, 0x7F, 0xBC, 0x02, 0xBD,
    0x02, 0xBE, 0x02, 0xBF, 0x55, 0xB0, 0xEE, 0x41, 0xF0, 0xFF, 0xFF, 0xF0, 0xFF, 0x55, 0x00, 0xE4,
    0x2B, 0xFF, 0x00, 0xFE, 0x14, 0x13, 0x02, 0xC0, 0x02, 0xC1, 0x02, 0xC2, 0xFF, 0x02, 0xC3, 0x02,
    0xC4, 0x02, 0xC5, 0x02, 0xC6, 0xFF, 0x02, 0xC7, 0x02, 0xC8, 0x02, 0xC9, 0x02, 0xCA, 0xFF, 0x02,
    0xCB, 0x02, 0xCC, 0x02, 0xCD, 0x02, 0xCE, 0xFF, 0x02, 0xCF, 0x02, 0xD0, 0x02, 0xD1, 0x02, 0xD2,
    // 0x200
    0x8E, 0x00, 0xFE, 0x17, 0x0A, 0x30, 0xBC, 0xFF, 0xD3, 0x02, 0xD4, 0x02, 0xD5, 0x02, 0xD6, 0x02,
    0x1F, 0xD7, 0x02, 0xD8, 0x02, 0xD9, 0xFF, 0xF0, 0xDA, 0x02, 0xDB, 0x02, 0xDC, 0x02, 0xDD, 0xFF,
    0x02, 0xDE, 0x02, 0xDF, 0x02, 0xE0, 0x02, 0xE1, 0xD8, 0xEE, 0xE2, 0x00, 0xFE, 0x1C, 0xE3, 0xFF,
    0x02, 0xE4, 0x02, 0xE5, 0x02, 0xE6, 0x02, 0xE7, 0x8F, 0x02, 0xE8, 0x02, 0xE9, 0xF0, 0xFF, 0xEA,
    0x02, 0xEB, 0x02, 0xEC, 0x02, 0xED, 0x02, 0x7F, 0xEE, 0x02, 0xEF, 0x02, 0xF0, 0x02, 0xF1, 0xF4,
    0xDA, 0x00, 0xFE, 0x1D, 0xF2, 0x02, 0xF3, 0xFF, 0x02, 0xF4, 0x02, 0xF5, 0x02, 0xF6, 0x02, 0xF7,
    0xE3, 0x02, 0xF8, 0xF0, 0xF9, 0x02, 0xFF, 0xFA, 0x02, 0xFB, 0x02, 0xFC, 0x02, 0xFD, 0x02, 0x67,
    0xFE, 0x02, 0xFF, 0xDA, 0xFF, 0x00, 0xFE, 0x18, 0x02, 0x9C, 0x03, 0x00, 0x03, 0x01, 0x03, 0xC3,
    0x02, 0x03, 0xFF, 0x04, 0x03, 0xFF, 0x05, 0x03, 0x06, 0x03, 0x07, 0x03, 0x08, 0x03, 0xFF, 0x09,
    0x03, 0x0A, 0x03, 0x0B, 0x03, 0x0C, 0x03, 0xFF, 0x0D, 0x03, 0x0E, 0x03, 0x0F, 0x03, 0x10, 0x03,
    0xF7, 0x11, 0x03, 0x12, 0x60, 0xF3, 0x2B, 0x03, 0x13, 0x03, 0xFF, 0x14, 0x03, 0x15, 0x03, 0x16,
    0x03, 0x17, 0x03, 0xFF, 0x18, 0x03, 0x19, 0x03, 0x1A, 0x03, 0x1B, 0x03, 0xD5, 0x1C, 0xC0, 0xEA,
    0xFF, 0xF0, 0xFF, 0xFF, 0xF0, 0xFF, 0xDD, 0x03, 0xFF, 0x1D, 0x03, 0x1E, 0x03, 0x1F, 0x03, 0x20,
    0x03, 0xF7, 0x21, 0x03, 0x22, 0x50, 0xDB, 0x2B, 0x03, 0x23, 0x03, 0xFF, 0x24, 0x03, 0x25, 0x03,
    0x26, 0x03, 0x27, 0x03, 0xFF, 0x28, 0x03, 0x29, 0x03, 0x2A, 0x03, 0x2B, 0x03, 0xFD, 0x2C, 0x56,
    0xDB, 0x03, 0x2D, 0x03, 0x2E, 0x03, 0xFF, 0x2F, 0x03, 0x30, 0x03, 0x31, 0x03, 0x32, 0x03, 0xFF,
    // 0x300
    0x33, 0x02, 0xC8, 0x03, 0x34, 0x03, 0x35, 0x03, 0xFF, 0x36, 0x03, 0x37, 0x03, 0x38, 0x03, 0x39,
    0x03, 0x17, 0x3A, 0x03, 0x3B, 0x51, 0xDB, 0xFE, 0xC8, 0x3C, 0x03, 0x3D, 0x03, 0x3E, 0x03, 0x3F,
    0xFF, 0x03, 0x40, 0x03, 0x41, 0x03, 0x42, 0x03, 0x43, 0xEF, 0x03, 0x44, 0x03, 0x45, 0x54, 0xDF,
    0x03, 0x46, 0xFF, 0x03, 0x47, 0x03, 0x48, 0x03, 0x49, 0x03, 0x4A, 0xFF, 0x03, 0x4B, 0x03, 0x4C,
    0x03, 0x4D, 0x03, 0x4E, 0xFF, 0x03, 0x4F, 0x03, 0x50, 0x03, 0x51, 0x03, 0x52, 0xFF, 0x03, 0x53,
    0x03, 0x54, 0x03, 0x55, 0x03, 0x56, 0x87, 0x03, 0x57, 0x0A, 0xD0, 0x58, 0xFF, 0x03, 0x59, 0x03,
    0x5A, 0x03, 0x5B, 0x03, 0x5C, 0xFF, 0x03, 0x5D, 0x03, 0x5E, 0x03, 0x5F, 0x03, 0x60, 0xFB, 0x03,
    0x61, 0x05, 0xFE, 0x62, 0x03, 0x63, 0x03, 0xFF, 0x64, 0x03, 0x65, 0x03, 0x66, 0x03, 0x67, 0x03,
    0x7F, 0x68, 0x03, 0x69, 0x03, 0x6A, 0x03, 0x6B, 0xE1, 0x20, 0xDC, 0x10, 0xC0, 0x6C, 0x03, 0x6D,
    0xFF, 0x03, 0x6E, 0x03, 0x6F, 0x03, 0x70, 0x03, 0x71, 0xFF, 0x03, 0x72, 0x03, 0x73, 0x03, 0x74,
    0x03, 0x75, 0x8A, 0x56, 0xDB, 0xF0, 0xFF, 0x21, 0xC0, 0xFF, 0x76, 0x03, 0x77, 0x03, 0x78, 0x03,
    0x79, 0x03, 0xFF, 0x7A, 0x03, 0x7B, 0x03, 0x7C, 0x03, 0x7D, 0x03, 0xF7, 0x7E, 0x03, 0x7F, 0x00,
    0xFE, 0x2C, 0x80, 0x03, 0x81, 0xFF, 0x03, 0x82, 0x03, 0x83, 0x03, 0x84, 0x03, 0x85, 0xFF, 0x03,
    0x86, 0x03, 0x87, 0x03, 0x88, 0x03, 0x89, 0x0E, 0x54, 0xD9, 0x07, 0x00, 0xFE, 0xFF, 0x03, 0x8A,
    0x03, 0x8B, 0x03, 0x8C, 0x03, 0x8D, 0xFE, 0x53, 0xD9, 0x8E, 0x03, 0x8F, 0x03, 0x90, 0x02, 0xFF,
    0xA8, 0x03, 0x91, 0x03, 0x92, 0x03, 0x93, 0x03, 0xD7, 0x94, 0x03, 0x95, 0x56, 0xBF, 0x11, 0xF2,
    // 0x400
    0x96, 0xFF, 0x03, 0x97, 0x03, 0x98, 0x03, 0x99, 0x03, 0x9A, 0xAF, 0x03, 0x9B, 0x03, 0x9C, 0x80,
    0xCC, 0x1E, 0x50, 0xBF, 0xFF, 0xAA, 0xF0, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0,
    0xFF, 0xFF, 0x0A, 0xF0, 0xFF, 0xCC, 0x00, 0x00,
};

// Keyboard palette from PDS_Base at 0x00210728 (64 bytes, Saturn RGB555 format)
static u8 nameEntryPaletteData[] = {
    0x80, 0x00, 0xF7, 0xBD, 0xFF, 0xFF, 0xDE, 0xF7, 0xCE, 0x73, 0xFF, 0xFF, 0xC2, 0x10, 0xB9, 0xCE,
    0xFF, 0xFF, 0xB1, 0x8C, 0xA9, 0x4A, 0xA1, 0x08, 0x98, 0xC6, 0x94, 0xA5, 0x90, 0x84, 0x8C, 0x63,
    0x80, 0x00, 0xC7, 0xE0, 0xAE, 0x60, 0x99, 0x80, 0xFF, 0xFF, 0x80, 0xC8, 0x81, 0x0A, 0x85, 0x4C,
    0x85, 0x8E, 0x89, 0xD1, 0x8A, 0x13, 0x8E, 0x75, 0x8E, 0xB8, 0x92, 0xFA, 0x93, 0x3C, 0x97, 0x9F,
};

// 06023728
static s_nameEntryUITask* nameEntry(p_workArea parent, char* destBuffer)
{
    s_nameEntryUITask* pTask = createSiblingTask<s_nameEntryUITask>(parent);
    pTask->m38_destBuffer = destBuffer;
    if (graphicEngineStatus.m40AC.m1_isMenuAllowed != 0)
    {
        graphicEngineStatus.m40AC.m1_isMenuAllowed = 0;
        pTask->m3C_menuAllowedFlag = 1;
    }
    nameEntryUI_saveGraphicsState(&pTask->m40_savedGraphicsState);
    // Load cursor sprite NPC file (FUN_06031f5c with params 0, "NAME_ENT.CGB")
    // param 0 = no RAM file, "NAME_ENT.CGB" = VDP1 sprite data
    s32 cgbFileSize = getFileSize("NAME_ENT.CGB");
    pTask->m10_npcHandle = loadNPCFile(pTask, "", 0, "NAME_ENT.CGB", cgbFileSize, -1);
    setupNBG0(nameEntryNBG0Config);
    initLayerMap(0, 0x25E1E000, 0x25E1E800, 0x25E1E000, 0x25E1E800);
    unpackGraphicsToVDP2(nameEntryTileData, getVdp2Vram(0x1E000));
    asyncDmaCopy(nameEntryPaletteData, getVdp2Cram(0x800), 0x40, 0);
    loadFile("NAME_ENT.SCB", getVdp2Vram(0x10000), 0);
    return pTask;
}

// 06054ddc
static void nameEntryDelete(s_nameEntryUITask* pNameEntryUI)
{
    setupVDP2StringRendering(0xe, 2, 0x10, 2);
    clearVdp2TextArea();
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON &= 0xFFFE;
    vdp2Controls.m_isDirty = 1;
    if (pNameEntryUI)
    {
        pNameEntryUI->getTask()->markPaused();
    }
}

// 06054e3a
void s_nameEntryTask::nameEntryInit(s_nameEntryTask* pThis)
{
    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();

    // Pause the dragon task
    pFieldTask->m8_pSubFieldData->m338_pDragonTask->getTask()->markPaused();

    // Pause the LCS task
    pFieldTask->m8_pSubFieldData->m340_pLCS->getTask()->markPaused();

    // Clear update functions
    pFieldTask->m8_pSubFieldData->pUpdateFunction2 = nullptr;
    pFieldTask->m8_pSubFieldData->m374_pUpdateFunction1 = nullptr;

    reinitVdp2();
    initNBG1Layer();
    preloadNameEntryResourcesInternal();

    pThis->m4_nameEntryTask = nameEntry(pThis, mainGameState.gameStats.m94_playerName);
    pThis->m8_particleTask = createParticleSystemTask(pThis);
    nameEntryDelete(pThis->m4_nameEntryTask);
}

static std::vector<sVdp1Quad> s_fadeOutSpriteData;

struct s_nameEntryFadeOutTask : public s_workAreaTemplateWithArg<s_nameEntryFadeOutTask, s32>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, &Update, &Draw, nullptr };
        return &taskDefinition;
    }

    static void Init(s_nameEntryFadeOutTask* pThis, s32 exitArg);
    static void Update(s_nameEntryFadeOutTask* pThis);
    static void Draw(s_nameEntryFadeOutTask* pThis);

    s_fileBundle* m0_bundle;     // 0x00
    u32 m4_characterArea;        // 0x04
    sVec3_FP m8_position;        // 0x08
    s32 m14_pad;                 // 0x14
    s32 m18_yVelocity;           // 0x18
    s32 m1C_pad;                 // 0x1C
    fixedPoint m20_scale;        // 0x20
    s32 m24_scaleSpeed;          // 0x24
    sAnimatedQuad m28_quad;      // 0x28
    s32 m30_timer;               // 0x30
    s32 m34_fadeState;           // 0x34
    s32 m38_exitArg;             // 0x38
    // size 0x3C
};

// 06054C3E
void s_nameEntryFadeOutTask::Init(s_nameEntryFadeOutTask* pThis, s32 exitArg)
{
    s_memoryAreaOutput memArea;
    getMemoryArea(&memArea, 2);
    pThis->m0_bundle = memArea.m0_mainMemoryBundle;
    pThis->m4_characterArea = memArea.m4_characterArea;

    pThis->m8_position.m0_X = 0;
    pThis->m8_position.m4_Y = 0;
    pThis->m8_position.m8_Z = fixedPoint(0xFFFE0000);

    pThis->m20_scale = fixedPoint(0x8000);
    pThis->m24_scaleSpeed = 0xB6;
    pThis->m18_yVelocity = 0xFFFFFE67;

    // Load sprite data from overlay (06078524)
    if (s_fadeOutSpriteData.empty())
    {
        s_fadeOutSpriteData = initVdp1Quad(gFLD_D5->getSaturnPtr(0x06078524));
    }

    u16 vdp1Memory = 0;
    s_fieldTaskWorkArea* pFieldTask = getFieldTaskPtr();
    if (pFieldTask->m8_pSubFieldData->mC_characterArea[2])
    {
        vdp1Memory = (u16)(pFieldTask->m8_pSubFieldData->mC_characterArea[2] >> 3);
    }
    particleInitSub(&pThis->m28_quad, vdp1Memory, &s_fadeOutSpriteData);

    pThis->m30_timer = 0x10E;

    vdp2Controls.m4_pendingVdp2Regs->m20_BGON &= ~0x10;
    vdp2Controls.m_isDirty = 1;

    pThis->m38_exitArg = exitArg;
}

// 06054CF4
void s_nameEntryFadeOutTask::Update(s_nameEntryFadeOutTask* pThis)
{
    sGunShotTask_UpdateSub4(&pThis->m28_quad);
    pThis->m30_timer--;

    fixedPoint newScale = (s32)pThis->m20_scale + pThis->m24_scaleSpeed;
    pThis->m20_scale = newScale;
    if ((s32)newScale > 0x10000)
    {
        pThis->m20_scale = fixedPoint(0x10000);
    }

    if (pThis->m34_fadeState == 0)
    {
        if (pThis->m30_timer < 0xE2)
        {
            if ((s8)g_fadeControls.m_4C <= (s8)g_fadeControls.m_4D)
            {
                vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
                vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
            }
            u32 color = convertColorToU32ForFade(*(const sVec3_FP*)&g_fadeControls);
            fadePalette(&g_fadeControls.m0_fade0, color, 0xC210, 0xF0);
            pThis->m34_fadeState++;
        }
    }
    else if (pThis->m34_fadeState == 1)
    {
        pThis->m8_position.m4_Y += fixedPoint(pThis->m18_yVelocity);
        if (pThis->m30_timer < 1)
        {
            fieldA3_1_checkExitsTaskUpdate2Sub1(pThis->m38_exitArg);
        }
    }
}

// 06054D84
void s_nameEntryFadeOutTask::Draw(s_nameEntryFadeOutTask* pThis)
{
    vdp1DrawQuadScaled(&pThis->m28_quad, &pThis->m8_position, pThis->m20_scale);
}

// 06054d96
void createNameEntryFadeOutTask(p_workArea parent, s32 arg)
{
    createSiblingTaskWithArg<s_nameEntryFadeOutTask>(parent, arg);
}

// 06054ea6
static void nameEntryConfirmCallback(s_nameEntryTask* pNameEntryTask)
{
    pNameEntryTask->m0_state = 5;
    if (pNameEntryTask->m8_particleTask)
    {
        pNameEntryTask->m8_particleTask->m28_state = 2;
    }
}

// 06054eba
static void nameEntryRetryCallback(s_nameEntryTask* pNameEntryTask)
{
    pNameEntryTask->m0_state = 4;
    s_nameEntryUITask* pNameEntryUI = pNameEntryTask->m4_nameEntryTask;
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON |= 1;
    vdp2Controls.m_isDirty = 1;
    pNameEntryUI->m0_cursorMode = 0;
    pNameEntryUI->getTask()->clearPaused();
}

struct s_nameConfirmDialogTask : public s_workAreaTemplate<s_nameConfirmDialogTask>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static const TypedTaskDefinition taskDefinition = { &Init, NULL, &Draw, &Delete };
        return &taskDefinition;
    }

    static void Init(s_nameConfirmDialogTask*);
    static void Draw(s_nameConfirmDialogTask*);
    static void Delete(s_nameConfirmDialogTask*);

    typedef void (*confirmCallback)(s_nameEntryTask*);

    confirmCallback m0_confirmFunc;
    confirmCallback m4_retryFunc;
    u32 m8;
    u32 mC_state;
    s_nameEntryTask* m10_pNameEntryTask;
    // size 0x14
};

// 06068196
void s_nameConfirmDialogTask::Init(s_nameConfirmDialogTask*)
{
    // empty
}

// 0606819a
void s_nameConfirmDialogTask::Draw(s_nameConfirmDialogTask* pThis)
{
    s_fieldScriptWorkArea* pScriptTask = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    getFieldTaskPtr()->m28_status |= 0x40;

    switch (pThis->mC_state)
    {
    case 0:
        if (pScriptTask->m3C_multichoiceTask != nullptr)
        {
            pThis->mC_state++;
        }
        break;
    case 1:
        if (pScriptTask->m3C_multichoiceTask == nullptr)
        {
            pThis->getTask()->markFinished();
        }
        break;
    default:
        assert(0);
        break;
    }
}

// 06068210
void s_nameConfirmDialogTask::Delete(s_nameConfirmDialogTask* pThis)
{
    s_FieldSubTaskWorkArea* pSubFieldData = getFieldTaskPtr()->m8_pSubFieldData;
    if (pSubFieldData != nullptr)
    {
        s_fieldScriptWorkArea* pScriptTask = pSubFieldData->m34C_ptrToE;
        getFieldTaskPtr()->m28_status &= ~0x40;
        s32 result = pScriptTask->m54_currentResult;
        if (result >= 0)
        {
            confirmCallback callbacks[2] = { pThis->m0_confirmFunc, pThis->m4_retryFunc };
            if (callbacks[result] != nullptr)
            {
                callbacks[result](pThis->m10_pNameEntryTask);
            }
        }
    }
}

// 06068270
static p_workArea createNameConfirmDialog(s_nameConfirmDialogTask::confirmCallback confirmFunc, s_nameConfirmDialogTask::confirmCallback retryFunc, s_nameEntryTask* pNameEntryTask)
{
    s_fieldScriptWorkArea* pScriptTask = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    s_nameConfirmDialogTask* pDialog = createSubTask<s_nameConfirmDialogTask>(pScriptTask);
    if (pDialog != nullptr)
    {
        pDialog->m0_confirmFunc = confirmFunc;
        pDialog->m4_retryFunc = retryFunc;
        pDialog->m10_pNameEntryTask = pNameEntryTask;
    }
    return pDialog;
}

// 06054e18
static void nameEntryEnable(s_nameEntryUITask* pNameEntryUI)
{
    vdp2Controls.m4_pendingVdp2Regs->m20_BGON |= 1;
    vdp2Controls.m_isDirty = 1;
    if (pNameEntryUI)
    {
        pNameEntryUI->m0_cursorMode = 0;
        pNameEntryUI->getTask()->clearPaused();
    }
}

// 06054f00
void s_nameEntryTask::nameEntryDraw(s_nameEntryTask* pThis)
{
    s_fieldScriptWorkArea* pScriptTask = getFieldTaskPtr()->m8_pSubFieldData->m34C_ptrToE;
    s_dragonTaskWorkArea* pDragonTask = getFieldTaskPtr()->m8_pSubFieldData->m338_pDragonTask;

    pDragonTask->mF8_Flags |= 0x20000;

    switch (pThis->m0_state)
    {
    case 0:
        if (startFieldScript(0, -1))
        {
            pThis->m0_state++;
        }
        break;
    case 1:
        if (pScriptTask->m4_currentScript.m_offset == 0)
        {
            nameEntryEnable(pThis->m4_nameEntryTask);
            pThis->m0_state++;
        }
        break;
    case 2:
        if (pThis->m4_nameEntryTask)
        {
            // Wait for nameEntry task to signal completion (m0_cursorMode != 0)
            if (pThis->m4_nameEntryTask->m0_cursorMode == 0)
            {
                break;
            }
        }
        nameEntryDelete(pThis->m4_nameEntryTask);
        setGlobalStringTableEntry(0, mainGameState.gameStats.m94_playerName);
        pThis->m0_state++;
        // fall through to state 3
        [[fallthrough]];
    case 3:
        if (startFieldScript(1, -1))
        {
            createNameConfirmDialog(&nameEntryConfirmCallback, &nameEntryRetryCallback, pThis);
        }
        break;
    case 4:
        if (pScriptTask->m3C_multichoiceTask != nullptr)
        {
            break;
        }
        nameEntryEnable(pThis->m4_nameEntryTask);
        pThis->m0_state = 2;
        break;
    case 5:
        if (startFieldScript(2, -1))
        {
            pThis->m0_state++;
        }
        break;
    case 6:
        if ((s8)g_fadeControls.m_4C <= (s8)g_fadeControls.m_4D)
        {
            vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
            vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
        }
        {
            u32 color = convertColorToU32ForFade(*(const sVec3_FP*)&g_fadeControls);
            fadePalette(&g_fadeControls.m0_fade0, color, 0xffff, 0x78);
        }
        pThis->m0_state++;
        break;
    case 7:
        if (g_fadeControls.m0_fade0.m20_stopped == 0)
        {
            break;
        }
        createNameEntryFadeOutTask(pThis, -1);
        pThis->getTask()->markFinished();
        break;
    default:
        assert(0);
        break;
    }
}

// 06055060
void createNameEntryTask(p_workArea parent)
{
    createSubTask<s_nameEntryTask>(parent);
}
