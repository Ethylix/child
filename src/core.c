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

struct core *get_core() {
    return core;
}

void init_core() {
    ASSIGN_OR_DIE_IF_NULL(core, malloc(sizeof(struct core)));

    memset(core, 0, sizeof(struct core));

    ASSIGN_OR_DIE_IF_NULL(core->bots, hashmap_str_new());
}
