#include "PDS.h"
#include "movie.h"
#include "cinepak.h"
#include "movieAudio.h"
#include "mainMenuDebugTasks.h"
#include "menu_dragon.h"
#include "kernel/fade.h"
#include "heap.h"
#include "town/townCutscene.h"
#include "3dEngine_textureCache.h"

void unloadFnt(); // defined in mainMenuDebugTasks.cpp
void clearVdp2VRam(u32 offset, u32 size); // defined in VDP2.cpp
void displayMemoryLayout(); // defined in PDS.cpp
static inline s32 performDivision(s32 divisor, s32 dividend) { return dividend / divisor; }

// ============================================================================
// CPK file name table (50 entries, indexed 0x00–0x31)
// 06054d30 / 06054ea8
// ============================================================================
static const char* g_cpkFileNames[] = {
    "EVT000_1.CPK", // 0x00
    "EVT000_2.CPK", // 0x01
    "EVT000_3.CPK", // 0x02
    "EVT000_4.CPK", // 0x03
    "EVT000_5.CPK", // 0x04
    "EVT002.CPK",   // 0x05
    "EVT004_1.CPK", // 0x06
    "EVT004_2.CPK", // 0x07
    "EVT008.CPK",   // 0x08
    "EVT016_1.CPK", // 0x09
    "EVT016_2.CPK", // 0x0A
    "EVT016_3.CPK", // 0x0B
    "EVT029.CPK",   // 0x0C
    "EVT040.CPK",   // 0x0D
    "EVT041.CPK",   // 0x0E
    "EVT043.CPK",   // 0x0F
    "EVT054.CPK",   // 0x10
    "EVT060_1.CPK", // 0x11
    "EVT060_2.CPK", // 0x12
    "EVT067.CPK",   // 0x13
    "EVT068_1.CPK", // 0x14
    "EVT068_2.CPK", // 0x15
    "EVT068_3.CPK", // 0x16
    "EVT072.CPK",   // 0x17
    "EVT089.CPK",   // 0x18
    "EVT117.CPK",   // 0x19
    "EVT122_1.CPK", // 0x1A
    "EVT122_2.CPK", // 0x1B
    "EVT122_3.CPK", // 0x1C
    "EVT122_4.CPK", // 0x1D
    "EVT127_1.CPK", // 0x1E
    "EVT127_2.CPK", // 0x1F
    "EVT134_1.CPK", // 0x20
    "EVT134_2.CPK", // 0x21
    "EVT134_3.CPK", // 0x22
    "EVT135.CPK",   // 0x23
    "EVT137.CPK",   // 0x24
    "EVT144.CPK",   // 0x25
    "EVT152.CPK",   // 0x26
    "EVT153.CPK",   // 0x27
    "EVT155_1.CPK", // 0x28
    "EVT155_2.CPK", // 0x29
    "EVT155_3.CPK", // 0x2A
    "EVT155_4.CPK", // 0x2B
    "EVT155_5.CPK", // 0x2C
    "EVT160_1.CPK", // 0x2D
    "EVT160_2.CPK", // 0x2E  (index 46, '.')
    "EVT161_1.CPK", // 0x2F  (index 47, '/')
    "EVT161_2.CPK", // 0x30
    "MOVIE1.CPK",   // 0x31
};

// ============================================================================
// Movie timing table (33 entries) – {cpkIndex, countdown, fadePalTarget}
// 06054df8
// ============================================================================
struct s_movieTimingEntry
{
    u8  m0_cpkIndex;
    u8  m1_countdown;
    u16 m2_fadePalTarget;
};

static const s_movieTimingEntry g_movieTimingTable[] = {
    {0x00, 0x05, 0x8000}, // 0x00
    {0x05, 0x01, 0x8000}, // 0x01
    {0x06, 0x02, 0x8000}, // 0x02
    {0x08, 0x01, 0x8000}, // 0x03
    {0x09, 0x03, 0x8000}, // 0x04
    {0x0C, 0x01, 0xFFFF}, // 0x05
    {0x0D, 0x01, 0x8000}, // 0x06
    {0x0E, 0x01, 0x8000}, // 0x07
    {0x0F, 0x01, 0xFFFF}, // 0x08
    {0x10, 0x01, 0xFFFF}, // 0x09
    {0x11, 0x02, 0x8000}, // 0x0A
    {0x13, 0x01, 0x8000}, // 0x0B
    {0x14, 0x03, 0xFFFF}, // 0x0C
    {0x17, 0x01, 0x8000}, // 0x0D
    {0x18, 0x01, 0x8000}, // 0x0E
    {0x19, 0x01, 0x8000}, // 0x0F
    {0x1A, 0x04, 0x8000}, // 0x10
    {0x1E, 0x02, 0x8000}, // 0x11
    {0x20, 0x03, 0x8000}, // 0x12
    {0x23, 0x01, 0x8000}, // 0x13
    {0x24, 0x01, 0x8000}, // 0x14
    {0x25, 0x01, 0x8000}, // 0x15
    {0x26, 0x01, 0xFFFF}, // 0x16
    {0x27, 0x01, 0x8000}, // 0x17
    {0x28, 0x01, 0xFFFF}, // 0x18
    {0x29, 0x01, 0xFFFF}, // 0x19
    {0x2A, 0x01, 0xFFFF}, // 0x1A
    {0x2B, 0x01, 0xFFFF}, // 0x1B
    {0x2C, 0x01, 0xFFFF}, // 0x1C
    {0x2D, 0x02, 0x8000}, // 0x1D
    {0x2F, 0x01, 0x8000}, // 0x1E
    {0x30, 0x01, 0x8000}, // 0x1F
    {0x31, 0x01, 0xFFFF}, // 0x20
};

// ============================================================================
// Subtitle FNT / VRAM destination table (50 entries, 8 bytes each)
// {const char* fntName, u32 vramDest}  – null fntName = no subtitle
// 06055184 / 0605536c
// ============================================================================
struct s_movieFntEntry
{
    const char* m0_fntName;
    u32         m4_vramDest;
};

