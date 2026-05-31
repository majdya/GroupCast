/*
 * test_e2e.c  —  End-to-end protocol tests
 *
 * Spawns a real server process, connects as a client over TCP,
 * and exercises every request/response pair in the GroupCast protocol.
 *
 * Usage:  ./test_e2e [server_binary_path]
 *         (default: "./server")
 */

#include "comm_link.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

/* ------------------------------------------------------------------ */
/*  Test framework helpers                                            */
/* ------------------------------------------------------------------ */

#define TEST_PORT 18989
#define POLL_MS   10
#define POLL_MAX  200   /* 2 seconds total */

static int   passed   = 0;
static int   failed   = 0;
static pid_t g_child  = 0;

#define TEST(name)       do { printf("  %s ... ", name); } while (0)
#define PASS()           do { printf("PASS\n"); passed++; } while (0)
#define FAIL(msg)        do { printf("FAIL: %s\n", msg); failed++; } while (0)
#define ASSERT(cond, m)  do { if (!(cond)) { FAIL(m); return; } } while (0)

/* ------------------------------------------------------------------ */
/*  Server lifecycle                                                   */
/* ------------------------------------------------------------------ */

static int start_server(const char* bin, uint16_t port)
{
    pid_t pid = fork();
    if (pid < 0) return -1;

    if (pid == 0) {
        /* child — run server */
        char pbuf[16];
        snprintf(pbuf, sizeof(pbuf), "%u", port);
        execlp(bin, bin, pbuf, (char*)NULL);

        /* if exec fails */
        fprintf(stderr, "exec %s failed: %s\n", bin, strerror(errno));
        _exit(1);
    }

    /* parent — wait until server is ready to accept */
    for (int i = 0; i < POLL_MAX; i++) {
        CommPeer* c = Comm_Connect("127.0.0.1", port);
        if (c) {
            Comm_Close(c);
            return (int)pid;
        }
        usleep(POLL_MS * 1000);
    }

    /* timeout — kill the child */
    kill(pid, SIGTERM);
    waitpid(pid, NULL, 0);
    return -1;
}

static void stop_server(pid_t pid)
{
    if (pid <= 0) return;
    kill(pid, SIGTERM);
    waitpid(pid, NULL, 0);
}

/* ------------------------------------------------------------------ */
/*  Helper: non-blocking recv with polling                             */
/* ------------------------------------------------------------------ */

static CommResult recv_msg(CommPeer* peer, uint8_t* type, void* buf, uint8_t* len)
{
    for (int i = 0; i < POLL_MAX; i++) {
        CommResult r = Comm_TryRecv(peer, type, buf, len);
        if (r != COMM_AGAIN) return r;
        usleep(POLL_MS * 1000);
    }
    return COMM_AGAIN;
}

/* ------------------------------------------------------------------ */
/*  Helper: send-request / recv-response with status check             */
/* ------------------------------------------------------------------ */

static void req_resp_status(CommPeer* peer,
                            uint8_t  req_type,
                            uint8_t  resp_type,
                            const void* payload,
                            uint8_t  plen,
                            uint8_t  expected_status)
{
    uint8_t  rbuf[TLV_MAX_PAYLOAD];
    uint8_t  rtype, rlen;

    ASSERT(Comm_Send(peer, req_type, payload, plen) == 0,
           "Comm_Send failed");

    CommResult cr = recv_msg(peer, &rtype, rbuf, &rlen);
    ASSERT(cr == COMM_MSG_READY, "expected MSG_READY");
    ASSERT(rtype == resp_type, "wrong response type");
    ASSERT(rlen >= 1, "response too short");
    ASSERT(rbuf[0] == expected_status, Proto_StatusStr(rbuf[0]));
}

/* ------------------------------------------------------------------ */
/*  Test scenarios                                                     */
/* ------------------------------------------------------------------ */

