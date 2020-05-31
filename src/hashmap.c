#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "hashmap.h"

struct hashmap *hashmap_new(unsigned int (*hash)(const void *key),
                            int (*compare)(const void *k1, const void *k2))
{
    struct hashmap *hm;
    unsigned int i;

    if (!hash || !compare)
        return NULL;

    hm = malloc(sizeof(*hm));
    if (!hm)
        return NULL;

    hm->bucket_count = HASHMAP_DEFAULT_BUCKET_COUNT;
    hm->size = 0;

    hm->map = malloc(hm->bucket_count * sizeof(struct llist_head));
    if (!hm->map) {
        free(hm);
        return NULL;
    }

    for (i = 0; i < hm->bucket_count; i++)
        LLIST_INIT(&hm->map[i]);

    LLIST_INIT(&hm->keys);

    hm->hash = hash;
    hm->compare = compare;

    return hm;
}

void hashmap_free(struct hashmap *hm)
{
    hashmap_flush(hm);
    free(hm->map);
    free(hm);
}

int hashmap_bucket(const struct hashmap *hm, const void *key)
{
    return hm->hash(key) & (hm->bucket_count - 1);
}

static bool hashmap_must_grow(const struct hashmap *hm)
{
    return hm->size > (hm->bucket_count / 4 * 3);
}

static int hashmap_resize(struct hashmap *hm, size_t new_bucket_count)
{
    struct llist_head *new_map;
    struct hashmap_entry *he;
    unsigned int idx, i;

    new_map = malloc(new_bucket_count * sizeof(struct llist_head));
    if (!new_map)
        return -1;

    for (i = 0; i < new_bucket_count; i++)
        LLIST_INIT(&new_map[i]);

    // New bucket count must be set before rehashing for hashmap_bucket()
    // to return the correct value.
    hm->bucket_count = new_bucket_count;

    LLIST_FOREACH_ENTRY(&hm->keys, he, key_head) {
        LLIST_REMOVE(&he->map_head);
        idx = hashmap_bucket(hm, he->key);
        LLIST_INSERT_TAIL(&new_map[idx], &he->map_head);
    }

    free(hm->map);
    hm->map = new_map;

    return 0;
}

bool hashmap_insert(struct hashmap *hm, const void *key, void *value,
                    struct hashmap_entry **entry)
{
    struct hashmap_entry *he;
    unsigned int idx;

    idx = hashmap_bucket(hm, key);

    LLIST_FOREACH_ENTRY(&hm->map[idx], he, map_head) {
        if (hm->compare(he->key, key) == 0) {
            if (entry)
                *entry = he;
            return false;
        }
    }

    if (hashmap_must_grow(hm)) {
        if (hashmap_resize(hm, hm->bucket_count * 2) < 0)
            return false;

        idx = hashmap_bucket(hm, key);
    }

    he = malloc(sizeof(*he));
    if (!he)
        return false;

    he->key = key;
    he->value = value;

    LLIST_INSERT_TAIL(&hm->map[idx], &he->map_head);
    LLIST_INSERT_TAIL(&hm->keys, &he->key_head);

    hm->size++;

    if (entry)
        *entry = he;

    return true;
}

bool hashmap_find(const struct hashmap *hm, const void *key,
                  struct hashmap_entry **entry)
{
    struct hashmap_entry *he;
    unsigned int idx;

    idx = hashmap_bucket(hm, key);

    LLIST_FOREACH_ENTRY(&hm->map[idx], he, map_head) {
        if (hm->compare(he->key, key) == 0) {
            if (entry)
                *entry = he;
            return true;
        }
    }

    return false;
}

bool hashmap_erase(struct hashmap *hm, const void *key)
{
    struct hashmap_entry *he;
    unsigned int idx;

    idx = hashmap_bucket(hm, key);

    LLIST_FOREACH_ENTRY(&hm->map[idx], he, map_head) {
        if (hm->compare(he->key, key) == 0) {
            LLIST_REMOVE(&he->map_head);
            LLIST_REMOVE(&he->key_head);
            hm->size--;
            free(he);
            return true;
        }
    }

    return false;
}

void hashmap_flush(struct hashmap *hm)
{
    struct hashmap_entry *he;

    while (!LLIST_EMPTY(&hm->keys)) {
        he = LLIST_FIRST_ENTRY(&hm->keys, struct hashmap_entry, key_head);
        LLIST_REMOVE(&he->key_head);
        LLIST_REMOVE(&he->map_head);
        hm->size--;
        free(he);
    }
}
