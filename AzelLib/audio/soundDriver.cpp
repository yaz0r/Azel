#include "PDS.h"
#include "soundDriver.h"
#include "soundDataTable.h"
#include "commonOverlay.h"

extern "C" {
#include "ao.h"
#include "eng_ssf/m68k.h"
#include "eng_ssf/scsp.h"
#include "eng_ssf/sat_hw.h"
}

#include "SCSP_wavStream.h"

#define soundCounter_2msec (m68k_read_memory_8(0x8827))
#define timmingFlag (m68k_read_memory_8(0x4E0))

struct sSoundEngineSubSub
{
    s16 m0_soundIndex;
    s16 m2_volume;
    s8 m4_isStarted;
    s8 m5;
};

struct sSoundEngineSub
{
    std::array<sSoundEngineSubSub, 8> m0;
    s16 m30;
    s8 m32;
    s8 m33;
};

struct sSoundCommandQueueEntry
{
    s16 m0_soundIndex;
    s8 m2_bankIndex;
    s8 m3_volume;
    s8 m4;
    s8 m5_unk;
};

struct sSCSPCommand
{
    s8 m0;
    s8 m1_reserved;
    s8 m2_P1;
    s8 m3_P2;
    s8 m4_P3;
    s8 m5_P4;
    s8 m6_P5;
    s8 m7;
    s8 m8;
    s8 m9;
    s8 mA;
    s8 mB;
    s8 mC;
    s8 mD;
    s8 mE;
    s8 mF;

    sSCSPCommand()
    {
        memset(this, 0, sizeof(sSCSPCommand));
    }
};

struct sSoundEngine
{
    std::array<u32, 4> m0_sequenceStatus;
    std::array<sSoundEngineSub, 8> m10_sequenceTable;
    const sSequenceConfig* m1B0_pSequenceData;
    const sSequenceConfig* m1B4_pPreviousSequenceData;
    s16 m1B8_numActiveSequence;
    s16 m1BA;
    std::array<sSCSPCommand, 0x40> m1BC_SCSPCommands;
    s16 m5BC_numEnqueuedSCSPCommands;
    s16 m5BE_numMaxEnqueuedSCSPCommands;
    s16 m5C0_SCSPCurrentCommandEntry;
    std::array<sSoundCommandQueueEntry, 32> m5C2_soundCommandQueue;
    s16 m682_soundCommandQueueSize;
    s32 m684;
    s16 m688;
    s8 m68A_currentSoundLoadingState;
    s8 m68B;
    s8 m68C;
    s8 m68D;
    s8 m68E;
    s8 m68F_updateSoundLoadingState;
    s8 m690_waitForM68Ack;
    s8 m6C2_soundCounter;
    s8 m6C1_previousSoundCounter;
};

void resetSequence(sSoundEngineSub* pThis)
{
    pThis->m30 = -1;
    pThis->m32 = 1;
    pThis->m33 = 0;

    for (int i=0; i<8; i++)
    {
        pThis->m0[i].m0_soundIndex = -1;
        pThis->m0[i].m2_volume = 0x80;
    }
}

void resetSequenceTable(sSoundEngine* pThis)
{
    for (int i=0; i<8; i++)
    {
        resetSequence(&pThis->m10_sequenceTable[i]);
    }
}

sSoundEngine soundEngine;

void resetSoundEngine(sSoundEngine* pThis)
{
    pThis->m1B8_numActiveSequence = 0;
    pThis->m1BA = 0;
    resetSequenceTable(pThis);
    pThis->m5BC_numEnqueuedSCSPCommands = 0;
    pThis->m5BE_numMaxEnqueuedSCSPCommands = 0;
    pThis->m682_soundCommandQueueSize = 0;
    pThis->m1B0_pSequenceData = nullptr;
    pThis->m1B4_pPreviousSequenceData = nullptr;
    pThis->m68C = 0;
    pThis->m5C0_SCSPCurrentCommandEntry = 0;
    pThis->m68A_currentSoundLoadingState = -1;
    pThis->m68B = 0;
    pThis->m68E = 0;
    pThis->m68F_updateSoundLoadingState = 0;
    pThis->m690_waitForM68Ack = 0;
    pThis->m6C2_soundCounter = 0;
    pThis->m6C1_previousSoundCounter = 0;
    soundEngine.m684 = 0x200000;
    soundEngine.m688 = 100;
}

