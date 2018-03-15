#pragma once

#define _CRT_SECURE_NO_WARNINGS

#ifndef HEADLESS_TOOL
#include <SDL.h>
#endif

#include <imgui.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <algorithm>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint64_t u64;
typedef int64_t s64;

void yLog(...);

struct sSaturnMemoryFile
{
    char* m_name;
    u8* m_data;
    u32 m_dataSize;
    u32 m_base;
};

struct sSaturnPtr
{
    u32 m_offset;
    sSaturnMemoryFile* m_file;

    sSaturnPtr operator + (unsigned int i) const
    {
        sSaturnPtr newPtr = *this;
        newPtr.m_offset += i;
        return newPtr;
    }
};

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
#include "mainMenuDebugTasks.h"
#include "field.h"

#include "menu_dragon.h"
#include "menu_dragonMorph.h"

extern u8 COMMON_DAT[0x98000];
extern sSaturnMemoryFile gCommonFile;

struct sPortData2
{
    u16 field_8; // 8: start
    u16 field_C; // 0x10: up
                 // 0x20: down
                 // 0x40: left
                 // 0x80: right
};
extern sPortData2 PortData2;

struct sFileInfoSub
{
    sFileInfoSub* pNext; // 0
    char m_fileName[32]; // 4 was file id before
    FILE* fHandle;//8
                  //u32 m_numSectors;//C
    u32 m_fileSize; //10 ?
                    //u32 m_14;//14 ?
    u16 m_18;//18 ? word
    u8* m_patchPointerType; //1A ? word
};

struct sFileInfo
{
    u8 field_0; //0
    u8 field_3; //3
    u8 displayMemoryLayout; //5
    u16 field_8; //8

    sFileInfoSub* allocatedHead; //2C
    sFileInfoSub* freeHead; //30
    sFileInfoSub linkedList[15]; //34
};

extern sFileInfo fileInfoStruct;

extern bool debugEnabled;
extern int enableDebugTask;
extern u8 pauseEngine[4];
extern u32 azelCdNumber;

u32 READ_BE_U32(const void* ptr);
s32 READ_BE_S32(const void* ptr);
u16 READ_BE_U16(const void* ptr);
s16 READ_BE_S16(const void* ptr);

u32 getFileSize(const char* fileName);

void unimplemented(const char* name);

void initVDP1Projection(fixedPoint r4, u32 mode);

fixedPoint sqrt_F(fixedPoint r4);

#include "o_title.h"
#include "o_menuEn.h"
#include "o_fld_a3.h"