static const s_movieFntEntry g_movieFntTable[] = {
    {"EVT000_1.FNT", 0x00250000}, // 0x00
    {"EVT000_2.FNT", 0x002500D8}, // 0x01
    {"EVT000_3.FNT", 0x00250240}, // 0x02
    {"EVT000_4.FNT", 0x002502C8}, // 0x03
    {"EVT000_5.FNT", 0x00250390}, // 0x04
    {"EVT002.FNT",   0x00250438}, // 0x05
    {nullptr,        0x00000000}, // 0x06 – EVT004_1 has no subtitle
    {"EVT004_2.FNT", 0x00250460}, // 0x07
    {"EVT008.FNT",   0x00250478}, // 0x08
    {"EVT016_1.FNT", 0x00250560}, // 0x09
    {"EVT016_2.FNT", 0x00250598}, // 0x0A
    {"EVT016_3.FNT", 0x002506C8}, // 0x0B
    {"EVT029.FNT",   0x00250758}, // 0x0C
    {"EVT040.FNT",   0x002507B8}, // 0x0D
    {"EVT041.FNT",   0x002507E8}, // 0x0E
    {"EVT043.FNT",   0x00250908}, // 0x0F
    {"EVT054.FNT",   0x00250990}, // 0x10
    {"EVT060_1.FNT", 0x002509F0}, // 0x11
    {"EVT060_2.FNT", 0x00250A78}, // 0x12
    {"EVT067.FNT",   0x00250B68}, // 0x13
    {"EVT068_1.FNT", 0x00250B78}, // 0x14
    {"EVT068_2.FNT", 0x00250C40}, // 0x15
    {"EVT068_3.FNT", 0x00250D80}, // 0x16
    {"EVT072.FNT",   0x00250D90}, // 0x17
    {"EVT089.FNT",   0x00250DB8}, // 0x18
    {"EVT117.FNT",   0x00250EA0}, // 0x19
    {"EVT122_1.FNT", 0x00250ED8}, // 0x1A
    {"EVT122_2.FNT", 0x00250EF0}, // 0x1B
    {"EVT122_3.FNT", 0x00250F58}, // 0x1C
    {"EVT122_4.FNT", 0x00251080}, // 0x1D
    {"EVT127_1.FNT", 0x002511B0}, // 0x1E
    {"EVT127_2.FNT", 0x00251240}, // 0x1F
    {"EVT134_1.FNT", 0x00251268}, // 0x20
    {"EVT134_2.FNT", 0x00251308}, // 0x21
    {"EVT134_3.FNT", 0x00251498}, // 0x22
    {"EVT135.FNT",   0x00251570}, // 0x23
    {"EVT137.FNT",   0x002516A8}, // 0x24
    {"EVT144.FNT",   0x00251730}, // 0x25
    {"EVT152.FNT",   0x00251890}, // 0x26
    {nullptr,        0x00000000}, // 0x27 – EVT153 has no subtitle
    {"EVT155_1.FNT", 0x00251940}, // 0x28
    {"EVT155_2.FNT", 0x00251958}, // 0x29
    {"EVT155_3.FNT", 0x00251970}, // 0x2A
    {"EVT155_4.FNT", 0x00251980}, // 0x2B
    {"EVT155_5.FNT", 0x00251990}, // 0x2C
    {"EVT160_1.FNT", 0x002519B8}, // 0x2D
    {"EVT160_2.FNT", 0x00251A98}, // 0x2E
    {"EVT161_1.FNT", 0x00251AC0}, // 0x2F
    {"EVT161_2.FNT", 0x00251B10}, // 0x30
    {nullptr,        0x00000000}, // 0x31 – MOVIE1 has no subtitle
};

// ============================================================================
// NBG0 layer config for movie video display
// 06042fec
// ============================================================================
static const sLayerConfig g_movieNBG0Config[] = {
    {m2_CHCN,             4},
    {(eVdp2LayerConfig)4, 1},
    {m34_W0E,             1},
    {m37_W0A,             1},
    {m0_END,              0},
};

// ============================================================================
// Helpers
// ============================================================================

static void killTask(p_workArea pTask)
{
    if (pTask && !(pTask->getTask()->m14_flags & 1))
        pTask->getTask()->m14_flags |= 1;
}

// ============================================================================
// openMovieStream  –  060106c0
// ============================================================================

static void clearMovieVdp2Vram();

static GfsHn s_movieGfsHn;
static MovieAudioStream s_movieAudioStream;
static sMovieAudioBuffer s_movieAudioBuffer;
static SoLoud::handle s_movieAudioHandle = 0;

// 060106c0
static void openMovieStream(const char* cpkFileName)
{
    s_movieGfsHn.fHandle = fopen(cpkFileName, "rb");
    if (!s_movieGfsHn.fHandle)
        return;

    // Create Cinepak decoder from the opened file
    sCinepakDecoder* decoder = sCinepakDecoder::create(s_movieGfsHn.fHandle);
    fileInfoStruct.m10_movieDecoder = decoder;

    fileInfoStruct.mC_gfsHandle = &s_movieGfsHn;

    if (decoder)
    {
        fileInfoStruct.m20_videoWidth  = decoder->getWidth();
        fileInfoStruct.m24_videoHeight = decoder->getHeight();
    }
    else
    {
        fileInfoStruct.m20_videoWidth  = 320;
        fileInfoStruct.m24_videoHeight = 224;
    }

    // m1C_vramDest: ((h * -0x200 + 0x20160) - w) * 4 >> 1 + 0x25E00000
    u32 vramDest = (u32)((((s32)fileInfoStruct.m24_videoHeight * -0x200 + 0x20160) -
                          (s32)fileInfoStruct.m20_videoWidth) * 4) >> 1;
    fileInfoStruct.m1C_vramDest = vramDest + 0x25E00000u;

    // VDP2 window registers — center video in 352×224 display
    s_VDP2Regs* pRegs = vdp2Controls.m4_pendingVdp2Regs;
    s32 w = (s32)fileInfoStruct.m20_videoWidth;
    s32 h = (s32)fileInfoStruct.m24_videoHeight;
    pRegs->mC0_WPSX0 = (s16)(((0x160 - w) >> 1) << 1);
    pRegs->mC2_WPSY0 = (s16)((0xE0 - h) >> 1);
    pRegs->mC4_WPEX0 = (s16)(((w + 0x15F) >> 1) << 1);
    pRegs->mC6_WPEY0 = (s16)((h + 0xDF) >> 1);

    fileInfoStruct.m18_decodeBuffer = nullptr;
    fileInfoStruct.m14_frameCount   = 0;
    fileInfoStruct.m28_countdown    = 0x3C;

    if (VDP2Regs_.m4_TVSTAT & 1)
        vblankData.m14_numVsyncPerFrame = 1;

    // Start playback
    if (decoder)
    {
        // Set up audio streaming if the FILM has audio
        if (decoder->getAudioChannels() > 0 && decoder->getAudioSampleRate() > 0)
        {
            s_movieAudioBuffer.init(decoder->getAudioChannels());
            decoder->setAudioBuffer(&s_movieAudioBuffer);
            s_movieAudioStream.setup(decoder->getAudioSampleRate(), decoder->getAudioChannels(), &s_movieAudioBuffer);
            s_movieAudioHandle = gSoloud.play(s_movieAudioStream);
        }

        decoder->start();
    }
}

