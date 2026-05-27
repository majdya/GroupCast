#include "comm_link.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

static int passed = 0, failed = 0;

#define TEST(name) do { printf("  %s ... ", name); } while(0)
#define PASS() do { printf("PASS\n"); passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); failed++; } while(0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)

static void test_send_recv(void)
{
    int fds[2];
    ASSERT(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0, "socketpair failed");

    CommPeer* a = Comm_FromFd(fds[0]);
    CommPeer* b = Comm_FromFd(fds[1]);
    ASSERT(a && b, "Comm_FromFd failed");

    const char* payload = "hello";
    uint8_t len = (uint8_t)strlen(payload);

    TEST("Comm_Send and Comm_TryRecv roundtrip");
    ASSERT(Comm_Send(a, 0x42, payload, len) == 0, "send failed");
    uint8_t t, rlen;
    char rbuf[256];
    CommResult res = Comm_TryRecv(b, &t, rbuf, &rlen);
    ASSERT(res == COMM_MSG_READY, "expected MSG_READY");
    ASSERT(t == 0x42, "wrong type");
    ASSERT(rlen == len, "wrong length");
    ASSERT(memcmp(rbuf, payload, len) == 0, "wrong payload");
    PASS();

    Comm_Close(a);
    Comm_Close(b);
}

static void test_partial_read(void)
{
    int fds[2];
    ASSERT(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0, "socketpair failed");

    CommPeer* peer = Comm_FromFd(fds[1]);
    ASSERT(peer != NULL, "Comm_FromFd failed");

    const char* payload = "partial";
    uint8_t type = 0x77;
    uint8_t plen = (uint8_t)strlen(payload);

    TEST("Comm_TryRecv reassembles partial reads");
    write(fds[0], &type, 1);
    ASSERT(Comm_TryRecv(peer, NULL, NULL, NULL) == COMM_AGAIN, "should need len");
    write(fds[0], &plen, 1);
    ASSERT(Comm_TryRecv(peer, NULL, NULL, NULL) == COMM_AGAIN, "should need payload");
    write(fds[0], payload, plen);
    uint8_t t, rlen;
    char rbuf[256];
    ASSERT(Comm_TryRecv(peer, &t, rbuf, &rlen) == COMM_MSG_READY, "should be ready");
    ASSERT(t == type, "wrong type");
    ASSERT(rlen == plen, "wrong len");
    ASSERT(memcmp(rbuf, payload, plen) == 0, "wrong payload");
    PASS();

    Comm_Close(peer);
    close(fds[0]);
}

static void test_multi_message(void)
{
    int fds[2];
    ASSERT(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0, "socketpair failed");

    CommPeer* a = Comm_FromFd(fds[0]);
    CommPeer* b = Comm_FromFd(fds[1]);
    ASSERT(a && b, "Comm_FromFd failed");

    TEST("multiple messages in sequence");
    ASSERT(Comm_Send(a, 0x01, "abc", 3) == 0, "send 1 failed");
    ASSERT(Comm_Send(a, 0x02, "xy", 2) == 0, "send 2 failed");

    uint8_t t, rlen;
    char rbuf[256];

    CommResult res = Comm_TryRecv(b, &t, rbuf, &rlen);
    ASSERT(res == COMM_MSG_READY, "expected msg1");
    ASSERT(t == 0x01 && rlen == 3 && memcmp(rbuf, "abc", 3) == 0, "wrong msg1");

    res = Comm_TryRecv(b, &t, rbuf, &rlen);
    ASSERT(res == COMM_MSG_READY, "expected msg2");
    ASSERT(t == 0x02 && rlen == 2 && memcmp(rbuf, "xy", 2) == 0, "wrong msg2");
    PASS();

    Comm_Close(a);
    Comm_Close(b);
}

static void test_zero_payload(void)
{
    int fds[2];
    ASSERT(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0, "socketpair failed");

    CommPeer* a = Comm_FromFd(fds[0]);
    CommPeer* b = Comm_FromFd(fds[1]);
    ASSERT(a && b, "Comm_FromFd failed");

    TEST("zero-length payload");
    ASSERT(Comm_Send(a, 0xAA, NULL, 0) == 0, "send failed");
    uint8_t t, rlen;
    CommResult res = Comm_TryRecv(b, &t, NULL, &rlen);
    ASSERT(res == COMM_MSG_READY, "expected MSG_READY");
    ASSERT(t == 0xAA, "wrong type");
    ASSERT(rlen == 0, "wrong len");
    PASS();

    Comm_Close(a);
    Comm_Close(b);
}

static void test_close(void)
{
    int fds[2];
    ASSERT(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0, "socketpair failed");

    CommPeer* peer = Comm_FromFd(fds[1]);
    ASSERT(peer != NULL, "Comm_FromFd failed");

    TEST("Comm_TryRecv returns COMM_CLOSED on disconnect");
    close(fds[0]);
    uint8_t t, rlen;
    CommResult res = Comm_TryRecv(peer, &t, NULL, &rlen);
    ASSERT(res == COMM_CLOSED, "expected CLOSED");
    PASS();

    Comm_Close(peer);
}

int main(void)
{
    printf("comm_link tests:\n");
    test_send_recv();
    test_partial_read();
    test_multi_message();
    test_zero_payload();
    test_close();

    printf("\n%d passed, %d failed out of %d\n", passed, failed, passed + failed);
    return failed > 0 ? 1 : 0;
}
