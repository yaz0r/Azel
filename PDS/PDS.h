#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <SDL.h>
#include <imgui.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <basetsd.h>

typedef UINT8 u8;
typedef INT8 s8;
typedef UINT16 u16;
typedef INT16 s16;
typedef UINT32 u32;
typedef INT32 s32;

#include "dummy.h"

#include "heap.h"
#include "task.h"
#include "rootTask.h"

#include "VDP2.h"
#include "titleScreen.h"

#include "renderer.h"
#include "mainMenuDebugTasks.h"

enum e_dragonLevel : unsigned char {
    DR_LEVEL_0_BASIC_WING = 0,
    DR_LEVEL_1_VALIANT_WING,
    DR_LEVEL_2_STRIPE_WING,
    DR_LEVEL_3_PANZER_WING,
    DR_LEVEL_4_EYE_WING,
    DR_LEVEL_5_ARM_WING,
    DR_LEVEL_6_LIGHT_WING,
    DR_LEVEL_7_SOLO_WING,
    DR_LEVEL_8_FLOATER,

    DR_LEVEL_MAX
};

enum e_dragonAnim : unsigned char {
    DR_ANIM_0_BASIC_WING = 0,
    DR_ANIM_1_VALIANT_WING,
    DR_ANIM_2_STRIPE_WING,
    DR_ANIM_3_PANZER_WING,
    DR_ANIM_4_EYE_WING,
    DR_ANIM_5_ARM_WING,
    DR_ANIM_6_LIGHT_WING,
    DR_ANIM_7_SOLO_WING,
    DR_ANIM_8_FLOATER,
    DR_ANIM_9_PUP,

    DR_ANIM_MAX
};

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
u16 READ_BE_U16(const void* ptr);

#include "o_title.h"

