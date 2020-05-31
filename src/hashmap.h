// Hashmap implementation, not thread-safe.
// Inspired from research code I wrote during my PhD thesis.
// https://github.com/target0/thesis-data/blob/master/sdnres-src/lib/hashmap.h

#ifndef _HASHMAP_H
#define _HASHMAP_H

#include <stdint.h>
#include <string.h>

#include "llist.h"

#define HASHMAP_DEFAULT_BUCKET_COUNT	16

struct hashmap_entry {
	const void *key;
	void *value;
	struct llist_head map_head;
	struct llist_head key_head;
};

struct hashmap {
	size_t size;
	size_t bucket_count;
	struct llist_head *map;
	struct llist_head keys;

	unsigned int (*hash)(const void *key);
	int (*compare)(const void *k1, const void *k2);
};

#define HASHMAP_FOREACH_ENTRY(hmap, elem)	\
	LLIST_FOREACH_ENTRY(&(hmap)->keys, elem, key_head)

#define HASHMAP_FOREACH_ENTRY_SAFE(hmap, elem, tmp)	\
	LLIST_FOREACH_ENTRY_SAFE(&(hmap)->keys, elem, tmp, key_head)

#define HASHMAP_EMPTY(hmap)	\
	LLIST_EMPTY(&(hmap)->keys)

struct hashmap *hashmap_new(unsigned int (*hash)(const void *key),
			 int (*compare)(const void *k1, const void *k2));
void hashmap_free(struct hashmap *hm);
int hashmap_bucket(const struct hashmap *hm, const void *key);
bool hashmap_insert(struct hashmap *hm, const void *key, void *value,
                    struct hashmap_entry **entry);
bool hashmap_find(const struct hashmap *hm, const void *key,
                  struct hashmap_entry **entry);
bool hashmap_erase(struct hashmap *hm, const void *key);
void hashmap_flush(struct hashmap *hm);

static inline int hashmap_size(const struct hashmap *hm)
{
    return hm->size;
}

static inline int compare_str(const void *k1, const void *k2)
{
	return strcmp((char *)k1, (char *)k2);
}

// Source: http://www.cse.yorku.ca/~oz/hash.html
static inline unsigned int hash_str(const void *key)
{
	const unsigned char *str = key;
	unsigned int hash = 5381;
	unsigned int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;

	return hash;
}

static inline struct hashmap *hashmap_str_new(void)
{
    return hashmap_new(hash_str, compare_str);
}

static inline int compare_int(const void *k1, const void *k2)
{
	return (intptr_t)k1 != (intptr_t)k2;
}

// Source: https://github.com/skeeto/hash-prospector
static inline unsigned int hash_int(void *key)
{
	unsigned int x = (uintptr_t)key;

	x++;
	x ^= x >> 17;
	x *= UINT32_C(0xed5ad4bb);
	x ^= x >> 11;
	x *= UINT32_C(0xac4c1b51);
	x ^= x >> 15;
	x *= UINT32_C(0x31848bab);
	x ^= x >> 14;

	return x;
}

#endif
