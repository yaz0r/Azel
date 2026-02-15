#pragma once

#pragma warning(disable:4146)
#pragma warning(disable:4244)

#include <stdint.h>
#include <stdio.h>

#include <string>
#include <sstream>

//#include "strformat.h"

#define UINT32 uint32_t
#define UINT16 uint16_t
#define INT32 int32_t

#define offs_t uint64_t

//#define ARRAY_LENGTH(x) (sizeof(x)/(sizeof(x[0])))

typedef uint64_t flags_t;

// Extra flags for Igor
const flags_t DASMFLAG_OPERAND_IS_CODE_ADDRESS = 0x200000000;
const flags_t DASMFLAG_STOP = 0x100000000; // last instruction to be disassembled

const flags_t DASMFLAG_SUPPORTED = 0x80000000;   // are disassembly flags supported?
const flags_t DASMFLAG_STEP_OUT = 0x40000000;   // this instruction should be the end of a step out sequence
const flags_t DASMFLAG_STEP_OVER = 0x20000000;   // this instruction should be stepped over by setting a breakpoint afterwards
const flags_t DASMFLAG_OVERINSTMASK = 0x18000000;   // number of extra instructions to skip when stepping over
const flags_t DASMFLAG_OVERINSTSHIFT = 27;           // bits to shift after masking to get the value
const flags_t DASMFLAG_LENGTHMASK = 0x0000ffff;   // the low 16-bits contain the actual length

#define DASMFLAG_STEP_OVER_EXTRA(x)         ((x) << DASMFLAG_OVERINSTSHIFT)

flags_t DasmSH2(char *buffer, unsigned pc, UINT16 opcode);