void initSoundDriver()
{
    resetSoundEngine(&soundEngine);
    /*
    while (SMPC_Status & 1);

    SMPC_Status = 1;
    SMPC_Command = 7; // SOUND_OFF (stop 68000)

    while (SMPC_Status & 1);
    */

    SCSP.udata.data[0] = 2; // DAC18B = 0, MEM4MB = 1

    memset(sat_ram, 0, 0xB000);

    loadFile("SDDRVS.TSK", sat_ram, 0);
    loadFile("AREAMAP.SND", sat_ram + 0x0a000, 0);

    m68k_write_memory_8(0x4E1, m68k_read_memory_8(0x4E1) | 0x80);

    //while (SMPC_Status & 1);

    FunctionUnimplemented();


    /////// init the scsp emulation
    sat_hw_init();

    static SCSPStream gSCSPStream;
    gSoloud.play(gSCSPStream);
}

void getSequenceDataSub(sSoundEngineSub& entry)
{
    for (int i=0; i<8; i++)
    {
        entry.m0[i].m2_volume = 0x80;
    }
}

void getSequenceData(sSoundEngine* pSoundEngine, s32 sequenceNumber, s32 unk, s32 destIndex)
{
    sSoundEngineSub& entry = pSoundEngine->m10_sequenceTable[destIndex];
    s32 entryIndex = entry.m30;
    pSoundEngine->m1B4_pPreviousSequenceData = pSoundEngine->m1B0_pSequenceData;
    pSoundEngine->m1B0_pSequenceData = &SoundDataTable[sequenceNumber];
    entry.m33 = 0;
    entry.m30 = sequenceNumber;
    entry.m33 = unk;

    getSequenceDataSub(entry);

    pSoundEngine->m1BA = 0;
    if ((entryIndex < 0) || (pSoundEngine->m1B0_pSequenceData->m8_areaMapIndex != SoundDataTable[entryIndex].m8_areaMapIndex))
    {
        pSoundEngine->m68A_currentSoundLoadingState = 0;
    }
    else {
        if (entryIndex == sequenceNumber) {
            pSoundEngine->m68A_currentSoundLoadingState = 8;
        }
        else {
            pSoundEngine->m68A_currentSoundLoadingState = 2;
        }
    }
    pSoundEngine->m68D = 0;
}

void loadSoundBanks(s8 musicNumber, s8 unk1)
{
    if (soundEngine.m10_sequenceTable[soundEngine.m1B8_numActiveSequence].m33 == 0)
    {
        if (musicNumber < 0)
        {
            soundEngine.m10_sequenceTable[soundEngine.m1B8_numActiveSequence].m30 = -1;
        }
        else
        {
            getSequenceData(&soundEngine, musicNumber, unk1, soundEngine.m1B8_numActiveSequence);
            soundEngine.m68B = 0;
            soundEngine.m68F_updateSoundLoadingState = 1;
        }
    }
    else
    {
        assert(0);
    }

    static SoLoud::WavStream gWave;

    switch (musicNumber)
    {
    case 1:
        //gWave.load("1.OGG");
       // gSoloud.play(gWave);
        break;
    case 75:
        //gWave.load("75.OGG");
       // gSoloud.play(gWave);
        break;
    }
    PDS_unimplemented("playMusic");
}

s32 fadeOutAllSequences()
{
    //gSoloud.stopAll();
    return 0;
}

struct sPlayPCM : public s_workAreaTemplate<sPlayPCM>
{
    s16 m0_soundId;
    s32 m4_volume = 0x7F;
    // size 0x8
};

void playPCMFunction(sPlayPCM* pThis)
{
    if ((soundEngine.m68A_currentSoundLoadingState < 0) && (soundEngine.m68E == 0))
    {
        enqueuePlaySoundEffect(pThis->m0_soundId, 1, pThis->m4_volume, 0);
        pThis->getTask()->markFinished();
    }
}

void playPCM(p_workArea parent, u32 id)
{
    sPlayPCM* pNewTask = createSubTaskFromFunction<sPlayPCM>(parent, &playPCMFunction);
    pNewTask->m0_soundId = id;
    pNewTask->m4_volume = 0x7F;
}

