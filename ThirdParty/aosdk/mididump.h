/*
 * Audio Overload SDK
 *
 * MIDI dumping
 *
 * Author: Nmlgc
 */

extern ao_bool nomidi;

typedef enum {
	// MSB values of the 14-bit controller types from 0x00 to 0x31.
	// Use mididump_vchan_ctl14_set() to set those controllers to a 14-bit
	// value, which will emit MSB, LSB, or both CC events as necessary.
	CTL14_BANK_SELECT = 0,
	CTL14_MODULATION = 1,
	CTL14_BREATH_CONTROLLER = 2,
	CTL14_FOOT_CONTROLLER = 4,
	CTL14_PORTAMENTO_TIME = 5,
	CTL14_DATA_ENTRY = 6,
	CTL14_VOLUME = 7,
	CTL14_BALANCE = 8,
	CTL14_PAN = 10,
	CTL14_EXPRESSION = 11,
	CTL14_EFFECT_CONTROL_1 = 12,
	CTL14_EFFECT_CONTROL_2 = 13,
	CTL14_GEN_PURPOSE_CONTROL_1 = 16,
	CTL14_GEN_PURPOSE_CONTROL_2 = 17,
	CTL14_GEN_PURPOSE_CONTROL_3 = 18,
	CTL14_GEN_PURPOSE_CONTROL_4 = 19,

	// 7-bit controllers
	CTL7_HOLD_PEDAL = 64,
	CTL7_PORTAMENTO = 65,
	CTL7_SOSTENUTO = 66,
	CTL7_SOFT_PEDAL = 67,
	CTL7_LEGATO_PEDAL = 68,
	CTL7_HOLD_2 = 69,

	// http://www.midi.org/techspecs/rp21.php
	CTL7_SOUND_VARIATION = 70,
	CTL7_HARMONIC_INTENSITY = 71,
	CTL7_RELEASE_TIME = 72,
	CTL7_ATTACK_TIME = 73,
	CTL7_BRIGHTNESS = 74,
	CTL7_DECAY_TIME = 75,
	CTL7_VIBRATO_RATE = 76,
	CTL7_VIBRATO_DEPTH = 77,
	CTL7_VIBRATO_DELAY = 78,

	CTL7_GEN_PURPOSE_BUTTON_1 = 80,
	CTL7_GEN_PURPOSE_BUTTON_2 = 81,
	CTL7_GEN_PURPOSE_BUTTON_3 = 82,
	CTL7_GEN_PURPOSE_BUTTON_4 = 83,
	CTL7_PORTAMENTO_TIME = 84,

	// http://www.midi.org/techspecs/ca31.pdf
	CTL7_HIRES_VELOCITY_PREFIX = 88,

	CTL7_REVERB_LEVEL = 91,
	CTL7_FX2_LEVEL = 92,
	CTL7_CHORUS_LEVEL = 93,
	CTL7_VARIATION_LEVEL = 94,
	CTL7_FX5_LEVEL = 95,
	CTL7_DATA_INCREMENT = 96,
	CTL7_DATA_DECREMENT = 97,
	CTL7_NRPN_LSB = 98,
	CTL7_NRPN_MSB = 99,
	CTL7_RPN_LSB = 100,
	CTL7_RPN_MSB = 101,
	CTL7_ALL_SOUND_OFF = 120,
	CTL7_RESET_ALL_CONTROLLERS = 121,
	CTL7_LOCAL_CONTROL = 122,
	CTL7_ALL_NOTE_OFF = 123,
	CTL7_OMNI_MODE_OFF = 124,
	CTL7_OMNI_MODE_ON = 125,
	CTL7_MONO = 126,
	CTL7_POLY = 127,
} controller_type_t;

void mididump_vchan_note_on(int vchan, char note, char velocity);
void mididump_vchan_note_off(int vchan, char note, char velocity);
void mididump_vchan_ctl14_set(int vchan, int8 ctl14, int16 val);
void mididump_vchan_ctl7_set(int vchan, int8 ctl7, int8 val);
ao_bool mididump_write(const char *fn);
void mididump_free(void);
