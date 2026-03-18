#pragma once

p_workArea nameEntry(p_workArea parent, char* destBuffer);
bool isNameEntryComplete(p_workArea nameEntryTask);
void createNameEntryTask(p_workArea parent);
void createNameEntryFadeOutTask(p_workArea parent, s32 arg);
void preloadNameEntryResources();