// ============================================================================
// closeMovieStream  –  06010870
// ============================================================================

// 06010870
void closeMovieStream()
{
    if (fileInfoStruct.mC_gfsHandle)
    {
        // Stop movie audio stream
        if (s_movieAudioHandle)
        {
            gSoloud.stop(s_movieAudioHandle);
            s_movieAudioHandle = 0;
        }

        // Stop and destroy Cinepak decoder
        if (fileInfoStruct.m10_movieDecoder)
        {
            sCinepakDecoder* decoder = (sCinepakDecoder*)fileInfoStruct.m10_movieDecoder;
            decoder->stop();
            decoder->destroy();
            fileInfoStruct.m10_movieDecoder = nullptr;
        }
        if (s_movieGfsHn.fHandle)
        {
            fclose(s_movieGfsHn.fHandle);
            s_movieGfsHn.fHandle = nullptr;
        }
        fileInfoStruct.mC_gfsHandle = nullptr;
        if (VDP2Regs_.m4_TVSTAT & 1)
            vblankData.m14_numVsyncPerFrame = 2;
    }
}

// ============================================================================
// clearMovieVdp2Vram  –  06010584
// ============================================================================

// 06010584
static void clearMovieVdp2Vram()
{
    clearVdp2VRam(0x14040, 0x58000);
}

// ============================================================================
// setNBG0MapOffset  –  06028c54
// ============================================================================

// 06028c54
static void setNBG0MapOffset(s32 layer, u32 saturnVramAddr)
{
    u16 offset = (u16)(((saturnVramAddr + 0xDA200000u) >> 17) & 7);
    if (layer == 0)
    {
        vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN =
            (vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN & 0xFFF0) | offset;
    }
    else if (layer == 1)
    {
        vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN =
            (vdp2Controls.m4_pendingVdp2Regs->m3C_MPOFN & 0xFF0F) | (u16)(offset << 4);
    }
}

// ============================================================================
// setupMovieVdp2  –  0601058e
// ============================================================================

// 0601058e
void setupMovieVdp2()
{
    reinitVdp2();
    vdp2Controls.m4_pendingVdp2Regs->mE_RAMCTL  &= 0xFCFF;
    vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0   = 0x44444444;
    vdp2Controls.m4_pendingVdp2Regs->m18_CYCB0   = 0x44444444;
    vdp2Controls.m_isDirty = 1;
    setupNBG0(g_movieNBG0Config);
    setNBG0MapOffset(0, 0x25E00000);
    pauseEngine[4] = 0;
    if ((VDP2Regs_.m4_TVSTAT & 1) == 0)
    {
        setupVDP2CoordinatesIncrement2(0, 0x100000);
    }
    else
    {
        setupVDP2CoordinatesIncrement2(0, 0x200000);
        updateVDP2CoordinatesIncrement(0x10000, 0xE000);
    }
    pauseEngine[4] = 4;
    vdp2Controls.m4_pendingVdp2Regs->mF0_PRISA = 0x0707;
    vdp2Controls.m4_pendingVdp2Regs->mF2_PRISB = 0x0707;
    vdp2Controls.m4_pendingVdp2Regs->mF4_PRISC = 0x0707;
    vdp2Controls.m4_pendingVdp2Regs->mF6_PRISD = 0x0707;
    vdp2Controls.m4_pendingVdp2Regs->mF8_PRINA = 4;
    vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB = 0;
    vdp2Controls.m4_pendingVdp2Regs->mFC_PRIR  = 0;
    vdp2Controls.m_isDirty = 1;
    clearMovieVdp2Vram();
}

// ============================================================================
// Text sprite batch task  –  06055324 / 06054784
// On Saturn the work area is raw { s16 count, entries[][3] : {x, y, tileAddrDiv8} }.
// In C++ we use proper struct members since the work area has base class overhead.
// ============================================================================

struct s_movieTextSpriteBatchWorkArea : public s_workAreaTemplate<s_movieTextSpriteBatchWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = {nullptr, nullptr, &Draw, nullptr};
        return &taskDefinition;
    }

    struct SpriteEntry { s16 x; s16 y; s16 tileAddrDiv8; };

    s16 m0_count;
    std::vector<SpriteEntry> m_entries;

    // 06054784
    static void Draw(s_movieTextSpriteBatchWorkArea* pThis)
    {
        for (s32 i = 0; i < pThis->m0_count; i++)
        {
            SpriteEntry& sprite = pThis->m_entries[i];

            s_vdp1Command& cmd = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
            cmd.m0_CMDCTRL = 0x1000;
            cmd.m4_CMDPMOD = 0x0080;
            cmd.m6_CMDCOLR = 0x07C0;
            cmd.m8_CMDSRCA = sprite.tileAddrDiv8;
            cmd.mA_CMDSIZE = 0x0110;
            cmd.mC_CMDXA   =  sprite.x;
            cmd.mE_CMDYA   = (s16)(-sprite.y);

            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &cmd;
            graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;
            graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
            graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
            graphicEngineStatus.m14_vdp1Context[0].mC += 1;
        }
    }
};

// ============================================================================
// createTextSpriteTask  –  0605480c
// Scans VDP2 text memory and builds a VDP1 sprite draw list for subtitle glyphs.
// ============================================================================

