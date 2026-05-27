#include "comm_link.h"
#include "protocol.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct CommPeer {
    int      m_fd;
    uint8_t  m_rbuf[TLV_MAX_PAYLOAD + 2];
    uint16_t m_rlen;
    int      m_is_listener;
};

static CommPeer* peer_create(int fd, int is_listener)
{
    CommPeer* p = (CommPeer*)malloc(sizeof(CommPeer));
    if (!p) { close(fd); return NULL; }
    p->m_fd = fd;
    p->m_rlen = 0;
    p->m_is_listener = is_listener;
    return p;
}

CommPeer* Comm_Listen(uint16_t port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return NULL;

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return NULL; }
    if (listen(fd, 5) < 0) { close(fd); return NULL; }

    return peer_create(fd, 1);
}

CommPeer* Comm_Accept(CommPeer* server)
{
    if (!server || !server->m_is_listener) return NULL;

    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int fd = accept(server->m_fd, (struct sockaddr*)&addr, &addrlen);
    if (fd < 0) return NULL;

    return peer_create(fd, 0);
}

CommPeer* Comm_Connect(const char* addr, uint16_t port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return NULL;

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    if (inet_pton(AF_INET, addr, &saddr.sin_addr) <= 0) { close(fd); return NULL; }

    if (connect(fd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) { close(fd); return NULL; }

    return peer_create(fd, 0);
}

void Comm_Close(CommPeer* peer)
{
    if (!peer) return;
    close(peer->m_fd);
    free(peer);
}

int Comm_Send(CommPeer* peer, uint8_t type, const void* data, uint8_t len)
{
    if (!peer) return -1;
    if (len > 0 && !data) return -1;

    uint8_t buf[TLV_MAX_PAYLOAD + 2];
    buf[0] = type;
    buf[1] = len;
    if (len > 0 && data) memcpy(buf + 2, data, len);

    size_t total = 2 + len;
    size_t sent = 0;
    while (sent < total) {
        ssize_t n = write(peer->m_fd, buf + sent, total - sent);
        if (n <= 0) {
            if (n == 0) return -1;
            if (errno == EINTR) continue;
            return -1;
        }
        sent += n;
    }
    return 0;
}

CommResult Comm_TryRecv(CommPeer* peer, uint8_t* outType, void* outBuf, uint8_t* outLen)
{
    if (!peer) return COMM_ERROR;

    ssize_t n = recv(peer->m_fd, peer->m_rbuf + peer->m_rlen,
                     sizeof(peer->m_rbuf) - peer->m_rlen, MSG_DONTWAIT);
    if (n > 0) {
        peer->m_rlen += n;
    } else if (n == 0) {
        return COMM_CLOSED;
    } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
        return COMM_ERROR;
    }

    if (peer->m_rlen < 2) return COMM_AGAIN;
    uint8_t plen = peer->m_rbuf[1];
    if (peer->m_rlen < 2 + plen) return COMM_AGAIN;

    if (outType) *outType = peer->m_rbuf[0];
    if (outLen)  *outLen  = plen;
    if (plen > 0 && outBuf) memcpy(outBuf, peer->m_rbuf + 2, plen);

    uint16_t total = 2 + plen;
    peer->m_rlen -= total;
    if (peer->m_rlen > 0)
        memmove(peer->m_rbuf, peer->m_rbuf + total, peer->m_rlen);

    return COMM_MSG_READY;
}

int Comm_FD(CommPeer* peer)
{
    if (!peer) return -1;
    return peer->m_fd;
}

CommPeer* Comm_FromFd(int fd)
{
    if (fd < 0) return NULL;
    return peer_create(fd, 0);
}
