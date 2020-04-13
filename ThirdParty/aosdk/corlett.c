/*

	Audio Overload SDK

	Copyright (c) 2007-2008, R. Belmont and Richard Bannister.

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

// corlett.c

// Decodes file format designed by Neill Corlett (PSF, QSF, ...)

/*
 - First 3 bytes: ASCII signature: "PSF" (case sensitive)

- Next 1 byte: Version byte
  The version byte is used to determine the type of PSF file.  It does NOT
  affect the basic structure of the file in any way.

  Currently accepted version bytes are:
    0x01: Playstation (PSF1)
    0x02: Playstation 2 (PSF2)
    0x11: Saturn (SSF) [TENTATIVE]
    0x12: Dreamcast (DSF) [TENTATIVE]
    0x21: Nintendo 64 (USF) [RESERVED]
    0x41: Capcom QSound (QSF)

- Next 4 bytes: Size of reserved area (R), little-endian unsigned long

- Next 4 bytes: Compressed program length (N), little-endian unsigned long
  This is the length of the program data _after_ compression.

- Next 4 bytes: Compressed program CRC-32, little-endian unsigned long
  This is the CRC-32 of the program data _after_ compression.  Filling in
  this value is mandatory, as a PSF file may be regarded as corrupt if it
  does not match.

- Next R bytes: Reserved area.
  May be empty if R is 0 bytes.

- Next N bytes: Compressed program, in zlib compress() format.
  May be empty if N is 0 bytes.

The following data is optional and may be omitted:

- Next 5 bytes: ASCII signature: "[TAG]" (case sensitive)
  If these 5 bytes do not match, then the remainder of the file may be
  regarded as invalid and discarded.

- Remainder of file: Uncompressed ASCII tag data.
*/

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ao.h"
#include "corlett.h"

#include <zlib.h>

#define DECOMP_MAX_SIZE		((32 * 1024 * 1024) + 12)

uint32 total_samples;
uint32 decaybegin;
uint32 decayend;