// 0605480c
static p_workArea createTextSpriteTask(p_workArea pParent, s32 startX, s32 startY,
                                       s32 width, s32 height, u32 vdp1Addr)
{
    s32 halfHeight = (s32)(height + (u32)(height < 0)) >> 1;

    auto* pTask = createSubTask<s_movieTextSpriteBatchWorkArea>(pParent);
    if (pTask == nullptr)
        return nullptr;

    pTask->m0_count = 0;

    u32 vdp1AddrStart = vdp1Addr;

    // Cell offset into VDP2 text memory (in bytes)
    u32 cellOffset = vdp2TextMemoryOffset + (startY * 0x40 + startX) * 2;
    s32 screenY = startY << 3;

    for (s32 row = halfHeight; row != 0; --row)
    {
        s32 screenX = startX << 3;
        u32 curCell = cellOffset;
        s32 col = width;

        do
        {
            // VDP2 VRAM is stored big-endian; use getVdp2VramU16 (not raw u16* deref)
            u16 cellValue = getVdp2VramU16(curCell);
            if ((cellValue & 0xFFF) != 0)
            {
                s_movieTextSpriteBatchWorkArea::SpriteEntry entry;
                entry.x = (s16)(screenX - 0xB0);
                entry.y = (s16)(0x70 - screenY);
                entry.tileAddrDiv8 = (s16)(vdp1Addr >> 3);
                pTask->m_entries.push_back(entry);

                u16 cellBelow = getVdp2VramU16(curCell + 0x80);
                u8* pVdp1Dest = getVdp1Pointer(0x25C00000 + vdp1Addr);
                addToMemoryLayout(pVdp1Dest, 0x40);
                memcpy_dma(getVdp2Vram((cellValue & 0xFFF) * 0x20), pVdp1Dest, 0x20);
                memcpy_dma(getVdp2Vram((cellBelow & 0xFFF) * 0x20), pVdp1Dest + 0x20, 0x20);
                vdp1Addr += 0x40;

                pTask->m0_count++;
            }
            curCell += 2;
            col--;
            screenX += 8;
        } while (col != 0);

        screenY += 0x10;
        cellOffset += 0x100; // 0x80 u16 entries = 0x100 bytes = one row of 64 cells * 2 bytes
    }

    // Invalidate texture cache for the VDP1 VRAM range we just wrote to
    if (vdp1Addr > vdp1AddrStart)
    {
        invalidateVdp1TextureRange(vdp1AddrStart, vdp1Addr - vdp1AddrStart);
    }

    return pTask;
}

// ============================================================================
// Video decoder notify task  –  0605533c / 0605494c
// Kills old text sprite task and (in debug mode) prints frame counter.
// ============================================================================

struct s_videoDecoderNotifyWorkArea : public s_workAreaTemplate<s_videoDecoderNotifyWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = {nullptr, nullptr, &Draw, nullptr};
        return &taskDefinition;
    }

    p_workArea m0_textSpriteTask;

    // 0605494c
    static void Draw(s_videoDecoderNotifyWorkArea* pThis)
    {
        killTask(pThis->m0_textSpriteTask);
        pThis->m0_textSpriteTask = nullptr;

        if (enableDebugTask)
        {
            vdp2DebugPrintSetPosition(2, 1);
            s32 frame = performDivision(0x14, (s32)fileInfoStruct.m14_frameCount);
            vdp2PrintfLargeFont("%04d", frame);
            pThis->m0_textSpriteTask = createTextSpriteTask(pThis, 2, 1, 4, 2, 0x18800);
        }
    }
};

// ============================================================================
// Subtitle overlay task  –  06054e98 / 06054564
// Emits a VDP1 user-clipping command to mask the subtitle area.
// ============================================================================

struct s_subtitleOverlayWorkArea : public s_workAreaTemplate<s_subtitleOverlayWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = {nullptr, nullptr, &Draw, nullptr};
        return &taskDefinition;
    }

    u32 m0_state;

    // 06054564
    static void Draw(s_subtitleOverlayWorkArea* pThis)
    {
        if (!fileInfoStruct.mC_gfsHandle)
        {
            pThis->getTask()->m14_flags |= 1;
            return;
        }

        if (pThis->m0_state == 0)
        {
            if ((s32)fileInfoStruct.m14_frameCount < 0x5C6C)
                return;
            pThis->m0_state = 1;
        }
        else if (pThis->m0_state != 1)
        {
            return;
        }

        s_vdp1Command& cmd = *graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA;
        cmd.m0_CMDCTRL = 0x1006;
        cmd.m4_CMDPMOD = 0x00C0;
        cmd.m6_CMDCOLR = 0x8000;
        cmd.mC_CMDXA   =  0x009F;
        cmd.mE_CMDYA   = (s16)0xFFA8;
        cmd.m10_CMDXB  =  0x009F;
        cmd.m12_CMDYB  =  0x0057;

        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m4_bucketTypes = 0;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet->m6_vdp1EA = &cmd;
        graphicEngineStatus.m14_vdp1Context[0].m20_pCurrentVdp1Packet++;
        graphicEngineStatus.m14_vdp1Context[0].m1C += 1;
        graphicEngineStatus.m14_vdp1Context[0].m0_currentVdp1WriteEA++;
        graphicEngineStatus.m14_vdp1Context[0].mC += 1;
    }
};

// ============================================================================
// Subtitle data task  –  06055314 / 06054640 / 06054684 / 06054732
// ============================================================================