void enqueuePlaySoundEffect(s32 soundIndex, s32 bankIndex, s32 volume, s32 unk)
{
    if ((soundIndex > -1) && (soundEngine.m682_soundCommandQueueSize < 0x20))
    {
        sSoundCommandQueueEntry& entry = soundEngine.m5C2_soundCommandQueue[soundEngine.m682_soundCommandQueueSize++];
        entry.m0_soundIndex = soundIndex;
        entry.m2_bankIndex = bankIndex;
        entry.m3_volume = volume;
        entry.m4 = unk;
    }
}

void writeSCSPCommand(sSoundEngine* pSoundEngine, sSCSPCommand* command)
{
    s32 outputOffset = 0x700 + 0x10 * pSoundEngine->m5C0_SCSPCurrentCommandEntry;
    m68k_write_memory_8(outputOffset + 0x0, command->m0);
    m68k_write_memory_8(outputOffset + 0x1, command->m1_reserved);
    m68k_write_memory_8(outputOffset + 0x2, command->m2_P1);
    m68k_write_memory_8(outputOffset + 0x3, command->m3_P2);
    m68k_write_memory_8(outputOffset + 0x4, command->m4_P3);
    m68k_write_memory_8(outputOffset + 0x5, command->m5_P4);
    m68k_write_memory_8(outputOffset + 0x6, command->m6_P5);
    m68k_write_memory_8(outputOffset + 0x7, command->m7);
    m68k_write_memory_8(outputOffset + 0x8, command->m8);
    m68k_write_memory_8(outputOffset + 0x9, command->m9);
    m68k_write_memory_8(outputOffset + 0xA, command->mA);
    m68k_write_memory_8(outputOffset + 0xB, command->mB);
    m68k_write_memory_8(outputOffset + 0xC, command->mC);
    m68k_write_memory_8(outputOffset + 0xD, command->mD);
    m68k_write_memory_8(outputOffset + 0xE, command->mE);
    m68k_write_memory_8(outputOffset + 0xF, command->mF);

    pSoundEngine->m5C0_SCSPCurrentCommandEntry++;
    if (pSoundEngine->m5C0_SCSPCurrentCommandEntry > 7)
    {
        pSoundEngine->m5C0_SCSPCurrentCommandEntry = 0;
    }
    pSoundEngine->m690_waitForM68Ack = 1;
}

void sendMapChangeCommand(sSoundEngine* pSoundEngine, s32 map, s32 commandIndex)
{
    s32 outputOffset = 0x700 + 0x10 * pSoundEngine->m5C0_SCSPCurrentCommandEntry;
    m68k_write_memory_8(outputOffset + 0x0, commandIndex);
    m68k_write_memory_8(outputOffset + 0x1, 0);
    m68k_write_memory_8(outputOffset + 0x2, map);

    pSoundEngine->m5C0_SCSPCurrentCommandEntry++;
    if (pSoundEngine->m5C0_SCSPCurrentCommandEntry > 7)
    {
        pSoundEngine->m5C0_SCSPCurrentCommandEntry = 0;
    }
    pSoundEngine->m690_waitForM68Ack = 1;
}

void checkDataAndStopAllSequences(sSoundEngine* pSoundEngine)
{
    FunctionUnimplemented();
}

bool waitForStopSoundCompletion(sSoundEngine* pSoundEngine)
{
    FunctionUnimplemented();

    return 1;
}

bool isSoundFileLoaded(sSaturnPtr pConfig)
{
    return true;
}

void markMapAreaTransfered(sSoundEngine* pSoundEngine)
{
    for (int i=0; i< pSoundEngine->m1B0_pSequenceData->mC_numMapEntries; i++)
    {
        m68k_write_memory_8(0x504 + 8*i, 0x80);
    }
}

