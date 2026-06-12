#pragma once

// Call once per frame after input is finalized, before runTasks()
void updateInputRecorder();

void inputRecorder_startRecording(const char* path);
void inputRecorder_startPlayback(const char* path);

// Null when not installed
extern void (*gOnInputFinalized)();
