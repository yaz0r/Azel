#pragma once

void initSoundDriver();
void updateSoundInterrupt();
void updateSound();

void loadSoundBanks(s8 unk0, s8 unk1);
void playPCM(p_workArea, u32);
void enqueuePlaySoundEffect(s32 soundIndex, s32 bankIndex, s32 volume, s32 unk);

s32 fadeOutAllSequences();
s32 findSound(s32 soundIndex);
bool isSoundLoadingFinished();
void popSoundSequence(s32 param); // 0602b506

s32 computePositionalSoundVolume(sVec3_FP* pPos);
void startPositionalSound(s32 soundIndex, sVec3_FP* pPos);
void updatePositionalSound(s32 soundIndex, sVec3_FP* pPos);

void setSoundDistanceParams(s32 maxDistance, s16 baseVolume); // 0602c348
void battleLoading_InitSub0();
