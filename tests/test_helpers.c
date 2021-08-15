#include "test_helpers.h"

#include "child.h"
#include "core.h"
#include "core_api.h"
#include "string_utils.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MOCK_RAWS   256

struct mock_server {
    char *raws[MAX_MOCK_RAWS];
    int write_idx;
    int read_idx;
};

static struct mock_server mock_server;

static void mock_send_raw(const char *raw, ...)
{
    char buf[1024];
    va_list val;
    int next_write_idx;

    ircsprintf(buf, 1023, raw, val);

    next_write_idx = (mock_server.write_idx + 1) % MAX_MOCK_RAWS;

    // Buffer is full (N-1 elems), panic,
    if (next_write_idx == mock_server.read_idx)
        abort();

    assert(next_write_idx > mock_server.read_idx);

    mock_server.raws[mock_server.write_idx] = strdup(buf);
    mock_server.write_idx = next_write_idx;
}

bool expect_next_raw(const char *raw)
{
    int read_idx, next_read_idx;
    bool match;

    read_idx = mock_server.read_idx;

    // Buffer is empty.
    if (read_idx == mock_server.write_idx)
        return false;

    next_read_idx = (read_idx + 1) % MAX_MOCK_RAWS;
    assert(next_read_idx <= mock_server.write_idx);

    match = (strcmp(raw, mock_server.raws[read_idx]) == 0);
    mock_server.read_idx = next_read_idx;

    return match;
}

bool expect_any_raw(const char *raw)
{
    for (int i = 0; i < MAX_MOCK_RAWS; i++) {
        if (!mock_server.raws[i])
            continue;

        if (!strcmp(raw, mock_server.raws[i]))
            return true;
    }

    return false;
}

bool expect_raw_count(int count)
{
    return (mock_server.write_idx - mock_server.read_idx) == count;
}

void consume_mock_raws(void)
{
    for (int i = 0; i < MAX_MOCK_RAWS; i++) {
        if (mock_server.raws[i]) {
            free(mock_server.raws[i]);
            mock_server.raws[i] = NULL;
        }
    }

    mock_server.read_idx = mock_server.write_idx = 0;
}

User *create_mock_user(const char *name, const char *password)
{
    User *uptr;
    char *pass_hash;

    uptr = AddUser(name, 1);
    pass_hash = md5_hash(password);
    strncpy(uptr->md5_pass, pass_hash, MD5_LEN);
    free(pass_hash);

    return uptr;
}

void inject_parse_line(const char *buf)
{
    char *copy = strdup(buf);
    parse_line(copy);
    free(copy);
}

void setup_mock_server(const char *server_name, const char *sid)
{
    get_core_api()->send_raw = mock_send_raw;
    memset(&mock_server, 0, sizeof(mock_server));

    if (server_name && sid) {
        strncpy(get_core()->remote_server, server_name, SERVERNAMELEN);
        strncpy(get_core()->remote_sid, sid, SIDLEN);
        assert(add_server(server_name, sid, /*hub=*/NULL));
        get_core()->eos = false;
        get_core()->connected = 1;
    }
}

void free_mock_server(void)
{
    Server *server;

    consume_mock_raws();
    server = find_server(get_core()->remote_server);
    if (server)
        detach_server_recursive(server);
}
