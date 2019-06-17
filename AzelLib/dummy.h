#pragma once

#include "task.h"

extern u32 isInMenu2;

void soundFunc(int);
void playSoundEffect(int);
bool isSoundPlaying(int);
bool keyboardIsKeyDown(u8 keycode);
void titleMenuToggleTutorials(struct s_titleMenuEntry* menu1, struct s_titleMenuEntry* menu2);
void titleScreenDrawSub3(u32);

void playMusic(u8 unk0, u8 unk1);
void playPCM(p_workArea, u32);
bool soundFunc1();

void DEBUG_setRandomSeed(u32);
void initRandomSeed(u32 init);
u32 randomNumber();

bool readKeyboardToggle(u16 keyIndex);

