//
// Audio Overload SDK
//
// Fake ao.h to set up the general Audio Overload style environment
//

#ifndef __AO_H
#define __AO_H

#include <stdio.h>

#define AO_SUCCESS					1
#define AO_FAIL						0
#define AO_FAIL_DECOMPRESSION		-1

#define AUDIO_RATE					(44100)

enum
{
	COMMAND_NONE = 0,
	COMMAND_PREV,
	COMMAND_NEXT,
	COMMAND_RESTART,
	COMMAND_HAS_PREV,
	COMMAND_HAS_NEXT,
	COMMAND_GET_MIN,
	COMMAND_GET_MAX,
	COMMAND_JUMP
};

/* Compiler defines for Xcode */
#ifdef __BIG_ENDIAN__
	#undef LSB_FIRST
#endif

#ifdef __LITTLE_ENDIAN__
	#define LSB_FIRST	1
#endif

#define LSB_FIRST 1

typedef unsigned char ao_bool;

#ifdef __GNUC__
#include <stddef.h>	// get NULL
#include <stdbool.h>

#ifndef TRUE
#define TRUE  (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#endif

#ifdef _MSC_VER
#include <stddef.h>	// get NULL

#ifndef TRUE
#define TRUE  (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#define true (1)
#define false (0)

#define strcasecmp _strcmpi

#endif

#define countof(arr) (sizeof(arr) / sizeof(arr[0]))

#ifndef PATH_MAX
#define PATH_MAX	2048
#endif

typedef struct
{
	const char *title[9];
	const char *info[9];
} ao_display_info;

typedef unsigned char		uint8;
typedef unsigned char		UINT8;
typedef signed char			int8;
typedef signed char			INT8;
typedef unsigned short		uint16;
typedef unsigned short		UINT16;
typedef signed short		int16;
typedef signed short		INT16;
typedef signed int			int32;
typedef unsigned int		uint32;
#ifdef LONG_IS_64BIT
typedef signed long             int64;
typedef unsigned long           uint64;
#else
typedef signed long long	int64;
typedef unsigned long long	uint64;
#endif

#ifdef WIN32
//#include "win32_utf8/src/entry.h"

#ifndef _BASETSD_H
typedef signed int			INT32;
typedef unsigned int		UINT32;
typedef signed long long	INT64;
typedef unsigned long long	UINT64;
#endif
#else
typedef signed int			INT32;
typedef unsigned int		UINT32;
#ifdef LONG_IS_64BIT
typedef signed long         INT64;
typedef unsigned long       UINT64;
#else
typedef signed long long	INT64;
typedef unsigned long long	UINT64;
#endif
#endif

#ifndef INLINE
#if defined(_MSC_VER)
#define INLINE static __forceinline
#elif defined(__GNUC__)
#define INLINE static __inline__
#elif defined(_MWERKS_)
#define INLINE static inline
#elif defined(__powerc)
#define INLINE static inline
#else
#define INLINE
#endif
#endif

INLINE uint16 SWAP16(uint16 x)
{
	return (
		((x & 0xFF00) >> 8) |
		((x & 0x00FF) << 8)
	);
}

INLINE uint32 SWAP24(uint32 x)
{
	return (
		((x & 0xFF0000) >> 16) |
		((x & 0x00FF00) << 0) |
		((x & 0x0000FF) << 16)
	);
}

INLINE uint32 SWAP32(uint32 x)
{
	return (
		((x & 0xFF000000) >> 24) |
		((x & 0x00FF0000) >> 8) |
		((x & 0x0000FF00) << 8) |
		((x & 0x000000FF) << 24)
	);
}

#if LSB_FIRST
#define BE16(x) (SWAP16(x))
#define BE24(x) (SWAP24(x))
#define BE32(x) (SWAP32(x))
#define LE16(x) (x)
#define LE24(x) (x)
#define LE32(x) (x)

#ifndef __ENDIAN__ /* Mac OS X Endian header has this function in it */
#define Endian32_Swap(x) (SWAP32(res))
#endif

#else

#define BE16(x) (x)
#define BE24(x) (x)
#define BE32(x) (x)
#define LE16(x) (SWAP16(x))
#define LE24(x) (SWAP24(x))
#define LE32(x) (SWAP32(x))

#endif

typedef struct {
	int16 l;
	int16 r;
} stereo_sample_t;

int ao_get_lib(const char *filename, uint8 **buffer, uint64 *length);

#endif // AO_H

extern volatile ao_bool ao_song_done;

/// Portability functions defined in ao.c
/// -------------------------------------
FILE* ao_fopen(const char *fn, const char *mode);

// We don't care about permissions, even on Linux
int ao_mkdir(const char *dirname);

void ao_sleep(unsigned int seconds);

#define fopen ERROR_use_ao_fopen_instead!
#define mkdir ERROR_Use_ao_mkdir_instead!
/// -------------------------------------

#include "wavedump.h"
#include "sampledump.h"
