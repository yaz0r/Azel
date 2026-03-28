#include "PDS.h"
#include "cinepak.h"
#include "movieAudio.h"

#include <cstring>
#include <cstdlib>

// ============================================================================
// Big-endian read helpers (FILM container is big-endian)
// ============================================================================

static u32 readBE32(const u8* p)
{
    return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | (u32)p[3];
}

static u16 readBE16(const u8* p)
{
    return ((u16)p[0] << 8) | (u16)p[1];
}

static u32 readBE24(const u8* p)
{
    return ((u32)p[0] << 16) | ((u32)p[1] << 8) | (u32)p[2];
}

// ============================================================================
// YUV to XRGB8888 conversion
// ============================================================================

static inline u8 clampByte(s32 v)
{
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (u8)v;
}

static inline u32 yuvToXRGB(u8 y, s8 u, s8 v)
{
    // Cinepak YUV color space conversion
    s32 r = (s32)y + ((s32)v * 2);
    s32 g = (s32)y - ((s32)u / 2) - (s32)v;
    s32 b = (s32)y + ((s32)u * 2);

    return ((u32)clampByte(r) << 16) | ((u32)clampByte(g) << 8) | (u32)clampByte(b);
}

// ============================================================================
// FILM container parser
// ============================================================================

bool sCinepakDecoder::parseFilmHeader()
{
    u8 header[16];

    // Read FILM header
    if (fread(header, 1, 16, m_file) != 16)
        return false;

    if (header[0] != 'F' || header[1] != 'I' || header[2] != 'L' || header[3] != 'M')
        return false;

    u32 headerSize = readBE32(header + 4);
    (void)headerSize;

    // Read FDSC chunk header (8 bytes: 'FDSC' + size)
    u8 fdscHeader[8];
    if (fread(fdscHeader, 1, 8, m_file) != 8)
        return false;

    if (fdscHeader[0] != 'F' || fdscHeader[1] != 'D' || fdscHeader[2] != 'S' || fdscHeader[3] != 'C')
        return false;

    u32 fdscSize = readBE32(fdscHeader + 4);
    u32 fdscDataSize = fdscSize - 8; // size includes the 8-byte header

    u8 fdsc[64];
    memset(fdsc, 0, sizeof(fdsc));
    u32 toRead = (fdscDataSize < 64) ? fdscDataSize : 64;
    if (fread(fdsc, 1, toRead, m_file) != toRead)
        return false;
    // Skip any remaining FDSC data we didn't read
    if (fdscDataSize > toRead)
        fseek(m_file, (long)(fdscDataSize - toRead), SEEK_CUR);

    // FDSC data layout (offsets from start of FDSC data, after 8-byte chunk header):
    // 0-3: video codec ('cvid')
    // 4-7: height
    // 8-11: width
    // 12: audio sample resolution (bits)
    // 13: audio channels
    // 14-15: padding?
    // 16-19: audio sample rate

    // Verify codec is Cinepak ("cvid")
    if (fdsc[0] != 'c' || fdsc[1] != 'v' || fdsc[2] != 'i' || fdsc[3] != 'd')
        return false;

    m_height = readBE32(fdsc + 4);
    m_width  = readBE32(fdsc + 8);
    m_audioChannels   = (u32)fdsc[13];
    m_audioBitDepth   = (u32)fdsc[14];
    m_audioSampleRate = (u32)readBE16(fdsc + 16);

    (void)fdscSize;

    // Read STAB chunk header
    u8 stabHeader[16];
    if (fread(stabHeader, 1, 16, m_file) != 16)
        return false;

    if (stabHeader[0] != 'S' || stabHeader[1] != 'T' || stabHeader[2] != 'A' || stabHeader[3] != 'B')
        return false;

    u32 stabSize    = readBE32(stabHeader + 4);
    m_sampleRate    = readBE32(stabHeader + 8);
    m_sampleCount   = readBE32(stabHeader + 12);

    // Read sample table entries
    m_sampleTable = new sFilmSample[m_sampleCount];
    m_frameCount = 0;

    for (u32 i = 0; i < m_sampleCount; i++)
    {
        u8 entry[16];
        if (fread(entry, 1, 16, m_file) != 16)
            return false;

        m_sampleTable[i].offset = readBE32(entry + 0);
        m_sampleTable[i].length = readBE32(entry + 4);
        m_sampleTable[i].info1  = readBE32(entry + 8);
        m_sampleTable[i].info2  = readBE32(entry + 12);

        // Count video frames (info1 == 0xFFFFFFFF means audio in Sega FILM)
        if (m_sampleTable[i].length > 0 && m_sampleTable[i].info1 != 0xFFFFFFFF)
            m_frameCount++;
    }

    // Compute VBLs per frame from the first video frame's PTS.
    // info1 is a cumulative timestamp in sampleRate ticks (bit 31 may be a flag).
    // The first video frame's PTS gives the per-frame duration (since it starts from 0).
    // Compute VBLs per frame from PTS delta between first two video frames.
    m_ticksPerFrame = 2; // default ~15fps at 30fps engine
    if (m_sampleRate > 0)
    {
        u32 firstPTS = 0;
        bool foundFirst = false;
        for (u32 i = 0; i < m_sampleCount; i++)
        {
            if (m_sampleTable[i].length > 0 && m_sampleTable[i].info1 != 0xFFFFFFFF)
            {
                u32 pts = m_sampleTable[i].info1 & 0x7FFFFFFF;
                if (!foundFirst)
                {
                    firstPTS = pts;
                    foundFirst = true;
                }
                else
                {
                    u32 delta = pts - firstPTS;
                    if (delta > 0)
                        m_ticksPerFrame = delta * 30 / m_sampleRate;
                    if (m_ticksPerFrame == 0)
                        m_ticksPerFrame = 1;
                    break;
                }
            }
        }
    }

    // Data section starts immediately after STAB
    m_dataOffset = (u32)ftell(m_file);

    return true;
}

