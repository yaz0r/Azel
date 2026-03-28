#pragma once

#include "soloud.h"
#include <atomic>

struct sCinepakDecoder;

// Ring buffer for movie audio samples (single-producer, single-consumer)
struct sMovieAudioBuffer
{
    static constexpr u32 BUFFER_SIZE = 65536; // samples (not frames), ~1.5s at 22050Hz stereo

    s16  m_data[BUFFER_SIZE];
    std::atomic<u32> m_writePos{0};
    std::atomic<u32> m_readPos{0};
    u32  m_channels;
    bool m_finished; // set when movie ends, so audio stream can drain and stop

    void init(u32 channels);
    void reset();

    // Producer (main thread): write interleaved s16 samples
    u32  writeAvailable() const;
    void write(const s16* samples, u32 count);

    // Consumer (audio thread): read samples as float for SoLoud
    u32  readAvailable() const;
    u32  read(float* aBuffer, u32 aSamplesToRead, u32 aBufferSize, u32 channels);
};

class MovieAudioStream;

class MovieAudioStreamInstance : public SoLoud::AudioSourceInstance
{
public:
    MovieAudioStreamInstance(MovieAudioStream* aParent);
    virtual unsigned int getAudio(float* aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize);
    virtual bool hasEnded();
    virtual ~MovieAudioStreamInstance();

    sMovieAudioBuffer* m_buffer;
};

class MovieAudioStream : public SoLoud::AudioSource
{
public:
    MovieAudioStream();
    virtual ~MovieAudioStream();
    virtual MovieAudioStreamInstance* createInstance();

    void setup(u32 sampleRate, u32 channels, sMovieAudioBuffer* buffer);

    sMovieAudioBuffer* m_buffer;
};
