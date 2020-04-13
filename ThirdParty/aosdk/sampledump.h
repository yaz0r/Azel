/*
 * Audio Overload SDK
 *
 * Sample dumping
 *
 * Author: Nmlgc
 */

void sampledump_init(void);

// Returns whether the sample with the given ID hasn't been dumped yet.
ao_bool sampledump_is_new(int32 id);
