#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include "child.h"
#include "commands.h"
#include "channel.h"
#include "core.h"
#include "core_api.h"
#include "modules.h"
#include "net.h"
#include "test_helpers.h"

#include "prom.h"
#include <curl/curl.h>

// from https://curl.se/libcurl/c/getinmemory.html
struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

bool assert_metric(char *metric) {
    CURL *curl_handle;
    CURLcode res;
    bool match = false;

    struct MemoryStruct chunk;

    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    curl_global_init(CURL_GLOBAL_ALL);

    curl_handle = curl_easy_init();
    if(!curl_handle) {
        fprintf(stderr, "curl_easy_init() failed\n");
        free(chunk.memory);
        curl_global_cleanup();
        return match;
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, "http://localhost:8000/metrics");
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPALIVE, 1L);

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    res = curl_easy_perform(curl_handle);

    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
        curl_easy_strerror(res));
    }
    else {
        char *pch = strtok(chunk.memory, "\n");

        while (pch != NULL) {
            if (!strcmp(pch, metric)) {
                match = true;
                break;
            }
            pch = strtok(NULL, "\n");
        }
    }

    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();

    return match;
}

START_TEST(test_build_info)
{
    Module *mod;

    init_core();
    setup_mock_server(/*server_name=*/"ircd.test", /*sid=*/"042");

    mod = loadmodule("prometheus");
    ck_assert_ptr_ne(mod, NULL);

    ck_assert(assert_metric("child_build_info{version=\"2.0-dev\"} 1"));

    unloadmodule("prometheus");
    free_mock_server();
    free_core();
}
END_TEST

START_TEST(test_accounts_total)
{
    Module *mod;
    User *uptr;

    init_core();
    setup_mock_server(/*server_name=*/"ircd.test", /*sid=*/"042");

    mod = loadmodule("prometheus");
    ck_assert_ptr_ne(mod, NULL);

    ck_assert(assert_metric("child_accounts_total 0"));

    uptr = create_mock_user("test_user", "test_password");
    ck_assert_ptr_ne(uptr, NULL);

    ck_assert(assert_metric("child_accounts_total 1"));

    DeleteAccount(uptr);
    ck_assert(assert_metric("child_accounts_total 0"));

    unloadmodule("prometheus");
    free_mock_server();
    free_core();
}
END_TEST

START_TEST(test_channels_total)
{
    Module *mod;
    Chan *cptr;
    User *uptr;

    init_core();
    setup_mock_server(/*server_name=*/"ircd.test", /*sid=*/"042");

    mod = loadmodule("prometheus");
    ck_assert_ptr_ne(mod, NULL);

    ck_assert(assert_metric("child_channels_total 0"));

    uptr = create_mock_user("test_user", "test_password");
    ck_assert_ptr_ne(uptr, NULL);
    // TODO: replace with create_mock_channel once #35 is merged
    cptr = CreateChannel("#test", uptr->nick, 0);
    ck_assert_ptr_ne(cptr, NULL);

    ck_assert(assert_metric("child_channels_total 1"));

    DeleteChannel(cptr);
    ck_assert(assert_metric("child_channels_total 0"));

    DeleteAccount(uptr);
    unloadmodule("prometheus");
    free_mock_server();
    free_core();
}
END_TEST

START_TEST(test_hook_runs_total)
{
    Module *mod, *mod2;

    init_core();
    setup_mock_server(/*server_name=*/"ircd.test", /*sid=*/"042");

    mod = loadmodule("prometheus");
    ck_assert_ptr_ne(mod, NULL);
    mod2 = loadmodule("sasl");
    ck_assert_ptr_ne(mod2, NULL);

    inject_parse_line(":ircd.test SASL services.test 042AAAAAA S PLAIN");

    ck_assert(assert_metric("child_hook_runs_total{hook=\"sasl_start_session\",module=\"sasl\"} 1"));
    ck_assert(assert_metric("child_hook_runs_total{hook=\"hook_runs_total_inc\",module\"prometheus\"} 1") == false);

    unloadmodule("prometheus");
    unloadmodule("sasl");
    free_mock_server();
    free_core();
}
END_TEST

Suite *make_mod_prometheus_suite(void)
{
    Suite *s;
    TCase *tc;

    s = suite_create("mod_prometheus");

    tc = tcase_create("build_info");
    tcase_add_test(tc, test_build_info);
    suite_add_tcase(s, tc);

    tc = tcase_create("users_total");
    tcase_add_test(tc, test_accounts_total);
    suite_add_tcase(s, tc);

    tc = tcase_create("channels_total");
    tcase_add_test(tc, test_channels_total);
    suite_add_tcase(s, tc);

    tc = tcase_create("hook_runs_total");
    tcase_add_test(tc, test_hook_runs_total);
    suite_add_tcase(s, tc);

    return s;
}
