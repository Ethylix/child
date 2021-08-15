#ifndef _TEST_HELPERS_H
#define _TEST_HELPERS_H

#include "user.h"

#include <stdbool.h>

bool expect_next_raw(const char *raw);
bool expect_any_raw(const char *raw);
void consume_mock_raws(void);
User *create_mock_user(const char *name, const char *password);
void inject_parse_line(const char *buf);
void setup_mock_server(void);

#endif
