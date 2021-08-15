#include <check.h>
#include <stdlib.h>

#include "child.h"
#include "commands.h"
#include "core.h"
#include "core_api.h"
#include "modules.h"
#include "net.h"
#include "test_helpers.h"

START_TEST(test_sasl_auth_success)
{
    Module *mod;
    User *uptr;
    Nick *nptr;

    init_core();
    setup_mock_server(/*server_name=*/"ircd.test", /*sid=*/"042");

    mod = loadmodule("sasl");
    ck_assert_ptr_ne(mod, NULL);

    uptr = create_mock_user("test_user", "test_password");
    ck_assert_ptr_ne(uptr, NULL);

    inject_parse_line(":ircd.test SASL services.test 042AAAAAA S PLAIN");
    ck_assert(expect_raw_count(1));
    ck_assert(expect_next_raw(":services.test SASL ircd.test 042AAAAAA C +"));

    // base64 payload is "\x00test_user\x00test_password".
    // https://datatracker.ietf.org/doc/html/rfc4616
    inject_parse_line(":ircd.test SASL services.test 042AAAAAA C AHRlc3RfdXNlcgB0ZXN0X3Bhc3N3b3Jk");
    ck_assert(expect_raw_count(2));
    ck_assert(expect_next_raw(":services.test SVSLOGIN ircd.test 042AAAAAA test_user"));
    ck_assert(expect_next_raw(":services.test SASL ircd.test 042AAAAAA D S"));

    // Clear previous raws before proceeding to the second phase.
    consume_mock_raws();

    // Override uninitialized config.
    core_get_config()->maxclones = 1000;

    inject_parse_line(":042 UID test_nick 0 1629045439 test_ident test_host 042AAAAAA test_user +iwp * geek-ABCDEFGH.blah fwAAAQ== :test description");

    nptr = get_core_api()->find_nick("042AAAAAA");
    ck_assert_ptr_ne(nptr, NULL);
    ck_assert_str_eq(nptr->svid, "test_user");
    ck_assert(IsRegistered(nptr));
    ck_assert_int_eq(uptr->authed, 1);

    ck_assert(expect_any_raw("SVS2MODE test_nick +r"));

    DeleteAccount(uptr);
    unloadmodule("sasl");
    free_mock_server();
    free_core();
}
END_TEST

START_TEST(test_sasl_malformed_line)
{
    Module *mod;

    init_core();
    setup_mock_server(/*server_name=*/"ircd.test", /*sid=*/"042");

    mod = loadmodule("sasl");
    ck_assert_ptr_ne(mod, NULL);

    inject_parse_line(":ircd.test SASL");
    ck_assert(expect_raw_count(0));

    inject_parse_line(":ircd.test SASL services.test");
    ck_assert(expect_raw_count(0));

    inject_parse_line(":ircd.test SASL services.test 042AAAAAA");
    ck_assert(expect_raw_count(0));

    inject_parse_line(":ircd.test SASL services.test 042AAAAAA S");
    ck_assert(expect_raw_count(0));

    unloadmodule("sasl");
    free_mock_server();
    free_core();
}
END_TEST

START_TEST(test_sasl_unsupported_method)
{
    Module *mod;

    init_core();
    setup_mock_server(/*server_name=*/"ircd.test", /*sid=*/"042");

    mod = loadmodule("sasl");
    ck_assert_ptr_ne(mod, NULL);

    inject_parse_line(":ircd.test SASL services.test 042AAAAAA S DIGEST-MD5");
    ck_assert(expect_raw_count(2));
    ck_assert(expect_next_raw(":services.test SASL ircd.test 042AAAAAA M PLAIN"));
    ck_assert(expect_next_raw(":services.test SASL ircd.test 042AAAAAA D F"));

    unloadmodule("sasl");
    free_mock_server();
    free_core();
}
END_TEST

START_TEST(test_sasl_invalid_b64_encoding)
{
    Module *mod;

    init_core();
    setup_mock_server(/*server_name=*/"ircd.test", /*sid=*/"042");

    mod = loadmodule("sasl");
    ck_assert_ptr_ne(mod, NULL);

    inject_parse_line(":ircd.test SASL services.test 042AAAAAA C garbage_base64@@!!");
    ck_assert(expect_raw_count(1));
    ck_assert(expect_next_raw(":services.test SASL ircd.test 042AAAAAA D F"));

    unloadmodule("sasl");
    free_mock_server();
    free_core();
}
END_TEST

START_TEST(test_sasl_invalid_auth_payload)
{
    Module *mod;

    init_core();
    setup_mock_server(/*server_name=*/"ircd.test", /*sid=*/"042");

    mod = loadmodule("sasl");
    ck_assert_ptr_ne(mod, NULL);

    // base64 payload is "test" (missing \0)
    inject_parse_line(":ircd.test SASL services.test 042AAAAAA C dGVzdA==");
    ck_assert(expect_raw_count(1));
    ck_assert(expect_next_raw(":services.test SASL ircd.test 042AAAAAA D F"));

    // base64 payload is "\x00test" (missing second \0)
    inject_parse_line(":ircd.test SASL services.test 042AAAAAA C AHRlc3Q=");
    ck_assert(expect_raw_count(1));
    ck_assert(expect_next_raw(":services.test SASL ircd.test 042AAAAAA D F"));

    unloadmodule("sasl");
    free_mock_server();
    free_core();
}
END_TEST