static void test_register_login(CommPeer* peer)
{
    uint8_t payload[TLV_MAX_PAYLOAD];
    size_t  plen;

    /* register alice */
    plen = Proto_PackUserPass(payload, "alice", "secret");
    req_resp_status(peer, REGISTER_REQ, REGISTER_RESP, payload, (uint8_t)plen, SUCCESS);

    /* duplicate register */
    plen = Proto_PackUserPass(payload, "alice", "secret");
    req_resp_status(peer, REGISTER_REQ, REGISTER_RESP, payload, (uint8_t)plen, ERR_USER_EXISTS);

    /* login alice (correct password) */
    plen = Proto_PackUserPass(payload, "alice", "secret");
    req_resp_status(peer, LOGIN_REQ, LOGIN_RESP, payload, (uint8_t)plen, SUCCESS);

    PASS();
}

static void test_login_errors(CommPeer* peer)
{
    uint8_t payload[TLV_MAX_PAYLOAD];
    size_t  plen;

    /* login wrong password */
    plen = Proto_PackUserPass(payload, "alice", "wrongpass");
    req_resp_status(peer, LOGIN_REQ, LOGIN_RESP, payload, (uint8_t)plen, ERR_WRONG_PASSWORD);

    /* login non-existent user */
    plen = Proto_PackUserPass(payload, "nobody", "x");
    req_resp_status(peer, LOGIN_REQ, LOGIN_RESP, payload, (uint8_t)plen, ERR_USER_NOT_FOUND);

    PASS();
}

static void test_create_join_leave(CommPeer* peer)
{
    uint8_t payload[TLV_MAX_PAYLOAD];
    size_t  plen;

    /* create group */
    plen = Proto_PackStr(payload, "devs");
    req_resp_status(peer, CREATE_GROUP_REQ, CREATE_GROUP_RESP,
                    payload, (uint8_t)plen, SUCCESS);

    /* duplicate group */
    plen = Proto_PackStr(payload, "devs");
    req_resp_status(peer, CREATE_GROUP_REQ, CREATE_GROUP_RESP,
                    payload, (uint8_t)plen, ERR_GROUP_EXISTS);

    /* join group */
    plen = Proto_PackStr(payload, "devs");
    {
        uint8_t rbuf[TLV_MAX_PAYLOAD];
        uint8_t rtype, rlen;
        ASSERT(Comm_Send(peer, JOIN_GROUP_REQ, payload, (uint8_t)plen) == 0, "send failed");
        CommResult cr = recv_msg(peer, &rtype, rbuf, &rlen);
        ASSERT(cr == COMM_MSG_READY, "expected MSG_READY");
        ASSERT(rtype == JOIN_GROUP_RESP, "wrong type");
        uint8_t status;
        char    addr[16];
        uint16_t port;
        ASSERT(Proto_UnpackGroupResp(rbuf, rlen, &status, addr, sizeof(addr), &port) == 0,
               "unpack failed");
        ASSERT(status == SUCCESS, "join failed");
        ASSERT(strlen(addr) > 0, "empty MC addr");
        ASSERT(port > 0, "zero MC port");
    }

    /* already in group */
    plen = Proto_PackStr(payload, "devs");
    req_resp_status(peer, JOIN_GROUP_REQ, JOIN_GROUP_RESP,
                    payload, (uint8_t)plen, ERR_ALREADY_IN_GROUP);

    /* leave group */
    plen = Proto_PackStr(payload, "devs");
    req_resp_status(peer, LEAVE_GROUP_REQ, LEAVE_GROUP_RESP,
                    payload, (uint8_t)plen, SUCCESS);

    /* leave again — group was destroyed (last member), expect not-found */
    plen = Proto_PackStr(payload, "devs");
    req_resp_status(peer, LEAVE_GROUP_REQ, LEAVE_GROUP_RESP,
                    payload, (uint8_t)plen, ERR_GROUP_NOT_FOUND);

    /* join non-existent group */
    plen = Proto_PackStr(payload, "nosuchgroup");
    req_resp_status(peer, JOIN_GROUP_REQ, JOIN_GROUP_RESP,
                    payload, (uint8_t)plen, ERR_GROUP_NOT_FOUND);

    PASS();
}

static void test_logout(CommPeer* peer)
{
    /* logout */
    req_resp_status(peer, LOGOUT_REQ, LOGOUT_RESP, NULL, 0, SUCCESS);

    PASS();
}

