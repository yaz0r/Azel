#pragma once

p_workArea createMainDragonMenuTask(p_workArea workArea);
void setVdp2LayerScroll(u32 r4, u32 r5, u32 r6);
void resetVdp2LayersAutoScroll();
void clearVdp2TextArea();
void drawInventoryString(const char* string);
