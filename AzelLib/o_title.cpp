#include "PDS.h"
#include "audio/soundDriver.h"

namespace TITLE_OVERLAY {

struct s_titleOverlayWorkArea : public s_workAreaTemplate<s_titleOverlayWorkArea>
{
    static const TypedTaskDefinition* getTypedTaskDefinition()
    {
        static constexpr TypedTaskDefinition taskDefinition = { NULL, NULL, &titleOverlay_Update, NULL };
        return &taskDefinition;
    }

    static void titleOverlay_Update(s_titleOverlayWorkArea*);

    u32 m_status;
    u32 m_4;
};

u8 titleScreenPalette[512] =
{
    0xFF, 0xFF, 0x7F, 0xFF, 0x7B, 0xFF, 0x77, 0xFF, 0x73, 0xFF, 0x6F, 0xFF, 0x73, 0xDF, 0x6B, 0xFF,
    0x6F, 0xFE, 0x6B, 0xDF, 0x67, 0xBF, 0x6B, 0xBE, 0x77, 0x5E, 0x63, 0xDE, 0x67, 0xBE, 0x57, 0xFF,
    0x67, 0x9E, 0x7F, 0x1B, 0x53, 0xDF, 0x63, 0x9D, 0x63, 0x7C, 0x5F, 0x5E, 0x6B, 0x5B, 0x5F, 0x7C,
    0x47, 0xBF, 0x5F, 0x7B, 0x7A, 0xBA, 0x4B, 0x7F, 0x5F, 0x5B, 0x5B, 0x5B, 0x5F, 0x3B, 0x76, 0xB8,
    0x4B, 0x1F, 0x5B, 0x3A, 0x5F, 0x39, 0x3F, 0x5F, 0x43, 0x3F, 0x57, 0x3A, 0x37, 0x5F, 0x57, 0x19,
    0x5E, 0xBA, 0x76, 0x76, 0x4F, 0x1A, 0x56, 0xF8, 0x3A, 0xFE, 0x52, 0xF8, 0x3A, 0xDE, 0x46, 0xBC,
    0x52, 0xB9, 0x5E, 0x97, 0x32, 0xFE, 0x4E, 0xD7, 0x3A, 0x9D, 0x4E, 0xB7, 0x52, 0xB6, 0x62, 0x55,
    0x2E, 0xBE, 0x4E, 0xB6, 0x4A, 0xB6, 0x6A, 0x13, 0x3A, 0xB9, 0x4A, 0xB5, 0x4A, 0x96, 0x46, 0x78,
    0x2E, 0x7D, 0x4A, 0x95, 0x4E, 0x75, 0x46, 0x95, 0x2E, 0x9A, 0x46, 0x94, 0x46, 0x75, 0x4A, 0x74,
    0x5A, 0x13, 0x36, 0x59, 0x46, 0x74, 0x42, 0x74, 0x46, 0x73, 0x42, 0x73, 0x42, 0x54, 0x46, 0x53,
    0x42, 0x53, 0x3E, 0x53, 0x42, 0x52, 0x59, 0xB1, 0x26, 0x39, 0x3E, 0x52, 0x42, 0x32, 0x3E, 0x32,
    0x42, 0x31, 0x3D, 0xF4, 0x3A, 0x32, 0x3E, 0x31, 0x45, 0xF1, 0x4D, 0xB1, 0x29, 0xF7, 0x3E, 0x11,
    0x41, 0xF1, 0x3E, 0x10, 0x3A, 0x10, 0x3D, 0xF0, 0x31, 0xF2, 0x3A, 0x0F, 0x39, 0xF0, 0x3D, 0xEF,
    0x39, 0xEF, 0x3D, 0xCF, 0x35, 0xEF, 0x39, 0xEE, 0x39, 0xCF, 0x35, 0xCF, 0x39, 0xCE, 0x2D, 0xD0,
    0x35, 0xCE, 0x35, 0xAE, 0x39, 0x8E, 0x31, 0xCD, 0x31, 0xAE, 0x35, 0xAD, 0x31, 0xAD, 0x2D, 0xAD,
    0x31, 0xAC, 0x31, 0x8D, 0x2D, 0x8D, 0x31, 0x8C, 0x25, 0x8E, 0x2D, 0x8C, 0x35, 0x4C, 0x29, 0x8C,
    0x2D, 0x8B, 0x2D, 0x6C, 0x29, 0x6C, 0x29, 0x6B, 0x29, 0x4C, 0x2D, 0x4B, 0x25, 0x6B, 0x29, 0x4B,
    0x25, 0x4B, 0x29, 0x4A, 0x29, 0x2B, 0x25, 0x4A, 0x25, 0x2B, 0x25, 0x49, 0x25, 0x2A, 0x21, 0x49,
    0x21, 0x2A, 0x25, 0x29, 0x25, 0x0A, 0x19, 0x2B, 0x21, 0x29, 0x25, 0x09, 0x21, 0x28, 0x21, 0x09,
    0x25, 0x08, 0x1D, 0x28, 0x1D, 0x09, 0x21, 0x08, 0x1D, 0x08, 0x21, 0x07, 0x1C, 0xE9, 0x20, 0xE8,
    0x19, 0x08, 0x1D, 0x07, 0x1C, 0xE8, 0x18, 0xE8, 0x1C, 0xE7, 0x1C, 0xC8, 0x18, 0xE7, 0x1C, 0xE6,
    0x1C, 0xC7, 0x18, 0xE6, 0x14, 0xC8, 0x18, 0xC7, 0x14, 0xE6, 0x10, 0xC8, 0x18, 0xC6, 0x18, 0xA7,
    0x14, 0xE5, 0x14, 0xC6, 0x18, 0xC5, 0x18, 0xA6, 0x14, 0xC5, 0x14, 0xA6, 0x18, 0xA5, 0x18, 0x86,
    0x10, 0xC5, 0x0C, 0xA7, 0x10, 0xA6, 0x14, 0xA5, 0x14, 0x86, 0x10, 0xC4, 0x10, 0xA5, 0x14, 0xA4,
    0x14, 0x85, 0x10, 0xA4, 0x10, 0x85, 0x14, 0x84, 0x0C, 0xA4, 0x10, 0xA3, 0x0C, 0x85, 0x10, 0x84,
    0x10, 0x65, 0x14, 0x64, 0x08, 0x85, 0x0C, 0x84, 0x10, 0x83, 0x10, 0x64, 0x08, 0x84, 0x0C, 0x83,
    0x10, 0x82, 0x0C, 0x64, 0x10, 0x63, 0x08, 0x83, 0x0C, 0x82, 0x08, 0x64, 0x0C, 0x63, 0x0C, 0x44,
    0x10, 0x43, 0x08, 0x82, 0x04, 0x64, 0x08, 0x63, 0x0C, 0x62, 0x0C, 0x43, 0x04, 0x63, 0x08, 0x62,
    0x04, 0x44, 0x08, 0x43, 0x0C, 0x42, 0x04, 0x62, 0x08, 0x61, 0x04, 0x43, 0x08, 0x42, 0x08, 0x23,
    0x0C, 0x22, 0x00, 0x43, 0x04, 0x42, 0x08, 0x41, 0x04, 0x23, 0x08, 0x22, 0x00, 0x42, 0x04, 0x41,
    0x04, 0x22, 0x08, 0x21, 0x00, 0x41, 0x04, 0x40, 0x00, 0x22, 0x04, 0x21, 0x08, 0x20, 0x04, 0x02,
    0x08, 0x01, 0x00, 0x21, 0x04, 0x20, 0x04, 0x01, 0x00, 0x20, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00,
    /*0xFF, 0xFF, 0x77, 0xFF, 0x67, 0xDF, 0x5F, 0xDF, 0x77, 0x1D, 0x67, 0x7D, 0x53, 0x9F, 0x5F, 0x7D,
    0x5F, 0x5C, 0x5B, 0x3C, 0x7E, 0xB6, 0x57, 0x3B, 0x53, 0x1C, 0x43, 0x3E, 0x62, 0xD9, 0x53, 0x1A,
    0x53, 0x19, 0x52, 0xDB, 0x52, 0xF9, 0x4A, 0xFA, 0x52, 0xF8, 0x7E, 0x33, 0x3A, 0xFC, 0x4E, 0xD8,
    0x72, 0x34, 0x4A, 0xD8, 0x4E, 0xD7, 0x42, 0xBA, 0x4A, 0xD7, 0x7E, 0x10, 0x3E, 0x9B, 0x4A, 0xB7,
    0x4E, 0xB6, 0x3E, 0xD8, 0x46, 0xB7, 0x4A, 0xB6, 0x5A, 0x36, 0x2E, 0xFA, 0x42, 0xB7, 0x46, 0xB6,
    0x4A, 0x96, 0x75, 0xF0, 0x46, 0xB5, 0x42, 0x97, 0x46, 0x96, 0x4A, 0x95, 0x42, 0x96, 0x46, 0x95,
    0x5E, 0x13, 0x79, 0xCE, 0x2E, 0xD8, 0x3E, 0x96, 0x42, 0x95, 0x46, 0x94, 0x26, 0x7C, 0x3E, 0x95,
    0x42, 0x75, 0x46, 0x74, 0x46, 0x36, 0x3E, 0x75, 0x42, 0x74, 0x2A, 0xB7, 0x3A, 0x75, 0x3E, 0x74,
    0x42, 0x73, 0x3E, 0x36, 0x6D, 0xAE, 0x2A, 0x78, 0x3E, 0x54, 0x42, 0x53, 0x42, 0x34, 0x61, 0xCF,
    0x3A, 0x54, 0x36, 0x36, 0x3E, 0x53, 0x26, 0x96, 0x2A, 0x76, 0x36, 0x54, 0x3A, 0x53, 0x3E, 0x52,
    0x3E, 0x33, 0x3E, 0x14, 0x26, 0x95, 0x2A, 0x75, 0x3A, 0x33, 0x3E, 0x32, 0x45, 0xF2, 0x26, 0x75,
    0x26, 0x56, 0x36, 0x33, 0x3A, 0x32, 0x36, 0x14, 0x51, 0xB0, 0x61, 0x8D, 0x26, 0x74, 0x36, 0x32,
    0x3A, 0x12, 0x3E, 0x11, 0x45, 0xD1, 0x36, 0x12, 0x3A, 0x11, 0x35, 0xF3, 0x3D, 0xD2, 0x22, 0x54,
    0x2A, 0x33, 0x36, 0x11, 0x51, 0x8E, 0x61, 0x6B, 0x35, 0xF1, 0x39, 0xF0, 0x35, 0xD2, 0x22, 0x33,
    0x1D, 0xD7, 0x31, 0xF1, 0x35, 0xF0, 0x51, 0x6D, 0x31, 0xF0, 0x3D, 0x90, 0x45, 0x8E, 0x59, 0x4B,
    0x29, 0xF1, 0x2D, 0xF0, 0x35, 0xCF, 0x1E, 0x12, 0x25, 0xB3, 0x31, 0xCF, 0x55, 0x4A, 0x29, 0xD0,
    0x2D, 0xCF, 0x25, 0x92, 0x35, 0xAD, 0x31, 0x8F, 0x49, 0x4B, 0x21, 0xD0, 0x29, 0xAF, 0x2D, 0xAE,
    0x31, 0xAD, 0x51, 0x29, 0x29, 0xAE, 0x31, 0x8D, 0x39, 0x4D, 0x2D, 0x8D, 0x2D, 0x6E, 0x19, 0xCF,
    0x19, 0x91, 0x25, 0x8E, 0x29, 0x8D, 0x2D, 0x8C, 0x31, 0x4D, 0x2D, 0x8B, 0x2D, 0x6C, 0x29, 0x6C,
    0x2D, 0x6B, 0x29, 0x4D, 0x45, 0x08, 0x25, 0x6C, 0x29, 0x6B, 0x2D, 0x2C, 0x19, 0x4F, 0x29, 0x6A,
    0x29, 0x4B, 0x39, 0x09, 0x19, 0x6D, 0x29, 0x4A, 0x29, 0x2B, 0x1D, 0x4C, 0x21, 0x4B, 0x25, 0x4A,
    0x2D, 0x0A, 0x21, 0x4A, 0x25, 0x49, 0x25, 0x2A, 0x21, 0x2A, 0x25, 0x29, 0x25, 0x0A, 0x1D, 0x2A,
    0x21, 0x29, 0x25, 0x28, 0x21, 0x0A, 0x2C, 0xE8, 0x15, 0x0C, 0x21, 0x28, 0x21, 0x09, 0x1D, 0x28,
    0x21, 0x08, 0x20, 0xE9, 0x24, 0xE8, 0x2C, 0xC7, 0x19, 0x09, 0x1D, 0x08, 0x1C, 0xE9, 0x18, 0xCB,
    0x20, 0xE8, 0x15, 0x09, 0x1D, 0x07, 0x1C, 0xE8, 0x19, 0x07, 0x1C, 0xE7, 0x18, 0xE7, 0x10, 0xE8,
    0x18, 0xE6, 0x14, 0xC8, 0x18, 0xC7, 0x10, 0xE7, 0x0C, 0xC9, 0x18, 0xC6, 0x18, 0xA7, 0x14, 0xC6,
    0x18, 0xA6, 0x20, 0x85, 0x14, 0xC5, 0x14, 0xA6, 0x0C, 0xA7, 0x14, 0xA5, 0x14, 0x86, 0x0C, 0xA6,
    0x10, 0xA5, 0x10, 0xA4, 0x10, 0x85, 0x08, 0x86, 0x10, 0x84, 0x10, 0x65, 0x14, 0x64, 0x08, 0x85,
    0x0C, 0x84, 0x10, 0x64, 0x14, 0x63, 0x08, 0x84, 0x04, 0x66, 0x0C, 0x83, 0x0C, 0x64, 0x08, 0x64,
    0x0C, 0x63, 0x08, 0x63, 0x0C, 0x43, 0x10, 0x42, 0x08, 0x62, 0x08, 0x43, 0x04, 0x43, 0x08, 0x42,
    0x08, 0x23, 0x04, 0x42, 0x08, 0x41, 0x04, 0x41, 0x00, 0x23, 0x04, 0x22, 0x08, 0x21, 0x00, 0x22,
    0x04, 0x21, 0x00, 0x21, 0x00, 0x02, 0x04, 0x01, 0x00, 0x20, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00,*/
};

sLayerConfig titleNBG0Setup[] =
{
    m2_CHCN,  1, //256 colors
    m5_CHSZ,  1, //pattern is 1x1
    m6_PNB,  1, // 1 word
    m7_CNSM,  1,
    m0_END, // Character Number Supplementary mode 1 (12bits)
};

sLayerConfig titleNBG1Setup[] =
{
    m2_CHCN,  0, // 16 colors
    m5_CHSZ,  0, // pattern is 1x1
    m6_PNB,  1, // 1 word
    m7_CNSM,  1, // Character Number Supplementary mode 1 (12bits)
    m12_PLSZ, 0, // plane is 1H x 1V
    m40_CAOS, 7, // palette offset is 7 * 0x200 = 0xE00 (where the palette for the text is stored)
    m0_END,
};

void loadTitleScreenGraphics()
{
    reinitVdp2();

    vdp2Controls.m4_pendingVdp2Regs->m0_TVMD = (vdp2Controls.m4_pendingVdp2Regs->m0_TVMD & 0xFFF8) | 3; // HRESO 704
    vdp2Controls.m4_pendingVdp2Regs->m0_TVMD = (vdp2Controls.m4_pendingVdp2Regs->m0_TVMD & 0xFF3F) | 0xC0; // LSMD0 & 1 to 1 (double density interlace)
    vdp2Controls.m_isDirty = 1;

    loadFile("TITLEE.SCB", getVdp2Vram(0x20000), 0);
    addToMemoryLayout(getVdp2Vram(0x20000), 1);

    loadFile("TITLEE.PNB", getVdp2Vram(0x10000), 0);
    addToMemoryLayout(getVdp2Vram(0x10000), 1);

    asyncDmaCopy(titleScreenPalette, getVdp2Cram(0), 0x200, 0);

    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x15FFFFF;
    vdp2Controls.m4_pendingVdp2Regs->m14_CYCA1 = 0x44FFFFF;
    vdp2Controls.m4_pendingVdp2Regs->m18_CYCB0 = 0x44FFFFF;
    vdp2Controls.m4_pendingVdp2Regs->m1C_CYCB1 = 0x44FFFFF;

    setupNBG0(titleNBG0Setup);
    initLayerMap(0, (0x10000), (0x10800), (0x10000), (0x10800));

    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 0x706; // Layer0 = 6, Layer1 = 7
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR = 0;
    vdp2Controls.m_isDirty = 1; // because why not?

    if (VDP2Regs_.m4_TVSTAT & 1)
    {
        pauseEngine[4] = 0;
        updateVDP2CoordinatesIncrement(0x10000, 0xE000);
        pauseEngine[4] = 2;
    }

    setupNBG1(titleNBG1Setup);
    initLayerMap(1, vdp2TextMemoryOffset, vdp2TextMemoryOffset, vdp2TextMemoryOffset, vdp2TextMemoryOffset);

    pauseEngine[4] = 1;
    updateVDP2CoordinatesIncrement(0x8000, 0x8000);
    pauseEngine[4] = 2;
}

void s_titleOverlayWorkArea::titleOverlay_Update(s_titleOverlayWorkArea* pWorkArea)
{
    switch (pWorkArea->m_status)
    {
    case 0:
        loadTitleScreenGraphics();
        loadSoundBanks(0x4B, 0);
        pWorkArea->m_4 = 150;
        pWorkArea->m_status++;
    case 1:
        if (!isSoundLoadingFinished())
        {
            if(--pWorkArea->m_4)
                break;
        }
        //fileInfoStruct.m_1 = 3;
        //fileInfoStruct.m_2 = 0;
        //fileInfoStruct.m_4 = 0xF;
        pWorkArea->m_status++;
        break;
    case 2:
        pWorkArea->m_status++;
        break;
    case 3:
        if (g_fadeControls.m_4D >= g_fadeControls.m_4C)
        {
            vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
            vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
        }

        fadePalette(&g_fadeControls.m0_fade0, convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color), g_fadeControls.m_48, 30);

        //TODO: maybe more stuff here

        pWorkArea->getTask()->markFinished();
        break;
    default:
        assert(0);
    }

    //unk_6000014 = SYS_EXECDMP;
}

p_workArea overlayStart(s_workArea* workArea)
{
    return createSubTask<s_titleOverlayWorkArea>(workArea);
}
};

