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

#define KEY_CODE_F3 0x84
#define KEY_CODE_F1 0x85
#define KEY_CODE_F2 0x86
#define KEY_CODE_F12 0x87
#define KEU_CODE_TILDE 0x8E
#define KEY_CODE_PAGE_UP 0x10B
#define KEY_CODE_PAGE_DOWN 0x10C
#define KEY_CODE_PAGE_HOME 0x107
#define KEY_CODE_PAGE_END 0x108
#define KEY_CODE_PRINT_SCREEN 0x104

bool readKeyboardToggle(u16 keyIndex);

