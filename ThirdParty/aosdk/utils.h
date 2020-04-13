/*
 * Audio Overload SDK
 *
 * Random utility functions
 *
 * Author: Nmlgc
 */

#ifndef UTILS_H
#define UTILS_H

/// Hash table
/// ----------
// This hash table maps variable-size keys to constant-size values.  Before
// calling hashtable_get(), the hash table has to be initialized to the
// desired data size by calling hashtable_init().

typedef struct {
	void *buf;
	size_t len;
} blob_t;

typedef struct hashtable_bucket {
	struct hashtable_bucket *next;
	blob_t key;
	uint8 data[0];
} hashtable_bucket_t;

typedef struct {
	unsigned int i;
	hashtable_bucket_t *bucket;
} hashtable_iterator_t;

typedef struct {
	size_t data_size;
	hashtable_bucket_t *buckets;
} hashtable_t;

// Flags used for hash table lookup
typedef enum {
	HT_CREATE = 0x1, // create nonexisting entries
	HT_CASE_INSENSITIVE = 0x2, // case-insensitive key lookup
} hashtable_flags_t;

// Initializes the hash table with the given data size.
ao_bool hashtable_init(hashtable_t *table, size_t data_size);

// Looks up the entry in [table] with the given [key] and the given [flags].
// Returns a writable pointer to the entry data, or NULL if the entry doesn't
// exist and the HT_CREATE flag was not given.
void* hashtable_get(hashtable_t *table, const blob_t *key, hashtable_flags_t flags);

// Iterates over all entries in [table], using [iter] to store the iteration
// state.  [iter] should be cleared to 0 before the first call. Returns the
// current data entry as well as the [key] if that pointer is not NULL, or
// NULL if the end of [table] has been reached.
void* hashtable_iterate(blob_t **key, hashtable_t *table, hashtable_iterator_t *iter);

// Returns the number of entries in [table].
unsigned int hashtable_length(hashtable_t *table);

// Frees the hash table, assuming that any allocated blocks within the data
// already have been freed.
void hashtable_free(hashtable_t *table);
/// ----------

// Opens "[fn].[suffix]" for writing.
FILE* fopen_derivative(const char *fn, const char *suffix);

#endif /* UTILS_H */
