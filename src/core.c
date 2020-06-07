#include "core.h"

#include "hashmap.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define ASSIGN_OR_DIE_IF_NULL(name, value) \
    { \
        (name) = (value); \
        if ((name) == NULL) \
            abort(); \
    }

static struct core *core;

struct core *get_core(void) {
    return core;
}

#define HASH_BUF_LEN 1024

static char _hash_buf[HASH_BUF_LEN];

static unsigned int hash_str_nocase(const void *key)
{
    const char *key_str = key;
    size_t key_len = strlen(key_str);

    if (key_len >= HASH_BUF_LEN)
        abort();

    for (size_t i = 0; i < key_len; i++)
        _hash_buf[i] = tolower(key_str[i]);
    _hash_buf[key_len] = 0;

    return hash_str(_hash_buf);
}

static int compare_str_nocase(const void *k1, const void *k2)
{
    return strcasecmp(k1, k2);
}

static const void *create_key_str(const void *key)
{
    return strdup(key);
}

static void destroy_key_str(void *key)
{
    free(key);
}

void init_core(void) {
    struct hashmap_descriptor desc =
        {
            .hash = hash_str_nocase,
            .compare = compare_str_nocase,
            .create_key = create_key_str,
            .destroy_key = destroy_key_str,
        };

    ASSIGN_OR_DIE_IF_NULL(core, malloc(sizeof(struct core)));

    memset(core, 0, sizeof(struct core));

    ASSIGN_OR_DIE_IF_NULL(core->users, (void *)hashmap_new(&desc));
    ASSIGN_OR_DIE_IF_NULL(core->nicks, (void *)hashmap_new(&desc));
    ASSIGN_OR_DIE_IF_NULL(core->bots, (void *)hashmap_new(&desc));
    ASSIGN_OR_DIE_IF_NULL(core->clones, (void *)hashmap_new(&desc));
    ASSIGN_OR_DIE_IF_NULL(core->modules, (void *)hashmap_new(&desc));
    ASSIGN_OR_DIE_IF_NULL(core->trusts, (void *)hashmap_new(&desc));
    ASSIGN_OR_DIE_IF_NULL(core->links, (void *)hashmap_new(&desc));
    ASSIGN_OR_DIE_IF_NULL(core->guests, (void *)hashmap_new(&desc));
    ASSIGN_OR_DIE_IF_NULL(core->chans, (void *)hashmap_new(&desc));
    ASSIGN_OR_DIE_IF_NULL(core->wchans, (void *)hashmap_new(&desc));
    ASSIGN_OR_DIE_IF_NULL(core->fakeusers, (void *)hashmap_new(&desc));

    LLIST_INIT(&core->limits);
    LLIST_INIT(&core->timebans);
}

void free_core(void) {
    HASHMAP_FREE(core->users);
    HASHMAP_FREE(core->nicks);
    HASHMAP_FREE(core->bots);
    HASHMAP_FREE(core->clones);
    HASHMAP_FREE(core->modules);
    HASHMAP_FREE(core->trusts);
    HASHMAP_FREE(core->links);
    HASHMAP_FREE(core->guests);
    HASHMAP_FREE(core->chans);
    HASHMAP_FREE(core->wchans);
    HASHMAP_FREE(core->fakeusers);

    free(core);
    core = NULL;
}
