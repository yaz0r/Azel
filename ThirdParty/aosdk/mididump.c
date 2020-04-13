/*
 * Audio Overload SDK

 * MIDI dumping
 *
 * Author: Nmlgc
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ao.h"
#include "corlett.h"
#include "mididump.h"
#include "utils.h"

ao_bool nomidi = false;

#define MSB(ctl) ctl
#define LSB(ctl) ctl+0x20

/// MIDI events
/// -----------
typedef enum {
	NOTE_OFF = 0x8,
	NOTE_ON = 0x9,
	NOTE_AFTERTOUCH = 0xA,
	CONTROLLER = 0xB,
	PROGRAM_CHANGE = 0xC,
	CHANNEL_AFTERTOUCH = 0xD,
	PITCH_BEND = 0xE,
} event_type_t;

typedef enum {
	META_TRACK_NAME = 0x3,
	META_INSTRUMENT_NAME = 0x4,
	META_TRACK_END = 0x2F,
	META_TEMPO = 0x51,
} meta_event_type_t;

typedef union {
	int8 byte[2];
	int16 word;
} event_param_t;

typedef struct event {
	unsigned long time;
	event_type_t type;
	event_param_t param;
	struct event *next;
} event_t;
/// -----------

/// Virtual channels
/// ----------------
typedef struct {
	int id;
	int8 cur_ctl_vals[128];
	event_t *first;
	event_t *last;
} vchan_t;

int64 first_note_distance = -1;

static void vchan_event_push(
	vchan_t *vchan, event_type_t type, event_param_t param
)
{
	event_t *event_out = malloc(sizeof(event_t));

	assert(vchan);

	if(vchan->last) {
		vchan->last->next = event_out;
	}
	if(!vchan->first) {
		vchan->first = event_out;
	}
	vchan->last = event_out;
	if(first_note_distance == -1) {
		event_out->time = 0;
		if(type == NOTE_ON) {
			first_note_distance = corlett_sample_count() / 2;
		}
	} else {
		event_out->time = (corlett_sample_count() / 2) - first_note_distance;
	}
	event_out->type = type;
	event_out->param.word = param.word;
	event_out->next = NULL;
}

static void vchan_ctl7_push(vchan_t *vchan, int8 ctl7, int8 val)
{
	assert(vchan);
	assert(ctl7 >= 0 && ctl7 <= 127);
	assert(val >= 0);
	if(vchan->cur_ctl_vals[ctl7] != val) {
		event_param_t param = {ctl7, val};
		vchan_event_push(vchan, CONTROLLER, param);
		vchan->cur_ctl_vals[ctl7] = val;
	}
}

static int vchan_compare(const vchan_t **a, const vchan_t **b)
{
	return (*a)->id - (*b)->id;
}

static void vchan_free(vchan_t *vchan)
{
	event_t *e;

	assert(vchan);

	e = vchan->first;
	while(e) {
		event_t *e_cur = e;
		e = e->next;
		free(e_cur);
	};
	vchan->first = NULL;
	vchan->last = NULL;
}
/// ----------------

/// Virtual channel hash table
/// --------------------------
hashtable_t vchans;

vchan_t* vchans_get(int id)
{
	vchan_t *ret;
	blob_t id_blob = {&id, sizeof(int)};
	hashtable_init(&vchans, sizeof(vchan_t));
	ret = hashtable_get(&vchans, &id_blob, HT_CREATE);
	if(ret->id == 0) {
		memset(ret->cur_ctl_vals, 0xFF, sizeof(ret->cur_ctl_vals));
		ret->id = id;
	}
	return (vchan_t*)ret;
}

vchan_t* vchans_iterate(hashtable_iterator_t *iter)
{
	return (vchan_t*)hashtable_iterate(NULL, &vchans, iter);
}

static void vchans_free(void)
{
	hashtable_iterator_t iter = {0};
	vchan_t *vchan = NULL;
	while((vchan = vchans_iterate(&iter))) {
		vchan_free(vchan);
	}
	hashtable_free(&vchans);
}
/// --------------------------

/// BPM recognition
/// ---------------
static double samples_to_bpm(double samples)
{
	return (60.0 / samples) * 22050.0;
}

#define bpm_to_samples(bpm) ((uint32)samples_to_bpm(bpm))

// Analyzes all virtual channels and returns the most common note distance in
// samples that would imply a tempo between 40.37 and 300 BPM.
static uint16 mididump_beat_distance_find(void)
{
	const uint16 DISTANCE_MIN = bpm_to_samples(300) + 2; // = 4412
	const uint16 DISTANCE_MAX = 0x7FFF; // MIDI limitation
	uint32 occurrences[(0x7FFF - 4412)] = {0}; // I hate C
	double bpm = 0;
	uint32 max_occurrences = 0;
	uint16 beat_distance = 0;
	uint32 i = 0;
	hashtable_iterator_t iter = {0};
	vchan_t *vchan = vchans_iterate(&iter);

	if(!vchan) {
		return false;
	}

	do {
		uint32 time_prev;
		const event_t *event = vchan->first;

		while(event && event->type != NOTE_ON) {
			event = event->next;
		}
		if(!event) {
			continue;
		}
		time_prev = event->time;
		// Intentionally skip the first element
		while((event = event->next)) {
			uint32 dist;

			if(event->type != NOTE_ON) {
				continue;
			}
			dist = event->time - time_prev;
			if(dist > DISTANCE_MIN && dist < DISTANCE_MAX) {
				occurrences[dist - DISTANCE_MIN]++;
			}
			time_prev = event->time;
		}
	} while((vchan = vchans_iterate(&iter)));

	memset(&iter, 0, sizeof(iter));
	for(i = 0; i < DISTANCE_MAX - DISTANCE_MIN; i++) {
		uint32 dist = i + DISTANCE_MIN;
		if(occurrences[i] > max_occurrences) {
			max_occurrences = occurrences[i];
			beat_distance = dist;
		}
	}
	return beat_distance;
}
/// ---------------

/// MIDI output
/// -----------
static FILE* midi_open(const char *fn)
{
	return fopen_derivative(fn, ".mid");
}

static void midi_varlen_write(FILE *midi, uint32 val)
{
	long buffer = val & 0x7f;

	while((val >>= 7) > 0) {
		buffer <<= 8;
		buffer |= 0x80;
		buffer += (val & 0x7f);
	}
	while(1) {
		putc(buffer, midi);
		if(buffer & 0x80) {
			buffer >>= 8;
		} else {
			break;
		}
	}
};

// Meta events
// -----------
const unsigned char MIDI_META_EVENT = 0xFF;

static void midi_meta_track_end_write(FILE *midi)
{
	const char type = META_TRACK_END;
	const char val = 0;

	midi_varlen_write(midi, 0); // delta time
	fwrite(&MIDI_META_EVENT, 1, 1, midi);
	fwrite(&type, 1, 1, midi);
	fwrite(&val, 1, 1, midi);
}
// -----------

// Track macros
// ------------
static long midi_track_begin(FILE *midi)
{
	const char *MTrk = "MTrk";
	long track_start = 0;

	fwrite(MTrk, 4, 1, midi);
	fwrite(&track_start, 4, 1, midi);
	return ftell(midi);
}

static void midi_track_end(FILE* midi, long track_start)
{
	long track_length;
	long track_end;

	midi_meta_track_end_write(midi);

	// Fix up length
	track_end = ftell(midi);
	fseek(midi, track_start - 4, SEEK_SET);
	track_length = BE32(track_end - track_start);
	fwrite(&track_length, 4, 1, midi);
	fseek(midi, track_end, SEEK_SET);
}
// ------------

static void midi_header_write(FILE *midi, uint16 beat_distance)
{
	const char *MThd = "MThd";
	const uint32 chunk_size = BE32(6);
	const uint16 format = BE16(1);
	const uint16 track_count = BE16(hashtable_length(&vchans) + 1);
	const uint16 time_division = BE16(beat_distance);

	// Sequence name
	// TODO: Should write all original song tags, somehow
	const char sequence_name_type = META_TRACK_NAME;
	const char *sequence_name = "Generated by aosdk";
	size_t sequence_name_len = strlen(sequence_name);

	// Tempo
	const char tempo_type = META_TEMPO;
	const char tempo_len = 3;
	const uint32 tempo = SWAP24(60000000.0 / samples_to_bpm(beat_distance));

	long track_start;

	fwrite(MThd, 4, 1, midi);
	fwrite(&chunk_size, 4, 1, midi);
	fwrite(&format, 2, 1, midi);
	fwrite(&track_count, 2, 1, midi);
	fwrite(&time_division, 2, 1, midi);

	track_start = midi_track_begin(midi);

	midi_varlen_write(midi, 0); // delta time
	fwrite(&MIDI_META_EVENT, 1, 1, midi);
	fwrite(&sequence_name_type, 1, 1, midi);
	fwrite(&sequence_name_len, 1, 1, midi);
	fwrite(sequence_name, sequence_name_len, 1, midi);

	midi_varlen_write(midi, 0); // delta time
	fwrite(&MIDI_META_EVENT, 1, 1, midi);
	fwrite(&tempo_type, 1, 1, midi);
	fwrite(&tempo_len, 1, 1, midi);
	fwrite(&tempo, tempo_len, 1, midi);

	midi_track_end(midi, track_start);
}

static void midi_track_write(FILE *midi, const vchan_t *vchan, char midi_channel)
{
	const event_t *event;
	uint32 time_prev = 0;

	long track_start = midi_track_begin(midi);

	// Instrument name
	const char type = META_TRACK_NAME;
	char track_name[2 + 8 + 1];
	char track_name_len = sprintf(track_name, "0x%x", vchan->id);

	midi_varlen_write(midi, 0); // delta time
	fwrite(&MIDI_META_EVENT, 1, 1, midi);
	fwrite(&type, 1, 1, midi);
	fwrite(&track_name_len, 1, 1, midi);
	fwrite(&track_name, track_name_len, 1, midi);

	// Event data
	event = vchan->first;
	while(event) {
		uint32 time_delta = event->time - time_prev;
		unsigned char type_and_channel =
			((event->type & 0xF) << 4) | (midi_channel & 0xF);

		midi_varlen_write(midi, time_delta);
		fwrite(&type_and_channel, 1, 1, midi);
		fwrite(&event->param.word, 2, 1, midi);

		time_prev = event->time;
		event = event->next;
	}
	midi_track_end(midi, track_start);
}
/// -----------

/// Public API
/// ----------
void mididump_vchan_note_on(int vchan_id, char note, char velocity)
{
	event_param_t param = {note, velocity};
	vchan_event_push(vchans_get(vchan_id), NOTE_ON, param);
}

void mididump_vchan_note_off(int vchan_id, char note, char velocity)
{
	event_param_t param = {note, velocity};
	vchan_event_push(vchans_get(vchan_id), NOTE_OFF, param);
}

void mididump_vchan_ctl7_set(int vchan_id, int8 ctl7, int8 val)
{
	vchan_ctl7_push(vchans_get(vchan_id), ctl7, val);
}

void mididump_vchan_ctl14_set(int vchan_id, int8 ctl14, int16 val)
{
	vchan_t *vchan = vchans_get(vchan_id);

	assert(ctl14 >= 0 && ctl14 <= 0x1F);
	assert(val >= 0 && val <= 0x3FFF);

	vchan_ctl7_push(vchan, MSB(ctl14), (val >> 7) & 0x7F);
	vchan_ctl7_push(vchan, LSB(ctl14), (val >> 0) & 0x7F);
}

ao_bool mididump_write(const char *fn)
{
	uint16 beat_distance;
	hashtable_iterator_t iter = {0};
	vchan_t *vchan = vchans_iterate(&iter);
	FILE *midi;
	char midi_channel = 0;
	int i = 0;
	int vchans_len = hashtable_length(&vchans);
	vchan_t **vchans_sorted = NULL;

	if(!vchan) {
		return false;
	}

	printf("Writing MIDI data... ");

	vchans_sorted = malloc(vchans_len * sizeof(*vchans_sorted));

	do {
		vchans_sorted[i++] = vchan;
	} while( (vchan = vchans_iterate(&iter)) );

	qsort(
		vchans_sorted, i, sizeof(*vchans_sorted),
		(int (*)(const void *, const void *))vchan_compare
	);

	printf("Analyzing BPM... ");
	beat_distance = mididump_beat_distance_find();
	if(beat_distance == 0) {
		beat_distance = 22050;
		printf(
			"(piece too slow, falling back to %.2f BPM)\n",
			samples_to_bpm(beat_distance)
		);
	} else {
		printf("%.2f\n", samples_to_bpm(beat_distance));
	}

	midi = midi_open(fn);
	midi_header_write(midi, beat_distance);
	for(i = 0; i < vchans_len; i++) {
		midi_track_write(midi, vchans_sorted[i], midi_channel++);
		// Mistakenly allocating a melody track to the drum channel is
		// way more more annoying than mistakely playing back percussion
		// with the Grand Piano, so...
		if((midi_channel & 0xF) == 9) {
			midi_channel++;
		}
	}
	fclose(midi);
	free(vchans_sorted);
	return true;
}

void mididump_free(void)
{
	vchans_free();
}
/// ----------
