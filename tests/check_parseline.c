#include <check.h>
#include <stdlib.h>

#include "child.h"
#include "commands.h"
#include "core.h"
#include "core_api.h"
#include "net.h"

static void expect_net_output(const char *buf)
{
    struct net *net = core_get_net();
    struct net_outdata *outdata = &net->outdata;

    ck_assert_int_eq(strlen(outdata->outbuf), strlen(buf));
    ck_assert_int_eq(memcmp(outdata->outbuf, buf, strlen(buf)), 0);
}

static void test_parse_line(const char *buf)
{
    char *copy = strdup(buf);
    parse_line(copy);
    free(copy);
}

START_TEST(test_parse_ping)
{
    init_core();

    test_parse_line("PING :this is a test");

    // The current implementation only echoes the first word.
    expect_net_output("PONG this\r\n");

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
