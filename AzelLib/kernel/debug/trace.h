#pragma once

#ifdef SHIPPING_BUILD
#define addTraceLog(...);
#else
void addTraceLog(const char* fmt, ...);
void readTraceLog(const char* fmt, u32& value);
void startTrace(const char* name);
bool isTraceEnabled();
#endif
