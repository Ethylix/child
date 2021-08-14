#include "hashmap.h"

#include "llist.h"

#include <check.h>
#include <stdlib.h>

#ifndef ck_assert_ptr_null
#define ck_assert_ptr_null(x) ck_assert_ptr_eq(x, NULL)
#endif

#ifndef ck_assert_ptr_nonnull
#define ck_assert_ptr_nonnull(x) ck_assert_ptr_ne(x, NULL)
#endif

START_TEST(test_hashmap_new)
{
    struct hashmap *hm;
    struct hashmap_descriptor desc = {};

    hm = hashmap_new(&desc);
    ck_assert_ptr_null(hm);

    desc.hash = hash_str;
    hm = hashmap_new(&desc);
    ck_assert_ptr_null(hm);

    desc.hash = NULL;
    desc.compare = compare_str;
    hm = hashmap_new(&desc);
    ck_assert_ptr_null(hm);

    desc.hash = hash_str;
    hm = hashmap_new(&desc);
    ck_assert_ptr_nonnull(hm);
    ck_assert_int_eq(hm->size, 0);
    ck_assert_int_eq(hm->bucket_count, HASHMAP_DEFAULT_BUCKET_COUNT);

    ck_assert_ptr_eq(hm->hash, hash_str);
    ck_assert_ptr_eq(hm->compare, compare_str);

    for (size_t i = 0; i < hm->bucket_count; i++)
        ck_assert(LLIST_EMPTY(&hm->map[i]));

    ck_assert(LLIST_EMPTY(&hm->keys));

    hashmap_free(hm);
}
END_TEST

START_TEST(test_hashmap_insert)
{
    struct hashmap_entry *entry;
    struct hashmap *hm;
    bool inserted;

    struct hashmap_descriptor desc =
        {
            .hash = hash_str,
            .compare = compare_str,
        };

    hm = hashmap_new(&desc);
    ck_assert_ptr_nonnull(hm);

    inserted = hashmap_insert(hm, "one", (void *)1, &entry);
    ck_assert(inserted);
    ck_assert_ptr_nonnull(entry);
    ck_assert_str_eq(entry->key, "one");
    ck_assert_int_eq((intptr_t)entry->value, 1);

    inserted = hashmap_insert(hm, "one", (void *)42, &entry);
    ck_assert(!inserted);
    ck_assert_ptr_nonnull(entry);
    ck_assert_str_eq(entry->key, "one");
    ck_assert_int_eq((intptr_t)entry->value, 1);

    inserted = hashmap_insert(hm, "two", (void *)2, &entry);
    ck_assert(inserted);
    ck_assert_ptr_nonnull(entry);
    ck_assert_str_eq(entry->key, "two");
    ck_assert_int_eq((intptr_t)entry->value, 2);

    hashmap_free(hm);
}
END_TEST

Suite *make_hashmap_suite(void)
{
    Suite *s;
    TCase *tc;

    s = suite_create("hashmap");

    tc = tcase_create("hashmap_new");
    tcase_add_test(tc, test_hashmap_new);
    suite_add_tcase(s, tc);

    tc = tcase_create("hashmap_insert");
    tcase_add_test(tc, test_hashmap_insert);
    suite_add_tcase(s, tc);

    return s;
}