void updateSoundLoadingState(sSoundEngine* pSoundEngine)
{
    switch (pSoundEngine->m68A_currentSoundLoadingState)
    {
    case 0:
    {
        sSCSPCommand command;
        command.m0 = 0x10; // sound initial
        command.m2_P1 = 1; // stop all sequence play
        command.m3_P2 = 1; // stop all PCM play
        command.m4_P3 = 1; // stop CD-DA play
        command.m5_P4 = 1; // initialize DSP
        command.m6_P5 = 1; // initialize mixer
        writeSCSPCommand(pSoundEngine, &command);
        pSoundEngine->m68A_currentSoundLoadingState++;
        break;
    }
    case 1:
        sendMapChangeCommand(pSoundEngine, pSoundEngine->m1B0_pSequenceData->m8_areaMapIndex, 0x8);
        pSoundEngine->m68A_currentSoundLoadingState++;
        break;
    case 2:
        checkDataAndStopAllSequences(pSoundEngine);
        pSoundEngine->m68A_currentSoundLoadingState++;
        break;
    case 3:
        if (waitForStopSoundCompletion(pSoundEngine))
        {
            pSoundEngine->m68A_currentSoundLoadingState++;
        }
        break;
    case 4: // wait for data loading to be kicked from main game loop
        break;
    case 5:
        sendMapChangeCommand(pSoundEngine, 0, 0x83); // effect change
        pSoundEngine->m68A_currentSoundLoadingState++;
        break;
    case 6:
        sendMapChangeCommand(pSoundEngine, 0, 0x87); // mixer change
        pSoundEngine->m68A_currentSoundLoadingState++;
        break;
    case 7:
        if (pSoundEngine->m68B)
        {
            // wait for all sounds to be loaded
            sSaturnPtr pConfig = soundEngine.m1B0_pSequenceData->m0;
            do
            {
                if (!isSoundFileLoaded(pConfig))
                {
                    return;
                }
                pConfig += 0xC;
            } while (readSaturnU32(pConfig));
        }
        pSoundEngine->m68A_currentSoundLoadingState++;
        markMapAreaTransfered(pSoundEngine);
        break;
    case 8:
        if (pSoundEngine->m10_sequenceTable[pSoundEngine->m1B8_numActiveSequence].m32 > 1)
        {
            assert(0);
        }
        pSoundEngine->m68A_currentSoundLoadingState = -1;
        pSoundEngine->m68F_updateSoundLoadingState = 0;
        pSoundEngine->m1BA = 0;
        if (pSoundEngine->m68D)
        {
            assert(0);
        }
        pSoundEngine->m68C = 0;
        break;
    case 9:
        break;
    default:
        assert(0);
    }
}

void updateSoundInterrupt()
{
    soundEngine.m6C1_previousSoundCounter = soundEngine.m6C2_soundCounter;
    soundEngine.m6C2_soundCounter = soundCounter_2msec;

    if ((timmingFlag & 0x80) == 0)
    {
        soundEngine.m690_waitForM68Ack = 0;
        soundEngine.m5C0_SCSPCurrentCommandEntry = 0;

        if (soundEngine.m68F_updateSoundLoadingState != 0)
        {
            updateSoundLoadingState(&soundEngine);
        }

        sSCSPCommand* pCommand = &soundEngine.m1BC_SCSPCommands[soundEngine.m5BE_numMaxEnqueuedSCSPCommands];

        do 
        {
            if (soundEngine.m5BE_numMaxEnqueuedSCSPCommands == soundEngine.m5BC_numEnqueuedSCSPCommands)
                break;

            soundEngine.m5BE_numMaxEnqueuedSCSPCommands = (soundEngine.m5BE_numMaxEnqueuedSCSPCommands + 1) & 0x3F;

            if (soundEngine.m5BE_numMaxEnqueuedSCSPCommands == 0)
            {
                pCommand = &soundEngine.m1BC_SCSPCommands[0];
            }
            else
            {
                pCommand += 1;
            }

            writeSCSPCommand(&soundEngine, pCommand);

        } while (soundEngine.m5C0_SCSPCurrentCommandEntry != 0);

        if (soundEngine.m690_waitForM68Ack)
        {
            m68k_write_memory_8(0x4E0, timmingFlag | 0x80);
        }
    }

    FunctionUnimplemented();
}

void getSequencesPlayStatus(sSoundEngine* pSoundEngine)
{
    pSoundEngine->m0_sequenceStatus[0] = m68k_read_memory_32(0x780);
    pSoundEngine->m0_sequenceStatus[1] = m68k_read_memory_32(0x784);
    pSoundEngine->m0_sequenceStatus[2] = m68k_read_memory_32(0x788);
    pSoundEngine->m0_sequenceStatus[3] = m68k_read_memory_32(0x78C);
}

void updateSequenceStatus(sSoundEngine* pSoundEngine)
{
    getSequencesPlayStatus(pSoundEngine);

    if (pSoundEngine->m68A_currentSoundLoadingState < 0)
    {
        //assert(0);
        FunctionUnimplemented();
    }
}

sSaturnPtr getConfigForSound(sSoundEngine* pSoundEngine, s16 soundIndex)
{
    if (soundIndex > 99)
    {
        return pSoundEngine->m1B0_pSequenceData->m4_soundConfigs + (soundIndex - 100) * 4;
    }
    return gCommonFile.getSaturnPtr(0x02131c4) + soundIndex * 4; // systemSoundsConfig
}

