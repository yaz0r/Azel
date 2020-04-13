/*
 * Audio Overload SDK
 *
 * Random utility functions
 *
 * Author: Nmlgc
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ao.h"
#include "utils.h"

/// Hash table
/// ----------
#define BUCKETS 15

// From http://www.cse.yorku.ca/~oz/hash.html
static unsigned long hash(const blob_t *key)
{
	unsigned long hash = 5381;
	const char *key_buf = (const char*)(key->buf);
	int i;
	for(i = 0; i < key->len; i++) {
		hash = ((hash << 5) + hash) ^ key_buf[i]; // hash * 33 ^ c
	}
	return hash;
}

// MSVC has memicmp(), but GCC does not, so this has to have a different name.
static int memcasecmp(const void *s1, const void *s2, size_t n)
{
	const char *sc1 = (const char *)s1;
	const char *sc2 = (const char *)s2;
	const char *sc2end = sc2 + n;

	while (sc2 < sc2end && toupper(*sc1) == toupper(*sc2)) {
		sc1++, sc2++;
	}
	if (sc2 == sc2end) {
		return 0;
	}
	return (int) (toupper(*sc1) - toupper(*sc2));
}

static int bucket_match(const hashtable_bucket_t *bucket, const blob_t *key, hashtable_flags_t flags)
{
	if(bucket->key.len != key->len) {
		return 0;
	}
	return flags & HT_CASE_INSENSITIVE
		? !memcasecmp(bucket->key.buf, key->buf, key->len)
		: !memcmp(bucket->key.buf, key->buf, key->len);
}

static size_t bucket_size(const hashtable_t *table)
{
	assert(table);
	assert(table->data_size);
	return sizeof(hashtable_bucket_t) + table->data_size;
}

static hashtable_bucket_t* bucket_get(const hashtable_t *table, unsigned int i)
{
	size_t size = bucket_size(table);
	return (hashtable_bucket_t*)((char*)(table->buckets) + i * size);
}

ao_bool hashtable_init(hashtable_t *table, size_t data_size)
{
	assert(table);
	assert(data_size > 0);
	if(table->buckets) {
		return false;
	}
	table->data_size = data_size;
	table->buckets = calloc(BUCKETS, sizeof(hashtable_bucket_t) + data_size);
	return true;
}

void* hashtable_get(hashtable_t *table, const blob_t *key, hashtable_flags_t flags)
{
	hashtable_bucket_t *bucket;
	ao_bool create = flags & HT_CREATE;

	assert(key);
	assert(key->buf);
	assert(key->len);

	if(!table->buckets) {
		return NULL;
	}

	bucket = bucket_get(table, hash(key) % BUCKETS);

	while(bucket && bucket->key.buf && !bucket_match(bucket, key, flags)) {
		if(!bucket->next && create) {
			bucket->next = calloc(1, bucket_size(table));
		}
		bucket = bucket->next;
	}
	if(bucket && !bucket->key.buf && create) {
		bucket->key.len = key->len;
		bucket->key.buf = malloc(key->len);
		memcpy(bucket->key.buf, key->buf, key->len);
	} else if(!bucket && !create) {
		return NULL;
	}
	return bucket->data;
}

void* hashtable_iterate(blob_t **key, hashtable_t *table, hashtable_iterator_t *iter)
{
	hashtable_t *ret = NULL;
	assert(table);
	assert(iter);
	if(!table->buckets) {
		return NULL;
	}
	while(iter->i < BUCKETS) {
		if(!iter->bucket) {
			iter->bucket = bucket_get(table, iter->i);
		}
		if(iter->bucket->key.buf) {
			ret = (void*)iter->bucket->data;
			if(key) {
				*key = &iter->bucket->key;
			}
		}
		if(iter->bucket->next) {
			iter->bucket = iter->bucket->next;
		} else {
			iter->i++;
			iter->bucket = NULL;
		}
		if(ret) {
			return ret;
		}
	}
	return ret;
}

unsigned int hashtable_length(hashtable_t *table)
{
	unsigned int ret = 0;
	hashtable_iterator_t iter = {0};
	assert(table);
	while(hashtable_iterate(NULL, table, &iter)) {
		ret++;
	}
	return ret;
}

void hashtable_free(hashtable_t *table)
{
	int i;
	assert(table);
	if(!table->buckets) {
		return;
	}
	for(i = 0; i < BUCKETS; i++) {
		// We only have to free the key of the first entry in each
		// bucket's list, as it was allocated as part of a single array
		// in hashtable_init().
		hashtable_bucket_t *bucket = bucket_get(table, i);
		if(bucket->key.buf) {
			free(bucket->key.buf);
		}
		bucket = bucket->next;
		while(bucket) {
			hashtable_bucket_t *bucket_cur = bucket;
			bucket = bucket->next;
			if(bucket_cur->key.buf) {
				free(bucket_cur->key.buf);
			}
			free(bucket_cur);
		}
	}
}
/// ----------

FILE* fopen_derivative(const char *fn, const char *suffix)
{
	size_t full_fn_len = strlen(fn) + strlen(suffix) + 1;
	char *full_fn = malloc(full_fn_len);
	FILE *ret = NULL;

	if(!full_fn) {
		fprintf(stderr, "Out of memory\n");
		return NULL;
	}

	sprintf(full_fn, "%s%s", fn, suffix);
	ret = ao_fopen(full_fn, "wb");
	if(!ret) {
		fprintf(stderr, "Error opening %s for writing: %s\n", full_fn, strerror(errno));
	}
	free(full_fn);
	return ret;
}
