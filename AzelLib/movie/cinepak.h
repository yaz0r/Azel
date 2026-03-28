#pragma once

#include <cstdio>

struct sMovieAudioBuffer;

struct sCinepakCodebook
{
    u8 y[4]; // luminance for 2x2 block
    s8 u;    // chrominance
    s8 v;    // chrominance
};

struct sCinepakStrip
{
    sCinepakCodebook v4Codebook[256]; // detailed: 4 entries per 4x4 block
    sCinepakCodebook v1Codebook[256]; // smooth: 1 entry per 4x4 block
};

struct sFilmSample
{
    u32 offset;   // byte offset from start of data section
    u32 length;   // byte length
    u32 info1;    // duration (video) or sample count (audio)
    u32 info2;    // 0xFFFFFFFF = video keyframe, other = interframe or audio
};

struct sCinepakDecoder
{
    FILE*   m_file;
    u32     m_dataOffset;      // file offset where frame data begins (after STAB)

    u32     m_width;
    u32     m_height;

    u32     m_frameCount;      // total frames in file
    u32     m_currentFrame;    // next frame to decode

    sFilmSample* m_sampleTable;
    u32          m_sampleCount;

    sCinepakStrip* m_strips;   // persistent codebook state (one per strip)
    u32            m_numStrips;

    u32*    m_frameBuffer;     // decoded XRGB8888, width * height pixels

    s32     m_status;          // 0=idle, 1=playing, 5=finished
    u32     m_audioChannels;   // 0 = no audio track in FILM
    u32     m_audioSampleRate; // audio sample rate from FDSC (e.g., 22050)
    u32     m_audioBitDepth;   // audio sample size in bits (8 or 16)
    u32     m_sampleRate;      // STAB sample rate (ticks per second)
    u32     m_ticksPerFrame;   // VBL ticks between frames (computed from first video PTS)
    u32     m_tickCounter;     // VBL tick countdown for frame pacing
    u32     m_currentPTS;      // PTS of last decoded video frame (in STAB sampleRate ticks)

    sMovieAudioBuffer* m_audioBuffer; // ring buffer for decoded audio (owned externally)

    // Create decoder from an already-opened file. Returns nullptr on failure.
    static sCinepakDecoder* create(FILE* file);
    void destroy();

    // Decode one frame if enough time has passed. Returns true if a frame was decoded.
    // Call once per VBL (engine tick). Handles frame rate pacing internally.
    bool decodeNextFrame();

    // Get decoded frame buffer (XRGB8888, width * height pixels)
    u32* getFrameBuffer() const { return m_frameBuffer; }
    u32  getWidth() const { return m_width; }
    u32  getHeight() const { return m_height; }
    u32  getTotalFrames() const { return m_frameCount; }
    s32  getStatus() const { return m_status; }

    void start();
    void stop();

    void setAudioBuffer(sMovieAudioBuffer* buffer) { m_audioBuffer = buffer; }
    u32  getAudioSampleRate() const { return m_audioSampleRate; }
    u32  getAudioChannels() const { return m_audioChannels; }
    u32  getCurrentPTS() const { return m_currentPTS; }

private:
    bool parseFilmHeader();
    bool decodeCinepakFrame(const u8* data, u32 size);
    void decodeStrip(sCinepakStrip* strip, const u8* data, u32 size,
                     u32 stripTop, u32 stripHeight);
    void decodeCodebook(sCinepakCodebook* codebook, const u8* data, u32 size, bool partial);
    void decodeVectors(sCinepakStrip* strip, const u8* data, u32 size,
                       u32 stripTop, u32 stripHeight, u32 chunkId);
    void drawCodebookEntry(const sCinepakCodebook* entry, u32 x, u32 y);
};