START_TEST(test_sasl_user_not_found)
{
    Module *mod;

    init_core();
    setup_mock_server(/*server_name=*/"ircd.test", /*sid=*/"042");

    mod = loadmodule("sasl");
    ck_assert_ptr_ne(mod, NULL);

    // base64 payload is "\x00test_user\x00test_password"
    inject_parse_line(":ircd.test SASL services.test 042AAAAAA C AHRlc3RfdXNlcgB0ZXN0X3Bhc3N3b3Jk");
    ck_assert(expect_raw_count(1));
    ck_assert(expect_next_raw(":services.test SASL ircd.test 042AAAAAA D F"));

    unloadmodule("sasl");
    free_mock_server();
    free_core();
}
END_TEST

START_TEST(test_sasl_invalid_password)
{
    Module *mod;
    User *uptr;

    init_core();
    setup_mock_server(/*server_name=*/"ircd.test", /*sid=*/"042");

    mod = loadmodule("sasl");
    ck_assert_ptr_ne(mod, NULL);

    uptr = create_mock_user("test_user", "wrong_password");

    // base64 payload is "\x00test_user\x00test_password"
    inject_parse_line(":ircd.test SASL services.test 042AAAAAA C AHRlc3RfdXNlcgB0ZXN0X3Bhc3N3b3Jk");
    ck_assert(expect_raw_count(1));
    ck_assert(expect_next_raw(":services.test SASL ircd.test 042AAAAAA D F"));

    DeleteAccount(uptr);
    unloadmodule("sasl");
    free_mock_server();
    free_core();
}
END_TEST

static int (*orig_sasl_finish_session)(Nick *, User *, Chan *, char *[]);

static int hook_sasl_finish_session(Nick *nptr, User *uptr, Chan *cptr, char *parv[])
{
    // Clear previously generated raws, allowing the below test to check for absence
    // of raws.
    consume_mock_raws();
    return orig_sasl_finish_session(nptr, uptr, cptr, parv);
}

START_TEST(test_sasl_finish_no_svid)
{
    Module *mod;
    User *uptr;
    Hook *hook;

    init_core();
    setup_mock_server(/*server_name=*/"ircd.test", /*sid=*/"042");

    mod = loadmodule("sasl");
    ck_assert_ptr_ne(mod, NULL);

    hook = find_hook(mod, "sasl_finish_session");
    ck_assert_ptr_ne(mod, NULL);

    orig_sasl_finish_session = hook->ptr;
    hook->ptr = hook_sasl_finish_session;

    uptr = create_mock_user("test_user", "test_password");

    // Override uninitialized config.
    core_get_config()->maxclones = 1000;

    // svid is set to "0"
    inject_parse_line(":042 UID test_nick 0 1629045439 test_ident test_host 042AAAAAA 0 +iwp * geek-ABCDEFGH.blah fwAAAQ== :test description");
    ck_assert(expect_raw_count(0));

    DeleteAccount(uptr);
    unloadmodule("sasl");
    free_mock_server();
    free_core();
}
END_TEST

START_TEST(test_sasl_finish_user_not_found)
{
    Module *mod;
    Nick *nptr;

    init_core();
    setup_mock_server(/*server_name=*/"ircd.test", /*sid=*/"042");

    mod = loadmodule("sasl");
    ck_assert_ptr_ne(mod, NULL);

    // Override uninitialized config.
    core_get_config()->maxclones = 1000;

    // svid is set to test_user
    inject_parse_line(":042 UID test_nick 0 1629045439 test_ident test_host 042AAAAAA test_user +iwp * geek-ABCDEFGH.blah fwAAAQ== :test description");

    ck_assert(expect_raw_count(1));
    ck_assert(expect_next_raw("SVS2MODE 042AAAAAA +d 0"));

    nptr = get_core_api()->find_nick("test_nick");
    ck_assert_ptr_ne(nptr, NULL);
    ck_assert_str_eq(nptr->svid, "0");

    unloadmodule("sasl");
    free_mock_server();
    free_core();
}
END_TEST

Suite *make_mod_sasl_suite(void)
{
    Suite *s;
    TCase *tc;

    s = suite_create("mod_sasl");

    tc = tcase_create("sasl_auth_success");
    tcase_add_test(tc, test_sasl_auth_success);
    suite_add_tcase(s, tc);

    tc = tcase_create("sasl_malformed_line");
    tcase_add_test(tc, test_sasl_malformed_line);
    suite_add_tcase(s, tc);

    tc = tcase_create("sasl_unsupported_method");
    tcase_add_test(tc, test_sasl_unsupported_method);
    suite_add_tcase(s, tc);

    tc = tcase_create("sasl_invalid_b64_encoding");
    tcase_add_test(tc, test_sasl_invalid_b64_encoding);
    suite_add_tcase(s, tc);

    tc = tcase_create("sasl_invalid_auth_payload");
    tcase_add_test(tc, test_sasl_invalid_auth_payload);
    suite_add_tcase(s, tc);

    tc = tcase_create("sasl_user_not_found");
    tcase_add_test(tc, test_sasl_user_not_found);
    suite_add_tcase(s, tc);

    tc = tcase_create("sasl_invalid_password");
    tcase_add_test(tc, test_sasl_invalid_password);
    suite_add_tcase(s, tc);

    tc = tcase_create("sasl_finish_no_svid");
    tcase_add_test(tc, test_sasl_finish_no_svid);
    suite_add_tcase(s, tc);

    tc = tcase_create("sasl_finish_user_not_found");
    tcase_add_test(tc, test_sasl_finish_user_not_found);
    suite_add_tcase(s, tc);

    return s;
}
