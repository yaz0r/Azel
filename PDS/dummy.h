#pragma once

#include "task.h"

extern u32 isInMenu2;

struct sMenuUnk0Sub
{
    u32 m_field0[8];
    u8 m_field20;
};

struct sMenuUnk0
{
    sMenuUnk0Sub m_field0;

    sMenuUnk0Sub m_field24;

    u16 m_48;
    u16 m_4A;
    u8 m_4C;
    u8 m_4D;
};

extern sMenuUnk0 menuUnk0;

void playSoundEffect(int);
bool keyboardIsKeyDown(u8 keycode);
void titleMenuToggleTutorials(struct s_titleMenuEntry* menu1, struct s_titleMenuEntry* menu2);
u32 titleScreenDrawSub1(sMenuUnk0* arrayData);
void resetMenu(sMenuUnk0Sub* arrayData, u32, u32, u32);
void titleScreenDrawSub3(u32);

void playMusic(u8 unk0, u8 unk1);
void playPCM(p_workArea, u32);
bool soundFunc1();

void initRandomSeed(u32 init);
u32 randomNumber();

bool readKeyboardToggle(u8 keyIndex);

