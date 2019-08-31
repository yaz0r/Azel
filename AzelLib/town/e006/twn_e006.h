#pragma once

p_workArea overlayStart_TWN_E006(p_workArea pThis, u32 arg);
s32 createEPKPlayer(s32 r4);
s32 e006_scriptFunction_605861eSub0();
s32 setupDragonEntityForCutscene(s32 r4);
s32 e006_scriptFunction_60573d8();
s32 e006_scriptFunction_6056918();

struct sStreamingFile_C8
{
    u32 m0_compressionType;
    p_workArea m4_pOwner;
    s_3dModel* m8_p3dModel;
};

struct sStreamingFile_28
{
    s32 m0;
    s32 m4;
    s32 m8;
    s32 mC;
    s32 m10;
    u32 m14;
    u32 m18;
    s32 m1C;
    sVdp2StringControl* m20_vdp2StringControl;
    u32 m24_currentPositionInFile;
    u32 m28;
    u8* m2C_pBufferWrite;
    u8* m30;
    u8* m34_pBufferRead;
    u8* m38;
    u8* m3C;
    u8* m40;
    u32 m44_headerSize;
    u32 m4C;
    u32 m48;
    u32 m50;
    u32 m54;
    u8* m58;
    u8* m5C_audioBuffer;
    u32 m60;
    u32 m64_sampleIndex;
    u32 m68_writePosition;
    u32 m6C;
    u32 m70;
    u32 m78;
    u32 m7C;
    u32 m80_cpuTimer;
    u32 m84_frameIndex;
    u32 m88;
    u32 m90;
    u32 m94;
    std::array<s32, 2> m98;
};

struct GfsHn
{
    FILE* fHandle;
};

struct sStreamingFile_198
{
    GfsHn m0_gfsHandle;
    u32 m4_transfertSectorSize;
    u32 m8_readIsPending;
    u32 mC_pendingReadSize;
    u32 m10_numBytesReadForCurrentRequest;
    u32 m14_numSectors;
    u32 m18_fileSize;
    //size 0x20?
};

struct sStreamingFile
{
    u8* m0_buffer;
    u32 m4_bufferSize;
    s32 m8;
    s32 m10;
    s32 m14;
    u8* m18_audioBuffer;
    u32 m1C_audioBufferSize;
    s32 m20;
    s32 m24;
    sStreamingFile_28 m28;
    std::array<sStreamingFile_C8, 0x10> mC8_cutsceneEntities;
    u32 m188_camera; // probably in m28
    u32 m18C;
    u32 m190_emptyBytes;
    u8* m194;
    sStreamingFile_198 m198;
    //size 0x1B8?
};

struct sE006Task0 : public s_workAreaTemplateWithArg<sE006Task0, s32>
{
    static TypedTaskDefinition* getTypedTaskDefinition()
    {
        static TypedTaskDefinition taskDefinition = { &sE006Task0::Init, nullptr, &sE006Task0::Draw, &sE006Task0::Delete };
        return &taskDefinition;
    }

    static void Init(sE006Task0* pThis, s32 arg);
    static void cutsceneDrawSub0(sE006Task0* pThis, s32 r5);
    static void Draw(sE006Task0* pThis);
    static void Delete(sE006Task0* pThis);

    sStreamingFile* m0;
    sStreamingFile m4;
    u8* m1D4;
    // size: 0x1d8
};

extern sE006Task0* e006Task0;

s32 getCutsceneFrameIndex(struct sStreamingFile*);

void setupCutsceneModelType2(sStreamingFile* r4, u32 r5, p_workArea r6, s_3dModel* pModel);
