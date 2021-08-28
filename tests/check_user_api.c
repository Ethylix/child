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
#include "test_helpers.h"

START_TEST(test_check_user_password)
{
    User *uptr;

    init_core();

    uptr = create_mock_user("test_user", "test_password");
    ck_assert_ptr_ne(uptr, NULL);

    ck_assert_int_eq(check_user_password(uptr, "test_password"), 0);
    ck_assert_int_eq(check_user_password(NULL, "dummypassword"), -1);

    DeleteAccount(uptr);
    free_core();
}
END_TEST

START_TEST(test_set_user_password)
{
    User *uptr;

    init_core();

    uptr = create_mock_user("test_user", "test_password");
    ck_assert_ptr_ne(uptr, NULL);

    ck_assert_int_eq(set_user_password(uptr, "test_password2"), 0);
    ck_assert_int_eq(check_user_password(uptr, "test_password2"), 0);

    ck_assert_int_eq(check_user_password(NULL, "dummypassword"), -1);

    DeleteAccount(uptr);
    free_core();
}
END_TEST

Suite *make_user_api_suite(void)
{
    Suite *s;
    TCase *tc;

    s = suite_create("user_api");

    tc = tcase_create("check_user_password");
    tcase_add_test(tc, test_check_user_password);
    suite_add_tcase(s, tc);

    tc = tcase_create("set_user_password");
    tcase_add_test(tc, test_set_user_password);
    suite_add_tcase(s, tc);

    return s;
}
