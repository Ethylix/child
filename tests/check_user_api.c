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

    // check md5 -> pwhash migration
    strncpy(uptr->md5_pass, "c4d8a57e2ca5dc5d71d2cf3dbbbbaabe", 32);
    ck_assert_int_eq(check_user_password(uptr, "testpassword2"), 0);
    ck_assert_str_eq(uptr->md5_pass, "");
    ck_assert_int_eq(check_user_password(uptr, "testpassword2"), 0);

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

    // check md5 -> pwhash migration
    strncpy(uptr->md5_pass, "0", 1);
    ck_assert_int_eq(set_user_password(uptr, "test_password3"), 0);
    ck_assert_str_eq(uptr->md5_pass, "");
    ck_assert_int_eq(check_user_password(uptr, "test_password3"), 0);

    ck_assert_int_eq(check_user_password(NULL, "dummypassword"), -1);

    DeleteAccount(uptr);
    free_core();
}
END_TEST

START_TEST(test_user_link)
{
    Chan *cptr;
    User *uptr, *uptr2;
    Nick *nptr, *nptr2;

    init_core();
    setup_mock_server(/*server_name=*/"ircd.test", /*sid=*/"042");
    strcpy(core_get_config()->nick,"C");

    uptr = create_mock_user("test_user", "test_password");
    ck_assert_ptr_ne(uptr, NULL);
    uptr->authed = 1;

    uptr2 = create_mock_user("test_user2", "test_password2");
    ck_assert_ptr_ne(uptr2, NULL);
    uptr2->authed = 1;

    AddLink(uptr->nick, uptr2->nick);
    
    // register a channel
    cptr = create_mock_chan("#test", uptr->nick);
    ck_assert_ptr_ne(cptr, NULL);

    // create 2 nicks, log them in
    nptr = get_core_api()->new_nick(/*nick=*/"test_user",
                                    /*ident=*/"test_ident",
                                    /*host=*/"test_host",
                                    /*uid=*/"042AABBCC",
                                    /*hiddenhost=*/"test_hiddenhost",
                                    /*umodes=*/UMODE_BOT | UMODE_SSL,
                                    /*reshost=*/"127.0.0.1");
    ck_assert_ptr_ne(nptr, NULL);
    strncpy(nptr->svid, "test_user", 10);
    uptr->authed_nick = nptr;

    nptr2 = get_core_api()->new_nick(/*nick=*/"test_user2",
                                    /*ident=*/"test_ident",
                                    /*host=*/"test_host",
                                    /*uid=*/"042BBCCDD",
                                    /*hiddenhost=*/"test_hiddenhost",
                                    /*umodes=*/UMODE_BOT | UMODE_SSL,
                                    /*reshost=*/"127.0.0.1");
    strncpy(nptr2->svid, "test_user2", 11);
    uptr2->authed_nick = nptr2;

    inject_parse_line(":042 SJOIN 1628161239 #test :042AABBCC");
    ck_assert(expect_raw_count(2));
    ck_assert(expect_next_raw(":C MODE #test +q test_user"));
    ck_assert(expect_next_raw(":C MODE #test +o test_user"));

    inject_parse_line(":042 SJOIN 1628161239 #test :042BBCCDD");
    ck_assert(expect_raw_count(2));
    ck_assert(expect_next_raw(":C MODE #test +q test_user2"));
    ck_assert(expect_next_raw(":C MODE #test +o test_user2"));

    free_mock_server();
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

    tc = tcase_create("user_link");
    tcase_add_test(tc, test_user_link);
    suite_add_tcase(s, tc);

    return s;
}
