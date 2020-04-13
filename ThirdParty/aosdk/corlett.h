//
// Audio Overload
// Emulated music player
//
// (C) 2000-2008 Richard F. Bannister
//

// corlett.h

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	hashtable_t tags;
	char *tag_buffer;

	uint32 *res_section;
	uint32 res_size;
} corlett_t;

// Engine-specific callback function called during corlett_decode() for all
// the libraries referenced in a Corlett file, as well as the main file
// itself. The function will be called in this order, and with these values
// for [libnum]:
//
//	• 1 ("_lib")
//	• 2 ("_lib2"), 3 ("_lib3"), 4, …, 9 ("_lib9")
// 	• 0 (the main file itself)
//
// All pointers passed for [lib] and [c] stay valid until after the final
// callback for library #0. This allows the callback function to change the
// order in which to process the libraries by caching the pointers 
// applying their data in later calls.
//
// The function should return either AO_SUCCESS, or AO_FAIL if an error
// occurred and corlett_decode() should be aborted.
typedef int corlett_lib_callback_t(int libnum, uint8 *lib, uint64 size, corlett_t *c);

int corlett_decode(uint8 *input, uint32 input_len, corlett_t *c, corlett_lib_callback_t *lib_callback);
void corlett_free(corlett_t *c);

// Returns a writable pointer to the tag data, which is created if it doesn't
// exist yet.
const char** corlett_tag_get(corlett_t *c, const char *tag);

// Read-only tag lookup. Returns NULL if there is no data for [tag].
const char* corlett_tag_lookup(corlett_t *c, const char *tag);

int corlett_tag_recognize(corlett_t *c, const char **target_value, int tag_num, const char *key);
void corlett_length_set(double length_seconds, double fade_seconds);
uint32 corlett_sample_count(void);
uint32 corlett_sample_total(void);
void corlett_sample_fade(stereo_sample_t *sample);
double psfTimeToSeconds(const char *str);

#ifdef __cplusplus
}
#endif
