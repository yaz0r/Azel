#pragma once

void initSoundDriver();
void updateSoundInterrupt();
void updateSound();

void playMusic(s8 unk0, s8 unk1);
void playPCM(p_workArea, u32);
void enqueuePlaySoundEffect(s32 soundIndex, s32 bankIndex, s32 volume, s32 unk);

s32 fadeOutAllSequences();
s32 findSound(s32 soundIndex);