struct s_subtitleDataWorkArea : public s_workAreaTemplateWithArg<s_subtitleDataWorkArea, s32>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = {&Init, nullptr, &Draw, &Delete};
        return &taskDefinition;
    }

    u32       m0_state;
    u32       m4_nextFrameTime;
    u8*       m8_subtitleData;   // pointer into townBuffer (MOVIE.DAT subtitle timing data)
    p_workArea mC_textSpriteTask;

    // Saturn address 0x00250000 is the base of townBuffer on Saturn
    static constexpr u32 SATURN_TOWN_BUFFER_BASE = 0x00250000;

    static u8* saturnAddrToHost(u32 saturnAddr)
    {
        return townBuffer + (saturnAddr - SATURN_TOWN_BUFFER_BASE);
    }

    // 06054640
    static void Init(s_subtitleDataWorkArea* pThis, s32 cpkIndex)
    {
        pThis->m0_state = 1;
        u32 vramDest = g_movieFntTable[cpkIndex].m4_vramDest;
        if (!vramDest)
        {
            pThis->m8_subtitleData = nullptr;
            pThis->getTask()->m14_flags |= 1;
            return;
        }
        pThis->m8_subtitleData = saturnAddrToHost(vramDest);
        loadFnt(g_movieFntTable[cpkIndex].m0_fntName);
        setupVDP2StringRendering(3, 0x19, 0x26, 2);
    }

    // 06054684
    static void Draw(s_subtitleDataWorkArea* pThis)
    {
        if (pThis->m0_state == 0)
        {
            if ((s32)pThis->m4_nextFrameTime <= (s32)fileInfoStruct.m14_frameCount)
            {
                clearVdp2TextArea();
                killTask(pThis->mC_textSpriteTask);
                pThis->mC_textSpriteTask = nullptr;
                pThis->m8_subtitleData += 8;
                pThis->m0_state = 1;
            }
        }
        else if (pThis->m0_state == 1)
        {
            s16 frameTime = READ_BE_S16(pThis->m8_subtitleData);
            pThis->m4_nextFrameTime = (u32)((s32)frameTime * 0x14);
            if ((s32)pThis->m4_nextFrameTime < 0)
            {
                pThis->getTask()->m14_flags |= 1;
                return;
            }
            pThis->m0_state = 2;
        }
        else if (pThis->m0_state == 2)
        {
            if ((s32)pThis->m4_nextFrameTime <= (s32)fileInfoStruct.m14_frameCount)
            {
                // String pointer is a Saturn address stored as BE32 at offset 4
                u32 saturnStrPtr = READ_BE_U32(pThis->m8_subtitleData + 4);
                const char* str = (const char*)saturnAddrToHost(saturnStrPtr);
                VDP2DrawString(str);
                pThis->mC_textSpriteTask = createTextSpriteTask(pThis, 3, 0x19, 0x26, 2, 0x18900);
                pThis->m4_nextFrameTime = (u32)((s32)READ_BE_S16(pThis->m8_subtitleData + 2) * 0x14);
                pThis->m0_state = 0;
            }
        }
    }

    // 06054732
    static void Delete(s_subtitleDataWorkArea* pThis)
    {
        if (pThis->m8_subtitleData)
        {
            clearVdp2TextArea();
            unloadFnt();
        }
    }
};

// ============================================================================
// Movie editor task  –  0605535c / 060549b0 / 06054a90 / 06054cf6
// Debug tool for adjusting subtitle timing with keyboard.
// ============================================================================

struct s_movieEditorWorkArea : public s_workAreaTemplateWithArg<s_movieEditorWorkArea, s32>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = {&Init, nullptr, &Draw, &Delete};
        return &taskDefinition;
    }

    u32  m0_state;
    u32  m4_vramDest;
    u16  m8_editField;
    s16  mA_selectedRow;

    // 060549b0
    static void Init(s_movieEditorWorkArea* pThis, s32 cpkIndex)
    {
        u32 vramDest = g_movieFntTable[cpkIndex].m4_vramDest;
        if (!vramDest)
        {
            pThis->getTask()->m14_flags |= 1;
            return;
        }
        pThis->m4_vramDest = vramDest;
        reinitVdp2();
        vdp2Controls.m4_pendingVdp2Regs->m10_CYCA0 = 0x3FFF7FFF;
        vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB =
            (vdp2Controls.m4_pendingVdp2Regs->mFA_PRINB & 0xF8FF) | 0x0700;
        vdp2Controls.m_isDirty = 1;
        if (g_fadeControls.m_4C <= g_fadeControls.m_4D)
        {
            vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
            vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
        }
        fadePalette(&g_fadeControls.m0_fade0, 0xC210, 0xC210, 1);
        vblankData.m14_numVsyncPerFrame = 1;
    }

    // 06054a90
    static void Draw(s_movieEditorWorkArea* pThis)
    {
        s16* pData = (s16*)(uintptr_t)pThis->m4_vramDest;

        if (readKeyboardTable1(0x106))
            pThis->m8_editField = 0;
        if (readKeyboardTable1(0x10d))
            pThis->m8_editField = 1;
        if (readKeyboardTable1(0x109) && pThis->mA_selectedRow > 0)
            pThis->mA_selectedRow--;
        if (readKeyboardTable1(0x10a) && pData[pThis->mA_selectedRow * 4 + 4] >= 0)
            pThis->mA_selectedRow++;

        if (readKeyboardTable1(0x10b))
        {
            s32 row = (s32)pThis->mA_selectedRow;
            s32 delta = (gCutsceneFlags & 2) ? 10 : 1;
            s32 newVal;
            if (pThis->m8_editField == 0)
            {
                newVal = (s32)pData[row * 4] + delta;
                pData[row * 4] = (s16)newVal;
                if (pData[row * 4 + 1] < newVal)
                    pData[row * 4 + 1] = (s16)newVal;
            }
            else
            {
                newVal = (s32)pData[row * 4 + 1] + delta;
                pData[row * 4 + 1] = (s16)newVal;
            }
            s32 propagateRow = row + 1;
            while (pData[propagateRow * 4] >= 0)
            {
                if (pData[propagateRow * 4] < newVal)
                    pData[propagateRow * 4] = (s16)newVal;
                if (pData[propagateRow * 4 + 1] < newVal)
                    pData[propagateRow * 4 + 1] = (s16)newVal;
                propagateRow++;
            }
        }

        if (readKeyboardTable1(0x10c))
        {
            s32 row = (s32)pThis->mA_selectedRow;
            s32 delta = (gCutsceneFlags & 2) ? 10 : 1;
            if (pThis->m8_editField == 0)
                pData[row * 4] -= (s16)delta;
            else
                pData[row * 4 + 1] -= (s16)delta;
        }

        s32 entryIdx = 0;
        s16* psEntry = pData;
        vdp2PrintStatus.m10_palette = 0xc000;
        for (; *psEntry >= 0; psEntry += 4)
        {
            u32 row    = performModulo(0x17, (u32)entryIdx);
            s32 col    = (s32)performDivision(0x17, (s32)entryIdx);
            vdp2DebugPrintSetPosition(col * 10 + 2, (s32)row);
            vdp2PrintfSmallFont(" %04d-%04d\n", (s32)psEntry[0], (s32)psEntry[1]);
            if (pThis->mA_selectedRow == entryIdx)
            {
                u32 cursorRow = performModulo(0x17, (u32)pThis->mA_selectedRow);
                s32 cursorCol = (s32)performDivision(0x17, (s32)pThis->mA_selectedRow);
                vdp2DebugPrintSetPosition(cursorCol * 10 + (s32)pThis->m8_editField * 5 + 2, (s32)cursorRow);
                vdp2PrintfSmallFont("\x7f");
            }
            entryIdx++;
        }

        if (readKeyboardToggle(0xf6))
            pThis->getTask()->m14_flags |= 1;
    }

    // 06054cf6
    static void Delete(s_movieEditorWorkArea* pThis)
    {
        vblankData.m14_numVsyncPerFrame = 2;
    }
};

