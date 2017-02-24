#pragma once

extern u32 isInMenu2;

extern u32 menuUnk0[8];

void playSoundEffect(int);
bool keyboardIsKeyDown(u8 keycode);
void titleMenuToggleTutorials(struct s_titleMenuEntry* menu1, struct s_titleMenuEntry* menu2);
u32 titleScreenDrawSub1(u32* arrayData);
void resetMenu(u32* arrayData, u32, u32, u32);
void titleScreenDrawSub3(u32);
