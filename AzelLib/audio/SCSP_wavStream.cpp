#include "SCSP_wavStream.h"
#include <algorithm>

extern "C" {
#include "ao.h"
#include "eng_ssf/scsp.h"
#include "eng_ssf/sat_hw.h"
#include "eng_ssf/m68k.h"
}


SCSPStreamInstance::SCSPStreamInstance(SCSPStream* aParent)
{

}

SCSPStreamInstance::~SCSPStreamInstance()
{

}

bool SCSPStreamInstance::hasEnded()
{
    return false;
}

unsigned int SCSPStreamInstance::getAudio(float* aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize)
{
    //aSamplesToRead = std::min<int>(aSamplesToRead, 20);
    for (int i=0; i<aSamplesToRead; i++)
    {
        stereo_sample_t sample;
        m68k_execute(((11300000 / 60) / 735) * 2);
        SCSP_Update(NULL, NULL, &sample);

        aBuffer[i] = sample.l / (float)0x8000;
        aBuffer[aBufferSize + i] = sample.r / (float)0x8000;
    }

    return aSamplesToRead;
}

SCSPStream::SCSPStream()
{
    mChannels = 2;
}

SCSPStream::~SCSPStream()
{

}

SCSPStreamInstance* SCSPStream::createInstance()
{
    return new SCSPStreamInstance(this);
}
