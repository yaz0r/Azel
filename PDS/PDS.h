#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <SDL.h>
#include <imgui.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;

void yLog(...);

#include "dummy.h"

#include "heap.h"
#include "task.h"
#include "rootTask.h"

#include "VDP2.h"
#include "titleScreen.h"

#include "common.h"
#include "renderer.h"
#include "3dEngine.h"
#include "mainMenuDebugTasks.h"
#include "field.h"

struct sPortData2
{
    u16 field_8; // 8: start
    u16 field_C; // 0x10: up
                 // 0x20: down
                 // 0x40: left
                 // 0x80: right
};
extern sPortData2 PortData2;

extern bool debugEnabled;
extern int enableDebugTask;
extern u8 pauseEngine[4];
extern u32 azelCdNumber;

u32 READ_BE_U32(const void* ptr);
s32 READ_BE_S32(const void* ptr);
u16 READ_BE_U16(const void* ptr);
s16 READ_BE_S16(const void* ptr);

#include "o_title.h"
#include "o_fld_a3.h"

