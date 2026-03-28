#include "PDS.h"
#include "movieAudio.h"

#include <algorithm>
#include <cstring>

// ============================================================================
// Ring buffer
// ============================================================================

void sMovieAudioBuffer::init(u32 channels)
{
    m_channels = channels;
    m_finished = false;
    m_writePos.store(0, std::memory_order_relaxed);
    m_readPos.store(0, std::memory_order_relaxed);
    memset(m_data, 0, sizeof(m_data));
}

void sMovieAudioBuffer::reset()
{
    m_finished = false;
    m_writePos.store(0, std::memory_order_relaxed);
    m_readPos.store(0, std::memory_order_relaxed);
}

u32 sMovieAudioBuffer::writeAvailable() const
{
    u32 w = m_writePos.load(std::memory_order_relaxed);
    u32 r = m_readPos.load(std::memory_order_acquire);
    return BUFFER_SIZE - 1 - ((w - r) & (BUFFER_SIZE - 1));
}

void sMovieAudioBuffer::write(const s16* samples, u32 count)
{
    u32 w = m_writePos.load(std::memory_order_relaxed);
    for (u32 i = 0; i < count; i++)
    {
        m_data[w & (BUFFER_SIZE - 1)] = samples[i];
        w++;
    }
    m_writePos.store(w, std::memory_order_release);
}

u32 sMovieAudioBuffer::readAvailable() const
{
    u32 w = m_writePos.load(std::memory_order_acquire);
    u32 r = m_readPos.load(std::memory_order_relaxed);
    return (w - r) & (BUFFER_SIZE - 1);
}

u32 sMovieAudioBuffer::read(float* aBuffer, u32 aSamplesToRead, u32 aBufferSize, u32 channels)
{
    u32 available = readAvailable();
    // available is in individual s16 samples; we need sample frames (1 frame = channels samples)
    u32 framesAvailable = available / channels;
    u32 framesToRead = std::min(aSamplesToRead, framesAvailable);

    u32 r = m_readPos.load(std::memory_order_relaxed);

    if (channels == 2)
    {
        for (u32 i = 0; i < framesToRead; i++)
        {
            s16 left  = m_data[r & (BUFFER_SIZE - 1)]; r++;
            s16 right = m_data[r & (BUFFER_SIZE - 1)]; r++;
            aBuffer[i]               = left  / 32768.0f;
            aBuffer[aBufferSize + i] = right / 32768.0f;
        }
    }
    else
    {
        // Mono
        for (u32 i = 0; i < framesToRead; i++)
        {
            s16 sample = m_data[r & (BUFFER_SIZE - 1)]; r++;
            aBuffer[i] = sample / 32768.0f;
        }
    }

    m_readPos.store(r, std::memory_order_release);

    // Zero-fill remainder if we ran out of data
    for (u32 i = framesToRead; i < aSamplesToRead; i++)
    {
        aBuffer[i] = 0.0f;
        if (channels == 2)
            aBuffer[aBufferSize + i] = 0.0f;
    }

    return aSamplesToRead;
}

// ============================================================================
// SoLoud AudioSource / Instance
// ============================================================================

MovieAudioStreamInstance::MovieAudioStreamInstance(MovieAudioStream* aParent)
{
    m_buffer = aParent->m_buffer;
}

MovieAudioStreamInstance::~MovieAudioStreamInstance()
{
}

bool MovieAudioStreamInstance::hasEnded()
{
    if (!m_buffer)
        return true;
    // End when movie is done AND buffer is drained
    return m_buffer->m_finished && m_buffer->readAvailable() < m_buffer->m_channels;
}

unsigned int MovieAudioStreamInstance::getAudio(float* aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize)
{
    if (!m_buffer)
    {
        for (unsigned int i = 0; i < aSamplesToRead; i++)
            aBuffer[i] = 0.0f;
        return aSamplesToRead;
    }

    return m_buffer->read(aBuffer, aSamplesToRead, aBufferSize, m_buffer->m_channels);
}

MovieAudioStream::MovieAudioStream()
{
    m_buffer = nullptr;
}

MovieAudioStream::~MovieAudioStream()
{
}

void MovieAudioStream::setup(u32 sampleRate, u32 channels, sMovieAudioBuffer* buffer)
{
    m_buffer = buffer;
    mBaseSamplerate = (float)sampleRate;
    mChannels = channels;
}

MovieAudioStreamInstance* MovieAudioStream::createInstance()
{
    return new MovieAudioStreamInstance(this);
}
