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

    init_core();
    setup_mock_server();

    mod = loadmodule("sasl");
    ck_assert_ptr_ne(mod, NULL);

    uptr = create_mock_user("test_user", "test_password");
    ck_assert_ptr_ne(uptr, NULL);

    inject_parse_line(":ircd.test SASL services.test 042AAAAAA S PLAIN");
    ck_assert(expect_next_raw(":services.test SASL ircd.test 042AAAAAA C +"));

    // TODO: check the rest of SASL auth.

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

    tc = tcase_create("sasl_auth_success");
    tcase_add_test(tc, test_sasl_auth_success);
    suite_add_tcase(s, tc);

    return s;
}
