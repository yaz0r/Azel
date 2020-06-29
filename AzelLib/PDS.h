#pragma once

#if defined(__APPLE__)
#include "TargetConditionals.h"
#endif

#define PDS_TOOL

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#define NOMINMAX

#undef TRACE_ENABLE

#if !defined(SHIPPING_BUILD)
#if defined(__APPLE__) && TARGET_OS_SIMULATOR
// can't use trace on simulator
#else
#define TRACY_ENABLE
#define TRACY_CALLSTACK 20
#endif
#endif

#include "Tracy.hpp"

#include <SDL.h>
#include <SDL_syswm.h>

#include <bgfx/bgfx.h>

#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>
extern SoLoud::Soloud gSoloud;

#include <dear-imgui/imgui.h>

#if defined(SHIPPING_BUILD)
#define isShipping() true
#else
#define isShipping() false
#endif

#define GLM_FORCE_LEFT_HANDED
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <string>
#include <sstream>
#include <iostream>

#include <cmath>
#include <unordered_map>
#include <algorithm>

void checkGLImpl(const char*, unsigned int line);
//#define checkGL() checkGLImpl(__FILE__, __LINE__);
#define checkGL()

#if (defined(__ANDROID__)) || (defined(__EMSCRIPTEN__))
    #define USE_GL_ES3
    #define WITH_GL
#else
    #if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV))
        #define USE_GL_ES3
        #define GLES_SILENCE_DEPRECATION
        #define WITH_GL
        //#define WITH_VK
    #elif (defined(__APPLE__) && TARGET_OS_WATCH)
        #define USE_NULL_RENDERER
    #else
        #define USE_GL
        //#define WITH_VK
        #define WITH_GL
    #endif
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

#include "PDS_Logger.h"

struct sSaturnPtr
{
    s32 m_offset;
    struct sSaturnMemoryFile* m_file;

    sSaturnPtr operator + (unsigned int i) const
    {
        sSaturnPtr newPtr = *this;
        newPtr.m_offset += i;
        return newPtr;
    }

    sSaturnPtr operator + (int i) const
    {
        sSaturnPtr newPtr = *this;
        newPtr.m_offset += i;
        return newPtr;
    }

    sSaturnPtr& operator ++ ()
    {
        m_offset++;
        return *this;
    }

    sSaturnPtr operator ++ (int)
    {
        sSaturnPtr result(*this);
        ++(*this);
        return result;
    }

    sSaturnPtr& operator -- ()
    {
        m_offset--;
        return *this;
    }

    sSaturnPtr operator -- (int)
    {
        sSaturnPtr result(*this);
        --(*this);
        return result;
    }


    sSaturnPtr& operator += (unsigned int i)
    {
        this->m_offset += i;
        return *this;
    }
    sSaturnPtr operator - (unsigned int i) const
    {
        sSaturnPtr newPtr = *this;
        newPtr.m_offset -= i;
        return newPtr;
    }
    sSaturnPtr& operator -= (unsigned int i)
    {
        this->m_offset -= i;
        return *this;
    }

    bool operator ==(const sSaturnPtr& b) const
    {
        if (b.m_file != m_file)
            return false;
        if (b.m_offset != m_offset)
            return false;
        return true;
    }

    bool operator !=(const sSaturnPtr& b) const
    {
        if ((b.m_file != m_file) || (b.m_offset != m_offset))
            return true;
        return false;
    }

    bool isNull()
    {
        if (m_offset == 0)
            return true;
        return false;
    }

    static sSaturnPtr& getNull()
    {
        static sSaturnPtr temp;
        temp.m_offset = 0;
        temp.m_file = NULL;
        return temp;
    }

    static sSaturnPtr& createFromRaw(u32 offset, struct sSaturnMemoryFile* pFile = nullptr)
    {
        static sSaturnPtr temp;
        temp.m_offset = offset;
        temp.m_file = pFile;
        return temp;
    }

    u8* getRawPointer();
};

struct sSaturnMemoryFile
{
    std::string m_name;
    u8* m_data;
    u32 m_dataSize;
    u32 m_base;

    sSaturnPtr getSaturnPtr(u32 base)
    {
        sSaturnPtr newPtr;
        newPtr.m_file = this;
        newPtr.m_offset = base;
        return newPtr;
    }

    virtual void init() {}
};

bool findFileOnDisc(const std::string& filename);

#include "fixedPoint.h"

#include "dummy.h"

#include "heap.h"
#include "task.h"
#include "rootTask.h"

#include "VDP1.h"
#include "VDP2.h"
#include "titleScreen.h"

