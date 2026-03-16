#pragma once

void townCamera_update(sCameraTask* pThis);
void townCamera_draw(sCameraTask* pThis);
void townCamera_drawWithPosition(sCameraTask* pThis);
void townCamera_setupLight(sCameraTask* pThis, sSaturnPtr lightData);
s32 townCamera_setupWithPosition(sSaturnPtr arg);
s32 townCamera_setup(s32 r4, s32 r5);
s32 TwnFadeOut(s32 arg0);
s32 TwnFadeIn(s32 arg0);
s32 SetupColorOffset(s32 r4);
u16 computeTimeOfDayColor(s32 time);
