/*
 * Audio Overload SDK
 *
 * Wave dumping
 *
 * Author: Nmlgc
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ao.h"
#include "utils.h"
#include "wavedump.h"

typedef struct {
	uint32 FOURCC;
	uint32 Size;
} RIFFCHUNK;

// This is pretty much identical to the WAVEFORMAT+PCMWAVEFORMAT structures
// from <windows.h>, which we can't use because they're stupidly declared in
// such a way as to come with 2 padding bytes before and after
// PCMWAVEFORMAT.wBitsPerSample.

typedef struct {
	uint16 wFormatTag; // format type
	uint16 nChannels; // number of channels (i.e. mono, stereo, etc.)
	uint32 nSamplesPerSec; // sample rate
	uint32 nAvgBytesPerSec; // for buffer estimation
	uint16 nBlockAlign; // block size of data
	uint16 wBitsPerSample;
} WAVEFORMAT_PCM;

// flags for wFormatTag field of WAVEFORMAT
#define WAVE_FORMAT_PCM 1

typedef struct {
	RIFFCHUNK cRIFF;
	uint32 WAVE;
	RIFFCHUNK cfmt;
	WAVEFORMAT_PCM Format;
	RIFFCHUNK cdata;
} WAVEHEADER;

typedef struct {
	RIFFCHUNK ccue;
	uint32 points;	// number of cue points
} CUEHEADER;

typedef struct {
	uint32 dwName; // unique identification value
	uint32 dwPosition; // play order position
	uint32 fccChunk; // RIFF ID of corresponding data chunk
	uint32 dwChunkStart; // offset from [fccChunk] to the LIST chunk, or 0 if no such chunk is present
	uint32 dwBlockStart; // offset from [fccChunk] to a (unspecified) "block" containing the sample
	uint32 dwSampleOffset; // offset from [dwBlockStart] to the sample
} CUEPOINT;

static void wavedump_LIST_adtl_labl_write(
	wavedump_t *wave, uint32 point_id, const char *label
)
{
	RIFFCHUNK cLIST;
	const uint32 adtl = LE32(*(uint32*)"adtl");
	RIFFCHUNK clabl;
	size_t label_len;

	assert(wave);
	assert(label);

	label_len = strlen(label) + 1;
	clabl.FOURCC = LE32(*(uint32*)"labl");
	cLIST.FOURCC = LE32(*(uint32*)"LIST");
	clabl.Size = sizeof(point_id) + label_len;
	cLIST.Size = sizeof(adtl) + sizeof(clabl) + clabl.Size;

	fwrite(&cLIST, sizeof(cLIST), 1, wave->file);
	fwrite(&adtl, sizeof(adtl), 1, wave->file);
	fwrite(&clabl, sizeof(clabl), 1, wave->file);
	fwrite(&point_id, sizeof(point_id), 1, wave->file);
	fwrite(label, label_len, 1, wave->file);
}

static void wavedump_header_fill(
	WAVEHEADER *h, uint32 data_size, uint32 file_size,
	uint32 sample_rate, uint16 bits_per_sample, uint16 channels
)
{
	h->cRIFF.FOURCC = LE32(*(uint32*)"RIFF");
	h->cRIFF.Size = LE32(file_size - sizeof(RIFFCHUNK));
	h->WAVE = LE32(*(uint32*)"WAVE");
	h->cfmt.FOURCC = LE32(*(uint32*)"fmt ");
	h->cfmt.Size = LE32(sizeof(h->Format));
	h->Format.wFormatTag = LE16(WAVE_FORMAT_PCM);
	h->Format.nChannels = LE16(channels);
	h->Format.nSamplesPerSec = LE16(sample_rate);
	h->Format.nBlockAlign = LE16((channels * bits_per_sample) / 8);
	h->Format.nAvgBytesPerSec = LE32(
		h->Format.nSamplesPerSec * h->Format.nBlockAlign
	);
	h->Format.wBitsPerSample = LE16(bits_per_sample);
	h->cdata.FOURCC = LE32(*(uint32*)"data");
	h->cdata.Size = LE32(data_size);
}

ao_bool wavedump_open(wavedump_t *wave, const char *fn)
{
	uint32 temp = 0;
	assert(wave);

	wave->data_size = 0;
	wave->loop_sample = 0;
	wave->file = fopen_derivative(fn, ".wav");
	if(!wave->file) {
		return false;
	}
	// Jump over header, we write that one later
	fwrite(&temp, 1, sizeof(WAVEHEADER), wave->file);
	return true;
}

void wavedump_loop_set(wavedump_t *wave, uint32 loop_sample)
{
	assert(wave);
	assert(wave->file);
	wave->loop_sample = loop_sample;
}

void wavedump_append(wavedump_t *wave, uint32 len, void *buf)
{
	assert(wave);
	if(wave->file) {
		wave->data_size += len;
		// XXX: Doesn't the data need to be swapped on big-endian platforms?
		// That would mean that we need to know the target wave format on
		// opening time.
		fwrite(buf, len, 1, wave->file);
	}
}

void wavedump_finish(
	wavedump_t *wave, uint32 sample_rate, uint16 bits_per_sample, uint16 channels
)
{
	assert(wave);
	if(wave->file) {
		WAVEHEADER h;
		// RIFF chunks have to be word-aligned, so we have to pad out the
		// data chunk if the number of samples happens to be odd.
		if(wave->data_size & 1) {
			uint8 pad = 0;
			// fwrite() rather than wavedump_append(), as the chunk size
			// obviously doesn't include the padding.
			fwrite(&pad, sizeof(pad), 1, wave->file);
		}
		if(wave->loop_sample) {
			// Write the "cue " chunk, as well as an additional
			// LIST-adtl-labl chunk for newer GoldWave versions
			CUEHEADER cue;
			CUEPOINT point;

			cue.ccue.FOURCC = LE32(*(uint32*)"cue ");
			cue.ccue.Size = LE32(4 + 1 * sizeof(CUEPOINT));
			cue.points = LE32(1);
			point.dwName = LE32(0);
			point.dwChunkStart = LE32(0);
			point.dwBlockStart = LE32(0);
			point.dwPosition = LE32(wave->loop_sample);
			point.dwSampleOffset = LE32(wave->loop_sample);
			point.fccChunk = LE32(*(uint32*)"data");

			fwrite(&cue, sizeof(cue), 1, wave->file);
			fwrite(&point, sizeof(point), 1, wave->file);
			wavedump_LIST_adtl_labl_write(wave, 0, "Loop point");
		}
		wavedump_header_fill(
			&h, wave->data_size, ftell(wave->file),
			sample_rate, bits_per_sample, channels
		);
		fseek(wave->file, 0, SEEK_SET);
		fwrite(&h, sizeof(h), 1, wave->file);
		fclose(wave->file);
		wave->file = NULL;
	}
}
