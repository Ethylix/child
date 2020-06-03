#include "core.h"

#include "hashmap.h"

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

void init_core(void) {
    ASSIGN_OR_DIE_IF_NULL(core, malloc(sizeof(struct core)));

    memset(core, 0, sizeof(struct core));

    ASSIGN_OR_DIE_IF_NULL(core->users, hashmap_str_new());
    ASSIGN_OR_DIE_IF_NULL(core->nicks, hashmap_str_new());
    ASSIGN_OR_DIE_IF_NULL(core->bots, (void *)hashmap_str_new());
}

void free_core(void) {
    hashmap_free(core->users);
    hashmap_free(core->nicks);
    hashmap_free(ACCESS_HASHMAP(core->bots));

    free(core);
    core = NULL;
}
