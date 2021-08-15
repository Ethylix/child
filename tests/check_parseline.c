#include <check.h>
#include <stdlib.h>

#include "child.h"
#include "commands.h"
#include "core.h"
#include "core_api.h"
#include "net.h"
#include "test_helpers.h"

START_TEST(test_parse_ping)
{
    init_core();
    setup_mock_server(/*server_name=*/NULL, /*sid=*/NULL);

    inject_parse_line("PING :this is a test");
    ck_assert(expect_next_raw("PONG this"));

    consume_mock_raws();
    free_core();
}
END_TEST

Suite *make_parseline_suite(void)
{
    Suite *s;
    TCase *tc;

    s = suite_create("parse_ping");

    tc = tcase_create("parse_ping_success");
    tcase_add_test(tc, test_parse_ping);
    suite_add_tcase(s, tc);

    return s;
}
