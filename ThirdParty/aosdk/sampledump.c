/*
 * Audio Overload SDK
 *
 * Sample dumping
 *
 * Author: Nmlgc
 */

#include "ao.h"
#include "utils.h"

// int32 -> ao_bool
hashtable_t samples_seen;

void sampledump_init(void)
{
	hashtable_init(&samples_seen, sizeof(ao_bool));
}

ao_bool sampledump_is_new(int32 id)
{
	// TODO: Hm, the start address in sound memory may not be enough to
	// uniquely identify a sample? I can totally imagine different end
	// addresses and different loop points between those… but whatever.
	ao_bool *seen;
	blob_t id_blob = {&id, sizeof(id)};

	if(!samples_seen.buckets) {
		return false;
	}

	seen = (ao_bool*)hashtable_get(&samples_seen, &id_blob, HT_CREATE);
	if(!seen || *seen) {
		return false;
	}
	printf("Dumping sample at %#x.\n", id);
	*seen = true;
	return true;
}
