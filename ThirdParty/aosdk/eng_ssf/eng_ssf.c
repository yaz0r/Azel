/*
	Audio Overload SDK - SSF file format engine

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

/*

Sega driver commands:

00 - NOP
01 - SEQUENCE_START
02 - SEQUENCE_STOP
03 - SEQUENCE_PAUSE
04 - SEQUENCE_CONTINUE
05 - SEQUENCE_VOLUME
06 - SEQUENCE_ALLSTOP
07 - SEQUENCE_TEMPO
08 - SEQUENCE_MAP
09 - HOST_MIDI
0A - VOLUME_ANALYZE_START
0B - VOLUME_ANALYZE_STOP
0C - DSP CLEAR
0D - ALL OFF
0E - SEQUENCE PAN
0F - N/A
10 - SOUND INITIALIZE
11 - Yamaha 3D check (8C)
12 - QSound check (8B)
13 - Yamaha 3D init (8D)
80 - CD level
81 - CD pan
82 - MASTER VOLUME
83 - EFFECT_CHANGE
84 - NOP
85 - PCM stream play start
86 - PCM stream play end
87 - MIXER_CHANGE
88 - Mixer parameter change
89 - Hardware check
8A - PCM parameter change
8B - QSound check
8C - Yamaha 3D check
8D - Yamaha 3D init

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ao.h"
#include "eng_protos.h"
#include "corlett.h"
#include "sat_hw.h"
#include "scsp.h"
#include "m68k.h"

static corlett_t	c = {0};

int ssf_lib(int libnum, uint8 *lib, uint64 size, corlett_t *c)
{
	// patch the file into ram
	uint32 offset = lib[0] | lib[1]<<8 | lib[2]<<16 | lib[3]<<24;

	// guard against invalid data
	if ((offset + (size-4)) > 0x7ffff)
	{
		size = 0x80000-offset+4;
	}
	memcpy(&sat_ram[offset], lib+4, size-4);

	return AO_SUCCESS;
}

int32 ssf_start(uint8 *buffer, uint32 length)
{
	int i;

	// clear Saturn work RAM before we start scribbling in it
	memset(sat_ram, 0, 512*1024);

	// Decode the current SSF
	if (corlett_decode(buffer, length, &c, ssf_lib) != AO_SUCCESS)
	{
		return AO_FAIL;
	}

	#ifdef DEBUG
	{
		FILE *f;

		f = ao_fopen("satram.bin", "wb");
		fwrite(sat_ram, 512*1024, 1, f);
		fclose(f);
	}
	#endif

	// now flip everything (this makes sense because he's using starscream)
	for (i = 0; i < 512*1024; i+=2)
	{
		uint8 temp;

		temp = sat_ram[i];
		sat_ram[i] = sat_ram[i+1];
		sat_ram[i+1] = temp;
	}

	sat_hw_init();

	return AO_SUCCESS;
}

int32 ssf_sample(stereo_sample_t *sample)
{
	m68k_execute((11300000/60)/735);
	SCSP_Update(NULL, NULL, sample);
	corlett_sample_fade(sample);

	return AO_SUCCESS;
}

int32 ssf_frame(void)
{
	return AO_SUCCESS;
}

int32 ssf_stop(void)
{
	return AO_SUCCESS;
}

int32 ssf_command(int32 command, int32 parameter)
{
	switch (command)
	{
		case COMMAND_RESTART:
			return AO_SUCCESS;

	}
	return AO_FAIL;
}

int32 ssf_fill_info(ao_display_info *info)
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