#include "common.h"
#include "renderer.h"
#include "3dEngine.h"
#include "3dEngine_flush.h"
#include "mainMenuDebugTasks.h"
#include "field.h"
#include "LCS.h"

#include "kernel/receiveItemTask.h"

#include "menu_dragon.h"
#include "menu_dragonMorph.h"

#include "3dModels.h"

extern u8 COMMON_DAT[0x98000];

struct sFileInfoSub
{
    sFileInfoSub* pNext; // 0
    char m_fileName[32]; // 4 was file id before
    FILE* fHandle;//8
                  //u32 m_numSectors;//C
    u32 m_fileSize; //10 ?
                    //u32 m_14;//14 ?
    u16 m_18;//18 ? word
    u32 m1A_vdp1Data; //1A ? word
};

struct sFileInfo
{
    u8 m0; //0
    u8 m3; //3
    u8 displayMemoryLayout; //5
    u16 m8; //8

    sFileInfoSub* m2C_allocatedHead; //2C
    sFileInfoSub* freeHead; //30
    sFileInfoSub linkedList[15]; //34
};

extern sFileInfo fileInfoStruct;

extern bool debugEnabled;
extern int enableDebugTask;
extern u8 pauseEngine[8];
extern u32 azelCdNumber;

void WRITE_BE_U16(const void* ptr, u16 value);
void WRITE_BE_U32(const void* ptr, u32 value);

sVec3_FP READ_BE_Vec3(const void* ptr);

u32 READ_BE_U32(const void* ptr);
s32 READ_BE_S32(const void* ptr);
u16 READ_BE_U16(const void* ptr);
s16 READ_BE_S16(const void* ptr);
u8 READ_BE_U8(const void* ptr);
s8 READ_BE_S8(const void* ptr);

u32 getFileSize(const char* fileName);

//void unimplemented(const char* name);

void initVDP1Projection(fixedPoint r4, u32 mode);
void getVdp1ProjectionParams(s16* r4, s16* r5);

s32 setDividend(s32 r4, s32 r5, s32 divisor);

fixedPoint sqrt_F(fixedPoint r4);
s32 sqrt_I(s32 r4);
void initFileLayoutTable();

extern bool hasEncounterData;
extern u8 encounterTaskVar0;

#include "trigo.h"

// Script macro stuff
#if 0
#define WRITE_SCRIPT_U16(value) value & 0xFF, (value >> 8) & 0xFF
#define WRITE_SCRIPT_U32(value) value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF
#if 0
#define WRITE_SCRIPT_POINTER(stringPtr) (u64)stringPtr & 0xFF, ((u64)stringPtr >> 8) & 0xFF, ((u64)stringPtr >> 16) & 0xFF, ((u64)stringPtr >> 24) & 0xFF, ((u64)stringPtr >> 32) & 0xFF, ((u64)stringPtr >> 40) & 0xFF, ((u64)stringPtr >> 48) & 0xFF, ((u64)stringPtr >> 56) & 0xFF
#else 
#define WRITE_SCRIPT_POINTER(stringPtr) WRITE_SCRIPT_U32(0)
#endif

#define op_END() 0x1
#define op_CALL_NATIVE_0(funcPtr) 0xE, 0x0, WRITE_SCRIPT_POINTER(funcPtr)
#define op_CALL_NATIVE_1(funcPtr, arg0) 0xE, 0x1, WRITE_SCRIPT_POINTER(funcPtr), WRITE_SCRIPT_U32(arg0)
#define op_START_CUTSCENE() 0x18
#define op_WAIT(delay) 0x02, WRITE_SCRIPT_U16(delay)
#define op_DISPLAY_DIALOG_STRING(stringPtr) 0x15, WRITE_SCRIPT_POINTER(stringPtr)
#define op_CLEAR_DIALOG_STRING() 0x16
#define op_ADD_CINEMATIC_BARS() 0x19
#define op_END_CUTSCENE() 0x1A
#define op_PLAY_PCM(stringPtr) 0x22, WRITE_SCRIPT_POINTER(stringPtr)
#endif

extern p_workArea(*gFieldOverlayFunction)(p_workArea workArea, u32 arg);
fixedPoint distanceSquareBetween2Points(const sVec3_FP& r4_vertice0, const sVec3_FP& r5_vertice1);

void RendererSetFov(float fovInDegree);
s32 MTH_Product2d(s32(&r4)[2], s32(&r5)[2]);
fixedPoint MulVec2(const sVec2_FP& r4, const sVec2_FP& r5);
void adjustMatrixTranslation(fixedPoint r4);

typedef u16 saturnPackedColor;
typedef std::array<saturnPackedColor, 4> quadColor;
