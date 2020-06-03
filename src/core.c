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

    ASSIGN_OR_DIE_IF_NULL(core->users, (void *)hashmap_str_new());
    ASSIGN_OR_DIE_IF_NULL(core->nicks, (void *)hashmap_str_new());
    ASSIGN_OR_DIE_IF_NULL(core->bots, (void *)hashmap_str_new());
    ASSIGN_OR_DIE_IF_NULL(core->clones, (void *)hashmap_str_new());
}

void free_core(void) {
    HASHMAP_FREE(core->users);
    HASHMAP_FREE(core->nicks);
    HASHMAP_FREE(core->bots);
    HASHMAP_FREE(core->clones);

    free(core);
    core = NULL;
}
