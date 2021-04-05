#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include "child.h"
#include "commands.h"
#include "core.h"
#include "core_api.h"
#include "hashmap.h"
#include "llist.h"
#include "net.h"

START_TEST(test_new_nick)
{
    Nick *nptr;

    init_core();

    nptr = get_core_api()->new_nick(/*nick=*/"test",
                                    /*ident=*/"test_ident",
                                    /*host=*/"test_host",
                                    /*uid=*/"042AABBCC",
                                    /*hiddenhost=*/"test_hiddenhost",
                                    /*umodes=*/UMODE_BOT | UMODE_SSL,
                                    /*reshost=*/"127.0.0.1");

    ck_assert_ptr_ne(nptr, NULL);
    ck_assert_str_eq(nptr->nick, "test");
    ck_assert_str_eq(nptr->ident, "test_ident");
    ck_assert_str_eq(nptr->host, "test_host");
    ck_assert_str_eq(nptr->uid, "042AABBCC");
    ck_assert_str_eq(nptr->hiddenhost, "test_hiddenhost");
    ck_assert_str_eq(nptr->reshost, "127.0.0.1");
    ck_assert_int_eq(nptr->umodes, UMODE_BOT | UMODE_SSL);
    ck_assert_int_eq(LLIST_EMPTY(&nptr->wchans), true);

    ck_assert_int_eq(HASHMAP_SIZE(core_get_nicks()), 1);
}
END_TEST

START_TEST(test_find_nick)
{
    Nick *nptr;

    init_core();

    nptr = get_core_api()->new_nick(/*nick=*/"test",
                                    /*ident=*/"test_ident",
                                    /*host=*/"test_host",
                                    /*uid=*/"042AABBCC",
                                    /*hiddenhost=*/"test_hiddenhost",
                                    /*umodes=*/UMODE_BOT | UMODE_SSL,
                                    /*reshost=*/"127.0.0.1");

    ck_assert_ptr_ne(nptr, NULL);
    ck_assert_ptr_eq(nptr, get_core_api()->find_nick("test"));
    ck_assert_ptr_eq(nptr, get_core_api()->find_nick("042AABBCC"));
    ck_assert_ptr_eq(get_core_api()->find_nick("non_existent"), NULL);
}
END_TEST

START_TEST(test_delete_nick)
{
    Nick *nptr1, *nptr2;

    init_core();

    nptr1 = get_core_api()->new_nick(/*nick=*/"test",
                                     /*ident=*/"test_ident",
                                     /*host=*/"test_host",
                                     /*uid=*/"042AABBCC",
                                     /*hiddenhost=*/"test_hiddenhost",
                                     /*umodes=*/UMODE_BOT | UMODE_SSL,
                                     /*reshost=*/"127.0.0.1");

    ck_assert_ptr_ne(nptr1, NULL);
    ck_assert_int_eq(HASHMAP_SIZE(core_get_nicks()), 1);

    nptr2 = get_core_api()->new_nick(/*nick=*/"test2",
                                     /*ident=*/"test_ident2",
                                     /*host=*/"test_host2",
                                     /*uid=*/"042AABBDD",
                                     /*hiddenhost=*/"test_hiddenhost2",
                                     /*umodes=*/UMODE_BOT | UMODE_SSL,
                                     /*reshost=*/"127.0.0.2");

    ck_assert_ptr_ne(nptr2, NULL);
    ck_assert_int_eq(HASHMAP_SIZE(core_get_nicks()), 2);

    get_core_api()->delete_nick(nptr1);
    ck_assert_int_eq(HASHMAP_SIZE(core_get_nicks()), 1);
    ck_assert_ptr_eq(get_core_api()->find_nick("test"), NULL);
    ck_assert_ptr_eq(get_core_api()->find_nick("042AABBCC"), NULL);
    ck_assert_ptr_eq(get_core_api()->find_nick("test2"), nptr2);
    ck_assert_ptr_eq(get_core_api()->find_nick("042AABBDD"), nptr2);
}
END_TEST

START_TEST(test_clear_nicks)
{
    Nick *nptr;

    init_core();

    nptr = get_core_api()->new_nick(/*nick=*/"test",
                                    /*ident=*/"test_ident",
                                    /*host=*/"test_host",
                                    /*uid=*/"042AABBCC",
                                    /*hiddenhost=*/"test_hiddenhost",
                                    /*umodes=*/UMODE_BOT | UMODE_SSL,
                                    /*reshost=*/"127.0.0.1");
    ck_assert_ptr_ne(nptr, NULL);

    nptr = get_core_api()->new_nick(/*nick=*/"test2",
                                    /*ident=*/"test_ident2",
                                    /*host=*/"test_host2",
                                    /*uid=*/"042AABBDD",
                                    /*hiddenhost=*/"test_hiddenhost2",
                                    /*umodes=*/UMODE_BOT | UMODE_SSL,
                                    /*reshost=*/"127.0.0.2");
    ck_assert_ptr_ne(nptr, NULL);

    ck_assert_int_eq(HASHMAP_SIZE(core_get_nicks()), 2);
    get_core_api()->clear_nicks();
    ck_assert_int_eq(HASHMAP_SIZE(core_get_nicks()), 0);
}
END_TEST

Suite *nick_api_suite(void)
{
    Suite *s;
    TCase *tc;

    s = suite_create("nick_api");

    tc = tcase_create("new_nick");
    tcase_add_test(tc, test_new_nick);
    suite_add_tcase(s, tc);

    tc = tcase_create("find_nick");
    tcase_add_test(tc, test_find_nick);
    suite_add_tcase(s, tc);

    tc = tcase_create("delete_nick");
    tcase_add_test(tc, test_delete_nick);
    suite_add_tcase(s, tc);

    tc = tcase_create("clear_nicks");
    tcase_add_test(tc, test_clear_nicks);
    suite_add_tcase(s, tc);

    return s;
}

int main(void)
{
    Suite *s;
    SRunner *sr;
    int number_failed;

    s = nick_api_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