// ============================================================================
// Main movie task  –  06054e7c / 06054010 / 060540fa / 06054514
// ============================================================================

struct s_movieMainWorkArea : public s_workAreaTemplateWithArg<s_movieMainWorkArea, s32>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = {&Init, nullptr, &Draw, &Delete};
        return &taskDefinition;
    }

    u8         m0_state;
    u8         m1_debugMode;
    u8         m2_cpkIndex;
    u8         m3_countdown;
    u8         m4_savedVblank;
    u8         m5;
    s16        m6_lastPitch;
    u16        m8_fadePalTarget;
    s16        mA_timer;
    p_workArea mC_subtask;
    void*      m10_movieDatBuffer;
    u32        m14_movieDatSize;
    void*      m18_vdp1Buffer;

    // 06054010
    static void Init(s_movieMainWorkArea* pThis, s32 movieIndex)
    {
        pThis->m0_state = 0;
        pThis->mC_subtask = nullptr;

        if (movieIndex < 0)
        {
            // Debug mode: find first available CPK
            s32 cpkIndex = 0;
            while (!findFileOnDisc(g_cpkFileNames[cpkIndex]))
                cpkIndex++;
            pThis->m1_debugMode   = 1;
            pThis->m2_cpkIndex    = (u8)cpkIndex;
            pThis->m8_fadePalTarget = 0xC210;
        }
        else
        {
            const s_movieTimingEntry& e = g_movieTimingTable[movieIndex];
            pThis->m2_cpkIndex      = e.m0_cpkIndex;
            pThis->m3_countdown     = e.m1_countdown;
            pThis->m8_fadePalTarget = e.m2_fadePalTarget;
        }

        s32 datSize = (s32)getFileSize("MOVIE.DAT");
        pThis->m14_movieDatSize = (u32)datSize;

        void* pDatBuf = allocateHeapForTask(pThis, (u32)datSize);
        pThis->m10_movieDatBuffer = pDatBuf;
        if (!pDatBuf)
        {
            pThis->getTask()->m14_flags |= 1;
            return;
        }

        memcpy_dma(townBuffer, (u8*)pThis->m10_movieDatBuffer, pThis->m14_movieDatSize);
        loadFile("MOVIE.DAT", townBuffer, 0);
        addToMemoryLayout(townBuffer, 1);

        void* pVdp1Buf = allocateHeapForTask(pThis, 0x8000);
        pThis->m18_vdp1Buffer = pVdp1Buf;
        if (!pVdp1Buf)
        {
            pThis->getTask()->m14_flags |= 1;
            return;
        }

        memcpy_dma(getVdp2Vram(0x70000), (u8*)pThis->m18_vdp1Buffer, 0x8000);
        pThis->m4_savedVblank = (u8)vblankData.m14_numVsyncPerFrame;
        vblankData.m14_numVsyncPerFrame = 1;
        createSubTask<s_videoDecoderNotifyWorkArea>(pThis);
        setupMovieVdp2();
    }

    // 060540fa
    static void Draw(s_movieMainWorkArea* pThis)
    {
        switch (pThis->m0_state)
        {
        case 0:
            pThis->mC_subtask = createSubTaskWithArg<s_subtitleDataWorkArea>(
                pThis, (s32)pThis->m2_cpkIndex);
            // If subtitle task immediately killed itself (no subtitle data), clear the pointer
            if (pThis->mC_subtask && (pThis->mC_subtask->getTask()->m14_flags & 1))
                pThis->mC_subtask = nullptr;
            if (pThis->m2_cpkIndex == 0x2E) // debug-only overlay
            {
                createSubTask<s_subtitleOverlayWorkArea>(pThis);
            }
            openMovieStream(g_cpkFileNames[pThis->m2_cpkIndex]);
            if (g_fadeControls.m_4C <= g_fadeControls.m_4D)
            {
                vdp2Controls.m20_registers[1].m112_CLOFSL = 0;
                vdp2Controls.m20_registers[0].m112_CLOFSL = 0;
            }
            pThis->m6_lastPitch = -1;
            pThis->mA_timer     = -1;
            pThis->m0_state     = 1;
            return;

        case 1:
            pThis->m0_state = 2;
            return;

        case 2:
        {
            u32 fromColor = convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color);
            fadePalette(&g_fadeControls.m0_fade0, fromColor,
                        (s32)(s16)g_fadeControls.m_48, 0x1E);
            pThis->m0_state = 3;
            [[fallthrough]];
        }
        case 3:
            if (!fileInfoStruct.mC_gfsHandle)
            {
                // Movie stream finished — kill subtitle task and advance to state 4
                if (pThis->mC_subtask)
                {
                    if (!(pThis->mC_subtask->getTask()->m14_flags & 1))
                        pThis->mC_subtask->getTask()->markFinished();
                }
                // Note: do NOT reset m3_countdown here — the table value is used
                // by state 4 to determine how many sequential CPKs to play
            }
            else
            {
                // Analog pitch control (if analog pad connected)
                if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m0_inputType == 2)
                {
                    s32 pitch = ((s32)(u8)graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m4 -
                                 (s32)(u8)graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m5)
                                * 4 + 0x400;
                    if (pThis->m6_lastPitch != (s16)pitch)
                    {
                        pThis->m6_lastPitch = (s16)pitch;
                        Unimplemented(); // FUN_0603763c(fileInfoStruct.m10_movieDecoder, pitch, 1)
                    }
                }

                // B button or F7: skip / fade out
                if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 8) != 0
                    || readKeyboardToggle(0x87) != 0)
                {
                    pThis->m3_countdown = 1;
                    u32 fromColor = convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color);
                    fadePalette(&g_fadeControls.m0_fade0, fromColor,
                                (s32)(s16)pThis->m8_fadePalTarget, 0x1E);
                    closeMovieStream();
                }
                else
                {
                    if (!pThis->m1_debugMode)
                        break;

                    // Debug: left/right cycle CPK; F3 opens editor
                    if ((graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 7) == 0)
                    {
                        if (readKeyboardToggle(0x86) != 0)
                        {
                            killTask(pThis->mC_subtask);
                            closeMovieStream();
                            pThis->mC_subtask = createSubTaskWithArg<s_movieEditorWorkArea>(
                                pThis, (s32)pThis->m2_cpkIndex);
                            pThis->m0_state = 7;
                            return;
                        }
                        break;
                    }

                    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 4)
                        pThis->m2_cpkIndex -= 2;
                    if (graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 1)
                        pThis->m2_cpkIndex -= 1;

                    killTask(pThis->mC_subtask);
                    pThis->mC_subtask = nullptr;
                    closeMovieStream();
                }
            }
            // Increment state (state 2→3, 3→4)
            pThis->m0_state++;
            break;

        case 4:
            // Countdown check: decrement and if done, fade out
            if (!pThis->m1_debugMode)
            {
                u8 cd = pThis->m3_countdown;
                pThis->m3_countdown = cd - 1;
                if ((s8)(cd - 1) < 1)
                {
                    u32 fromColor = convertColorToU32ForFade(g_fadeControls.m0_fade0.m0_color);
                    fadePalette(&g_fadeControls.m0_fade0, fromColor,
                                (s32)(s16)pThis->m8_fadePalTarget, 0x1E);
                    pThis->m0_state = 5;
                    break;
                }
            }

            // Cycle to next CPK file on disc (used for multi-CPK sequences and debug)
            {
                u8 cpkIdx = pThis->m2_cpkIndex;
                if (cpkIdx == 0 || cpkIdx == 1 || cpkIdx == 0x2D)
                {
                    VDP2Regs_.m114_COAR = 0x100;
                    VDP2Regs_.m116_COAG = 0x100;
                    VDP2Regs_.m118_COAB = 0x100;
                    clearMovieVdp2Vram();
                    VDP2Regs_.m114_COAR = 0;
                    VDP2Regs_.m116_COAG = 0;
                    VDP2Regs_.m118_COAB = 0;
                }
                do
                {
                    cpkIdx = pThis->m2_cpkIndex;
                    pThis->m2_cpkIndex = cpkIdx + 1;
                    if ((s8)(cpkIdx + 1) < 0x32)
                    {
                        if ((s8)pThis->m2_cpkIndex < 0)
                            pThis->m2_cpkIndex = 0x31;
                    }
                    else
                    {
                        pThis->m2_cpkIndex = 0;
                    }
                } while (!findFileOnDisc(g_cpkFileNames[pThis->m2_cpkIndex]));

                if (pThis->m2_cpkIndex != 0x2E)
                {
                    pThis->m0_state = 0;
                    break;
                }
                pThis->mA_timer = 3;
                pThis->m0_state = 8;
            }
            break;

        case 5:
            if (g_fadeControls.m0_fade0.m20_stopped == 0)
                break;

            if (pThis->m2_cpkIndex == 0x2E)
            {
                pThis->mA_timer = 0;
                pThis->m0_state = (u8)0x78; // 120 frames (sentinel for "done")
            }
            else
            {
                pThis->mA_timer = (pThis->m2_cpkIndex == 0x2F) ? 0x3C : 0;
                pThis->m0_state = 6;
            }
            break;

        case 6:
        {
            s16 timer = pThis->mA_timer;
            pThis->mA_timer = timer - 1;
            if (timer - 1 < 0)
                pThis->getTask()->m14_flags |= 1;
            break;
        }

        case 7:
            if (!pThis->mC_subtask
                || (pThis->mC_subtask->getTask()->m14_flags & 1))
            {
                setupMovieVdp2();
                pThis->m0_state = 0;
            }
            break;

        case 8:
        {
            s16 timer = pThis->mA_timer;
            pThis->mA_timer = timer - 1;
            if (timer - 1 < 1)
                pThis->m0_state = 0;
            if ((s32)graphicEngineStatus.m4514.m0_inputDevices[0].m0_current.m8_newButtonDown & 0xF807)
                pThis->m0_state = 0;
            break;
        }

        default:
            break;
        }
        // Update VDP2 window from current video dimensions
        {
            u32 w = fileInfoStruct.m20_videoWidth;
            u32 h = fileInfoStruct.m24_videoHeight;
            s_VDP2Regs* pRegs = vdp2Controls.m4_pendingVdp2Regs;
            pRegs->mC0_WPSX0 = (s16)(((s32)((0x160 - w) + (u32)(0x160 - w < 0)) >> 1) << 1);
            pRegs->mC2_WPSY0 = (s16)((s32)((0xE0 - h)  + (u32)(0xE0 - h < 0))  >> 1);
            pRegs->mC4_WPEX0 = (s16)(((s32)((w + 0x15F) + (u32)(w + 0x15F < 0)) >> 1) << 1);
            pRegs->mC6_WPEY0 = (s16)((s32)((h + 0xDF)  + (u32)(h + 0xDF < 0))  >> 1);
        }
    }

    // 06054514
    static void Delete(s_movieMainWorkArea* pThis)
    {
        if (fileInfoStruct.mC_gfsHandle)
            closeMovieStream();

        if (pThis->m10_movieDatBuffer)
        {
            memcpy_dma((u8*)pThis->m10_movieDatBuffer, townBuffer, pThis->m14_movieDatSize);
            freeHeapForTask(pThis, pThis->m10_movieDatBuffer);
        }
        if (pThis->m18_vdp1Buffer)
        {
            memcpy_dma((u8*)pThis->m18_vdp1Buffer, getVdp2Vram(0x70000), 0x8000);
            freeHeapForTask(pThis, pThis->m18_vdp1Buffer);
        }
        vblankData.m14_numVsyncPerFrame = (u32)pThis->m4_savedVblank;
    }
};

