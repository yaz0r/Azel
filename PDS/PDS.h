#pragma once

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

#include "task.h"
#include "rootTask.h"

#include "VDP2.h"
#include "titleScreen.h"

int drawMenuString(char* text);

struct sPortData2
{
    u16 field_8;
};
extern sPortData2 PortData2;

extern bool debugEnabled;
extern int enableDebugTask;
extern char pauseEngine[3];

