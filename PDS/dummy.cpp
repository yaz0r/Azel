#include "PDS.h"

u32 isInMenu2;

sMenuUnk0 menuUnk0;

void playSoundEffect(int) {}
bool keyboardIsKeyDown(u8 keycode) { return false; }
void titleMenuToggleTutorials(struct s_titleMenuEntry* menu1, struct s_titleMenuEntry* menu2) {}
u32 titleScreenDrawSub1(sMenuUnk0* arrayData) { return 0; }
void fadePalette(sMenuUnk0Sub* arrayData, u32 from, u32 to, u32 steps) {}
void titleScreenDrawSub3(u32) {}
void playMusic(u8 musicNumber, u8 unk1) {}
bool soundFunc1() { return 0; }

void playPCM(p_workArea, u32) {}

void initRandomSeed(u32 init) {}
u32 randomNumber() { return rand(); }
bool readKeyboardToggle(u8 keyIndex) { return false; }

