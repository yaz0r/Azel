/*
 * Audio Overload SDK
 *
 * Audio backend function declarations
 */

#ifndef M1SDR_H
#define M1SDR_H

typedef enum {
	M1SDR_OK,
	M1SDR_ERROR,
	M1SDR_WAIT
} m1sdr_ret_t;

typedef void m1sdr_callback_t(
	unsigned long sample_count, stereo_sample_t *buffer
);
extern m1sdr_callback_t *m1sdr_Callback;

extern ao_bool hw_present;

// Shared and implemented in m1sdr.c
void m1sdr_SetCallback(m1sdr_callback_t *function);
ao_bool m1sdr_HwPresent(void);

// Backend-specific
INT16 m1sdr_Init(char *device, int sample_rate);
void m1sdr_PrintDevices(void);
void m1sdr_Exit(void);
void m1sdr_PlayStart(void);
void m1sdr_PlayStop(void);
m1sdr_ret_t m1sdr_TimeCheck(void);
void m1sdr_SetSamplesPerTick(UINT32 spf);
void m1sdr_FlushAudio(void);
void m1sdr_Pause(int);
void m1sdr_SetNoWait(int nw);

#endif /* M1SDR_H */
