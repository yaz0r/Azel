/*
 * Audio Overload SDK
 *
 * Implementations of functions shared by all audio backends
 */

#include <stdio.h>
#include "ao.h"
#include "m1sdr.h"

m1sdr_callback_t *m1sdr_Callback;
ao_bool hw_present;

void m1sdr_SetCallback(m1sdr_callback_t *function)
{
	if (function == NULL)
	{
		printf("ERROR: NULL CALLBACK!\n");
	}

	m1sdr_Callback = function;
}

ao_bool m1sdr_HwPresent(void)
{
	return hw_present;
}
