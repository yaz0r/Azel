#pragma once

p_workArea createMainDragonMenuTask(p_workArea workArea);
void setVdp2LayerScroll(u32 r4, u32 r5, u32 r6);
void resetVdp2LayersAutoScroll();
void clearVdp2TextArea();
void drawObjectName(const char* string);
void printVdp2Number(s32 number);
void startVdp2LayerScroll(s32 layerId, s32 x, s32 y, s32 numSteps);