// ============================================================================
// Create / Destroy
// ============================================================================

sCinepakDecoder* sCinepakDecoder::create(FILE* file)
{
    if (!file)
        return nullptr;

    sCinepakDecoder* dec = new sCinepakDecoder();
    memset(dec, 0, sizeof(sCinepakDecoder));

    dec->m_file = file;
    dec->m_status = 0; // idle

    if (!dec->parseFilmHeader())
    {
        delete dec;
        return nullptr;
    }

    // Allocate frame buffer
    dec->m_frameBuffer = new u32[dec->m_width * dec->m_height];
    memset(dec->m_frameBuffer, 0, dec->m_width * dec->m_height * sizeof(u32));

    // Allocate strip codebook state (will be resized on first frame)
    dec->m_strips = nullptr;
    dec->m_numStrips = 0;

    return dec;
}

void sCinepakDecoder::destroy()
{
    delete[] m_frameBuffer;
    m_frameBuffer = nullptr;
    delete[] m_sampleTable;
    m_sampleTable = nullptr;
    delete[] m_strips;
    m_strips = nullptr;

    // Note: we don't close m_file — the caller (openMovieStream) owns it
    m_file = nullptr;

    delete this;
}

// ============================================================================
// Playback control
// ============================================================================

void sCinepakDecoder::start()
{
    m_currentFrame = 0;
    m_tickCounter = 0;
    m_status = 1; // playing
}

void sCinepakDecoder::stop()
{
    m_status = 5; // finished
    if (m_audioBuffer)
        m_audioBuffer->m_finished = true;
}

// ============================================================================
// Decode one frame
// ============================================================================

