#pragma once

#include <stdio.h>
#include "soloud.h"

class SCSPStream;
class File;

class SCSPStreamInstance : public SoLoud::AudioSourceInstance
{
public:
    SCSPStreamInstance(SCSPStream* aParent);
    virtual unsigned int getAudio(float* aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize);
    //virtual SoLoud::result rewind();
    virtual bool hasEnded();
    virtual ~SCSPStreamInstance();
};

class SCSPStream : public SoLoud::AudioSource
{
public:
    SCSPStream();
    virtual ~SCSPStream();
    virtual SCSPStreamInstance* createInstance();
};

