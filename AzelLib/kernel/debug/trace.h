#pragma once

#ifdef SHIPPING_BUILD
#define addTraceLog(...);
#define readTraceLog(...);
#define isTraceEnabled() false
#define startTrace(...);
#else
void addTraceLog(const s8&, const char* name);
void addTraceLog(const s16&, const char* name);
void addTraceLog(const u16&, const char* name);
void addTraceLog(const fixedPoint&, const char* name);
void addTraceLog(const sVec3_FP&, const char* name);
void addTraceLog(const sVec2_FP&, const char* name);
void addTraceLog(const sMatrix4x3&, const char* name);
void addTraceLog(const char* fmt, ...);
void readTraceLogU8(u8& value, const char* name);
void readTraceLogS8(s8& value, const char* name);
void readTraceLogU16(u16& value, const char* name);
void readTraceLogU32(u32& value, const char* name);
void startTrace(const char* name);
bool isTraceEnabled();
#endif
