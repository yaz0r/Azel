#pragma once

struct s_menuSprite
{
    s16 SRCA;
    s16 SIZE;
    s16 X;
    s16 Y;
};

void drawMenuSprite(const s_menuSprite* r4, s16 r5, s16 r6, u32 r7);
void drawMenuSprite2(const s_menuSprite* r4, s16 r5, s16 r6, u32 r7);