void enqueueSCSPCommand(sSCSPCommand* pCommand)
{
    int commandIndex = (soundEngine.m5BC_numEnqueuedSCSPCommands + 1) & 0x3F;
    if (commandIndex == soundEngine.m5BE_numMaxEnqueuedSCSPCommands)
    {
        soundEngine.m5BE_numMaxEnqueuedSCSPCommands = (soundEngine.m5BE_numMaxEnqueuedSCSPCommands + 1) & 0x3F;
    }

    soundEngine.m1BC_SCSPCommands[commandIndex] = *pCommand;

    soundEngine.m5BC_numEnqueuedSCSPCommands = commandIndex;
}

void updateSoundSub(sSoundEngine* pSoundEngine)
{
    sSoundEngineSub& pSequence = pSoundEngine->m10_sequenceTable[pSoundEngine->m1B8_numActiveSequence + pSoundEngine->m1BA];
    for (int i=0; i< pSoundEngine->m682_soundCommandQueueSize; i++)
    {
        sSoundCommandQueueEntry& pSoundCommand = pSoundEngine->m5C2_soundCommandQueue[i];
        if (pSoundCommand.m2_bankIndex == 1)
        {
            if ((pSoundEngine->m68A_currentSoundLoadingState < 0) && (pSoundEngine->m68E == 0))
            {
                sSaturnPtr pSoundConfig = getConfigForSound(pSoundEngine, pSoundCommand.m0_soundIndex);

                s8 sequenceDataBankNumber = readSaturnS8(pSoundConfig + 0);
                s8 soundNumberInSequenceData = readSaturnS8(pSoundConfig + 1);
                s8 soundControlId = readSaturnS8(pSoundConfig + 3);

                if (pSoundCommand.m3_volume != pSequence.m0[sequenceDataBankNumber].m2_volume)
                {
                    sSCSPCommand command;
                    command.m0 = 0x5; // Sequence volume
                    command.m2_P1 = soundControlId;
                    command.m3_P2 = pSoundCommand.m3_volume;
                    command.m4_P3 = 0;
                    enqueueSCSPCommand(&command);
                    pSequence.m0[sequenceDataBankNumber].m2_volume = pSoundCommand.m3_volume;
                }

                sSCSPCommand command;
                command.m0 = 0x1; // Sequence start
                command.m2_P1 = soundControlId;
                command.m3_P2 = sequenceDataBankNumber;
                command.m4_P3 = soundNumberInSequenceData;
                enqueueSCSPCommand(&command);
                pSequence.m0[sequenceDataBankNumber].m0_soundIndex = pSoundCommand.m0_soundIndex;
                pSequence.m0[sequenceDataBankNumber].m4_isStarted = 1;
            }
        }
        else
        {
            s32 foundSound = findSound(pSoundCommand.m0_soundIndex);
            if (foundSound != -1)
            {
                switch (pSoundCommand.m2_bankIndex)
                {
                default:
                    assert(0);
                }
            }
        }
    }
    pSoundEngine->m682_soundCommandQueueSize = 0;
}

sSaturnPtr getPlayerSoundBankName(const sSequenceConfig* pConfig)
{
    switch(pConfig->mD_playerSoundTypes)
    {
    case 0:
        if (mainGameState.gameStats.m1_dragonLevel > 8)
            return readSaturnEA(gCommonFile.getSaturnPtr(0x213D90));
        else
            return readSaturnEA(gCommonFile.getSaturnPtr(0x213D90) + mainGameState.gameStats.m1_dragonLevel * 4);
        break;
    case 1:
        return gCommonFile.getSaturnPtr(0x213dc0);
    case 2:
        return gCommonFile.getSaturnPtr(0x213de4);
    default:
        assert(0);
    }
}