static void test_requires_login(CommPeer* peer)
{
    uint8_t payload[TLV_MAX_PAYLOAD];
    size_t  plen;

    /* must be logged in for group ops */
    plen = Proto_PackStr(payload, "anygroup");
    req_resp_status(peer, CREATE_GROUP_REQ, CREATE_GROUP_RESP,
                    payload, (uint8_t)plen, ERR_GENERAL);
    req_resp_status(peer, JOIN_GROUP_REQ, JOIN_GROUP_RESP,
                    payload, (uint8_t)plen, ERR_GENERAL);
    req_resp_status(peer, LEAVE_GROUP_REQ, LEAVE_GROUP_RESP,
                    payload, (uint8_t)plen, ERR_GENERAL);

    PASS();
}

static void test_multi_session(void)
{
    uint8_t payload[TLV_MAX_PAYLOAD];
    uint8_t rbuf[TLV_MAX_PAYLOAD];
    uint8_t rtype, rlen;
    size_t  plen;

    /* second client: register + login as bob */
    CommPeer* bob = Comm_Connect("127.0.0.1", TEST_PORT);
    ASSERT(bob != NULL, "bob connect failed");

    plen = Proto_PackUserPass(payload, "bob", "bobpass");
    req_resp_status(bob, REGISTER_REQ, REGISTER_RESP, payload, (uint8_t)plen, SUCCESS);

    plen = Proto_PackUserPass(payload, "bob", "bobpass");
    req_resp_status(bob, LOGIN_REQ, LOGIN_RESP, payload, (uint8_t)plen, SUCCESS);

    /* bob creates and joins his own group */
    plen = Proto_PackStr(payload, "bob-chat");
    req_resp_status(bob, CREATE_GROUP_REQ, CREATE_GROUP_RESP,
                    payload, (uint8_t)plen, SUCCESS);

    plen = Proto_PackStr(payload, "bob-chat");
    ASSERT(Comm_Send(bob, JOIN_GROUP_REQ, payload, (uint8_t)plen) == 0, "send failed");
    CommResult cr = recv_msg(bob, &rtype, rbuf, &rlen);
    ASSERT(cr == COMM_MSG_READY, "expected MSG_READY");
    ASSERT(rtype == JOIN_GROUP_RESP, "wrong type");
    uint8_t status;
    char    addr[16];
    uint16_t port;
    ASSERT(Proto_UnpackGroupResp(rbuf, rlen, &status, addr, sizeof(addr), &port) == 0, "unpack");
    ASSERT(status == SUCCESS, "bob join failed");

    /* bob leaves */
    plen = Proto_PackStr(payload, "bob-chat");
    req_resp_status(bob, LEAVE_GROUP_REQ, LEAVE_GROUP_RESP,
                    payload, (uint8_t)plen, SUCCESS);

    Comm_Close(bob);
    PASS();
}

/* ------------------------------------------------------------------ */
/*  Main                                                               */
/* ------------------------------------------------------------------ */

int main(int argc, char* argv[])
{
    const char* server_bin = (argc > 1) ? argv[1] : "./server";

    printf("Starting server (binary: %s) on port %d ...\n", server_bin, TEST_PORT);
    g_child = start_server(server_bin, TEST_PORT);
    if (g_child < 0) {
        fprintf(stderr, "FATAL: could not start server\n");
        return 1;
    }
    printf("  server pid = %d\n", g_child);

    CommPeer* peer = Comm_Connect("127.0.0.1", TEST_PORT);
    if (!peer) {
        fprintf(stderr, "FATAL: could not connect to server\n");
        stop_server(g_child);
        return 1;
    }

    /* ---- run scenarios ---- */

    printf("\ne2e — registration / login:\n");
    test_register_login(peer);

    printf("\ne2e — login error paths:\n");
    test_login_errors(peer);

    printf("\ne2e — group create / join / leave:\n");
    test_create_join_leave(peer);

    printf("\ne2e — logout:\n");
    test_logout(peer);

    printf("\ne2e — grouped ops without login:\n");
    test_requires_login(peer);

    printf("\ne2e — multi-session:\n");
    test_multi_session();

    /* ---- cleanup ---- */

    Comm_Close(peer);
    stop_server(g_child);

    printf("\n%d passed, %d failed out of %d\n", passed, failed, passed + failed);
    return failed > 0 ? 1 : 0;
}