bool sCinepakDecoder::decodeNextFrame()
{
    if (m_status != 1)
        return false;

    // Frame pacing: wait m_ticksPerFrame VBLs between decoded frames.
    if (m_tickCounter > 0)
    {
        m_tickCounter--;
        return false;
    }

    // Find next video sample
    while (m_currentFrame < m_sampleCount)
    {
        sFilmSample& sample = m_sampleTable[m_currentFrame];
        m_currentFrame++;

        if (sample.length == 0)
            continue; // skip empty samples

        // In Sega FILM, info1 == 0xFFFFFFFF means audio sample
        if (sample.info1 == 0xFFFFFFFF)
        {
// Decode audio sample into ring buffer
            if (m_audioBuffer && m_audioChannels > 0 && sample.length > 0)
            {
                fseek(m_file, (long)(m_dataOffset + sample.offset), SEEK_SET);
                u8* audioData = new u8[sample.length];
                if (fread(audioData, 1, sample.length, m_file) == sample.length)
                {
                    if (m_audioBitDepth == 16)
                    {
                        // Big-endian signed 16-bit PCM — convert to native s16
                        u32 numSamples = sample.length / 2;
                        u32 canWrite = m_audioBuffer->writeAvailable();
                        u32 toWrite = (numSamples < canWrite) ? numSamples : canWrite;
                        s16* tempBuf = new s16[toWrite];
                        for (u32 i = 0; i < toWrite; i++)
                        {
                            tempBuf[i] = (s16)((audioData[i * 2] << 8) | audioData[i * 2 + 1]);
                        }
                        m_audioBuffer->write(tempBuf, toWrite);
                        delete[] tempBuf;
                    }
                    else if (m_audioBitDepth == 8)
                    {
                        // Unsigned 8-bit PCM — convert to s16
                        u32 numSamples = sample.length;
                        u32 canWrite = m_audioBuffer->writeAvailable();
                        u32 toWrite = (numSamples < canWrite) ? numSamples : canWrite;
                        s16* tempBuf = new s16[toWrite];
                        for (u32 i = 0; i < toWrite; i++)
                        {
                            tempBuf[i] = (s16)((audioData[i] - 128) << 8);
                        }
                        m_audioBuffer->write(tempBuf, toWrite);
                        delete[] tempBuf;
                    }
                }
                delete[] audioData;
            }
            continue;
        }

        // Record current PTS for subtitle timing
        m_currentPTS = sample.info1 & 0x7FFFFFFF;

        // Set pacing delay for next frame
        m_tickCounter = m_ticksPerFrame - 1; // -1 because this tick already counts

        // Seek to sample data
        fseek(m_file, (long)(m_dataOffset + sample.offset), SEEK_SET);

        // Read sample data
        u8* data = new u8[sample.length];
        if (fread(data, 1, sample.length, m_file) != sample.length)
        {
            delete[] data;
            m_status = 5;
            return false;
        }

        // Decode the Cinepak frame
        bool ok = decodeCinepakFrame(data, sample.length);
        delete[] data;

        if (!ok)
        {
            m_status = 5;
            return false;
        }

        return true;
    }

    // No more frames
    m_status = 5;
    return false;
}

// ============================================================================
// Cinepak frame decoder
// ============================================================================

bool sCinepakDecoder::decodeCinepakFrame(const u8* data, u32 size)
{
    if (size < 10)
        return false;

    u8  flags     = data[0];
    u32 frameSize = readBE24(data + 1);
    u32 width     = readBE16(data + 4);
    u32 height    = readBE16(data + 6);
    u32 numStrips = readBE16(data + 8);

    (void)flags;
    (void)width;
    (void)height;

    // Reallocate strips if needed
    if (numStrips > m_numStrips)
    {
        delete[] m_strips;
        m_strips = new sCinepakStrip[numStrips];
        memset(m_strips, 0, numStrips * sizeof(sCinepakStrip));
        m_numStrips = numStrips;
    }

    // Sega FILM Cinepak has extra "skip bytes" after the 10-byte frame header.
    // Detect using the encoded size vs actual size mismatch (per FFmpeg cinepak.c).
    u32 skipBytes = 0;
    if (frameSize != size && (size % frameSize) != 0)
    {
        if (size >= 16 && data[10] == 0xFE && data[11] == 0x00 &&
            data[12] == 0x00 && data[13] == 0x06 &&
            data[14] == 0x00 && data[15] == 0x00)
            skipBytes = 6;
        else
            skipBytes = 2;
    }

    const u8* ptr = data + 10 + skipBytes;
    const u8* end = data + size;
    u32 stripTop = 0;

    for (u32 s = 0; s < numStrips && ptr + 12 <= end; s++)
    {
        // Strip header: 1-byte ID, 3-byte size (includes 12-byte header),
        // then y1(2), x1(2), y2(2), x2(2)
        // u8  stripId    = ptr[0];
        u32 stripSize     = readBE24(ptr + 1); // includes 12-byte header
        u16 topY          = readBE16(ptr + 4);
        // u16 topX       = readBE16(ptr + 6);
        u16 bottomY       = readBE16(ptr + 8);
        // u16 bottomX    = readBE16(ptr + 10);

        u32 stripDataSize = (stripSize > 12) ? (stripSize - 12) : 0;

        // Clamp to available data
        if (ptr + 12 + stripDataSize > end)
            stripDataSize = (u32)(end - ptr - 12);

        u32 stripHeight = (u32)bottomY - (u32)topY;
        if (!topY)
            stripHeight = bottomY - stripTop; // handle topY==0 case like FFmpeg

        if (stripDataSize > 0)
        {
            decodeStrip(&m_strips[s], ptr + 12, stripDataSize, stripTop, stripHeight);
        }

        stripTop += stripHeight;
        ptr += 12 + stripDataSize;
    }

    return true;
}