void updateSound()
{
    if (soundEngine.m1B0_pSequenceData == 0)
        return;

    updateSequenceStatus(&soundEngine);
    updateSoundSub(&soundEngine);

    if (soundEngine.m68A_currentSoundLoadingState == 4)
    {
        sSaturnPtr pPlayerConfig = getPlayerSoundBankName(soundEngine.m1B0_pSequenceData);
        sSaturnPtr pConfig = soundEngine.m1B0_pSequenceData->m0;
        if (soundEngine.m68B == 0)
        {
            do 
            {
                std::string filename = readSaturnString(readSaturnEA(pConfig + 0));
                u32 destAddress = readSaturnU32(pConfig + 4);
                loadFile(filename.c_str(), sat_ram + (destAddress - 0x25A00000), 0);
                pConfig += 0xC;
            } while (readSaturnU32(pConfig));

            do
            {
                std::string filename = readSaturnString(readSaturnEA(pPlayerConfig + 0));
                u32 destAddress = readSaturnU32(pPlayerConfig + 4);
                loadFile(filename.c_str(), sat_ram + (destAddress - 0x25A00000), 0);
                pPlayerConfig += 0xC;
            } while (readSaturnU32(pPlayerConfig));
        }
        else
        {
            assert(0);
        }
        soundEngine.m68A_currentSoundLoadingState++;
    }
}

s32 findSound(s32 soundIndex)
{
    sSoundEngineSub& psVar2 = soundEngine.m10_sequenceTable[soundEngine.m1B8_numActiveSequence + soundEngine.m1BA];

    for (int i = 0; i < 8; i++)
    {
        if (psVar2.m0[i].m0_soundIndex == soundIndex)
        {
            return i;
        }
    }

    return -1;
}

extern "C" {
    int m68k_instructionCallback();
}

int m68k_instructionCallback()
{
    u32 PC = m68k_get_reg(nullptr, m68k_register_t::M68K_REG_PC);

    switch (PC)
    {
    case 0x010F4:
        PDS_CategorizedLog(log_m68k, "Start executing command 0x%02X\n", m68k_read_memory_8(0x9ffc));
        switch (m68k_read_memory_8(0x9ffc))
        {
        case 1:
            PDS_CategorizedLog(log_m68k, "P1: 0x%02X\n", m68k_read_memory_8(m68k_get_reg(nullptr, m68k_register_t::M68K_REG_A0) + 2));
            PDS_CategorizedLog(log_m68k, "P2: 0x%02X\n", m68k_read_memory_8(m68k_get_reg(nullptr, m68k_register_t::M68K_REG_A0) + 3));
            PDS_CategorizedLog(log_m68k, "P3: 0x%02X\n", m68k_read_memory_8(m68k_get_reg(nullptr, m68k_register_t::M68K_REG_A0) + 4));
            PDS_CategorizedLog(log_m68k, "P4: 0x%02X\n", m68k_read_memory_8(m68k_get_reg(nullptr, m68k_register_t::M68K_REG_A0) + 5));
            break;
        default:
            break;
        }
        break;
    case 0x1116:
        PDS_CategorizedLog(log_m68k, "Clear timing flag\n");
        break;
    /*case 0x1B56:
        PDS_CategorizedLog(log_m68k, "Jump to command handler\n");
        break;*/
    case 0x10f8:
        if (m68k_read_memory_16(0x416) || m68k_read_memory_32(0x420))
        {
            u16 ErrorStatus = m68k_read_memory_16(0x416);
            u32 ErrorStatusBitMap = m68k_read_memory_32(0x420);
            //assert(0);
        }
        //PDS_CategorizedLog(log_m68k, "Command done\n");
        break;
        /*
    case 0x20da:
        PDS_CategorizedLog(log_m68k, "Start executing command 0x10\n");
        break;
    case 0x20e6:
        PDS_CategorizedLog(log_m68k, "Command 0x10: starting to stop all sequence play\n");
        break;
    case 0x2108:
        PDS_CategorizedLog(log_m68k, "Command 0x10: starting to stop all pcm play\n");
        break;
    case 0x2136:
        PDS_CategorizedLog(log_m68k, "Command 0x10: starting to init DSP\n");
        break;
    case 0x2152:
        PDS_CategorizedLog(log_m68k, "DSP init done\n");
        break;
    case 0x2158:
        PDS_CategorizedLog(log_m68k, "Command 0x10: starting to init mixer\n");
        break;
    case 0x215E:
        PDS_CategorizedLog(log_m68k, "Command 0x10: restore SR!\n");
        break;
    case 0x2160:
        PDS_CategorizedLog(log_m68k, "Command 0x10: done!\n");
        break;*/
    default:
        break;
    }

    return 0;
}

bool isSoundLoadingFinished()
{
    return (u32)(-1 < soundEngine.m68A_currentSoundLoadingState) ^ 1;
}



