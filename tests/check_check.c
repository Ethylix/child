#include "check_check.h"

#include <check.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    SRunner *sr;
    int number_failed;

    sr = srunner_create(make_hashmap_suite());
    srunner_add_suite(sr, make_parseline_suite());
    srunner_add_suite(sr, make_nick_api_suite());
    srunner_add_suite(sr, make_mod_sasl_suite());

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
