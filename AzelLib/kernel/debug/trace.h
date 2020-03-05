#pragma once

#ifdef SHIPPING_BUILD
#define addTraceLog(...);
#define readTraceLog(...);
#define isTraceEnabled() false
#define startTrace(...);
#else
void addTraceLog(const sVec3_FP&, const char* name);
void addTraceLog(const sVec2_FP&, const char* name);
void addTraceLog(const sMatrix4x3&, const char* name);
void addTraceLog(const char* fmt, ...);
void readTraceLog(const char* fmt, u32& value);
void startTrace(const char* name);
bool isTraceEnabled();
#endif
