#ifndef _CHECK_CHECK_H
#define _CHECK_CHECK_H

#include <check.h>

Suite* make_hashmap_suite(void);
Suite* make_parseline_suite(void);
Suite* make_nick_api_suite(void);
Suite* make_user_api_suite(void);
Suite* make_mod_sasl_suite(void);
Suite* make_mod_prometheus_suite(void);

#endif
