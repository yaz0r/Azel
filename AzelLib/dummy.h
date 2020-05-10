#pragma once

#include "task.h"

extern u32 isInMenu2;

void soundFunc(int);
#ifdef SHIPPING_BUILD
#define keyboardIsKeyDown(x) false
#else
bool keyboardIsKeyDown(u8 keycode);
#endif
void titleMenuToggleTutorials(struct s_titleMenuEntry* menu1, struct s_titleMenuEntry* menu2);
void titleScreenDrawSub3(u32);

void DEBUG_setRandomSeed(u32);
void initRandomSeed(u32 init);
u32 randomNumber();

void closeAllOpenFiles();
bool readKeyboardToggle(u16 keyIndex);