// ============================================================================
// overlayStart_MOVIE  –  MOVIE::06054000
// ============================================================================

// MOVIE::06054000
p_workArea overlayStart_MOVIE(p_workArea pWorkArea, s32 movieIndex)
{
    return createSubTaskWithArg<s_movieMainWorkArea>(pWorkArea, movieIndex);
}

// ============================================================================
// Base-binary movie wrapper task  –  0602a0b2 / 0602a14c
// Wraps the overlay call; when overlay finishes, triggers title screen.
// ============================================================================

struct s_movieWrapperWorkArea : public s_workAreaTemplate<s_movieWrapperWorkArea>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = {&Init, nullptr, &Draw, nullptr};
        return &taskDefinition;
    }

    p_workArea m8_overlayTask;

    // 0602a0b2
    static void Init(s_movieWrapperWorkArea* pThis)
    {
        resetTempAllocators();
        pThis->m8_overlayTask = overlayStart_MOVIE(pThis, 0x20);
    }

    // 0602a14c
    static void Draw(s_movieWrapperWorkArea* pThis)
    {
        if (!pThis->m8_overlayTask
            || (pThis->m8_overlayTask->getTask()->m14_flags & 1))
        {
            initialTaskStatus.m_pendingTask = createTitleScreenTask;
        }
    }
};