static int corlett_decode_tags(corlett_t *c, uint8 *input, uint32 input_len)
{
	hashtable_init(&c->tags, sizeof(const char**));

	if (input_len < 5)
		return AO_SUCCESS;

	if ((input[0] == '[') && (input[1] == 'T') && (input[2] == 'A') && (input[3] == 'G') && (input[4] == ']'))
	{
		int at_data;
		char *p, *start;
		const char *name = NULL, *data = NULL;

		// Tags found!
		input += 5;
		input_len -= 5;

		// Add 1 more byte to the end to easily support cases
		// in which the data of the last tag ends directly at EOF,
		// without being terminated by a '\n' or '\0' before.
		c->tag_buffer = malloc(input_len + 1);
		if (!c->tag_buffer) {
			return AO_FAIL;
		}
		memcpy(c->tag_buffer, input, input_len);
		c->tag_buffer[input_len] = 0;

		input_len++;
		at_data = false;
		p = c->tag_buffer;
		start = NULL;
		while (input_len)
		{
			int set_condition;
			const char **set_str;

			if (!at_data)
			{
				set_condition = *p == '=';
				set_str = &name;
			}
			else
			{
				set_condition = (*p == '\n') || (*p == '\0');
				set_str = &data;
			}

			if (set_condition)
			{
				*p = '\0';
				*set_str = start;
				at_data = !at_data;
				start = NULL;
			}
			else if (!start && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n')
			{
				start = p;
			}
			if(name && data) {
				const char **tag_ptr = corlett_tag_get(c, name);
				*tag_ptr = data;
				name = NULL;
				data = NULL;
			}

			p++;
			input_len--;
		}
	}
	return true;
}

static int corlett_decode_lib(int libnum, uint8 *input, uint32 input_len, uint8 **output, uint64 *size, corlett_t *c, corlett_lib_callback_t *lib_callback)
{
	int ret = AO_SUCCESS;
	uint32 *buf;
	uint32 res_area, comp_crc,  actual_crc;
	uint8 *decomp_dat;
	uLongf decomp_length, comp_length;

	// 32-bit pointer to data
	buf = (uint32 *)input;

	// Check we have a PSF format file.
	if ((input[0] != 'P') || (input[1] != 'S') || (input[2] != 'F'))
	{
		return AO_FAIL;
	}

	// Get our values
	res_area = LE32(buf[1]);
	comp_length = LE32(buf[2]);
	comp_crc = LE32(buf[3]);

	if (comp_length > 0)
	{
		// Check length
		if (input_len < comp_length + 16)
			return AO_FAIL;

		// Check CRC is correct
		actual_crc = crc32(0, (unsigned char *)&buf[4+(res_area/4)], comp_length);
		if (actual_crc != comp_crc)
			return AO_FAIL;

		// Decompress data if any
		decomp_dat = malloc(DECOMP_MAX_SIZE);
		decomp_length = DECOMP_MAX_SIZE;
		if (uncompress(decomp_dat, &decomp_length, (unsigned char *)&buf[4+(res_area/4)], comp_length) != Z_OK)
		{
			goto err_free;
		}

		// Resize memory buffer to what we actually need
		decomp_dat = realloc(decomp_dat, (size_t)decomp_length + 1);
	}
	else
	{
		decomp_dat = NULL;
		decomp_length =  0;
	}

	memset(c, 0, sizeof(corlett_t));

	// set reserved section pointer
	c->res_section = &buf[4];
	c->res_size = res_area;

	// Return it
	*output = decomp_dat;
	*size = decomp_length;

	// Next check for tags
	input_len -= (comp_length + 16 + res_area);
	input += (comp_length + res_area + 16);

	#ifdef DEBUG
	printf("New corlett: input len %d\n", input_len);
	#endif

	if (corlett_decode_tags(c, input, input_len) != AO_SUCCESS)
	{
		goto err_free;
	}

	// Fully recursive loading of libraries is probably a bad idea anyway.
	if (libnum == 0)
	{
		int i;
		char lib_tag_name[6] = "_lib";
		uint8 *lib_raw[9] = {0};
		uint8 *lib_data[9] = {0};
		corlett_t lib_tags[9] = {{0}};

		for(i = 0; i < 9; i++) {
			#define BREAK_ON_ERR(call) \
				ret = call; \
				if(ret != AO_SUCCESS) { \
					break; \
			}

			uint64 lib_len, lib_raw_length;

			if (i >= 1 && i <= 8)
			{
				lib_tag_name[4] = '0' + i + 1;
			}

			const char *libfile = corlett_tag_lookup(c, lib_tag_name);
			if(!libfile)
			{
				continue;
			}

			#ifdef DEBUG
			printf("Loading library #%d: %s\n", 1 + i, libfile);
			#endif

			BREAK_ON_ERR(ao_get_lib(libfile, &lib_raw[i], &lib_raw_length));
			BREAK_ON_ERR(corlett_decode_lib(1 + i, lib_raw[i], lib_raw_length, &lib_data[i], &lib_len, &lib_tags[i], lib_callback));
		}

		if (ret == AO_SUCCESS)
		{
			ret = lib_callback(libnum, decomp_dat, decomp_length, c);
		}
		if (ret == AO_SUCCESS)
		{
			// now figure out the time in samples for the length/fade
			double length_seconds = psfTimeToSeconds(corlett_tag_lookup(c, "length"));
			double fade_seconds = psfTimeToSeconds(corlett_tag_lookup(c, "fade"));

			#ifdef DEBUG
			printf("length %f fade %f\n", length_seconds, fade_seconds);
			#endif

			corlett_length_set(length_seconds, fade_seconds);
		}

		for (i = 0; i < 9; i++)
		{
			if (lib_data[i])
			{
				free(lib_data[i]);
			}
			if (lib_raw[i])
			{
				free(lib_raw[i]);
			}
			corlett_free(&lib_tags[i]);
		}
	}
	else
	{
		ret = lib_callback(libnum, decomp_dat, decomp_length, c);
	}

	return ret;

err_free:
	free(decomp_dat);
	return AO_FAIL;
}

int corlett_decode(uint8 *input, uint32 input_len, corlett_t *c, corlett_lib_callback_t *lib_callback)
{
	int ret = AO_FAIL;
	uint8 *output = NULL;
	uint64 size;

	if (lib_callback)
	{
		ret = corlett_decode_lib(0, input, input_len, &output, &size, c, lib_callback);
		if(output)
		{
			free(output);
		}
	}
	return ret;
}

void corlett_free(corlett_t *c)
{
	if (c->tag_buffer)
	{
		free(c->tag_buffer);
	}
	hashtable_free(&c->tags);
	memset(c, 0, sizeof(corlett_t));
}

static const char** corlett_tag_hashtable_get(corlett_t *c, const char *tag, hashtable_flags_t flags)
{
	blob_t tag_blob;
	assert(c);
	assert(tag);
	tag_blob.buf = (void*)tag;
	tag_blob.len = strlen(tag) + 1;
	return (const char**)hashtable_get(&c->tags, &tag_blob, flags);
}

const char** corlett_tag_get(corlett_t *c, const char *tag)
{
	return corlett_tag_hashtable_get(c, tag, HT_CASE_INSENSITIVE | HT_CREATE);
}

const char *corlett_tag_lookup(corlett_t *c, const char *tag)
{
	const char **ret = corlett_tag_hashtable_get(c, tag, HT_CASE_INSENSITIVE | HT_CREATE);
	return ret ? *ret : NULL;
}

void corlett_length_set(double length_seconds, double fade_seconds)
{
	total_samples = 0;
	if (length_seconds == 0)
	{
		decaybegin = ~0;
	}
	else
	{
		uint32 length_samples = length_seconds * 44100;
		uint32 fade_samples = fade_seconds * 44100;

		decaybegin = length_samples;
		decayend = length_samples + fade_samples;
	}
}

uint32 corlett_sample_count(void)
{
	return total_samples;
}

uint32 corlett_sample_total(void)
{
	return decayend;
}

void corlett_sample_fade(stereo_sample_t *sample)
{
	if(total_samples >= decaybegin)
	{
		int32 fader;
		if(total_samples >= decayend)
		{
			ao_song_done = 1;
			sample->l = 0;
			sample->r = 0;
		}
		else
		{
			fader = 256 - (256 * (total_samples - decaybegin) / (decayend - decaybegin));
			sample->l = (sample->l * fader) >> 8;
			sample->r = (sample->r * fader) >> 8;
		}
	}
	total_samples++;
}

double psfTimeToSeconds(const char *str)
{
	int x, c = 0;
	uint32 part_val = 0, digit = 1;
	double acc = 0.0;

	if (!str)
	{
		return(0.0);
	}

	for (x = strlen(str); x >= 0; x--)
	{
		if (str[x] >= '0' && str[x] <= '9')
		{
			part_val += (str[x] - '0') * digit;
			digit *= 10;
		}
		if (str[x]=='.' || str[x]==',')
		{
			acc = (double)part_val / (double)digit;
			part_val = 0;
			digit = 1;
		}
		else if (str[x]==':')
		{
			if(c==0)
			{
				acc += part_val;
			}
			else if(c==1)
			{
				acc += part_val * 60;
			}

			c++;
			part_val = 0;
			digit = 1;
		}
		else if (x==0)
		{
			if(c==0)
			{
				acc += part_val;
			}
			else if(c==1)
			{
				acc += part_val * 60;
			}
			else if(c==2)
			{
				acc += part_val * 60 * 60;
			}
			part_val = 0;
			digit = 1;
		}
	}

	return(acc);
}