// ============================================================================
// Strip decoder — processes chunks within a strip
// ============================================================================

void sCinepakDecoder::decodeStrip(sCinepakStrip* strip, const u8* data, u32 size,
                                   u32 stripTop, u32 stripHeight)
{
    const u8* ptr = data;
    const u8* end = data + size;

    while (ptr + 4 <= end)
    {
        // Chunk header: 1-byte ID, 3-byte size (includes 4-byte header)
        u8  chunkId   = ptr[0];
        s32 chunkSize = (s32)readBE24(ptr + 1) - 4;

        if (chunkSize < 0)
            break;

        const u8* chunkData = ptr + 4;
        u32 chunkDataSize = ((chunkData + chunkSize) > end) ? (u32)(end - chunkData) : (u32)chunkSize;

        switch (chunkId)
        {
        case 0x20: // V4 codebook (intra, full)
            decodeCodebook(strip->v4Codebook, chunkData, chunkDataSize, false);
            break;
        case 0x21: // V4 codebook (inter, partial)
            decodeCodebook(strip->v4Codebook, chunkData, chunkDataSize, true);
            break;
        case 0x22: // V1 codebook (intra, full)
            decodeCodebook(strip->v1Codebook, chunkData, chunkDataSize, false);
            break;
        case 0x23: // V1 codebook (inter, partial)
            decodeCodebook(strip->v1Codebook, chunkData, chunkDataSize, true);
            break;
        case 0x30: // V1-only image data
        case 0x31: // V4-only image data (rare)
        case 0x32: // Mixed V1/V4 image data with bitmask
            decodeVectors(strip, chunkData, chunkDataSize, stripTop, stripHeight, chunkId);
            break;
        default:
            break;
        }

        ptr += 4 + chunkDataSize;
    }
}

// ============================================================================
// Codebook decoder
// ============================================================================

void sCinepakDecoder::decodeCodebook(sCinepakCodebook* codebook, const u8* data, u32 size, bool partial)
{
    const u8* ptr = data;
    const u8* end = data + size;

    if (!partial)
    {
        // Full codebook update: 256 entries, 6 bytes each
        for (u32 i = 0; i < 256 && ptr + 6 <= end; i++)
        {
            codebook[i].y[0] = ptr[0];
            codebook[i].y[1] = ptr[1];
            codebook[i].y[2] = ptr[2];
            codebook[i].y[3] = ptr[3];
            codebook[i].u    = (s8)ptr[4];
            codebook[i].v    = (s8)ptr[5];
            ptr += 6;
        }
    }
    else
    {
        // Partial codebook update: uses 32-bit bitmasks to select which entries to update
        u32 i = 0;
        while (i < 256 && ptr < end)
        {
            if ((i & 31) == 0)
            {
                // Read 32-bit bitmask for next 32 entries
                if (ptr + 4 > end)
                    break;
                u32 mask = readBE32(ptr);
                ptr += 4;

                for (u32 bit = 0; bit < 32 && i < 256; bit++, i++)
                {
                    if (mask & (1u << (31 - bit)))
                    {
                        if (ptr + 6 > end)
                            return;
                        codebook[i].y[0] = ptr[0];
                        codebook[i].y[1] = ptr[1];
                        codebook[i].y[2] = ptr[2];
                        codebook[i].y[3] = ptr[3];
                        codebook[i].u    = (s8)ptr[4];
                        codebook[i].v    = (s8)ptr[5];
                        ptr += 6;
                    }
                }
            }
        }
    }
}