// ============================================================================
// startTitleScreenVideo  –  0602a166
// ============================================================================

// 0602a166
p_workArea startTitleScreenVideo(p_workArea pWorkArea)
{
    return createSubTask<s_movieWrapperWorkArea>(pWorkArea);
}

// ============================================================================
// loadMovieOverlay  –  0602773e
// ============================================================================

// 0602773e
p_workArea loadMovieOverlay(p_workArea pWorkArea, s32 movieIndex)
{
    return overlayStart_MOVIE(pWorkArea, movieIndex);
}

// ============================================================================
// lastUpdateFunction  –  060108a8
// Per-frame file I/O / movie decoder polling.
// On Saturn this polls async CD reads and the CPK decoder; on PC file loads
// are synchronous so states 2 (file loading) and 4 (CD audio) are stubs.
// ============================================================================

// 060108a8
u32 lastUpdateFunction()
{
    // Saturn: CDC_GetHirqReq() & 0x20 → FUN_0603a008(0)  — CD interrupt handling, skip on PC

    if (fileInfoStruct.m0 == 0)
    {
        if (fileInfoStruct.m2C_allocatedHead != nullptr)
        {
            fileInfoStruct.m0 = 2;
            goto state2;
        }
        if (fileInfoStruct.mC_gfsHandle == nullptr)
        {
            if (fileInfoStruct.m1 == 0 && fileInfoStruct.m2 == 0)
                goto done;
            fileInfoStruct.m0 = 4;
            goto state4;
        }
        fileInfoStruct.m0 = 3;
        goto state3;
    }
    else if (fileInfoStruct.m0 == 2)
    {
state2:
        // Saturn: polls GFS async file loading (GFS_NwExecOne / GFS_NwGetStat)
        // On PC, file loading is synchronous — m2C_allocatedHead is always null here.
        // If somehow we get here, just clear state.
        if (fileInfoStruct.m2C_allocatedHead == nullptr)
        {
            if (enableDebugTask)
            {
                vdp2DebugPrintSetPosition(0xC, 1);
                clearVdp2TextSmallFont();
            }
        }
    }
    else if (fileInfoStruct.m0 == 3)
    {
state3:
        // Movie decoder countdown — on Saturn, waits for sound driver heartbeat
        if (fileInfoStruct.m28_countdown != 0)
        {
            // FUN_0602c114: checks soundEngine counter update — stub as true on PC
            fileInfoStruct.m28_countdown = 0;
        }

        if (fileInfoStruct.mC_gfsHandle != nullptr)
        {
            sCinepakDecoder* decoder = (sCinepakDecoder*)fileInfoStruct.m10_movieDecoder;
            if (decoder)
            {
                // Update frame counter from decoder PTS (subtitle timing depends on this).
                // On Saturn, VBL runs at 60fps with numVsyncPerFrame=1.
                // Our engine ticks at 30fps, so we use the PTS directly which
                // already tracks at the STAB sample rate.
                fileInfoStruct.m14_frameCount = decoder->getCurrentPTS();

                // Decode one frame and copy to VDP2 VRAM
                if (decoder->decodeNextFrame())
                {
                    // Update PTS after decode
                    fileInfoStruct.m14_frameCount = decoder->getCurrentPTS();

                    // Copy decoded XRGB8888 frame to VDP2 VRAM
                    u32 w = decoder->getWidth();
                    u32 h = decoder->getHeight();
                    u32 vramOffset = fileInfoStruct.m1C_vramDest - 0x25E00000u;
                    u32* src = decoder->getFrameBuffer();

                    // VDP2 bitmap stride is 512 pixels (2048 bytes at 4 bytes/pixel)
                    for (u32 y = 0; y < h; y++)
                    {
                        u8* dst = getVdp2Vram(vramOffset + y * 2048);
                        memcpy(dst, src + y * w, w * 4);
                    }
                }

                // Check if decoder finished (status 5)
                if (decoder->getStatus() == 5)
                {
                    closeMovieStream();
                }
            }
        }
    }
    else if (fileInfoStruct.m0 == 4)
    {
state4:
        // Saturn: CD audio play / seek commands (CDC_CdPlay, CDC_CdSeek, SCSP commands)
        // Not applicable on PC — sound bank loading is handled by the audio subsystem.
        (void)0;
    }
    else
    {
        goto done;
    }

    fileInfoStruct.m0 = 0;

done:
    u32 result = 0;
    if (readKeyboardToggle(0x104))
    {
        u32 flags = (u32)gCutsceneFlags & 0x86;
        if (flags == 4)
        {
            fileInfoStruct.displayMemoryLayout = (fileInfoStruct.displayMemoryLayout == 0) ? 1 : 0;
            if (fileInfoStruct.displayMemoryLayout)
            {
                displayMemoryLayout();
            }
            else
            {
                clearVdp2TextMemory();
            }
        }
        result = flags;
    }
    return result;
}

// ============================================================================
// createMovieDebugTask  –  0602a314
// ============================================================================

// 0602a314
p_workArea createMovieDebugTask(p_workArea pWorkArea)
{
    resetTempAllocators();
    overlayStart_MOVIE(pWorkArea, -1);
    return nullptr;
}
