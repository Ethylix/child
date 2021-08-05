#include <check.h>
#include <stdlib.h>

#include "child.h"
#include "commands.h"
#include "core.h"
#include "net.h"

START_TEST(test_parse_ping)
{
    init_core();
    InitMem();
    indata.nextline = indata.chunkbufentry = indata.chunkbuf;

    strcpy(indata.chunkbufentry, "PING :this is a test");
    GetLineFromChunk();
    ParseLine();

    // The current implementation only echoes the first word.
    const char *expected = "PONG this\r\n";
    ck_assert_int_eq(strlen(outdata.outbuf), strlen(expected));
    ck_assert_int_eq(memcmp(outdata.outbuf, expected, strlen(expected)), 0);

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
