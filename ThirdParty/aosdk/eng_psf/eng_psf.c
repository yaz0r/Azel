/*
	Audio Overload SDK - PSF file format engine

	Copyright (c) 2007 R. Belmont and Richard Bannister.

	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	* Neither the names of R. Belmont and Richard Bannister nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ao.h"
#include "eng_protos.h"
#include "cpuintrf.h"
#include "psx.h"

#include "peops/stdafx.h"
#include "peops/externals.h"
#include "peops/regs.h"
#include "peops/registers.h"
#include "peops/spu.h"


#include "corlett.h"

static corlett_t	c = {0};
int			psf_refresh  = -1;


// main RAM
extern uint32 psx_ram[((2*1024*1024)/4)+4];
extern uint32 psx_scratch[0x400];
extern uint32 initial_ram[((2*1024*1024)/4)+4];
extern uint32 initial_scratch[0x400];
static uint32 initialPC, initialGP, initialSP;

extern void mips_init( void );
extern void mips_reset( void *param );
extern int mips_execute( int cycles );
extern void mips_set_info(UINT32 state, union cpuinfo *info);
extern void psx_hw_init(void);
extern void psx_hw_slice(void);
extern void psx_hw_frame(void);

static void psf_lib_set_refresh(int libnum, corlett_t *c)
{
	if (c != NULL && psf_refresh == -1)
	{
		const char *refresh = corlett_tag_lookup(c, "_refresh");
		if(!refresh) {
			return;
		}
		if (refresh[0] == '5')
		{
			psf_refresh = 50;
		}
		if (refresh[0] == '6')
		{
			psf_refresh = 60;
		}
	}
}

static void psf_lib_set_registers(int libnum, const uint8 *lib)
{
	if (lib == NULL)
	{
		return;
	}

	initialPC = lib[0x10] | lib[0x11]<<8 | lib[0x12]<<16 | lib[0x13]<<24;
	initialGP = lib[0x14] | lib[0x15]<<8 | lib[0x16]<<16 | lib[0x17]<<24;
	initialSP = lib[0x30] | lib[0x31]<<8 | lib[0x32]<<16 | lib[0x33]<<24;

	#ifdef DEBUG
	printf("Library #%d: PC %x GP %x SP %x\n", libnum, initialPC, initialGP, initialSP);
	#endif
}

static void psf_lib_patch(int libnum, uint8 *lib, uint64 size)
{
	uint32 offset, plength;

	if (lib == NULL || size == 0)
	{
		return;
	}

	offset = lib[0x18] | lib[0x19]<<8 | lib[0x1a]<<16 | lib[0x1b]<<24;
	offset &= 0x3fffffff;	// kill any MIPS cache segment indicators
	plength = lib[0x1c] | lib[0x1d]<<8 | lib[0x1e]<<16 | lib[0x1f]<<24;

	// Philosoma has an illegal "plength".  *sigh*
	if (plength > (size-2048))
	{
		plength = size-2048;
	}

	#ifdef DEBUG
	printf("Library #%d: offset: %x plength: %d\n", libnum, offset, plength);
	#endif
	memcpy(&psx_ram[offset/4], lib+2048, plength);
}

int psf_lib(int libnum, uint8 *lib, uint64 size, corlett_t *c)
{
	static struct
	{
		uint8 *lib;
		uint64 size;
		corlett_t *tags;
	} cache[10] = {{0}};

	if (strncmp((char *)lib, "PS-X EXE", 8))
	{
		printf("Major error!  Library %d was not OK!\n", libnum);
		return AO_FAIL;
	}

	#ifdef DEBUG
	{
		uint32 offset;

		offset = lib[0x18] | lib[0x19]<<8 | lib[0x1a]<<16 | lib[0x1b]<<24;
		printf("Library #%d: Text section start: %x\n", libnum, offset);
		offset = lib[0x1c] | lib[0x1d]<<8 | lib[0x1e]<<16 | lib[0x1f]<<24;
		printf("Library #%d: Text section size: %x\n", libnum, offset);
		printf("Library #%d: Region: [%s]\n", libnum, &lib[0x4c]);
		printf("Library #%d: refresh: [%s]\n", libnum, corlett_tag_lookup(c, "_refresh"));
	}
	#endif

	cache[libnum].lib = lib;
	cache[libnum].size = size;
	cache[libnum].tags = c;

	// now reconstruct the braindead original initialization order
	if (libnum == 0)
	{
		int i;

		psf_lib_set_registers(0, cache[0].lib);
		psf_lib_set_registers(1, cache[1].lib);
		psf_lib_set_refresh(0, cache[0].tags);
		psf_lib_set_refresh(1, cache[1].tags);
		psf_lib_patch(1, cache[1].lib, cache[1].size);
		psf_lib_patch(0, cache[0].lib, cache[0].size);

		for (i = 2; i < 10; i++)
		{
			psf_lib_patch(i, cache[i].lib, cache[i].size);
		}

		memset(cache, 0, sizeof(cache));
	}

	return AO_SUCCESS;
}

int32 psf_start(uint8 *buffer, uint32 length)
{
	union cpuinfo mipsinfo;

	// clear PSX work RAM before we start scribbling in it
	memset(psx_ram, 0, 2*1024*1024);

//	printf("Length = %d\n", length);

	// Decode the current GSF
	if (corlett_decode(buffer, length, &c, psf_lib) != AO_SUCCESS)
	{
		return AO_FAIL;
	}

	mips_init();
	mips_reset(NULL);

	// set the initial PC, SP, GP
	#ifdef DEBUG
	printf("Initial PC %x, GP %x, SP %x\n", initialPC, initialGP, initialSP);
	printf("Refresh = %d\n", psf_refresh);
	#endif
	mipsinfo.i = initialPC;
	mips_set_info(CPUINFO_INT_PC, &mipsinfo);

	// set some reasonable default for the stack
	if (initialSP == 0)
	{
		initialSP = 0x801fff00;
	}

	mipsinfo.i = initialSP;
	mips_set_info(CPUINFO_INT_REGISTER + MIPS_R29, &mipsinfo);
	mips_set_info(CPUINFO_INT_REGISTER + MIPS_R30, &mipsinfo);

	mipsinfo.i = initialGP;
	mips_set_info(CPUINFO_INT_REGISTER + MIPS_R28, &mipsinfo);

	#ifdef DEBUG
	{
		FILE *f;

		f = ao_fopen("psxram.bin", "wb");
		fwrite(psx_ram, 2*1024*1024, 1, f);
		fclose(f);
	}
	#endif

	psx_hw_init();
	SPUinit();
	SPUopen();

	// patch illegal Chocobo Dungeon 2 code - CaitSith2 put a jump in the delay slot from a BNE
	// and rely on Highly Experimental's buggy-ass CPU to rescue them.  Verified on real hardware
	// that the initial code is wrong.
	{
		const char *game = corlett_tag_lookup(&c, "game");
		if (game && !strcmp(game, "Chocobo Dungeon 2"))
		{
			if (psx_ram[0xbc090/4] == LE32(0x0802f040))
			{
				psx_ram[0xbc090/4] = LE32(0);
				psx_ram[0xbc094/4] = LE32(0x0802f040);
				psx_ram[0xbc098/4] = LE32(0);
			}
		}
	}

//	psx_ram[0x118b8/4] = LE32(0);	// crash 2 hack

	// backup the initial state for restart
	memcpy(initial_ram, psx_ram, 2*1024*1024);
	memcpy(initial_scratch, psx_scratch, 0x400);

	mips_execute(5000);

	return AO_SUCCESS;
}

int32 psf_sample(stereo_sample_t *sample)
{
	psx_hw_slice();
	SPUsample(sample);

	return AO_SUCCESS;
}

int32 psf_frame(void)
{
	psx_hw_frame();

	return AO_SUCCESS;
}

int32 psf_stop(void)
{
	SPUclose();
	corlett_free(&c);

	return AO_SUCCESS;
}

int32 psf_command(int32 command, int32 parameter)
{
	union cpuinfo mipsinfo;

	switch (command)
	{
		case COMMAND_RESTART:
			SPUclose();

			memcpy(psx_ram, initial_ram, 2*1024*1024);
			memcpy(psx_scratch, initial_scratch, 0x400);

			mips_init();
			mips_reset(NULL);
			psx_hw_init();
			SPUinit();
			SPUopen();

			mipsinfo.i = initialPC;
			mips_set_info(CPUINFO_INT_PC, &mipsinfo);
			mipsinfo.i = initialSP;
			mips_set_info(CPUINFO_INT_REGISTER + MIPS_R29, &mipsinfo);
			mips_set_info(CPUINFO_INT_REGISTER + MIPS_R30, &mipsinfo);
			mipsinfo.i = initialGP;
			mips_set_info(CPUINFO_INT_REGISTER + MIPS_R28, &mipsinfo);

			mips_execute(5000);

			return AO_SUCCESS;

	}
	return AO_FAIL;
}

int32 psf_fill_info(ao_display_info *info)
{
	info->title[1] = "Name: ";
	info->info[1] = corlett_tag_lookup(&c, "title");

	info->title[2] = "Game: ";
	info->info[2] = corlett_tag_lookup(&c, "game");

	info->title[3] = "Artist: ";
	info->info[3] = corlett_tag_lookup(&c, "artist");

	info->title[4] = "Copyright: ";
	info->info[4] = corlett_tag_lookup(&c, "copyright");

	info->title[5] = "Year: ";
	info->info[5] = corlett_tag_lookup(&c, "year");

	info->title[6] = "Length: ";
	info->info[6] = corlett_tag_lookup(&c, "length");

	info->title[7] = "Fade: ";
	info->info[7] = corlett_tag_lookup(&c, "fade");

	return AO_SUCCESS;
}
