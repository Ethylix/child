#include <check.h>
#include <stdlib.h>

#include "child.h"
#include "commands.h"
#include "core.h"
#include "core_api.h"
#include "modules.h"
#include "net.h"
#include "test_helpers.h"

START_TEST(test_sasl_start_session_success)
{
    Module *mod;
    User *uptr;

    init_core();
    setup_mock_server();

    mod = loadmodule("sasl");
    ck_assert_ptr_ne(mod, NULL);

    uptr = create_mock_user("test_user", "test_password");
    ck_assert_ptr_ne(uptr, NULL);

    inject_parse_line(":ircd.test SASL services.test 042AAAAAA S PLAIN");
    ck_assert(expect_next_raw(":services.test SASL ircd.test 042AAAAAA C +"));

    // base64 payload is "\x00test_user\x00test_password".
    // https://datatracker.ietf.org/doc/html/rfc4616
    inject_parse_line(":ircd.test SASL services.test 042AAAAAA C AHRlc3RfdXNlcgB0ZXN0X3Bhc3N3b3Jk");
    ck_assert(expect_next_raw(":services.test SVSLOGIN ircd.test 042AAAAAA test_user"));
    ck_assert(expect_next_raw(":services.test SASL ircd.test 042AAAAAA D S"));

    consume_mock_raws();
    DeleteAccount(uptr);
    unloadmodule("sasl");
    free_core();
}
END_TEST

Suite *make_mod_sasl_suite(void)
{
    Suite *s;
    TCase *tc;

    s = suite_create("mod_sasl");

    tc = tcase_create("sasl_start_session_success");
    tcase_add_test(tc, test_sasl_start_session_success);
    suite_add_tcase(s, tc);

    return s;
}
