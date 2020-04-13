/*
	Audio Overload SDK - QSF file engine

	Copyright (c) 2007, R. Belmont and Richard Bannister.

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

//
// eng_qsf.c
// by R. Belmont
//

/*
The program section of a QSF file, once decompressed, contains a series of
data blocks of the form:

3 bytes - ASCII section name tag
4 bytes - Starting offset (LSB-first)
4 bytes - Length (N) (LSB-first)
N bytes - Data

The data is then loaded to the given starting offset in the section described
by the ASCII tag.

The following sections are defined:

"KEY" - Kabuki decryption key.  This section should be 11 bytes and contain
        the following:
        4 bytes - swap_key1 (MSB-first)
        4 bytes - swap_key2 (MSB-first)
        2 bytes - addr_key  (MSB-first)
        1 bytes - xor_key
"Z80" - Z80 program ROM.
"SMP" - QSound sample ROM.

If the KEY section isn't given or both swap_keys are zero, then it is assumed
that no encryption is used.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ao.h"
#include "qsound.h"
#include "z80.h"

#include "corlett.h"

// timer rate is 285 Hz
static int32 samples_per_tick = 44100/285;
static int32 samples_to_next_tick = 44100/285;

static corlett_t c = {0};
static uint32 skey1, skey2;
static uint16 akey;
static uint8  xkey;
static int32 uses_kabuki = 0;

static char *Z80ROM, *QSamples;
static char RAM[0x1000], RAM2[0x1000];
static int32 cur_bank;

static struct QSound_interface qsintf =
{
	QSOUND_CLOCK,
	NULL
};

extern void cps1_decode(unsigned char *rom, int swap_key1,int swap_key2,int addr_key,int xor_key);

static void qsf_walktags(uint8 *buffer, uint8 *end)
{
	uint8 *cbuf = buffer;
	uint32 offset, length;

	while (cbuf < end)
	{
		#ifdef DEBUG
		printf("cbuf: %p end: %p\n", cbuf, end);
		#endif
		offset = cbuf[3] | cbuf[4]<<8 | cbuf[5]<<16 | cbuf[6]<<24;
		length = cbuf[7] | cbuf[8]<<8 | cbuf[9]<<16 | cbuf[10]<<24;

		#ifdef DEBUG
		printf("Tag: %c%c%c @ %08x, length %08x\n", cbuf[0], cbuf[1], cbuf[2], offset, length);
		#endif

		switch (cbuf[0])
		{
			case 'Z':
				memcpy(&Z80ROM[offset], &cbuf[11], length);
				break;

			case 'S':
				memcpy(&QSamples[offset], &cbuf[11], length);
				break;

			case 'K':
				skey1 = cbuf[11]<<24 | cbuf[12]<<16 | cbuf[13]<<8 | cbuf[14];
				skey2 = cbuf[15]<<24 | cbuf[16]<<16 | cbuf[17]<<8 | cbuf[18];
				akey = cbuf[19]<<8 | cbuf[20];
				xkey = cbuf[20];
				break;

			default:
				printf("ERROR: Unknown QSF tag!\n");
				break;
		}

		cbuf += 11;
		cbuf += length;
	}
}

static int32 qsf_irq_cb(int param)
{
	return 0x000000ff;	// RST_38
}

int qsf_lib(int libnum, uint8 *lib, uint64 size, corlett_t *c)
{
	// use the contents
	qsf_walktags(lib, lib+size);

	return AO_SUCCESS;
}

int32 qsf_start(uint8 *buffer, uint32 length)
{
	z80_init();

	Z80ROM = malloc(512*1024);
	QSamples = malloc(8*1024*1024);

	skey1 = skey2 = 0;
	akey = 0;
	xkey = 0;
	cur_bank = 0;

	memset(RAM, 0, 0x1000);
	memset(RAM2, 0, 0x1000);

	// Decode the current QSF
	if (corlett_decode(buffer, length, &c, qsf_lib) != AO_SUCCESS)
	{
		return AO_FAIL;
	}

	if ((skey1 != 0) && (skey2 != 0))
	{
		#ifdef DEBUG
		printf("Decoding Kabuki: skey1 %08x skey2 %08x akey %04x xkey %02x\n", skey1, skey2, akey, xkey);
		#endif

		uses_kabuki = 1;
		cps1_decode((unsigned char *)Z80ROM, skey1, skey2, akey, xkey);
	}

	z80_reset(NULL);
	z80_set_irq_callback(qsf_irq_cb);
	qsintf.sample_rom = QSamples;
	qsound_sh_start(&qsintf);

	return AO_SUCCESS;
}

static void timer_tick(void)
{
	z80_set_irq_line(0, ASSERT_LINE);
	z80_set_irq_line(0, CLEAR_LINE);
}

int32 qsf_sample(stereo_sample_t *sample)
{
	z80_execute((8000000/44100));
	qsound_update(0, sample);

	samples_to_next_tick --;

	if (samples_to_next_tick <= 0)
	{
		timer_tick();
		samples_to_next_tick = samples_per_tick;
	}

	return AO_SUCCESS;
}

int32 qsf_frame(void)
{
	return AO_SUCCESS;
}

int32 qsf_stop(void)
{
	free(Z80ROM);
	free(QSamples);

	return AO_SUCCESS;
}

int32 qsf_command(int32 command, int32 parameter)
{
	switch (command)
	{
		case COMMAND_RESTART:
			return AO_SUCCESS;

	}
	return AO_FAIL;
}

int32 qsf_fill_info(ao_display_info *info)
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

uint8 qsf_memory_read(uint16 addr)
{
	if (addr < 0x8000)
	{
		return Z80ROM[addr];
	}
	else if (addr < 0xc000)
	{
		return Z80ROM[(addr - 0x8000) + cur_bank];
	}
	else if (addr <= 0xcfff)
	{
		return RAM[addr - 0xc000];
	}
	else if (addr == 0xd007)
	{
		return qsound_status_r();
	}
	else if (addr >= 0xf000)
	{
		return RAM2[addr-0xf000];
	}
}

uint8 qsf_memory_readop(uint16 addr)
{
	if (!uses_kabuki)
	{
		return qsf_memory_read(addr);
	}

	if (addr < 0x8000)
	{
		return Z80ROM[addr + (256*1024)];
	}

	return qsf_memory_read(addr);
}

uint8 qsf_memory_readport(uint16 addr)
{
	return Z80ROM[0x11];
}

void qsf_memory_write(uint16 addr, uint8 byte)
{
	if (addr >= 0xc000 && addr <= 0xcfff)
	{

		RAM[addr-0xc000] = byte;
		return;
	}
	else if (addr == 0xd000)
	{
		qsound_data_h_w(byte);
		return;
	}
	else if (addr == 0xd001)
	{
		qsound_data_l_w(byte);
		return;
	}
	else if (addr == 0xd002)
	{
		qsound_cmd_w(byte);
		return;
	}
	else if (addr == 0xd003)
	{
		cur_bank = (0x8000 + (byte & 0xf) * 0x4000);
		if (cur_bank > (256*1024))
		{
			cur_bank = 0;
		}
//		printf("Z80 bank to %x (%x)\n", cur_bank, byte);
		return;
	}
	else if (addr >= 0xf000)
	{
		RAM2[addr-0xf000] = byte;
		return;
	}
}

void qsf_memory_writeport(uint16 addr, uint8 byte)
{
	printf("Unk port %x @ %x\n", byte, addr);
}