// ============================================================================
// Draw a 2x2 codebook entry at pixel position (x, y) in the frame buffer
// ============================================================================

void sCinepakDecoder::drawCodebookEntry(const sCinepakCodebook* entry, u32 x, u32 y)
{
    if (x + 1 >= m_width || y + 1 >= m_height)
        return;

    u32 stride = m_width;
    u32* row0 = m_frameBuffer + y * stride + x;
    u32* row1 = row0 + stride;

    row0[0] = yuvToXRGB(entry->y[0], entry->u, entry->v);
    row0[1] = yuvToXRGB(entry->y[1], entry->u, entry->v);
    row1[0] = yuvToXRGB(entry->y[2], entry->u, entry->v);
    row1[1] = yuvToXRGB(entry->y[3], entry->u, entry->v);
}

// ============================================================================
// Vector decoder — decodes image data using codebook entries
// ============================================================================

void sCinepakDecoder::decodeVectors(sCinepakStrip* strip, const u8* data, u32 size,
                                     u32 stripTop, u32 stripHeight, u32 chunkId)
{
    // Unified vector decoder matching FFmpeg's cinepak_decode_vectors.
    // Chunk ID bit meanings (for 1-byte Sega FILM IDs):
    //   bit 0 (0x01): inter-frame — has "skip" bitmask (unset blocks keep previous data)
    //   bit 1 (0x02): V1-only — if set, all blocks use V1; if clear, V1/V4 selection bitmask
    //
    // 0x30: intra, V1/V4 mixed (selection bitmask)
    // 0x31: inter (skip bitmask), V1/V4 mixed (selection bitmask)
    // 0x32: intra, all V1
    // 0x33: inter (skip bitmask), all V1

    const u8* ptr = data;
    const u8* end = data + size;

    bool hasSkipMask     = (chunkId & 0x01) != 0;  // inter-frame
    bool allV1           = (chunkId & 0x02) != 0;   // no V1/V4 selection

    // FFmpeg uses a SINGLE shared flag/mask for both skip and selection bits.
    // They're interleaved in the same bitstream: each skipped block consumes
    // 1 bit, each updated block consumes 1 bit (skip) + 1 bit (selection) = 2 bits.
    u32 flag = 0, mask = 0;

    for (u32 blockY = stripTop; blockY < stripTop + stripHeight; blockY += 4)
    {
        for (u32 blockX = 0; blockX < m_width; blockX += 4)
        {
            // Skip mask: for inter-frame chunks, consume 1 bit from shared mask
            if (hasSkipMask)
            {
                if (!(mask >>= 1))
                {
                    if (ptr + 4 > end)
                        return;
                    flag = readBE32(ptr);
                    ptr += 4;
                    mask = 0x80000000;
                }

                // If bit is NOT set, skip this block (keep previous frame data)
                if (!(flag & mask))
                    continue;
            }

            // V1/V4 selection: consume another bit from the SAME shared mask
            if (!allV1)
            {
                if (!(mask >>= 1))
                {
                    if (ptr + 4 > end)
                        return;
                    flag = readBE32(ptr);
                    ptr += 4;
                    mask = 0x80000000;
                }
            }

            if (allV1 || (~flag & mask))
            {
                // V1: one codebook entry covers 4x4 block
                if (ptr >= end)
                    return;
                u8 idx = *ptr++;
                const sCinepakCodebook* cb = &strip->v1Codebook[idx];
                drawCodebookEntry(cb, blockX + 0, blockY + 0);
                drawCodebookEntry(cb, blockX + 2, blockY + 0);
                drawCodebookEntry(cb, blockX + 0, blockY + 2);
                drawCodebookEntry(cb, blockX + 2, blockY + 2);
            }
            else
            {
                // V4: four codebook entries for 4x4 block
                if (ptr + 4 > end)
                    return;
                drawCodebookEntry(&strip->v4Codebook[ptr[0]], blockX + 0, blockY + 0);
                drawCodebookEntry(&strip->v4Codebook[ptr[1]], blockX + 2, blockY + 0);
                drawCodebookEntry(&strip->v4Codebook[ptr[2]], blockX + 0, blockY + 2);
                drawCodebookEntry(&strip->v4Codebook[ptr[3]], blockX + 2, blockY + 2);
                ptr += 4;
            }
        }
    }
}
